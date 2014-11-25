#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<dirent.h>

void list_directory(char *dirname, int count);

int main(int argc, char* argv[]){
  list_directory(".", 1);
}

void list_directory(char* dirname, int count){
  DIR *dp;
  struct dirent *dirp;
  struct stat fstat;

  dp = opendir(dirname);
  if(!dp){
    fprintf(stderr, "can't open directory (%s)\n", dirname);
    perror("open directory");
    return;
  }

  chdir(dirname);

  while((dirp = readdir(dp)) != NULL){
    if(strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
      continue;
    
    if(stat(dirp->d_name, &fstat) == -1){
      fprintf(stderr, "can't stat file (%s)\n", dirp->d_name);
      perror("stat file");

      continue;
    }

    if(S_ISDIR(fstat.st_mode)){
      printf("%*c./%s:\n", 2*count, ' ', dirp->d_name);
      list_directory(dirp->d_name, count+1);
    }else{
      printf("%*c%s\n", 2*count, ' ',dirp->d_name);
      }
    }
    closedir(dp);
    chdir("..");
}
