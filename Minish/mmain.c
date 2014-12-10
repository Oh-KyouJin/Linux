 /////////////////////////////////////////////////////
 // MiniSH 헤더 파일
 // MiniSH 구현을 위하여 필요한 변수 및 함수정의
 ////////////////////////////////////////////////////
 
 #ifndef _MiniSH_H_
 #define _MiniSH_H_
 
 #include<stdio.h>
 #include<stdlib.h>
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
 void __history(char **argvp);
 void __exit(char **argvp);
  
 //history 처리 함수
 void hisproc(char **argvp);
 void init_list(void);
 void open_list(void);
 void add_list(char *command);
 void del_list(char **argvp);
 void prn_list(char **argvp);
 void sav_list(char **argvp);
  
 //redirection및 pipe처리 함수
 void pipeproc(char **argvp);
 int ispipe(char **argvp);
 void redirection(char **argvp);
 void redirectionin(char **argvp);
 void redirectionout(char **argvp, int choice);
  
 #endif  //_MiniSH_H

//////////////////////////////////////////////////////////////
// main 함수 만들기
//쉘 기능을 하는 프로세스를 생성하고, 명령어를 입력받아 처리
/////////////////////////////////////////////////////////////



int main(int argc, char *argv[])
{
  //변수 선언 및 초기화 부분
  char   command[MAXSIZE] = {0};  //인자로 받은 명령어들
  char   *com_tmp;                //command저장하는 임시 문자열
  char   *buff;                   //여러 명령어 중 하나를 저장
  char   *buff_tmp;               //buff를 저장하는 임시 문자열
  char   **argvp;                 //명령어를 분리하여 저장할 포인터 배열
  char   *token;                  //토큰
  char   c;                       //입력 받은 문자 하나

  int    status;                  //process status
  int    com_count = 1;           //명령어 개수
  int    buff_count = 0;          //명령어가 몇 개의 인자로 이루어져있는지
  int    i=0, n=0;                //임시 변수

  memset(abspath, 0, sizeof(abspath)); //MiniSH가 위치한 절대경로
  background = 0;                 //백그라운드인지 여부를 체크
  work_num = 1;                   //작업 번호
  //////////////////////////////////////////////////////////////////
  
  /////////////////////////////////////////////////////////////
  //쉘 프롬프트를 띄우기 전에 필요한 사항 초기화
  //MiniSH을 usage에 맞게 수행했는지 여부와 프롬프트를 띄우기 위ㅐ
  //사용자 이름과 현재위치, 그리고 signal초기화

  if(argc != 1)
  {
    perror("Usage : MiniSH");
    exit(-1);
  }

  //사용자 이름 파악
  userinfo = getpwuid(getuid());

  //MiniSH가 위치한 절대 경로 저장
  getcwd(abspath, MAXSIZE);

  //쉘 변수 초기화
  memset(shell, 0, sizeof(shell));

  //history 초기화
  init_list();

  //SIGINT 신호 무시함
  signal(SIGINT, SIG_IGN);

  ///////////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////
  //사용자가 입력한 명령을 받아서 처리함
  while(1)
  {
    //프롬프트 띄우고 명령 입력 받음
    if(getCommand(command)==0)
      continue;

    //입력 받은 명령어 history에 저장
    add_list(command);

    //command를 임시 문자열 com_tmp에 저장, 에러시 다시 프롬프트 띄움
    if((com_tmp = (char *)malloc(strlen(command)+1)) == NULL)
    {
      perror("Can't allocate command's temp string variable");
      continue;
    }
    else
    {
      strcpy(com_tmp, command);
    }

    //command에 저장된 명령어 개수 세기(com_tmp를 이용)
    if(strtok(com_tmp, ";") != NULL)
      for(com_count = 1; strtok(NULL, ";") != NULL; com_count++);
    

    //////////////////////////////////////////////////////////////
    //command의 개수만큼 수행

    for(i=1;i<=com_count;i++)
    {
      ///////////////////////////
      //각 comman를 공백으로 다시구분하여 argvp 이중포인터배열에 저장
      //com_tmp를 다시 사용하기 위하여 command를 다시 저장해둠
      strcpy(com_tmp,command);

      //i위치에 해당하는 명령을 token으로 분리
      token = strtok(com_tmp, ";");
      for(n=1;n<i;n++)
        token=strtok(NULL, ";");
      
      //분리한 token을 buff와 buff_tmp에 저장해둠
      if((buff = (char *)malloc(strlen(token)+1)) == NULL)
      {
        perror("Can't allocate buff string variable");
	continue;
      }
      else
      {
         if((buff_tmp = (char *)malloc(strlen(token)+1)) == NULL)
	 {
	   perror("Can't allocate buff_tmp string variable");
	   continue;
	 }
	 strcpy(buff,token);
	 strcpy(buff_tmp, token);
      }

      //buff_tmp에 저장된 토큰의 수를 셈
      if(strtok(buff_tmp, " ") != NULL)
        for(buff_count = 1; strtok(NULL, " ")!=NULL;buff_count++);

      //각 토큰을 가리키는 포인터들이 들어갈 인자 배열 만듬
      if((argvp = (char **)malloc((buff_count+1)*sizeof(char*))) == NULL)
      {
        free(buff);
	free(buff_tmp);
	perror("Can't allocate argvp string variable");
	continue;
      }
      //토큰을 가리키는 포인터들을 인자 배열에 넣음
      *argvp = strtok(buff, " ");
      for(n=1;n<buff_count;n++)
      { 
        *(argvp + n) = strtok(NULL, " " );

	//인자 중에 백그라운드 표시가 있으면 체크하고 &는
	//argvp에 포함시키지 않음
	if(strcmp(*(argvp +n), "&") == 0)
	{
	  background = 1;
	  buff_count -= 1;
	}
       }
       *(argvp + buff_count) = NULL;
       //
       //
       ///////////////////////////////////////////////////////////

       //쉘 내부 명령어 처리
       //각 명령어에 따른 실행
       //명령어를 실행 했으면 continue 하여 다음 command받고
       //내부 명령어가 아니었다면 다음 처리를 함
       //단 리다이렉션 또는 파이프일경우엔 fork후 사용해야 하므로 건너띔

       if(!ispipe(argvp))
          if(inCommand(argvp))
	    continue;
	//
	//
	////////////////////////////////////////////////////////////

	//자식과 부모 프로세스의 수행과정
	//fork수행, 에러시 에러 메세지 출력후 for문 탈출
	if(forkproc(argvp) == 0)
	  continue;
	else
	  break;
	//
	//
	///////////////////////////////////////////////////////////
      }
   }

   return 0;
 }

