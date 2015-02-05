#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <iostream>
void error(const char *msg)
{
    perror(msg);
    exit(0);
}
class InputCommand
{
public:
	int code;
	unsigned char data[4];
	InputCommand(int code = -1 ):code(code)
	{
		for(int i=0; i<4; i++) data[i]=0;
	}
};
int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    sockaddr_in serv_addr;
    hostent *server;

    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
//     printf("Please enter the message: ");
    InputCommand cmd(2);
    bzero(buffer,256);
//     fgets(buffer,255,stdin);
    std::cout << "sizeof command is: " << sizeof(cmd)<<std::endl;
    n = write(sockfd,&cmd,sizeof(cmd));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);
    close(sockfd);
    return 0;
}
