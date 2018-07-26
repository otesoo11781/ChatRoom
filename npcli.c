#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<strings.h>
#include<string.h>
#include<sys/select.h>

int main(int argc , char **argv){
  struct sockaddr_in servaddr ;
  int sockfd  , stdineof = 0;
  char buf[1500] ,recv[1500] ;
  int i , maxfdp1 ;
  fd_set rset ;

  sockfd = socket(AF_INET , SOCK_STREAM ,0) ;

  bzero(&servaddr , sizeof(servaddr)) ;
  servaddr.sin_family = AF_INET ;
  servaddr.sin_port = htons(atoi(argv[2]) ) ;
  inet_pton(AF_INET , argv[1] , &servaddr.sin_addr) ;

  connect(sockfd ,(struct sockaddr *) &servaddr , sizeof(servaddr)) ;
 
  FD_ZERO(&rset) ;
  
  for(;;){
    if(stdineof == 0)FD_SET(fileno(stdin) , &rset) ;
    FD_SET(sockfd , &rset) ;
    maxfdp1 = sockfd + 1 ;
    select(maxfdp1 ,&rset , NULL , NULL , NULL) ;
    if(FD_ISSET(sockfd , &rset)){
    	bzero(recv  , sizeof(recv)) ;
	if(read(sockfd , recv , sizeof(recv) ) == 0 ){
		if(stdineof) exit(0) ;
		fprintf(stderr , "cli: server terminate prematurely\n") ;
		exit(-1) ;
		}
	else fputs(recv , stdout) ;
     }
    if(FD_ISSET(fileno(stdin) , &rset)){
	bzero(buf , sizeof(buf)) ;
	if(fgets(buf , sizeof(buf) , stdin) == NULL ){
		stdineof = 1 ;
		shutdown(sockfd , SHUT_WR) ;
		FD_CLR(fileno(stdin) , &rset) ;
		continue ;
		}
	if(strcmp(buf , "exit\n") == 0 ) exit(0) ;
	write(sockfd , buf , strlen(buf)) ;
	}
    }
 
  
}
