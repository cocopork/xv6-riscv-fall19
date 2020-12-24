#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200

//mycode
#define PROT_READ   (1L << 1)    //PROT_READ和PROT_WRITE设置于pte的位置一样，可以方便配置pte
#define PROT_WRITE  (1L << 2)
#define MAP_SHARED  (1L << 1)
#define MAP_PRIVATE (1L << 2)
