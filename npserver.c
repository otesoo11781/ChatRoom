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
#include<sys/time.h>

#define maxlen 1500
#define clinum 15
struct client_info{
  char name[100]  ;
  int fd ;
  struct sockaddr_in net_info;
};

int main(int argc , char **argv){
 int listenfd , connfd ,n , i ,j ,maxi , maxfd , nready ;
 struct sockaddr_in servaddr,cliaddr ;
 socklen_t clilen ;
 char buf[maxlen] , msg[maxlen] ;
 fd_set allset , rset ;
 struct client_info client[clinum] ;
 //listen socket
 listenfd = socket(AF_INET , SOCK_STREAM , 0) ;

 bzero(&servaddr , sizeof(servaddr)) ;
 servaddr.sin_family = AF_INET ;
 servaddr.sin_addr.s_addr = htonl(INADDR_ANY) ;
 servaddr.sin_port = htons(atoi(argv[1]));
 
 bind(listenfd , (struct sockaddr *)&servaddr , sizeof(servaddr) ) ;
 listen(listenfd , 5) ;
 //intitial 
 maxfd = listenfd ;
 maxi = -1 ;
 FD_ZERO(&allset) ;
 FD_SET(listenfd , &allset) ; 
 for(i = 0 ; i < clinum ; i++){
   strcpy(client[i].name , "anonymous") ;
   client[i].fd = -1 ;
   bzero(&(client[i].net_info) , sizeof( (client[i]).net_info ));
   }

 for(;;){
    rset = allset ;
    nready = select(maxfd + 1 , &rset ,NULL , NULL , NULL) ;
   //new connetion 
   if(FD_ISSET(listenfd , &rset)){
      bzero(msg , sizeof(msg)) ;
      clilen = sizeof(cliaddr) ;
      connfd = accept(listenfd , (struct sockaddr* )&cliaddr , &clilen) ;
   //update client information 
    for(i = 0 ; i < clinum ; i++){
	if(client[i].fd == -1){
	   client[i].fd = connfd ;
	   client[i].net_info = cliaddr ;
	   break;
		}	
	}
    if(i == clinum){ fprintf(stderr,"too many clients\n") ;}
    else{
	 //send hello message begin
	 sprintf(msg ,"[Server] Hello, anonymous! From: %s/%d\n" 
			,inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port) ) ;
	 write(connfd , msg , strlen(msg));
	 sprintf(msg , "Someone is coming!\n") ;
	 for(j = 0 ; j < clinum ;j++){
	    if(client[j].fd >= 0 && client[j].fd != connfd )write(client[j].fd , msg ,strlen(msg)) ;
		}
	 //send hello massage end
    	 FD_SET(connfd , &allset) ;
   	 if(connfd > maxfd) maxfd = connfd ;
   	 if(i > maxi) maxi = i ;
    }
    if(--nready <= 0) continue ;
    }
   //process client command
   for(i = 0 ; i <= maxi; i++){
      if(client[i].fd < 0) continue ;
      if(FD_ISSET(client[i].fd , &rset)){
	bzero(buf , sizeof(buf)) ;
	bzero(msg , sizeof(msg)) ;
	if( (n = read(client[i].fd , buf , sizeof(buf))) == 0  ){
		close(client[i].fd) ;
		FD_CLR(client[i].fd , &allset) ;
		client[i].fd = -1 ;
		//offline message
		sprintf(msg , "[Server] %s is offline.\n",client[i].name ) ;
		for(j = 0 ; j <= maxi ; j++)
			if(client[j].fd >= 0) write(client[j].fd , msg , strlen(msg)) ;
		strcpy(client[i].name , "anonymous") ;
		}
	else{
	   //cmd process
	   strtok(buf , "\n\r") ;
	   char *ptr = strtok(buf , " ") ;
	   if(strcmp(ptr , "who") == 0){
		//who message
		for(j = 0 ; j <= maxi ; j++){
			if(client[j].fd == -1) continue;
			
			if(j == i) 
			   {sprintf(msg , "[Server] %s %s/%d ->me\n",client[j].name , 
				inet_ntoa(client[j].net_info.sin_addr ) ,
				 ntohs(client[j].net_info.sin_port) ) ;
			          write(client[i].fd , msg , strlen(msg)) ;
			}else
			    {sprintf(msg , "[Server] %s %s/%d\n",client[j].name , 
				inet_ntoa(client[j].net_info.sin_addr ) ,
				 ntohs(client[j].net_info.sin_port) ) ;
			     write(client[i].fd , msg , strlen(msg)) ;}
			}
		}
	   else if(strcmp(ptr , "name") == 0){
		char *newname = strtok(NULL , "\n") ;
		int flag = 0 ;//test for name duplicated
	        for(j = 0 ; j <= maxi ; j++){
		     if(client[j].fd == -1) continue ;
		     if(j == i) continue ;
		     if(strcmp(client[j].name ,newname) == 0){
			   flag = 1 ;
			    break ;
			  }
	 	   }
		if(flag){
		    sprintf(msg , "[Server] ERROR: %s has been used by ohters.\n" ,newname);
	            write(client[i].fd , msg , strlen(msg)) ;
		    }
		else if(strcmp(newname , "anonymous") == 0){
			sprintf(msg , "[Server] ERROR: Username cannot be anonymous.\n") ;
			write(client[i].fd , msg , strlen(msg)) ;
		  }
		else {
			int len_flag = 0 , english_flag = 0 , k ;
			if(strlen(newname) > 12 || strlen(newname) < 2 ) len_flag = 1 ;
		        for(k = 0 ; k < strlen(newname) ;k++){
				if( (*(newname + k) < 'A') || (*(newname +k) > 'z') 
				|| ( (*(newname +k) < 'a') &&( *(newname +k) > 'Z') ) ) {
				english_flag = 1 ;
				break ;
				}
			}
			if(len_flag || english_flag){
			sprintf(msg , 
			"[Server] ERROR: Username can only consists of 2~12 English letters.\n");
			write(client[i].fd , msg , strlen(msg)) ;
		   	}
		        else{
				for(j = 0 ; j <= maxi ; j++){
			   	if(client[j].fd == -1) continue ;
			   	if(j == i){
			     	   sprintf(msg ,"[Server] Your're now known as %s.\n" ,newname);
			    	   write(client[j].fd ,msg , strlen(msg) ) ;
			    	 }
			  	 else{
			    	   sprintf(msg , "[Server] %s is now known as %s.\n"
					 ,client[i].name ,newname) ;
			    	   write(client[j].fd , msg , strlen(msg));
					}
		  	          }
			        strcpy(client[i].name , newname);
		       }
		    }
		}
	    else if(strcmp(ptr , "tell" ) == 0){
		//tell cmd
		char *usrname = strtok(NULL , " ") ;
		char *message = strtok(NULL , "\n") ;
	
		if(strcmp(client[i].name,"anonymous") == 0 ){
			sprintf(msg , "[Server] ERROR: You are anonymous.\n") ;
			write(client[i].fd , msg , strlen(msg)) ;
			}
		else if(strcmp(usrname , "anonymous") == 0 ){
			sprintf(msg,"[Server] ERROR: The client to which you sent is anonymous.\n");
			write(client[i].fd , msg, strlen(msg)) ;
			}
		else{
		    for(j = 0 ; j <= maxi ;j++){
			if(client[j].fd == -1) continue ;
			if(strcmp(client[j].name , usrname ) == 0) break;
			}
		    if(j > maxi){
			sprintf(msg , "[Server] ERROR: The receiver doesn't exist.\n") ;
			write(client[i].fd , msg , strlen(msg)) ;
			}
		    else{
			sprintf(msg , "[Server] SUCEESS: Your message has been sent.\n") ;
			write(client[i].fd , msg, strlen(msg)) ;
			sprintf(msg ,"[Server] %s tell you %s\n" ,client[i].name , message  );
			write(client[j].fd , msg, strlen(msg)) ;
			}	
		    }
		}
	    else if(strcmp(ptr , "yell") == 0){
		//yell cmd
		char *message = strtok(NULL , "\n") ;
		sprintf(msg , "[Server] %s yell %s\n" ,client[i].name , message) ;
		for(j = 0 ; j <= maxi ;j++){
			if(client[j].fd == -1) continue ;
			write(client[j].fd , msg , strlen(msg)) ;
			}
		}
	    else{
		//error cmd
		sprintf(msg , "[Server] ERROR: Error command.\n") ;
		write(client[i].fd , msg , strlen(msg)) ;
		}
	    }//end for the  client cmd
	if(--nready <= 0 )break ;	
	}//end for ISSET
     }//end for loop
  
  }//end for server forerver loop		

  
}
