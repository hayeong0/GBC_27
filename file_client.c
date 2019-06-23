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
	int sd;
	FILE *fp;
	
	char buf[BUF_SIZE];
	int read_cnt;
	struct sockaddr_in serv_adr;
	if(argc!=3) {
		printf("Usage: %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
    /* 서버 ---> 클라이언트 파일 저장 */
    //파일 생성 및 열기 (binary mode -> wb)
	fp=fopen("received file.dat", "wb"); 
    // 클라이언트 소켓
	sd=socket(PF_INET, SOCK_STREAM, 0);   

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));

    //서버에게 연결 요청!
	connect(sd, (struct sockaddr*)&serv_adr, sizeof(serv_adr));
	
    //파일 쓰기 (서버로부터의 데이터 저장 EOF 받을때까지)
	while((read_cnt=read(sd, buf, BUF_SIZE ))!=0)
		fwrite((void*)buf, 1, read_cnt, fp);
	
	puts("Successfully received file!");
    //서버에게 메시지 전송
	write(sd, "Success", 8);
	fclose(fp); //파일 닫기
	close(sd); //소켓 종료
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}