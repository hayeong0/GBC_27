#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100 //채팅시 메시지 최대 길이
#define MAX_CLNT 256 //동시 접속 최대 클라이언트 수

void *handle_clnt(void * arg);  //클라이언트 스레드용
void send_msg(char * msg, int len); //메시지 전송
void error_handling(char * msg);    //예외 처리
//void transport(int clnsock);

int clnt_cnt=0; //접속한 클라이언트 카운트
/* 서버에 접속한 클라이언트의 소켓 관리를 위한 변수, 배열 */
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

//인자로 포트번호 받음
int main(int argc, char *argv[])
{
    //소켓 통신용 서버 소켓, 클라이언트 소켓
	int serv_sock, clnt_sock;
    //서버, 클라이언트 소켓 구조체 선언
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;    
	pthread_t t_id; //클라이언트 스레드용 ID

	if(argc!=2) {
        //포트 입력 확인
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
    //스레드의 동시접속 막기 위해 mutex 선언
	pthread_mutex_init(&mutx, NULL);
    // 서버 소켓(리스닝 소켓) 생성 
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);

    //소켓 미생성 했을 경우 에러 출력
    if(serv_sock == -1){
        error_handling("socket() ERROR!");
    }

    /* 주소 정보 초기화 */
	memset(&serv_adr, 0, sizeof(serv_adr));
    //IPv4 사용
	serv_adr.sin_family=AF_INET; 
    //현재 IP입력받아 반환
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    //사용자가 입력한 포트 사용
	serv_adr.sin_port=htons(atoi(argv[1])); 
	
    //binding으로 IP와 port번호 할당
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
    
    //서버 소켓을 연결요청 가능한 상태로 설정
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	while(1)
	{
        //클라이언트 구조체의 크기 얻기
		clnt_adr_sz = sizeof(clnt_adr);
        //클라이언트의 접속 대기
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
		
        //임계영역 시작, mutex
		pthread_mutex_lock(&mutx);
        //새로운 연결 형성될 때마다 변수 clnt_cnt와 배열 clnt_socks에 해당 연결 정보 등록
		clnt_socks[clnt_cnt++]=clnt_sock;
        //임계영역 끝
		pthread_mutex_unlock(&mutx);
	
        //스레드 생성 + 클라이언트 구조체의 주소를 스레드에 넘김
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		//스레드 종료시, 스스로 소멸
        pthread_detach(t_id);
        //접속된 클라이언트의 IP 출력
		printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
	}
    //서버 소켓 종료
	close(serv_sock);
	return 0;
}

/* 스레드용 함수, 클라이언트와 통신 */	
void * handle_clnt(void * arg)
{
    //스레드가 통신할 클라이언트 소켓
	int clnt_sock = *((int*)arg);
	int str_len=0, i;
    //메시지 buffer size
	char msg[BUF_SIZE];
	
    // 클라이언트가 통신 종료하기 전까지
	while((str_len=read(clnt_sock, msg, sizeof(msg)))!=0)
		send_msg(msg, str_len);
	
    //mutex 임계영역 시작
	pthread_mutex_lock(&mutx);
    //remove disconnected client
	for(i=0; i<clnt_cnt; i++)   
	{
		if(clnt_sock==clnt_socks[i])
		{
			while(i++<clnt_cnt-1)
				clnt_socks[i]=clnt_socks[i+1];
			break;
		}
	}
	clnt_cnt--;
    //mutex 임계영역 끝
	pthread_mutex_unlock(&mutx);
	close(clnt_sock); //소켓 close
	return NULL;    //통신 종료
}
/* send to all (broadcast기능) */
void send_msg(char * msg, int len) 
{
	int i;
	pthread_mutex_lock(&mutx);
    //접속중인 모든 클라이언트 소켓에 메시지 출력
	for(i=0; i<clnt_cnt; i++)
		write(clnt_socks[i], msg, len);
	pthread_mutex_unlock(&mutx);
}
/* Error 처리 */
void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}