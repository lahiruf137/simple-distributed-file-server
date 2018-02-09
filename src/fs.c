#include "sdfs.h"
char *strchomp(char *str);
int create_active_socket(char *arg,int tcp_port);
int create_listen_socket();
//int delete(int control_line,char *filename);
int hostname_to_ip(char * hostname , char* ip);
int upload(int control_line,char *filename,int data_line);
//void download(int control_line,char *filename,int data_line);
void senddata(int contrl_line,char buffer[]);
void sig_h_int(int signo);
int cli_pid=0;
int main(int argc,char *argv[]){
	/*																		*
	* This program have 2 processes. One for handling data line and another *
	* for handling control line. Data line is handled by a passive socket	*
	* and the control line handled by an active socket.						*
	*																		*/
	printf(ANSI_HEADING "                  Client Application                 " ANSI_RESET "\n");
		signal(SIGINT,sig_h_int);
	int control_line, data_line;
	char ipa[128];
	if(argc!=3){
		printf("Usage : ./fs <Server ip or hostname> <server port>\n");
		return 0;
	}
	
	
	
	printf("### Starting application \t\t\t [ok] \n");
	
	char *hostname=argv[1];
	int tcp_port=atoi(argv[2]);
	if(!hostname_to_ip(hostname,ipa)){
		control_line=create_active_socket(ipa,tcp_port);
	}
	else{
		control_line=create_active_socket(hostname,tcp_port);
	}
	
	if(control_line<0){
		printf("Unable to connect to %s\nProgram will exit now\n",argv[1]);
		return 0;
	}
	
	data_line=create_listen_socket();
	
	/*				SECTION 1												*
	* The following chlid process will handle any inomcing requets for		*
	* data line.															*
	*																		*/
		
	int pid=fork();
	cli_pid=pid;
	if(pid==0){
		int connfd;
		struct sockaddr_in cliaddr;
		socklen_t clilen;
		while(1){
			
			if((connfd=accept(data_line,(SA *)&cliaddr, &clilen))<0){
				perror("Unable to accept connection\n");
				return -1;
			}
			
			char buffer[LINE];
			bzero(buffer,LINE);
			int n=read(connfd,buffer,LINE-1);
			char *data=strchomp(buffer);
			
			char *delemeter=" ";
			char *token=strtok(buffer,delemeter);
			char *command=token;
			token=strtok(NULL,delemeter);
			char *filename=token;
			
	/*				SECTION 1.1												*
	* The following code will handle any download reqests made by itself 	*
	* from main process. This will write the new file contents to disk.		*
	*																		*/
			
			if(!strcasecmp(command,"in")){
				char data[LINE];
				int n=snprintf(data,LINE,"downloads/%s",filename);
				FILE *fp=fopen(data,"w");
				write(connfd,"OK",2);
				while((read(connfd,buffer,LINE-1))>0){
					fputs(buffer,fp);
				}
				fclose(fp);
				
			}
	/*				SECTION 1.2												*
	* The following code will respond to any upload reqests made by server 	*
	* This will read the requested file from disk and write to socket		*
	*																		*/
		
			if(!strcasecmp(command,"out")){
				char filepath[256];
				snprintf(filepath,sizeof(filepath),"u_files/%s",filename);
				if( access( filepath, F_OK ) != -1 ) {
	   				 // file exists
					write(connfd,"OK",2);
					FILE *fp=fopen(filepath,"r");
					char line[LINE];
					while(fgets(line,sizeof(line),fp)){
						write(connfd,line,strlen(line));
					}
					fclose(fp);
					close(connfd);
			
				} else {
  				  // file doesn't exist
					write(connfd,"NOK",3);
					continue;
				}
				
			}
			
		}
	}
	
	
	
	/*				SECTION 2												*
	* The following main process will handle any requets for				*
	* control line based on user input.										*
	*																		*/
		
	fd_set wset;
	fd_set rset;
	
	struct timeval timeout;
	timeout.tv_sec = SELECT_TIMEOUT;
	timeout.tv_usec = 0;
		
		
		printf("### Connected to %s at port %s\t\t [ok]\n",argv[1],argv[2]);
		
	while(1){
		
		
		
	/*				SECTION 2.1												*
	* The following initilaze necessary varibles and print the prompt. And	*
	* get an input from user.												*
	*																		*/
		
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		FD_SET(control_line,&rset);
		FD_SET(control_line,&wset);
		
		
		
		
	/*				SECTION 2.2												*
	* The following code will check for writeability of the socket. Break 	*
	* the loop and go to SECTION X if scoket is not writable.				*
	*																		*/
		
		int i=select(control_line+1,&rset,&wset,NULL,&timeout);
	
	if(FD_ISSET(control_line,&rset)){
		char data[LINE];
		bzero(data,LINE);
		read(control_line,data,LINE-1);
		
		if(!strcasecmp(data,"timeout")){
			printf("Server sent goodbye !\n");
			kill(cli_pid,SIGKILL);
			exit(0);
		}
	}
		
		if(i<=0 || FD_ISSET(control_line,&wset)<=0){
			printf("Disconnected from Server\n");
			printf("Reason: Socket Error");
			break;
		}
		
	/*				SECTION 2.3												*
	* The following code will use strtok(string tokenizer) and 				*
	* strchomp(SEE LINE X) functions on user input to check the user input.	*
	* If the user input is blank it will go back to the begining of the 	*
	* loop to get another input.											*
	*																		*/
		
		printf("FS>");
		printf(" ");
		char buffer[LINE];
		bzero(buffer,LINE);
		fgets(buffer,LINE-1,stdin);
		
		char *data=strchomp(buffer);
		char *delemeter=" ";
		char *token=strtok(data,delemeter);
		
		if(token==NULL){
			continue;
		}
		
		
	/*				SECTION 2.4a												*
	* The following code will check weather the user input command is an 		*
	* upload command. If it is an upload command and arguments are correct 		*
	* invoke upload function (SEE LINE Y). Otherwise print the usage of 		*
	* upload command. 															*
	*																			*/
		
		if(!strcasecmp(token,"upload")){
			token=strtok(NULL,delemeter);
			char *filename=token;
			if(filename!=NULL){
				char filepath[256];
				snprintf(filepath,sizeof(filepath),"u_files/%s",filename);
				if( access( filepath, F_OK ) != -1 ) {
					int n=upload(control_line,filename,data_line);
				}
				else{
					printf("No such file in u_files directory\n\n");
				}
			}
			else{
				printf("Usage: upload <filename>");
				continue;
			}
		}
		
		

		
		
	/*				SECTION 2.4e												*
	* The following code will check weather the user input command is quit. 	*
	* If user input is quit, then kill the child process (SEE SECTION 1), 		*
	* Colose data and control_line sockets and exit.							*
	*																			*/
		
		else if(!strcasecmp(token,"quit")){
			struct sockaddr_in sin;
			socklen_t len = sizeof(sin);
			if (getsockname(data_line, (SA *)&sin, &len) == -1){
   			 	perror("getsockname");
			}
			int port= ntohs(sin.sin_port);
			
			char buff[LINE];
			snprintf(buff,LINE,"quit %d",port);
			write(control_line,buff,20);
			shutdown(control_line,SHUT_WR);
			close(control_line);
			close(data_line);
			break;
			
		}
		
		
	/*				SECTION 2.4f												*
	* The following code will execute if the user input doesnt match any		*
	* valid commands. This prints details of all available commands and useages.*
	*																			*/
		
		else {
			printf(ANSI_WORNING "Invalid command" ANSI_RESET "\n");
			printf("List of available commands  \n");
			printf("+----------------------------------------------------+\n");
			printf("| Command Usage       | Description                  |\n");
			printf("|---------------------+------------------------------|\n");
			printf("| quit                | Quit client application      |\n");
			printf("| upload   <filename> | Upload a file in u_files     |\n");
			printf("+----------------------------------------------------+\n");
		}
		
	}
	kill(pid,SIGKILL);
	return 0;
}


