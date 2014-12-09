//////////////////////////////////////////////////////
// terminal 처리 함수
// 터미널 상에서 입력 처리
//////////////////////////////////////////////////////////

#include "MiniSH.h"

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
  printf("[MiniSH@%s]$", curpath);
  sprintf(path, "[MiniSH@%s]$", curpath);

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
	 if((c = fgetc(Stdin)) != 91)
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

