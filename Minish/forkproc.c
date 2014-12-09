//////////////////////////////////////////////////////////////
// fork 자식 프로세스
// 쉘에서 받은 명령어 자식 프로세스 생성 후 수행
/////////////////////////////////////////////////////////////

#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<pwd.h>
#include<signal.h>
#include<termio.h>
#include "MiniSH.h"

//////////////////////////////////////////////////////////
//Function : int forkproc(char *argvp)
//========================================================
//Input : command를 분리한 argument vector
//Output : 함수가 제대로 수행되었으면 0, 에러 발생시 -1
//Purpose : fork()로 자식 프로세스를 생성하여 명령 수행
///////////////////////////////////////////////////////////

int forkproc(char **argvp)
{
  int  pid;
  int  status;
  int  pipefd[2];
  char tmppath[MAXSIZE] = {0};

  if((pid = fork()) < 0)
  {
    perror("Can't fork process");
    return -1;
  }
  //자식 프로세스 수행
  else if(pid == 0)
  {
    //자식 프로세스가 백그라운드로 수행중이라면 SIGINT 무시
    //작업번호, 프로세스ID,프롬프트가 먼저 뜨도록 만들어줌
    if(background == 1)
    {
      signal(SIGINT, SIG_IGN);
      printf("[%d] %d\n", work_num++, getpid());
    }
    //자식 프로세스가 백그라운드 수행중이 아니라면
    //SIGINT 시그널 받을 수 있도록 함
    else
    {
      signal(SIGINT, SIG_DFL);
    }

    //파이프 작업 일 경우 수행
    //pipeproc함수 내의 부모(현재)프로세스가 작업이 끝나면 종료되므로
    //작업이 완료되면 자식(현재)프로세스가 종료됨

    if(ispipe(argvp))
      pipeproc(argvp);
    
    execvp(*argvp, argvp);

    exit(0);
  }
  //부모 프로세스 수행
  else
  {
    //자식 프로세스가 백그라운드로 수행중이라면
    //부모 프로세스는 자식 프로세스를 기다리지 않고 바로 다음 명령 수행
    if(background == 1)
    {
      background = 0;
      return 0;
    }
    //자식 프로세스가 백그라운드로 수행중이 아니라면
    //부모 프로세스는 자식 프로세스를 기달렸다가 다음 명령 수행
    else
    {
      if((pid = waitpid(pid, &status, 0)) < 0)
      {
         perror("wait pid error");
	 return -1;
       }
       usleep(10);
       return 0;
     }
  }
}
