#include "sdfs.h"
char *get_time();
char *strchomp(char *str);
char *update_record(char *record);
int create_active_socket(char *ip,int port);
void delete_client_entries(char *ip,char *port);
void execute_download(char *sip,int sport,char *ip,int port, char *filename);
void *serverThread(void *parmPtr);
void sig_h_int(int signo);
pthread_mutex_t lock;
struct serverParm {
           int connectionDesc;
		   int timeout;
       };
	  
int main(int argc, char *argv[]){
	/*																		*
	* This program have main process and POIX threads. Main process will 	*
	* accept clients and create a seperate thread for each connection. 		*
	* Clients are served by threads											*
	*																		*/
		
	/*       		SECTION 1 												*
	 *			   Main Process												*/
		
		
	/*				SECTION 1.1												*
	* The following validate necessary requiremnts when starting server 	*
	* and cread additional varibles which are needed. This also installs	*
	* the signal handler function for keyboard inerrupts (Ctrl+C)			*
	*																		*/
		
		signal(SIGINT,sig_h_int);
		printf(ANSI_HEADING "         Server Application        " ANSI_RESET "\n");
		if(argc!=4){
			printf("Usage: server <port> <max clients> <time out>\n");
			return 0;
		}
		
		printf("### Server application starting\n");
		
		int tcp_port=atoi(argv[1]);
		int que=atoi(argv[2]);
		int cli_timeout=atoi(argv[3]);
		int listenfd;
		struct sockaddr_in addr_in;
		pthread_t threadID;
		pthread_t t1, t2;
	
		
		
		
	/*				SECTION 1.2												*
	* The following code will initialize the listening socket basesd on 	*
	* passed arguments.														*
	*																		*/
	
		if((listenfd= socket(AF_INET,SOCK_STREAM,0))<0){
			printf("Unable to create listing socket\n");
			return -1;
		}
		
		bzero(&addr_in,sizeof(addr_in));
		addr_in.sin_family = AF_INET;
		addr_in.sin_addr.s_addr=htonl(INADDR_ANY);
		addr_in.sin_port=htons(tcp_port);
		
		if ((bind(listenfd, (SA *) &addr_in, sizeof(addr_in))) < 0){
			perror("Unable to bind");
			return -1;
		}
		if(listen(listenfd,que)<0){
			perror("Unable to listen");
			return -1;
		}
		
		int connfd;
		struct sockaddr_in cliaddr;
		socklen_t clilen;
	
		
		
		
	/*				SECTION 1.3												*
	* The following code will clear all existing entries of file list table	*
	* as the srver starts.													*
	*																		*/
	
	FILE *fp=fopen("db/filelist","w");
	fclose(fp);
	
	
	
	
	/*				SECTION 1.4												*
	* The following code will initialize the mutex.							*
	*																		*/
	
	if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n Mutex init failed\n");
        return 1;
    }
	
	
	
	
	/*				SECTION 1.5												*
	* The following code will accept each client and serve the client by	*
	* creating a new thread.												*
	*																		*/
	
	while(1){
		if((connfd=accept(listenfd,(SA *)&cliaddr, &clilen))<0){
			perror("Unable to accept connection");
			return -1;
		}
		struct serverParm *parmPtr;
		parmPtr = (struct serverParm *)malloc(sizeof(struct serverParm));
		parmPtr->connectionDesc =connfd;
		parmPtr->timeout=cli_timeout;
		int t1=pthread_create(&threadID, NULL, serverThread, (void *)parmPtr);
		pthread_detach(threadID);
		
		
		
	}
	
	close(listenfd);
	return 0;
	
}
char *get_time(){
	/*																		*
	* This Function will return current date and time of the system. The 	*
	* return type is char pointer.											*
	*																		*/
		
		time_t timer;
   static  char buffer[20];
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(buffer, 20, "%d/%m/%y %H:%M:%S", tm_info);
	
	return buffer;
}

char *strchomp(char *str){
	/*																		*
	* This Function will take char type pointer and it will remove any		*
	* trailing newline character and return the same string.				*
	*																		*/
		char *pos;
		if ((pos=strchr(str, '\n')) != NULL)
	    *pos = '\0';
		return str;
}