//////////////////////////////////////////////////////////////
// fork 자식 프로세스
// 쉘에서 받은 명령어 자식 프로세스 생성 후 수행
/////////////////////////////////////////////////////////////

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

///////////////////////////////////
//  hisproc.c를 지정한다.
//  history 기능을 구현한 부분
//////////////////////////////////
list_t *head;
list_t *tail;

///////////////////////////////////////
//Function : void hisproc(char **argvp)
//=====================================
//Input : command를 분리한 argument vector
//Output : 없음
//Purpose : 각 옵션에 따른 각 함수 호출
////////////////////////////////////////
void hisproc(char **argvp)
{
  if(argvp[1] == NULL)
    prn_list(argvp);
  else if(strcmp(argvp[1], "-c") == 0)
    del_list(argvp);
  else if(strcmp(argvp[1], "-d") == 0)
    del_list(argvp);
  else if(strcmp(argvp[1], "-w") == 0)
    sav_list(argvp);
  else if(atoi(argvp[1]) != 0)
    prn_list(argvp);
}

////////////////////////////////////////////
//Function : void init_list(void)
//============================================
//Input : 없음
//Output : 없음
//Purpose : history list 초기화
/////////////////////////////////////////////

void init_list(void)
{
  //history list 초기화
  hishead = (list_t *)malloc(sizeof(list_t));
  histail = (list_t *)malloc(sizeof(list_t));

  hishead->name = NULL;
  hishead->value = NULL;
  hishead->prev = hishead;
  hishead->next = histail;
  histail->name = NULL;
  histail->value = NULL;
  histail->prev = hishead;
  histail->next = histail;
}

