#include <iostream>   
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/socket.h> 
#include <unistd.h>
#include <pthread.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <string.h> 
#include <fcntl.h> 
#include <event.h>
using namespace std; 
#define BUF_SIZE 1024
#define MAX_CONNECT 20000 
int recv_num = 0;

/**  
 * ���ӵ�server�ˣ�����ɹ�������fd�����ʧ�ܷ���-1  
 */  
int connectServer(char* ip, int port){
	int savefl;
	int fd = socket( AF_INET, SOCK_STREAM, 0 );
		cout<<"fd= "<<fd<<endl;
	if(-1 == fd){
		cout<<"Error, connectServer() quit"<<endl;
		return -1;
	}
	/*unsigned long ul = 1;
	int rm = ioctl(fd,FIONBIO,&ul);
	if(rm == -1)
	{
		close(fd);
		return 0;
	}*/

	struct sockaddr_in remote_addr; //�������������ַ�ṹ��
	memset(&remote_addr,0,sizeof(remote_addr)); //���ݳ�ʼ��--����
	remote_addr.sin_family=AF_INET; //����ΪIPͨ��
	remote_addr.sin_addr.s_addr=inet_addr(ip);//������IP��ַ
	remote_addr.sin_port=htons(port); //�������˿ں�
	int con_result = connect(fd, (struct sockaddr*) &remote_addr, sizeof(struct sockaddr));
	/*struct timeval timeout;
	fd_set r;
	FD_ZERO(&r);
	FD_SET(fd,&r);
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	int retval = select(fd+1,NULL,&r,NULL,&timeout);
	if(retval == -1)
	{
		perror("select");
		return 0;
	}
	else if(retval == 0)
	{
		fprintf(stderr,"timeout\n");
		return 0;
	}*/

	if(con_result < 0){ 
		cout<<"Connect Error!"<<endl;
		close(fd);
		return -1;
	}  
	cout<<"con_result="<<con_result<<endl;
	return fd;  
} 

void on_read(int sock, short event, void* arg)   
{ 
	char* buffer = new char[BUF_SIZE];
	memset(buffer, 0, sizeof(char)*BUF_SIZE); 
	//--����Ӧ����whileһֱѭ��������������libevent��ֻ�ڿ��Զ���ʱ��Ŵ���on_read(),�ʲ�����while��
	int size = read(sock, buffer, BUF_SIZE);
	if(0 == size){//˵��socket�ر� 
	//	cout<<"read size is 0 for socket:"<<sock<<endl;
		return;		
	}
	if(size > 0)
	{
		//cout<<"Received from server---"<<buffer<<endl;
		recv_num++;
		printf("Received from server:%s -recv_num:%d\n",buffer,recv_num);
	}
	delete[]buffer;
} 

void* init_read_event(void* arg){
    long long_sock = (long)arg;
	int sock = (int)long_sock; 
    //-----��ʼ��libevent�����ûص�����on_read()------------  
	struct event_base* base = event_base_new(); 
	struct event* read_ev = (struct event*)malloc(sizeof(struct event));//�������¼��󣬴�socket��ȡ������ 
	event_set(read_ev, sock, EV_READ|EV_PERSIST, on_read, NULL);  
	event_base_set(base, read_ev);  
    event_add(read_ev, NULL);  
	event_base_dispatch(base);  
	//--------------  
	event_del(read_ev);  
    free(read_ev); 
	event_base_free(base);  
}  
 /**  
 * ����һ�����̣߳������߳����ʼ��libevent���¼���������ã�������event_base_dispatch  
 */ 
 void init_read_event_thread(int sock){  
	pthread_t thread;  
	pthread_create(&thread,NULL,init_read_event,(void*)sock); 
	pthread_detach(thread);  
}  

int main() { 
	//cout << "main started" << endl; // prints Hello World!!! 
	//cout << "Please input server IP:"<<endl;  
	char ip[16];  
	//sprintf(ip,"183.63.118.58");
	sprintf(ip,"127.0.0.1");
	//cin >> ip;  
	//cout << "Please input port:"<<endl;  
	int port;  
	//cin >> port; 
	//cout << "ServerIP is "<<ip<<" ,port="<<port<<endl; 
		
		int socket_fd[MAX_CONNECT]; 
	for(int i=0;i<MAX_CONNECT;i++)
	{
//		usleep(1000); 
		socket_fd[i] = connectServer(ip, 9880);  
		cout << "socket_fd="<<socket_fd[i]<<endl;  
		init_read_event_thread(socket_fd[i]);  
	}
	//--------------------------  
	char buffer[BUF_SIZE];  
	bool isBreak = false;
	struct timeval val1,val2;
	int total = 0;
	gettimeofday(&val1,NULL);  
	while(!isBreak){  
//		cout << "Input your data to server(\'q\' or \"quit\" to exit)"<<endl;  
//		cin >> buffer; 
		/*	
		for(int i=0;i<MAX_CONNECT;i++)
		{
			gettimeofday(&val2,NULL); 
			total = (val2.tv_sec - val1.tv_sec) * 10000 + val2.tv_usec/100 - val1.tv_usec/100; 
			usleep(3000); 
			sprintf(buffer,"[%d] 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890... fd=%d\r\n",total,socket_fd[i]);
			if(strcmp("q", buffer)==0 || strcmp("quit", buffer)==0){  
			isBreak=true; 
			close(socket_fd[i]);
			break; 
			} 
		cout << "Your input is "<<buffer<<endl; 
		int write_num = write(socket_fd[i], buffer, strlen(buffer)+1); 
		cout << write_num <<" characters written"<<endl;
		} */
//		sleep(2); 
	} 
	cout<<"main finished"<<endl; 
	return 0; 
}
