#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"//文件读取方式宏定义

int match(char *re, char *text);
int matchhere(char*, char*);
int matchstar(int, char*, char*);
/**
* 将路径格式转化为文件名，只提取路径最后一个文件
*/
char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p)+1);
  memset(buf+strlen(p)+1, ' ', DIRSIZ-strlen(p)-1);
  return buf;
}

/**
* 比较函数
*/
void equal_str(char *path,char * fileName,int *findflag){
  if(strcmp(fmtname(path),fileName)== 0){
    printf("%s\n",path);
    *findflag = 1;
  }
  else if(match(fileName,fmtname(path))){
    printf("%s\n",path);
    *findflag = 1;
  }
  // else{
  //   printf("%s %s ",path,fileName);
  //   printf("%d\n",strcmp(fmtname(path),fileName));
  // }
}

/**
* 主要函数
*/
void find(char *path,char *fileName,int *findflag){
  char buf[512], *p;
  int fd;
  struct dirent de;//目录名，包含id和名字
  struct stat st;//打开文件的状态

  //打开文件路径
  if((fd = open(path, O_RDONLY)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  //统计文件
  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
    case T_FILE:
      equal_str(path,fileName,findflag);
      break;

    case T_DIR:
      if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
        printf("find: path too long\n");
        break;
      }
      equal_str(path,fileName,findflag);

      //当前路径复制到buf
      strcpy(buf, path);
      p = buf+strlen(buf);
      *p++ = '/';
      //不断读取directory下的所有文件或目录
      while(read(fd, &de, sizeof(de)) == sizeof(de)){
        if(de.inum==0||de.inum==1){
          //printf("de.name %d : %s\n",de.inum,de.name);
          //printf("path :%s\n\n",buf);
          continue;
        }
          
        if(strcmp(de.name,".")==0||strcmp(de.name,"..")==0){
            continue;
        }
          memmove(p, de.name, strlen(de.name));
          p[strlen(de.name)] = 0;//结尾添加个\0
          find(buf,fileName,findflag);
      }
      break;
    }
  close(fd);
}


int
main(int argc, char *argv[])
{
  int i;
  int findflag=0;
  if(argc < 2){
    printf("please input file name!");
    exit();
  }
  else if(argc < 3){
    find(".",argv[1],&findflag);
    if(findflag==0){
      printf("not such file or directory.\n");
    }
    exit();
  }
  else{
    for(i=2; i<argc; i++)
      find(argv[1],argv[i],&findflag);
    if(findflag==0){
      printf("not such file or directory.\n");
    }    
    exit();
  }
  // int fd;
  // struct stat st;
  // if((fd=open(argv[1],0))<0){
  //   fprintf(2,"open error");
  // }else{
  //   fstat(fd,&st);
  //   printf("%d",st.type);
  // }
  // exit();
}

int
match(char *re, char *text)
{
  if(re[0] == '^')
    return matchhere(re+1, text);
  do{  // must look at empty string
    if(matchhere(re, text))
      return 1;
  }while(*text++ != '\0');
  return 0;
}

// matchhere: search for re at beginning of text
int matchhere(char *re, char *text)
{
  if(re[0] == '\0')
    return 1;
  if(re[1] == '*')
    return matchstar(re[0], re+2, text);
  if(re[0] == '$' && re[1] == '\0')
    return *text == '\0';
  if(*text!='\0' && (re[0]=='.' || re[0]==*text))
    return matchhere(re+1, text+1);
  return 0;
}

// matchstar: search for c*re at beginning of text
int matchstar(int c, char *re, char *text)
{
  do{  // a * matches zero or more instances
    if(matchhere(re, text))
      return 1;
  }while(*text!='\0' && (*text++==c || c=='.'));
  return 0;
}