////////////////////////////////////////////////////
//Function : void open_list(void)
//==================================================
//Input : 없음
//Output : 없음
//Purpose : history 파일에서 history목록을 읽어와서 history list에 저장
///////////////////////////////////////////////////////
void open_list(void)
{
  FILE *fp;
  char line[MAXSIZE];
  char *history;
  int n, hismax;
  list_t *t = hishead;
  list_t *l;
  
  //환경변수에서 history의 최대 크기를 받아줌
  hismax = atoi(getenv("HISTSIZE"));

  //파일이 존재하면 내용을 읽어서 리스트 작성
  if((fp = fopen("history", "r")) > 0)
  {
    for(n=1;n<=hismax;n++)
    {
      //파일에서 한줄 씩 읽어와서 history리스트에 추가
      if(fgets(line, MAXSIZE, fp) > 0)
      {
        if(line[strlen(line)-1] == '\n')
	   line[strlen(line)-1] = '\0';

	l = (list_t *)malloc(sizeof(list_t));
	l->value = (char *)malloc(strlen(line)+1);
	strcpy(l->value, line);

	l->prev = t;
	l->next = t->next;
	t->next->prev = l;
	t->next = l;

	t=t->next;
       }
     }
   }
}

//////////////////////////////////////////////////////////////
//Function : void add_list(char *command)
//==============================================================
//Input : command(명령어 한 라인)
//Output : 없음
//Purpose : 명령어 라인을 history list에 추가
/////////////////////////////////////////////////////////////

void add_list(char *command)
{
  char *tmp;
  list_t *t;
  int hismax = 1000;

  //history list가 꽉 찼으면 가장 앞에 노드 삭제
  if(hiscount == hismax)
  {
    t = hishead->next;
    t->prev->next = t->next;
    t->next->prev = t->prev;
    free(t);
  }

  //history list에 명령어 추가
  t = (list_t *)malloc(sizeof(list_t));
  t->value = (char *)malloc(strlen(command)+1);
  strcpy(t->value,command);

  t->prev = histail->prev;
  t->next = histail;
  histail->prev->next = t;
  histail->prev = t;

  hiscount++;
}

//////////////////////////////////////////////////////////////
//Function : void del_list(char **argvp)
//=============================================================
//Input : command를 분리한 argument vector
//Output : 없음
//Purpose : history list에서 인자값에 따라서 목록 삭제
////////////////////////////////////////////////////////////////
void del_list(char **argvp)
{
  int n;
  list_t *t, *del;

  //offsetd위치의 history삭제
  if(argvp[2] != 0)
  {
     n = atoi(argvp[2]);
     t = hishead;

     //offset위치만큼 이동
     while(n > 0)
     {
       t = t->next;
       n--;
     }

     //offset이 history의 범위를 넘지 않았으면 삭제 수행
     if(t != hishead || t != histail)
     {
        t->prev->next = t->next;
	t->next->prev = t->prev;
	free(t);
	hiscount--;
     }
   }
   //history 전부 삭제
   else
   {
     t = hishead->next;
     while(t != histail)
     {
       del = t;
       t = t->next;
       free(del);
     }

     hishead->next = histail;
     histail->prev = hishead;
     hiscount = 0;
   }
}

/////////////////////////////////////////////////////////////
//Function : void prn_list(char **argvp)
//=========================================================
//Input : command를 분리한 argument vector 
//Output : 없음
//Purpose : history list 내용를 화면에 출력
/////////////////////////////////////////////////////////////
void prn_list(char **argvp)
{
  int n;
  list_t *t;

  //인자가 없으면 history 전부 출력
  if(argvp[1] == 0)
  {
    t = hishead->next;
    while(t != histail)
    {
      printf("%s\n", t->value);
      t = t->next;
    }
  }
  //출력 목록 개수가 지정되어 있으면
  else
  {
    n = atoi(argvp[1]);
    t = histail;

    //n개의 위치만큼 뒤에서부터 이동
    for(n; n>0; n--)
    {
      t = t->prev;
      if(t == hishead->next)
        break;
    }

    //이동한 위치에서부터 마지막까지 출력
    while(t != histail)
    {
      printf("%s\n", t->value);
      t = t->next;
    }

  }
}

