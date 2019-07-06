/*  
    GBC_Linux_homework #1
    21800758 Choi Hayeong
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int parseCmd(char *cmd, char *argList[]);
void execCmd(char *argList[], int bg);

int main(int argc, char *argv[])
{
    char command[256];
    char *argList[256];
    int argCount = 0;
    int i = 0;
    char curDir[256];

    while(1){
        //print a prompt message
        printf("myshell %s> ", getwd(curDir));
        //read a command string
        gets(command);
        //parse the command into an argument list
        argCount = parseCmd(command, argList);

        printf("%d arguments.\n", argCount);
        for(i = 0; argList[i] != NULL; i++){
            printf("\targList[%d] = %s\n", i, argList[i]);
        }

        //exit 입력했을 경우, 루프 종료
        if(strcmp(argList[0],"exit") == 0){
            break;
        }
        // If the command is “cd”, call chdir() with argList[1]
        else if(strcmp(argList[0], "cd") == 0){
            chdir(argList[1]);
        }
        else{
            //백그라운드 실행
            if(strcmp(argList[argCount-1], "&") == 0){
                //bg 1로 설정, 부모 프로세스는 자식 프로세스 생성 후 계속 진행
                argList[argCount-1] = NULL;
                execCmd(argList, 1);
            }
            //포그라운드
            execCmd(argList, 0);
        }
    }
    printf("Good bye!");
    return 0;
}

int parseCmd(char *cmd, char *argList[]){
    char *result = NULL;
    int cnt = 0;
    int i = 0;

    result = strtok(cmd, " ");

    while(result){
        argList[i] = result;
        i++;
        result = strtok(NULL, " ");
        cnt++;
    }
    argList[cnt] = 0;
    return cnt;
}

void execCmd(char *argList[], int bg){
    pid_t pid;
    pid = fork(); //실패 -1, 부모 process 새로운 PID, 자식 process 0
    int tmp = 0;
    //if pid == -1
    if(pid < 0){
        fprintf(stderr, "fork failed\n");
        exit(-1);
    }
    //in the child process, call execvp() to execute the command
    else if(pid == 0){
        tmp = execvp(argList[0], argList);
        if(tmp == -1){
            printf("Invaild command!\n");
            exit(1);
        }
    }
    //if pid != 0 --> parent process
    else{     
        //if bg = 1, background
        if(bg == 1) return;

        pid = wait(NULL);
    }
}