int create_active_socket(char *ip, int port){
	/*																		*
	* This Function will take an IPv4 address and a port number as 			*
	* argumnets and will return an active scoket decscriptor. 				*
	*																		*/
		int sockfd,n;
		struct sockaddr_in servaddr;
		
		
		if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
			perror("Unable to open socket");
			return -1;
		}
		
		bzero(&servaddr,sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(port);
		
		if(inet_pton(AF_INET,ip,&servaddr.sin_addr)<=0){
			perror("inet_pton error");
			return -1;
		}
		
		if(connect(sockfd,(SA *)&servaddr,sizeof(servaddr))<0){
			perror("Unable to connect");
			sockfd=-1;
		}
		
		return sockfd;
}
void execute_download(char *sip,int sport,char *ip,int port,char *filename){
	/*																		*
	* This Function will take source IP address, source port number, 		*
	*  IP address, destination port number and filename as 					*
	* arguments and execute the code for downloading file from client to 	*
	* client																*
	*																		*/
		
		
		
		printf("### Processing download command\n");
		char buffer[LINE];
		char buffer2[LINE];
		char data[LINE];
		char data2[LINE];
		
		bzero(buffer,LINE);
		bzero(buffer2,LINE);
		bzero(data,LINE);
		bzero(data2,LINE);
		
		printf("    Trying to connect \t\t");
		int source=create_active_socket(sip,sport);
		int destination=create_active_socket(ip,port);
		if(source<0 || destination<0){
			printf("[error]\n");
			return;
		}
		
		printf("[ok]\n");
		snprintf(data,LINE,"out %s",filename);
		snprintf(data2,LINE,"in %s",filename);
					
					
		write(source,data,strlen(data));
		read(source,buffer,LINE-1);
					
		if(!strcasecmp(strchomp(buffer),"OK")){
			
			write(destination,data2,strlen(data2));
			
			bzero(buffer,LINE);
			read(destination,buffer,LINE-1);
			
			if(!strcasecmp(strchomp(buffer),"OK")){
				bzero(buffer,LINE);
				printf("    Downloading \t\t");
				
				while((read(source,buffer,LINE-1))>0){
					write(destination,buffer,strlen(buffer));
					bzero(buffer,LINE);
				}
				
				printf("[ok]\n");
			}
			else{
				printf("### Download aborted\n");
			}
		}
		
		else{
				printf("### Download aborted\n");
		}
					close(destination);
					close(source);
					printf("### Download complete\n");
}