int upload(int control_line,char *filename,int data_line){
	/*																		*
	* This Function will take connected socket descrptor,an input filename	*
	* and a passive socket descriptor. It will generate the upload command	*
	* and send through the `senddata` method.								*
	*																		*/
			struct sockaddr_in sin;
			socklen_t len = sizeof(sin);
			if (getsockname(data_line, (SA *)&sin, &len) == -1){
   			 	perror("getsockname");
				return -1;	
			}
			int port= ntohs(sin.sin_port);
			char data[LINE];
			int n=snprintf(data,LINE,"upload %s %d",filename,port);
			senddata(control_line,data);
			return 0;
}

void senddata(int sockfd,char buffer[]){
	/*																		*
	* This Function will take connected socket descrptor, a character array *
	* and send data to the provided socket with write system call.			*
	* After writing data it also invoke the read system call to get 		*
	* server's response. The response is printed on screen.					*
	*																		*/	
		struct timeval stop, start;
		gettimeofday(&start, NULL);
		int n=write(sockfd,buffer,255);
		if(n<0){
			perror("Error writing to socket");
			exit(0);
		}
		bzero(buffer,LINE);
		
		char data[LINE];
		bzero(data,LINE);
		if(read(sockfd,data,LINE-1)<=0){
			printf("Disconnected from server\n");
			printf("Program will exit now\n");
			kill(cli_pid,SIGKILL);
			exit(0);
		}
		
		if(!strcasecmp(data,"timeout")){
			printf("Server sent goodbye !\n");
			kill(cli_pid,SIGKILL);
			exit(0);
		}
		else {
			gettimeofday(&stop, NULL);
			printf("%s\n",data);
			double usec=(double)stop.tv_usec - start.tv_usec;
			double sec=(double)stop.tv_sec-start.tv_sec;
			double ms=(sec*1000.0)+(usec/1000);
			printf("Completed in %.3fms\n",ms );
		}
}

