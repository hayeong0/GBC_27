#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
	
#define BUF_SIZE 100    //채팅시 메시지 최대 길이
#define NAME_SIZE 20    //이름의 길이 제한
	
void * send_msg(void * arg);    //송신 스레드
void * recv_msg(void * arg);    //수신 스레드
void error_handling(char * msg);    //예외 처리
	
char name[NAME_SIZE]="[DEFAULT]";
char msg[BUF_SIZE];

//첫번째 인자로 IP, 두번째 인자로 port번호 받음
int main(int argc, char *argv[])
{
	int sock;   //통신용 소켓 파일 디스크립터
	struct sockaddr_in serv_addr;   //서버 주소 구조체 변수
	pthread_t snd_thread, rcv_thread;   //송신, 수신 스레드
	void *thread_return;   
    //실행시 잘못 입력했을 경우 가이드라인 제시!
	if(argc!=4) {
		printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		exit(1);
	 }

	sprintf(name, "[%s]", argv[3]); //username
    
	sock = socket(PF_INET, SOCK_STREAM, 0); 
	/* 주소 정보 초기화 */
	memset(&serv_addr, 0, sizeof(serv_addr));   
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	
    //서버 접속
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");
	
    //송신 스레드
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	//수신 스레드
    pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);
	close(sock);  //소켓 소멸 -> 종료
	return 0;
}
	
void * send_msg(void * arg)   // send thread main
{
	int sock=*((int*)arg);  //소켓 받아오기
	char name_msg[NAME_SIZE+BUF_SIZE];
	while(1) 
	{
		fgets(msg, BUF_SIZE, stdin);
        //키보드 입력 받아서 q 또는 Q가 들어오면 종료
		if(!strcmp(msg,"q\n")||!strcmp(msg,"Q\n")) 
		{
			close(sock);
			exit(0);
		}
        //q 또는 Q가 아닌 입력 -> name + msg 출력
		sprintf(name_msg,"%s %s", name, msg);
        //server로 메시지를 보내준다
		write(sock, name_msg, strlen(name_msg));
	}
	return NULL;
}
	
void * recv_msg(void * arg)   // read thread main
{
	int sock = *((int*)arg);    //소켓
	char name_msg[NAME_SIZE+BUF_SIZE];
	int str_len;
	while(1)
	{
        //클라이언트가 메세지 입력시
		str_len = read(sock, name_msg, NAME_SIZE+BUF_SIZE-1);
        //통신 끊겼을 경우 스레드 종료
		if(str_len==-1) 
			return (void*)-1;
        // 
		name_msg[str_len]=0;
        //화면에 수신된 메시지 출력
		fputs(name_msg, stdout);
	}
	return NULL;
}
	
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}