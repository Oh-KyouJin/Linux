////////////////////////////////////////////////////////////
// incommand 구현
// 쉘 내부에서 처리되는 명령어들을 구현
//////////////////////////////////////////////////////////////

#include "MiniSH.h"

///////////////////////////////////////////////////////////////
//Function : int inCommand(char **argvp)
//============================================================
//Input : command를 분리한 argument vector
//Output : 쉘내부 명령어 처리를 했으면 1, 안했으면 0
//Purpose : 명령어가 쉘 내부 명령이라면 그에 해당하는 함수 호출
//////////////////////////////////////////////////////////////
int inCommad(char **argvp)
{
  int n = 1;
  //argvp가 쉘 내부 명령이라면 그에 따른 함수를 호출
  if(strcmp(argvp[0], "env") == 0)
    __env(argvp);
  else if(strcmp(argvp[0], "cd") == 0)
    __cd(argvp);
  else if(strcmp(argvp[0], "pwd") == 0)
    __pwd(argvp);
  else if(strcmp(argvp[0], "umask") == 0)
    __umask(argvp);
  else if(strcmp(argvp[0], "echo") == 0)
    __echo(argvp);
  else if(strcmp(argvp[0], "kill") == 0)
    __kill(argvp);
  else if(strcmp(argvp[0], "history") == 0)
    hisproc(argvp);
  else if(strcmp(argvp[0], "exit") == 0)
    __exit(argvp);
  else 
    n = 0;

  //위 명령어 중 어느 것에도 해당이 안되었으면
  //1을 리턴함
  return n;
}

//////////////////////////////////////////////////////////////
//Function : void _env(char **argvp)
//=============================================================
//Input : comman를 분리한 argument vector
//Output : 없음
//Purpose : 환경변수를 출력한다.
////////////////////////////////////////////////////////////////
void __env(char **argvp)
{
  int n;
  for(n=0;environ[n] != NULL; n++)
    printf("%s\n", environ[n]);
}

/////////////////////////////////////////////////////////////
//Function : void _cd(char **argvp)
//============================================================
//Input : command를 분리한 argument vector
//Output : 없음
//Purpose : argvp에 저장된 상대 경로 또는 절대 경로 위치로 현재 위치를 이동
/////////////////////////////////////////////////////////////////
void __cd(char **argvp)
{
  char curpath[MAXSIZE];

  //현재 디렉토리 OLDPWD에 저장
  getcwd(curpath, MAXSIZE);
  setenv("OLDPWD", curpath, 1);

  //디렉토리 변경하고 변경한 디렉토리 PWD에 저장
  chdir(argvp[1]);
  getcwd(curpath, MAXSIZE);
  setenv("PWD", curpath, 1);
}

/////////////////////////////////////////////////////////////////
//Function : void _pwd(char **argvp)
//===============================================================
//Input : command를 분리한 argument vector
//Output : 없음
//Purpose : 현재 위치를 절대경로로 화면에 출력한다.
////////////////////////////////////////////////////////////////////////
void __pwd(char **argvp)
{
  char buf[MAXSIZE] = {0};
  getcwd(buf, MAXSIZE);
  printf("%s\n", buf);
}

//////////////////////////////////////////////////////////////////////////
//Function : void _umask(char **argvp)
//==================================================================
//Input : command를 분리한 argument vector
//Output : 없음
//Purpose : 사용자 파일 생성 마스크값을 설정하거나 출력
//////////////////////////////////////////////////////////////////////
void __umask(char **argvp)
{
  mode_t mode;
  int n[3] = {0};
  int oct, i ,err = 0;

  //인자가 없거나 -S 옵션이면 현재 umask모드 출력
  //umask(0)하면 umask값이 0으로 바뀌므로
  //mode를 통하여 원래대로 복귀시켜놓음

  if(argvp[1] == 0 || strcmp(argvp[1], "-S") == 0)
  {
    printf("%03o\n", mode = umask(0));
    umask(mode);
  }
  //설정 값이 있으면 해당 값으로 umask설정
  else
  {
    //인자 중에 숫자가 아닌 것이 있는지 검색
    for(i=0;i<3;i++)
    {
      if(argvp[1][i] < 48 || argvp[1][i] > 57)
        err = 1;
    }

    //인자수가 틀리거나 인자 중에 숫자가 아닌것이 있으면 에러
    if(strlen(argvp[1]) != 3 || err == 1)
      perrer("Can't set umask");
    else
    { 
       n[0] = (int)(argvp[1][0]-48);
       n[1] = (int)(argvp[1][1]-48);
       n[2] = (int)(argvp[1][2]-48);

       oct = 64*n[0] + 8*n[1] + n[2];
       umask(oct);
    }
  }
}

////////////////////////////////////////////////////////////////
//Function : void _echo(char **argvp)
//===========================================================
//Input : command를 분리한 argument vector
//Output : 없음
//Purpose : 인자들을 화면에 출력한다.
///////////////////////////////////////////////////////////////
void __echo(char **argvp)
{
  int n;
  char *name, *value;

  for(n=1;argvp[n]!=NULL;n++)
  {
    //환경변수 또는 쉘변수라면 원래 값으로 치환하여 출력
    if(argvp[n][0] == $)
    {
      //쉘 변수 라면
      if(argvp[n][1] > 48 && argvp[n][1] < 58)
      {
        printf("%s", shell[argvp[n][1]-49]);
      }
      //환경변수라면
      else
      {
        name = strlok(argvp[n], "$");
	value = (char *)getenv(name);
	printf("%s", value);
      }
    }
    //일반 값이라면 그대로 출력
    else
    {
      printf("%s", argvp[n]);
    }
  }
  printf("\n");
}
///////////////////////////////////////////////////////////////////
//Function : void _kill(char **argvp)
//================================================================
//Input : command를 분리한 argument vector
//Output : 없음
//Purpose : 프로세스에 시그널 신호를 보냄
//////////////////////////////////////////////////////////////////////
void __kill(char **argvp)
{
  int signo;
  int pid;
  
  //시그널과 pid를 얻어옴
  signo = issig(argvp[1]);
  pid = atoi(argvp[2]);

  //해당 pid에 signo 신호를 보냄
  if(kill(pid, signo) == -1)
    perror("Failed to send signal");

}

/////////////////////////////////////////////////////////////////////////////
//Function : void _exit(char **argvp)
//==================================================================
//Input : command를 분리한 argument vector
//Output : 없음
//Purpose : 인자를 상대값으로 가지고 프로세스 종료
//////////////////////////////////////////////////////////////////
void __exit(char **argvp)
{
  int n;

  //넘어온 인자에 따른 exit number 설정
  if(argvp[1])
    n = atoi(argvp[1]);
  else
    n = 0;

  //history 파일 저장
  sav_list(argvp);

  exit(n);
}






























