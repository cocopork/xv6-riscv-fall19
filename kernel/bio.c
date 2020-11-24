// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKET 13

struct {
  struct spinlock bcache_lock[NBUCKET];
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // head.next is most recently used.
  struct buf head[NBUCKET];
} bcache;

void
binit(void)
{
  struct buf *b;
  for(int i=0;i<NBUCKET;i++){
    initlock(&bcache.bcache_lock[i], "bcache");
    bcache.head[i].prev = &bcache.head[i];
    bcache.head[i].next = &bcache.head[i];
  }
    

  // Create linked list of buffers
  // bcache.head.prev = &bcache.head;
  // bcache.head.next = &bcache.head;
  // for(b = bcache.buf; b < bcache.buf+NBUF; b++){
  //   b->next = bcache.head.next;
  //   b->prev = &bcache.head;
  //   initsleeplock(&b->lock, "buffer");
  //   bcache.head.next->prev = b;
  //   bcache.head.next = b;

  for(b = bcache.buf; b < bcache.buf+NBUF; b++){//头插法
    b->next = bcache.head[0].next;
    b->prev = &bcache.head[0];
    initsleeplock(&b->lock, "buffer");
    bcache.head[0].next->prev = b;
    bcache.head[0].next = b;
  }

}

int hash(uint blockno){
  return blockno%NBUCKET;
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  
  struct buf *b;
  int h = hash(blockno);
  acquire(&bcache.bcache_lock[h]);

  // Is the block already cached?
  // for(b = bcache.head.next; b != &bcache.head; b = b->next){
  //   if(b->dev == dev && b->blockno == blockno){
  //     b->refcnt++;
  //     release(&bcache.lock);
  //     acquiresleep(&b->lock);
  //     return b;
  //   }
  // }
  
  //先在对应哈希锁下寻找blockno
  for(b = bcache.head[h].next; b != &bcache.head[h]; b = b->next){
    //printf("1\n");
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.bcache_lock[h]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached; recycle an unused buffer.
  // for(b = bcache.head.prev; b != &bcache.head; b = b->prev){
  //   if(b->refcnt == 0) {
  //     b->dev = dev;
  //     b->blockno = blockno;
  //     b->valid = 0;
  //     b->refcnt = 1;
  //     release(&bcache.lock);
  //     acquiresleep(&b->lock);
  //     return b;
  //   }
  // }

  //未匹配，在本哈希值中找空闲的
  for(b = bcache.head[h].prev; b != &bcache.head[h]; b = b->prev){//从后往前找，先找已经缓存的
    //printf("2\n");
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcache.bcache_lock[h]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  //还是没找到空闲的，就去其他哈希值里找空闲的
  int h_next = (h+1)%NBUCKET;

  while (h_next!=h)
  {

    acquire(&bcache.bcache_lock[h_next]);
    for(b = bcache.head[h_next].prev; b != &bcache.head[h_next]; b = b->prev){//从后往前找，先找已经缓存的
      if(b->refcnt == 0) {
        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        //移动到原哈希值锁
        b->prev->next = b->next;
        b->next->prev = b->prev;
        release(&bcache.bcache_lock[h_next]);

        b->prev = &bcache.head[h];
        b->next = bcache.head[h].next;

        bcache.head[h].next->prev = b;
        bcache.head[h].next = b;
        release(&bcache.bcache_lock[h]);
        acquiresleep(&b->lock);
        return b;
      }
    }
    release(&bcache.bcache_lock[h_next]);//实在找不到就换下一家哈希值，把本次哈希锁释放
    h_next = (h_next+1)%NBUCKET;
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b->dev, b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b->dev, b, 1);
}

// Release a locked buffer.
// Move to the head of the MRU list.
void
brelse(struct buf *b)
{
  
  int h;
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);
  h = hash(b->blockno);
  acquire(&bcache.bcache_lock[h]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.head[h].next;//空闲buf放在头部
    b->prev = &bcache.head[h];
    bcache.head[h].next->prev = b;
    bcache.head[h].next = b;
  }
  
  release(&bcache.bcache_lock[h]);
}

void
bpin(struct buf *b) {
  int h = hash(b->blockno);
  acquire(&bcache.bcache_lock[h]);
  b->refcnt++;
  release(&bcache.bcache_lock[h]);
}

void
bunpin(struct buf *b) {
  int h = hash(b->blockno);
  acquire(&bcache.bcache_lock[h]);
  b->refcnt--;
  release(&bcache.bcache_lock[h]);
}