////////////////////////////////////////////////////////////
//Function : void sav_list(char **argvp)
//==========================================================
//Input : command를 분리한 argument vector
//Output : 없음
//Purpose : 인자가 있으면 해당 파일에 없으면 history파일에 
//    history 목록 출력
//////////////////////////////////////////////////////////////
void sav_list(char **argvp)
{
  list_t *t = hishead->next;
  FILE *fp;

  //파일이름이 없으면 history파일 open
  if(argvp[2] == NULL)
    fp = fopen("history", "w");
  //파일이름이 존재하면 해당파일 open
  else
     fp = fopen(argvp[2], "w");

  //열린 파일에 history list 출력
  while(t != histail)
  {
    fprintf(fp, "%s\n", t->value);
    t = t->next;
  }

  //열린 파일 닫음
  fclose(fp);
}

////////////////////////////////////////////////////////////
// incommand 구현
// 쉘 내부에서 처리되는 명령어들을 구현
//////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
//Function : int inCommand(char **argvp)
//============================================================
//Input : command를 분리한 argument vector
//Output : 쉘내부 명령어 처리를 했으면 1, 안했으면 0
//Purpose : 명령어가 쉘 내부 명령이라면 그에 해당하는 함수 호출
//////////////////////////////////////////////////////////////
int inCommand(char **argvp)
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
      perror("Can't set umask");
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
    if(argvp[n][0] == '$')
    {
      //쉘 변수 라면
      if(argvp[n][1] > 48 && argvp[n][1] < 58)
      {
        printf("%s", shell[argvp[n][1]-49]);
      }
      //환경변수라면
      else
      {
        name = strtok(argvp[n], "$");
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
////////////////////////////////////////////////////////////
// pipe프로세스
// 파이프/리다이렉션 명령어 처리
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//Function : void pipeproc(char **argvp)
//==========================================================
//Input : command 를 분리한 argument vector
//Output : 없음
//Purpose : 명령어 내 파이프/리다이렉션 기호가 존재하면 그에 따른 처리흫 해줌
////////////////////////////////////////////////////////////////
void pipeproc(char **argvp)
{
  int n, i, j;
  int subpid;
  int status;
  int exist = 0;
  int fds[2];
  char **argvp2 = (char **)malloc(16*sizeof(char *));

  //'|'를 찾아냄
  for(n=0;argvp[n]!=NULL;n++)
  {
    if(strcmp(argvp[n], "|") == 0)
    {
      exist = 1;
      break;
    }
  }
  //"|"앞의 명령은 argvp로 명령은 argvp2로 옮김
  //"|"가 존재한다면
  if(exist)
  {
    argvp[n] = NULL;
    for(i=n+1, j=0; argvp[i] != NULL; i++, j++)
    {
      argvp2[j] = (char *)malloc(strlen(argvp[i]+1));
      strcpy(argvp2[j], argvp[i]);
      argvp[i] = NULL;
    }
    argvp2[j] = NULL;
  }
  //"|"가 존재하지 않는다면
  else
  {
    argvp2 = argvp;
  }

  //"|"가 존재하면
  //fork()를 이용하여 각 명령어 처리
  if(exist)
  {
    //파이프 처리
    pipe(fds);

    if((subpid = fork()) < 0)
    {
      perror("Can't fork process");
      exit(1);
    }
    //자식 프로세스의 수행결과는 표준 출력 보냄
    else if(subpid)
    {
      dup2(fds[1], STDOUT_FILENO);
      close(fds[0]);
      close(fds[1]);
      redirection(argvp);
      exit(1);
    }
    //부모프로세스일 경우 자식의 결과를 표준 입력으로 받음
    dup2(fds[0], STDIN_FILENO);
    close(fds[0]);
    close(fds[1]);
  }
  //A|B 일경우 B에 해당하는 부분 처리
  redirection(argvp2);
  exit(1);
}

//////////////////////////////////////////////////////////
//Function : void redirection(char **argvp)
//======================================================
//Input : command를 분리한 argument vector
//Output : 없음
//Purpose : 명령 중에 리다이렉션이 포함되면 처리해줌
///////////////////////////////////////////////////////////
void redirection(char **argvp)
{
  int  n;
  int  check[3] = {0};
  char tmppath[MAXSIZE] = {0};

  //명령어 내 어떤 리다이렉션 기호가 포함되어 있는지 확인
  for(n=0; argvp[n]!=NULL;n++)
  {
     if(strcmp(argvp[n], "<") == 0)
       check[0] = 1;
     else if(strcmp(argvp[n], ">") == 0)
       check[1] = 1;
     else if(strcmp(argvp[n], ">>") == 0)
       check[2] = 1;
   }
   //check의 경과에 따라서 '<' 또는 '>', '>>' 처리를 해줌
   if((check[1] == 1) || (check[2] == 1))
     redirectionout(argvp, check[1]);
   if(check[0] == 1)
     redirectionin(argvp);

   //쉘 내부 명령어라면 수행하고 아니면 if문 내의 명령들로 수행
   if(inCommand(argvp) == 0)
   {
     execvp(*argvp, argvp);
   }
}

/////////////////////////////////////////////////////////////////////
//Function : int ispipe(char **argvp)
//=======================================================================
//Input : command를 분리한 argument vector
//Output : 명령어가 파이프/리다이렉션이면 1, 아니면 0
//Purpose : 명령어가 파이프/리다이렉션인지 여부 확인
///////////////////////////////////////////////////////////////////////
int ispipe(char **argvp)
{
  int n, check = 0;

  for(n=0; argvp[n] != NULL; n++)
  {
    if(strcmp(argvp[n], "<") == 0)
    {
      check = 1;
      break;
    }
    else if(strcmp(argvp[n], ">") == 0)
    {
      check = 1;
      break;
    }
    else if(strcmp(argvp[n], ">>") == 0)
    { 
      check = 1;
      break;
    }
    else if(strcmp(argvp[n], "|") == 0)
    {
      check = 1;
      break;
    }
    else
      check = 0;
  }
  return check;
}
////////////////////////////////////////////////////////////////
//Function : void redirectionout(char **argvp, int choice)
//===================================================================
//Input : command를 분리한 argument vector, '>' '>>' 구분인자
//Output : 없음
//Purpose : 명령어 내 프로그램의 표준 출력을 파일로 재지향
///////////////////////////////////////////////////////////////
void redirectionout(char **argvp, int choice)
{
  int outfd,n;

  //재지향 파일을 찾음
  for(n=0; argvp[n]!=NULL;n++)
  {
    if((strcmp(argvp[n], ">") == 0) || (strcmp(argvp[n], ">>") == 0))
      break;
  }

  //">" 이었다면
  if(choice == 1)
  {
    //재지향 파일을 염
    if((outfd = open(argvp[n+1], OUTFLAG, FMODE)) == -1)
      perror("Can't open file for redirection out");
  }
  // ">>"이었다면
  else
  {
     //재지향 파일을 염
     if((outfd = open(argvp[n+1], APPFLAG)) == -1)
       perror("Can't open file for redirection out");
   }

   // > 또는 >> 이므로 파일을 stdout으로 dup함
   if(dup2(outfd, STDOUT_FILENO) == -1)
     perror("Can't duplicate outfd to STDOUT_FILENO");

   //argvp에서 파일 부분을 제거한다.
   argvp[n] = NULL;
   argvp[n+1] = NULL;

   //outfd 닫음
   close(outfd);

 }

////////////////////////////////////////////////////////////////
//Function : void redirectionin(char **argvp)
//=============================================================
//Input : command를 분리한 argument vector
//Output : 없음
//Purpose : 명령어 내 프로그램의 표준 입력을 파일로 재지향
/////////////////////////////////////////////////////////////
void redirectionin(char **argvp)
{
  int infd, n;
  
  //지향할 파일을 염
  if((infd = open(argvp[2], O_RDONLY)) == -1)
    perror("Can't open file for redirection in");

  //<이므로 파일을 stdin으로 dup함
  if(dup2(infd, STDIN_FILENO) == -1)
    perror("Can't duplication infd to STDIN_FILENO");

  //수행할 프로그램만 argvp에 남겨둔다.
  for(n=0; argvp[n]!=NULL;n++)
    if(strcmp(argvp[n], "<") == 0)
      break;

  argvp[n] = NULL;
  argvp[n+1] = NULL;

  //infd 닫음
  close(infd);
}

//////////////////////////////////////////////////////
// terminal 처리 함수
// 터미널 상에서 입력 처리
//////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
//Function : int getcommand(char *command)
//======================================================
//Input : command(명령어 라인)
//Output : command에 내용이 있으면 1, 없으면 0
//Purpose : 터미널에서 개행문자까지 입력받아서 command에 저장
////////////////////////////////////////////////////////////

int getCommand(char *command)
{
  int n = 0, space_chk=0;
  char path[MAXSIZE] = {0};
  char c;
  nowhistory = histail;

  fflush(NULL);

  //프롬프트 띄우고 명령 입력을 대기
  curpath = (char *)getenv("PWD");
  printf("[MiniSH@%s]$ ", curpath);
  sprintf(path, "[MiniSH@%s]$ ", curpath);

  //명령 입력을 받음
  while((c = fgetc(stdin)))
  {
    //enter key를 입력 받으면 입력 종료
    if(c == 10)
    {
      fputc('\n', stdout);
      break;
    }

    //Ctrl + D를 입력받으면 입력 종료
    if(c == 4)
    {
      fputc('\n', stdout);
      return 0;
    }

    //backspace key를 제외한 입력 무시
    if((c <= 26) && (c != 8))
       break;

    switch(c)
    {
       //backspace key를 입력받을때 처리
       case 8:
         if(n == 0)
	   break;

	 fputc('\b', stdout);
	 fputc(' ', stdout);
	 fputc('\b', stdout);
	 command[--n] = (char)0;
	 break;
       //^[ (esc) 기호이면서
       case 27:
         //[ 기호가 아니면 switch문 탈출
	 if((c = fgetc(stdin)) != 91)
	    break;

	 //[ 기호가 들어왔다면
	 switch(fgetc(stdin))
	 {
	    //^[[A 기호, 즉 key가 up일때 history내용 출력
	    case 65:
	      nowhistory = nowhistory->prev;
	      if(nowhistory != hishead)
	      {
	        fprintf(stdout, "\r%80s", " ");
		fprintf(stdout, "\r%s%s", path, nowhistory->value);
		n = strlen(command);
	      }
	      else
	      {
	        nowhistory = hishead->next;
	      }
	      break;
	    //^[[ 기호, 즉 key가 down일때 history내용 출력
	    case 66:
	      nowhistory = nowhistory->next;
	      if(nowhistory != histail)
	      {
	        fprintf(stdout, "\r%80s", " ");
		fprintf(stdout, "\r%s%s", path, nowhistory->value);
		strcpy(command, nowhistory->value);
		n = strlen(command);
	      }
	      else
	      {
	        fprintf(stdout, "\r%80s", " ");
		fprintf(stdout, "\r%s", path);
		n = 0;
	      }
	      break;
	    //^[[C 기호, 즉 key가 right일때 아무 처리 안함
	    case 67:
	      break;
	    //^[[D 기호, 즉 key가 left일때 아무 처리 안함
	    case 68:
	      break;
	  }
	  break;
     //그 외 일반적인 문자이면
     default:
       fputc(c, stdout);
       command[n++] = (char)c;
       break;
     }
   }
   command[n] = '\0';

   //아무런 입력이 없었으면 프롬프트를 다시 띄움
   if(strlen(command) == 0)
     return 0;

   for(n=0;n<strlen(command); n++)
     if(command[n] != ' ' && command[n] != '\t')
       return 1;

   return 0;
}




