char *strchomp(char *str){
	/*																		*
	* This Function will take char type pointer and it will remove any		*
	* trailing newline character and return the same string					*
	*																		*/
		char *pos;
		if ((pos=strchr(str, '\n')) != NULL)
	    *pos = '\0';
		return str;
}

int create_listen_socket(){
	/*																		*
	* This Function will create a passive scoket and the socked descriptor  *
	* is returned.															*
	*																		*/
			int data_line;
			struct sockaddr_in addr_in;
			if((data_line= socket(AF_INET,SOCK_STREAM,0))<0){
				printf("Unable to create listing socket\n");
				return -1;
			}
			bzero(&addr_in,sizeof(addr_in));
			addr_in.sin_family = AF_INET;
			addr_in.sin_addr.s_addr=htonl(INADDR_ANY);
			
			if ((bind(data_line, (SA *) &addr_in, sizeof(addr_in))) < 0){
				printf("Unable to bind\n");
				return -1;
			}
			
			if(listen(data_line,LISTENQ)<0){
				printf("Unable to listen\n");
				return -1;
			}
			
			return data_line;
}


int create_active_socket(char *ip,int tcp_port){
	/*																		*
	* This Function will take an IPv4 address as an argumnet and will 		*
	* return an active scoket decscriptor. 									*
	*																		*/
		int sockfd;
		struct sockaddr_in servaddr;
		//char *ip="127.0.0.1";
		
		if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){ 
			perror("Unable to open socket");
			return -1;
		}
		
		bzero(&servaddr,sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(tcp_port);
		
		if(inet_pton(AF_INET,ip,&servaddr.sin_addr)<=0){
			perror("inet_pton error");
			return -1;
		}
		
		if(connect(sockfd,(SA *)&servaddr,sizeof(servaddr))<0){
			perror("Unable to connect");
			return -1;
		}
		return sockfd;
}

int hostname_to_ip(char * hostname , char* ip){
	/*																		*
	* This Function will take two char type pointers each hostname(dns)		*
	* and IPv4 address. When a hostanme resolved the ip pointer is passed	*
	* by refrence. It will return 0 on sucess and 1 on error.				*
	*																		*/
	    struct hostent *he;
	    struct in_addr **addr_list;
	    int i;
	         
	    if((he = gethostbyname(hostname)) == NULL){
	        perror("gethostbyname");
	        return 1;
	    }
	    addr_list = (struct in_addr **) he->h_addr_list;
	    for(i =0;addr_list[i] != NULL;i++) {
	        strcpy(ip , inet_ntoa(*addr_list[i]) );
	        return 0;
	    }
	    return 1;
}
void sig_h_int(int signo){
	/*       																*
	* This function fill take signal as an agrument(to be installed on 		*
	* signal function), And check if it is an SIGINT (commonly known as 	*
	* keyboard interrupt Ctrl+C). If it is terminate client application.	*
	*																		*/
	if(signo==SIGINT){
		printf("\n");
		kill(cli_pid,SIGKILL);
		exit(0);
	}
}

