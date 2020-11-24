// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);
void my_kfree(void *pa,int cpuid);
extern char end[]; // first address after kernel.
                   // defined by kernel.ld.
void * steal(int cpu_id);


struct run {
  struct run *next;
};

struct kmem1{
  struct spinlock lock;
  struct run *freelist;
};

struct {
  struct spinlock lock;
  struct run *freelist;
}kmem;

struct kmem1 kmems[3];

void
kinit()
{
  for (int i = 0; i < 3; i++)
  {
    initlock(&kmems[i].lock, "kmem");
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  int page_num;
  p = (char*)PGROUNDUP((uint64)pa_start);//对页的大小向上取整
  page_num = ((char*)pa_end-p)/PGSIZE;
  for(int i = 0; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    if(i<page_num/3){
      my_kfree(p,0);
    }
    else if(i<page_num/3*2){
      my_kfree(p,1);
    }
    else{
      my_kfree(p,2);
    }
    
}

void my_kfree(void *pa,int cpu_id){
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmems[cpu_id].lock);
  r->next = kmems[cpu_id].freelist;
  kmems[cpu_id].freelist = r;
  release(&kmems[cpu_id].lock);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;
  int cpu_id;
  push_off();
  cpu_id = cpuid();
  pop_off();
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmems[cpu_id].lock);
  r->next = kmems[cpu_id].freelist;
  kmems[cpu_id].freelist = r;
  release(&kmems[cpu_id].lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  int cpu_id;
  push_off();
  cpu_id = cpuid();
  pop_off();
  acquire(&kmems[cpu_id].lock);

  r = kmems[cpu_id].freelist;
  if(r)
    kmems[cpu_id].freelist = r->next;
  release(&kmems[cpu_id].lock);
  if(!r){
    r = steal(cpu_id);
  }

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

void * steal(int cpu_id){
  struct run *r=0;
  for(int i=0;i<3;i++){
    if(holding(&kmems[i].lock)){
      continue;
    }
    acquire(&kmems[i].lock);
    if(kmems[i].freelist!=0){
      r = kmems[i].freelist;
      kmems[i].freelist = r->next;
      release(&kmems[i].lock);
      return (void *)r;
    }
    release(&kmems[i].lock);
  }
  return (void *)r;
}
