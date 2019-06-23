#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
void error_handling(char *message);

int main(int argc, char *argv[])
{
	int serv_sd, clnt_sd;
	FILE * fp;
	char buf[BUF_SIZE];
	int read_cnt;
	
	struct sockaddr_in serv_adr, clnt_adr;
	socklen_t clnt_adr_sz;
	
	//잘못 입력했을 시, 방법 제시
	if(argc!=2) {
		printf("Usage: %s <port>\n", argv[0]);
		exit(1);
	}
	
	/* 서버 파일 ---> 클라이언트 */
	//파일 열기 (binary mode -> rb)
	//binary mode는 img, png, txt등 많은 파일 지원
	fp=fopen("test.txt", "rb"); 
	//소켓 열기
	serv_sd=socket(PF_INET, SOCK_STREAM, 0);   
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	bind(serv_sd, (struct sockaddr*)&serv_adr, sizeof(serv_adr));
	listen(serv_sd, 5);
	
	clnt_adr_sz=sizeof(clnt_adr); 

	/* 클라이언트 연결요청 기다리기 */  
	clnt_sd=accept(serv_sd, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
	
	/* 데이터 전송 */
	while(1)
	{
		read_cnt=fread((void*)buf, 1, BUF_SIZE, fp);
		if(read_cnt<BUF_SIZE)
		{
			write(clnt_sd, buf, read_cnt);
			break;
		}
		write(clnt_sd, buf, BUF_SIZE);
	}
	
	//파일 전송 후, 출력스트림 Half-close
	//클라이언트에게 EOF 전달, 파일 전송 완료
	shutdown(clnt_sd, SHUT_WR);	
	//Half-close를 통해 출력스트림만 닫음
	//입력 스트림 데이터 수신
	read(clnt_sd, buf, BUF_SIZE);
	printf("Message from client: %s \n", buf);
	//파일 닫기
	fclose(fp);
	//클라이언트와 통신하는 소켓, 서버소켓 닫기
	close(clnt_sd); close(serv_sd);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}