void *serverThread(void *parmPtr) {
	/*																		*
	* This Function will run in each thread as clients are connected. 		*
	* It takes  a pointer for serverParm Structure (SEE LINE 10) as an 		*
	* argument.																*
	*																		*/
		
		
	/*       		SECTION 2 												*
	*			   POIX Thread												*/
		
	/*       		SECTION 2.1												*
	* The following code will initilize a pointer to						*
	* access the serverParm structure passed by the pthread create 			*
	* function. Then it will access its connected socket descriptor 		*
	* and timout value.														*
	*																		*/
		
		#define PARMPTR ((struct serverParm *) parmPtr)
	    int connfd=PARMPTR->connectionDesc;
		int t_o=PARMPTR->timeout;
	    struct timeval tv;
		char *data_port;
	
		tv.tv_sec = t_o;  
		tv.tv_usec = 0;  
		int cli_set=0;
		char cli_port[20];
		char cli_ip[20];
		int t_id=(int)pthread_self();
		
		fd_set rset;
		
	
		printf("### Client connected %d\n",t_id);
	
	/*       		SECTION 2.2												*
	* The followijng code will run infinitly until the timeout or user 		*
	* asked to quit, whichever comes first. It will wait for user input 	*
	* from client application.												*
	*																		*/
		
		while(1){
			
				char buffer[LINE];
				bzero(buffer,LINE);
				printf("### Waiting for command\n");
				FD_ZERO(&rset);
				FD_SET(connfd,&rset);
				int i2=select(connfd+1,&rset,NULL,NULL,&tv);
				
				if(FD_ISSET(connfd,&rset)){
					if(read(connfd,buffer,LINE-1)<=0){
						break;
					}
				}
				
				
	/*       		SECTION 2.3												*
	* The followijng code will handle timeout event. If a client does not	*
	* sent any command for a specified time interval, server will send a 	*
	* goodbye message to terminate the client. If the client has uploaded 	*
	* any files, delete all records of them berfore terminating client.		*
	*																		*/
				
				if(i2<=0){
					printf("### Didn't receive any commands\n");
					
					printf("    Sending timeout \t\t");
					write(connfd,"timeout",7);
					close(connfd);
					printf("[ok]\n");
					if(cli_set==1){
					pthread_mutex_lock(&lock);
					delete_client_entries(cli_ip,cli_port);
					pthread_mutex_unlock(&lock);
				}
					printf("### Client terminated %d\n",t_id);
					free(PARMPTR);
					int a;
					pthread_exit( &a);		
				}
				
				
				
	/*       		SECTION 2.4												*
	* The followijng code will split the received user command into 		*
	* arguments to validate it. It will also get the client ip address 		*
	* from getpeername function. The ip address is used in later part of 	*
	* the code.																*
	*																		*/
				
				
				char *str=strchomp((char *)buffer);
				printf("### Server received \n    : %s\n",str);
				struct sockaddr_in peer;
				socklen_t peer_len;
				peer_len = sizeof(peer);
				if (getpeername(connfd,(SA *) &peer, &peer_len) == -1) {
					perror("getpeername() failed");
				}
		
		
		
				char *delemeter=" ";
				char *token=strtok(str,delemeter);
				char *ip=inet_ntoa(peer.sin_addr);
				char *command=token;
				char *arg1=strtok(NULL,delemeter);
				char *arg2=strtok(NULL,delemeter);
		
		
				
	/*       		SECTION 2.5												*
	* The followijng code will check weather user is asking for upload		*
	* file, If it is, open db/filelist file and add the new record of 	*
	* the file. db/filelist file contains all the uploaded files table.	*
	* This may also use the mutex varible to prevent file curruption if 	*
	* other threads tring to access the db/filelist						*
	*																		*/
				
		
				if(!strcasecmp(command,"upload") && arg1!=NULL){
					
					
					
					printf("### Processing upload command\n");
					char *filename=arg1;
					if(strlen(filename)>20){
						write(connfd,"Filename too large\n",19);
						continue;
					}
					pthread_mutex_lock(&lock);
					
					
					FILE *fp1=fopen("db/filelist","r");
					char line[LINE];
					int filefound=0;
					while(fgets(line,sizeof(line),fp1)){
						char *delemeter="\t";
						char *token=strtok(line,delemeter);
						if(!strcasecmp(token,filename)){
							filefound=1;
						}
					}
					fclose(fp1);
					pthread_mutex_unlock(&lock);
					if(filefound){
						write(connfd,"File exist with same name\n",26);
						continue;
					}
			
					pthread_mutex_lock(&lock);
					printf("    Opening filelist table\t");
					FILE *fp=fopen("db/filelist","a+");
					printf("[ok]\n");
					printf("    Adding new record \t\t");
					fprintf(fp,"%s\t%s\t%s\t%s\t%s\t%d\n",arg1,ip,arg2,strchomp(get_time()),"Never Downloaded",0);
					fclose(fp);
					printf("[ok]\n");
					pthread_mutex_unlock(&lock);
					printf("### Upload complete\n");
					char output[256];
					bzero(output,256);
					snprintf(output,256,"File %s Sucessfully added on %s\n",arg1,strchomp(get_time()));
					write(connfd,output,strlen(output));
					data_port=arg2;
					if(cli_set==0){
						const char *p1=arg2;
						const char *p2=ip;
						strcpy(cli_ip,p2);
						strcpy(cli_port,p1);
						cli_set=1;
					}
				}
				
				
				
	/*       		SECTION 2.6												*
	* The followijng code will check weather user is asking for delete		*
	* file, If it is, open db/filelist file and remove the  record in 		*
	* the file. db/filelist file contains all the uploaded files table. 	*
	* This may also use the mutex varible to prevent file curruption if 	*
	* other threads tring to access the db/filelist							*
	*																		*/
				
				
				else if(!strcasecmp(command,"delete") && arg1!=NULL){
					printf("### Processing delete command\n");
					pthread_mutex_lock(&lock);
					FILE *fp=fopen("db/filelist","r");
					FILE *fp1=fopen("db/temp","w");
					char line[LINE];
					char line2[LINE];
					int filefound=0;
					while(fgets(line,sizeof(line),fp)){
						char *token;
						char *delemeter="\t";
						strcpy(line2,line);
						token=strtok(line,delemeter);
						if(!strcasecmp(token,arg1)){
							filefound=1;
							continue;
						}
						else{
							fputs(line2,fp1);
						}
					}
					fclose(fp1);
					fclose(fp);
					
					FILE *fp2=fopen("db/filelist","w");
					FILE *fp3=fopen("db/temp","r");
					char ch;
					while((ch=fgetc(fp3))!=EOF){
						fputc(ch,fp2);
					}
					fclose(fp2);
					fclose(fp3);
					
					
					
					remove("db/temp");
					pthread_mutex_unlock(&lock);
					printf("### Delete complete\n");
					char output[256];
					bzero(output,256);
					if(filefound==1){
					snprintf(output,256,"File %s Sucessfully deleted on %s\n",arg1,strchomp(get_time()));}
					else{
						snprintf(output,256,"Unable to delete %s File not found\n",arg1);
					}
					write(connfd,output,strlen(output));
				}
				
				
	/*       		SECTION 2.7												*
	* The followijng code will check weather user is asking for get-table	*
	* command, If it is, open db/filelist file and send the records of 		*
	* all	available files. db/filelist file contains all the uploaded 	*
	* files table. This may also use the mutex varible to prevent file		* 
	* curruption if other threads tring to access the db/filelist			*
	*																		*/
				
				
				else if(!strcasecmp(command,"get-table")){
					pthread_mutex_lock(&lock);
					printf("    Sending table data \t\t");
					char *file_contents;
					long input_file_size;
					FILE *input_file = fopen("db/filelist", "rb");
					fseek(input_file, 0, SEEK_END);
					input_file_size = ftell(input_file);
					rewind(input_file);
					file_contents = malloc(input_file_size * (sizeof(char)));
					fread(file_contents, sizeof(char), input_file_size, input_file);
					fclose(input_file);
					if(strlen(file_contents)>0){
						write(connfd,file_contents,strlen(file_contents));
					}
					else{
						write(connfd,"empty\n",6);
					}
					printf("[ok]\n");
					pthread_mutex_unlock(&lock);
				}
		
				
				
				
				
	/*       		SECTION 2.8												*
	* The followijng code will check weather user is asking for download 	*
	* file, If it is, open db/filelist file and search of the specified 	*
	* file db/filelist file contains all the uploaded files table. When 	*
	* it finds the file it will invoke execute_download function to deliver *
	* the file to user from the uploded client. This may also use the mutex *
	* varible to prevent file curruption if other threads tring to access 	*
	* the db/filelist														*
	*																		*/
				
				
				else if(!strcasecmp(command,"download")){
					pthread_mutex_lock(&lock);
					char *filename=arg1;
					int port=atoi(arg2);
					printf("    Finding file \t\t");
					FILE *fp=fopen("db/filelist","r");
					FILE *temp=fopen("db/temp","w");
					char line[LINE];
					int filefound=0;
					while(fgets(line,sizeof(line),fp)){
						char line2[LINE];
						strcpy(line2,line);
						char *token;
						char *delemeter="\t";
						token=strtok(line,delemeter);
						
						if(!strcasecmp(token,filename)){
							filefound=1;
							printf("[ok]\n");
							token=strtok(NULL,delemeter);
							char *sip=token;
							token=strtok(NULL,delemeter);
							int sport=atoi(token);
							
							execute_download(sip,sport,ip,port,filename);
							char output[256];
							bzero(output,256);
							snprintf(output,256,"File %s Sucessfully downloaded on %s\n",arg1,strchomp(get_time()));
							write(connfd,output,strlen(output));
							char *line3=update_record(line2);
							fputs(line3,temp);
							
						}
						else{
							fputs(line2,temp);
							
						}
						
					}
					if(!filefound){
						printf("[not found]\n");
						write(connfd,"file not found\n",15);
					}
					fclose(fp);
					fclose(temp);
					
					FILE *fp2=fopen("db/filelist","w");
					FILE *fp3=fopen("db/temp","r");
					char ch;
					while((ch=fgetc(fp3))!=EOF){
						fputc(ch,fp2);
					}
					fclose(fp2);
					fclose(fp3);
					
					remove("db/temp");
					pthread_mutex_unlock(&lock);
					
				}
				
				
	/*       		SECTION 2.9												*
	* The followijng code will check weather user is asking to quit, 		*
	* If it is, break the loop to terminate the client connection 			*
	* gracefully.															*/		
				
				
				else if(!strcasecmp(command,"quit")){
					write(connfd,"ok/n",3);
					break;
				}
		
		
				else{
					printf("### Innvalid command received\n");
				}
			
	}
	
	
	
	/*       		SECTION 2.10											*
	* The followijng code will colse the connected socket of the client		*
	* and free up the serverParam structure. If the client uploaded any 	*
	* files,Then open db/filelist file and search of  files uploaded 		*
	* by this client. When it finds any files it will reomve those 			*
	* recoreds in the db/filelist. This may also use the mutex varible 		*
	* to prevent file  curruption if other threads tring to access 			*
	* the db/filelist. After cleaning up the print trminating message		*
	* on server and exit from the thread.									*
	*																		*/
	
	shutdown(connfd,SHUT_WR);
	close(connfd);
    free(PARMPTR);
	if(cli_set==1){
		pthread_mutex_lock(&lock);
		delete_client_entries(cli_ip,cli_port);
		pthread_mutex_unlock(&lock);
	}
	printf("### Client terminated %d\n",t_id);
	int a;
	pthread_exit( &a);
    return(0);                    
}

