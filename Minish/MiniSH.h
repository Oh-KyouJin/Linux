/////////////////////////////////////////////////////
// MiniSH 헤더 파일
// MiniSH 구현을 위하여 필요한 변수 및 함수정의
////////////////////////////////////////////////////

#ifndef _MiniSH_H_
#define _MiniSH_H_

#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<pwd.h>
#include<signal.h>
#include<termio.h>

#define MAXSIZE 1024

//리다이렉션 명령 처리시 필요한 open을 위한 flag 및 mode
#define OUTFLAG (O_WRONLY | O_CREAT | O_TRUNC)
#define APPFLAG (O_WRONLY | O_CREAT | O_APPEND)
#define FMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

extern char **environ;   //환경 변수
char shell[9][MAXSIZE];  //쉘 변수(9개)

char abspath[MAXSIZE];   //MiniSH가 위치한 절대경로
char *curpath;           //현재 경로
int background;          //백그라운드 여부 체크
int work_num;            //작업 번호

struct passwd  *userinfo;  //사용자 정보 구조체
struct termios termstat;   //terminal io stat 구조체

//list사용을 위하여 정의한 list구조체
typedef struct list
{
  char   *name;
  char   *value;
  struct list *prev;
  struct list *next;
}list_t;

list_t *hishead;   //history list head
list_t *histail;   //history list tail

list_t *nowhistory; //현재 history를 지정

int hiscount;      //history 개수

//쉘의 입력 처리 함수
int getCommand(char *command);

//fork를 이용한 실질적인 명령어 수행 처리 함수
int forkproc(char **argvp);

//쉘 커맨드 처리 함수
int inCommand(char **argvp);
//쉘 커맨드 함수들
void __env(char **argvp);
void __cd(char **argvp);
void __pwd(char **argvp);
void __umask(char **argvp);
void __echo(char **argvp);
void __kill(char **argvp);
void __history(char **argvp);
void __exit(char **argvp);

//history 처리 함수
void hisproc(char **argvp);
void init_list(void);
void open_list(void);
void add_list(void);
void del_list(void);
void pm_list(char **argvp);
void sav_list(char **argvp);

//redirection및 pipe처리 함수
void pipeproc(char **argvp);
void ispipe(char **argvp);
void redirection(char **argvp);
void redirectionin(char **argvp);
void redirectionout(char **argvp, int choice);

//usage 처리 함수 (도움말 명령어)
int showusage(char **argvp);
void usage_MiniSH(void);
void usage_env(void);
void usage_cd(void);
void usage_pwd(void);
void usage_umask(void);
void usage_echo(void);
void usage_kill(void);
void usage_exit(void);

#endif  //_MiniSH_H


























