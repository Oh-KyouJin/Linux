#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<stdio.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<dirent.h>

void leadfile();

#define chop(str) str[strlen(str)-1]=0x00;

int main(int argc, char **argv)
{
  char buf[256];
  printf("MiniSH Start!\n");
  
  int pid;
  while(1)
  {
  printf("[Input Command] ");
  fgets(buf,255,stdin);
  chop(buf);
    if(strncmp(buf,"quit",4)==0) // strncmp(cmptext1, cmptext2, size) 
    {
      printf("Good bye\n");
      exit(0);
    }
    if(strncmp(buf,"jin",3)==0)
    {
       printf("jinjinjin\n");
    }

    if(strncmp(buf, "ls", 2) == 0)
    {
       leadfile();
    }
   /* if(access(buf, X_OK)==0) // access(pathname, mode)
    {
      pid = fork();
      if(pid < 0)
      {
        fprintf(stderr,"Command error!\n");
      }
      if(pid == 0)
      {
        if(execl(buf,buf,NULL) == -1) // execl(dic*file, dic*file, NULL)
	fprintf(stderr,"Command error!\n");
        exit(0);
      }
      if(pid > 0)
      {
        int stat;
        waitpid(pid, &stat, WUNTRACED);
      } 
    }
    else
    {
       fprintf(stderr,"Command Not Found!\n\n");
    }*/
  }
}

void leadfile(){
   DIR *pdir;
   struct dirent *pde;
   int i = 0;

   if((pdir = opendir("./")) < 0) {
      perror("opendir");
      exit(1);
   }

   while((pde = readdir(pdir)) != NULL){
      printf("%20s", pde->d_name);
      if(++i%4 == 0){
        printf("\n");
      }
    }

    printf("\n");

    closedir(pdir);
}