char *update_record(char *record){
	/*																		*
	* This Function will char type pointer for entry in filelist table		*
	* and update it's last downloaded data and increase the download count 	*
	* by 1 and return it. 													*
	*																		*/
		
		char *copy=record;
		char *delemeter="\t";
		char *filename=strtok(copy,delemeter);
		char *ip=strtok(NULL,delemeter);
		char *port=strtok(NULL,delemeter);
		char *added=strtok(NULL,delemeter);
		char *last=strtok(NULL,delemeter);
		char *count=strtok(NULL,delemeter);
		int c=atoi(count);
		c++;
		last=strchomp(get_time());
		
		static char updated_record[LINE];
		int l=snprintf(updated_record,LINE,"%s\t%s\t%s\t%s\t%s\t%d\n",filename,ip,port,added,last,c);
		
		return updated_record;
	
}
void delete_client_entries(char *ip, char *port){
	/*																		*
	* This Function will take client's ip address and port number and		*
	* delete all files uploaded by the client.								*
	*																		*/
	
			printf("    Deleting client entries \t");
			FILE *fp=fopen("db/filelist","r");
			FILE *fp1=fopen("db/temp","w");
			char line[LINE];
			char line2[LINE];
			while(fgets(line,sizeof(line),fp)){
				char *delemeter="\t";
				strcpy(line2,line);
				char *filename=strtok(line,delemeter);
				char *fip=strtok(NULL,delemeter);
				char *fport=strtok(NULL,delemeter);
				if(!strcasecmp(fip,ip) && !strcasecmp(fport,port)){
					continue;
				}
				else{
					fputs(line2,fp1);
				}
			}
			fclose(fp1);
			fclose(fp);
			
			FILE *fp2=fopen("db/filelist","w");
			FILE *fp3=fopen("db/temp","r");
			char ch;
			while((ch=fgetc(fp3))!=EOF){
				fputc(ch,fp2);
			}
			fclose(fp2);
			fclose(fp3);
			
			printf("[ok]\n");
			remove("db/temp");
			
}
void sig_h_int(int signo){
	/*       																*
	* This function fill take signal as an agrument(to be installed on 		*
	* signal function), And check if it is an SIGINT (commonly known as 	*
	* keyboard interrupt Ctrl+C). If it is terminate server application.	*
	*																		*/
		
	if(signo==SIGINT){
		printf("\n");
		printf("### Terminating server application\n\n");
		exit(0);
	}
}
