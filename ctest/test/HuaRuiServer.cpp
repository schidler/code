/**************************************************************
 * Copyright (C) 2016-2016 All rights reserved.
 * @Version: 1.0
 * @Created: 2016-04-29 15:59
 * @Author: menqi - 1083734876@qq.com
 * @Description: 
 *
 * @History: 
 **************************************************************/
#include <iostream>
#include <sys/select.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <string.h>
#include <event.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <map>
#include <list>
#include <signal.h>
using namespace std;



pthread_mutex_t delepoll_lock;
pthread_mutex_t delepoll_lock_app;





/* 定义 */
#define LISTEN_HOST_PORT 9880
#define LISTEN_APP_PORT  9881
#define SERVER_IP "192.168.0.73" 
#define BUF_SIZE 1024
#define MAX_EPOLL_NUM 100000

#define  HOST_TYPE  0
#define  APP_TYPE   1
int  listen_app_fd;
int  listen_host_fd;

struct epoll_event epoll_event;
typedef map<int,string> SocketInfoMap;
SocketInfoMap hostinfomap; /* host 信息 map */
SocketInfoMap appinfomap;
typedef list<int> SockfdList;
typedef SockfdList::iterator SockfdIter;
SockfdList HostList;
SockfdList AppList;
int epoll_fd[2];//epoll创建的句柄   0  host  1 app   


/*  test  */
typedef map<int,int> SocketTestMap;
typedef SocketTestMap::iterator SocketTestIter;
SocketTestMap  hosttestmap;
SocketTestMap  apptestmap;
int host_count_id=0;
int app_count_id=0;
int total_count=0;
#include <time.h>
static unsigned  get_current_time(void){
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC,&now);
	unsigned mseconds = now.tv_sec * 1000+ now.tv_nsec/1000000;
	printf("current time: %d\n",mseconds);
	return mseconds;
}







/*线程池
 */
/*
 *线程池里所有运行和等待的任务都是一个CThread_worker
 *由于所有任务都在链表里，所以是一个链表结构
 */
typedef struct worker 
{
	/*回调函数，任务运行时会调用此函数，注意也可声明成其它形式*/
	void *(*process_in_new_thread_when_accepted)(void *arg);
	void *arg;/*回调函数的参数*/
	struct worker *next;
}CThread_worker;

/*线程池结构*/ 
typedef struct
{
	pthread_mutex_t queue_lock;
	pthread_cond_t queue_ready;

	/*链表结构，线程池中所有等待任务*/
	CThread_worker *queue_head;
	/*是否销毁线程池*/
	int shutdown;
	pthread_t *threadid;
	/*线程池中允许的活动线程数目*/
	int max_thread_num;
	/*当前等待队列的任务数目*/
	int cur_queue_size; 
}CThread_pool;

int pool_add_worker (void *(*process) (void *arg), void *arg);
void *thread_routine (void *arg); 
//share resource  
static CThread_pool *pool = NULL;
void pool_init (int max_thread_num)  
{ 
	pool = (CThread_pool *)malloc(sizeof(CThread_pool));

	pthread_mutex_init(&(pool->queue_lock),NULL);
	pthread_cond_init(&(pool->queue_ready),NULL);

	pool->queue_head=NULL;

	pool->max_thread_num = max_thread_num;
	pool->cur_queue_size = 0;

	pool->shutdown = 0;

	pool->threadid = (pthread_t *)malloc(max_thread_num * sizeof (pthread_t));
	pthread_attr_t attr;
	if(pthread_attr_init(&attr) == 0)
		pthread_attr_setstacksize(&attr,1 * 1024 * 1024);
	int i = 0;
	for (i = 0; i < max_thread_num; i++) 
	{ 
		pthread_create(&(pool->threadid[i]),NULL,thread_routine,NULL);
	}
} 

/*向线程池中加入任务*/ 
int pool_add_worker(void *(*process) (void *arg), void *arg) 
{
	/*构造一个新任务*/
	CThread_worker *newworker = (CThread_worker *)malloc(sizeof(CThread_worker));
	newworker->process_in_new_thread_when_accepted = process; 
	newworker->arg = arg; 
	newworker->next = NULL;/*别忘置空*/

	pthread_mutex_lock(&(pool->queue_lock));
	/*将任务加入到等待队列中*/
	CThread_worker *member = pool->queue_head;
	if(member != NULL)
	{
		while(member->next != NULL)
			member = member->next;

		member->next = newworker;
	}
	else
	{
		pool->queue_head = newworker;
	}

	assert(pool->queue_head != NULL);
	pool->cur_queue_size++;
	pthread_mutex_unlock(&(pool->queue_lock));

	/*好了，等待队列中有任务了，唤醒一个等待线程；
	  注意如果所有线程都在忙碌，这句没有任何作用*/
	pthread_cond_signal(&(pool->queue_ready));
	return 0;
}

/*销毁线程池，等待队列中的任务不会再被执行，但是正在运行的线程会一直? 
  把任务运行完后再退出*/
int pool_destroy()
{
	if(pool->shutdown)
		return -1;/*防止两次调用*/
	pool->shutdown= 1;

	/*唤醒所有等待线程，线程池要销毁了*/
	pthread_cond_broadcast(&(pool->queue_ready));

	/*阻塞等待线程退出，否则就成僵尸了*/
	int i;
	for(i = 0;i < pool->max_thread_num;i++)
		pthread_join(pool->threadid[i],NULL);
	free(pool->threadid);

	/*销毁等待队列*/
	CThread_worker *head = NULL;
	while(pool->queue_head != NULL)
	{
		head = pool->queue_head;
		pool->queue_head = pool->queue_head->next;
		free(head);
	}
	/*条件变量和互斥量也别忘了销毁*/
	pthread_mutex_destroy(&(pool->queue_lock));
	pthread_cond_destroy(&(pool->queue_ready));

	free(pool);
	/*销毁后指针置空是个好习惯*/
	pool=NULL;
	return 0; 
}

bool DelEpoll(int sockfd,int type)
{
	int del_epoll_fd;
	if(type==HOST_TYPE)
		del_epoll_fd=epoll_fd[0];
	else
		del_epoll_fd=epoll_fd[1];
	bool bret = false;
	struct epoll_event event_del;
	if(sockfd > 0)
	{
		event_del.data.fd = sockfd;
		event_del.events = 0;
		if(epoll_ctl(del_epoll_fd,EPOLL_CTL_DEL,event_del.data.fd,&event_del) == 0)
		{
			bret = true;
		}
		else
		{
			printf("Delepoll error,sockfd=%d\n",sockfd);
		}
	}
	else
	{
		bret = true;
	}
	return bret;
}

void *thread_routine(void *arg)
{
	//printf("starting thread 0x%lu\n",pthread_self());
	while(1)
	{
		pthread_mutex_lock(&(pool->queue_lock));
		/*如果等待队列为0并且不销毁线程池，则处于阻塞状态;
		  注意pthread_cond_wait是一个原子操作，等待前会解锁，唤醒后会加锁*/
		while(pool->cur_queue_size == 0 && !pool->shutdown)
		{
			//printf("thread 0x%lu is waiting\n",pthread_self());
			pthread_cond_wait(&(pool->queue_ready),&(pool->queue_lock));
		}

		/*线程池要销毁了*/
		if(pool->shutdown)
		{
			/*遇到break,continue,return等跳转语句，千万不要忘记先解锁*/
			pthread_mutex_unlock(&(pool->queue_lock));
			printf("thread 0x%lu will exit\n",pthread_self());
			pthread_exit(NULL);
		}

		//printf("thread 0x%lu is starting to work\n",pthread_self());
		/*assert是调试的好帮手*/
		assert(pool->cur_queue_size != 0);
		assert(pool->queue_head != NULL);

		/*等待队列长度减去1，并取出链表中的头元素*/
		pool->cur_queue_size--;
		CThread_worker *worker = pool->queue_head;
		pool->queue_head = worker->next;
		pthread_mutex_unlock(&(pool->queue_lock));

		/*调用回调函数，执行任务*/
		printf("work fd:%d\n",*(int *)worker->arg);
		(*(worker->process_in_new_thread_when_accepted))(worker->arg);
		free(worker);
		//pthread_cond_broadcast(&(pool->queue_ready));
		//pthread_mutex_lock(&(pool->queue_lock));
		worker = NULL;
	}

	/*这一句应该是不可达的*/
	pthread_exit(NULL);

	return (void *)0; 
}

//下面是测试代码 
/*void *myprocess(void *arg)
  {
  printf("threadid is 0x%lu,working on task %d\n",pthread_self(),*(int *)arg);
  sleep(1);//休息一秒，延长任务的执行时间
  return NULL;
  }*/

int getSocket(){  
	int fd =socket( AF_INET, SOCK_STREAM, 0 );  
	if(-1 == fd){  
		cout<<"Error, fd is -1"<<endl;  
	}  
	return fd;  
}  

/** 
 * main执行accept()得到新socket_fd的时候，执行这个方法 
 * 创建一个新线程，在新线程里反馈给client收到的信息 
 */
int readcount = 0; 
void* process_in_new_thread_when_host_accepted(void* arg){ 
	//long long_fd = (long)arg; 
	int fd = *(int *)arg; 
	char readbuffer[BUF_SIZE];  
	char writebuffer[BUF_SIZE];  
	if(fd<0){ 
		cout<<"process_in_new_thread_when_host_accepted() quit!"<<endl; 
		return 0; 
	} 
	int size = read(fd,readbuffer,BUF_SIZE);
	//	printf("read from fd=%d read:%s\n",fd,readbuffer);
	//if(size > 0) epoll_ctl(epoll_fd,EPOLL_CTL_MOD,epoll_event.data.fd,&epoll_event);
	if(size <= 0)//检测到对方断开连接，清除map的fd,移除epoll监听事件
	{
		/*SockfdIter  itor;
		for (itor=HostList.begin(); itor != HostList.end();)
		{
			if(*itor == fd)
			{
				//printf("fd:%d\n",fd);
				HostList.erase(itor);
				break;
			}
			else
			{
				itor++;
			}
		} */

		/*test  */

		pthread_mutex_lock(&delepoll_lock);
		hostinfomap.erase(fd);//清除map  
		hosttestmap.erase(fd);
		host_count_id--;
		bool ret=DelEpoll(fd,HOST_TYPE);//  移除 事件监听
		if(ret)
		{
			printf("clear hostfd :%d success,host_count=%d\n", fd,host_count_id);
		}
		close(fd);
		pthread_mutex_unlock(&delepoll_lock);
		
	}
	else if(size > 0)
	{
		//sprintf(writebuffer, "I'm 192.168.0.93 port is 9880 you send[%s]",readbuffer);  
		//int write_num = write(fd, writebuffer, strlen(writebuffer));
		SocketTestIter itor=hosttestmap.find(fd);
		int fd_tmp=itor->second;
		SocketTestIter it; 
		int hostfd;
		for(it=apptestmap.begin();it!=apptestmap.end(); it++) 
		{ 
			if(it->second==fd_tmp)
			{
				if(total_count==0)
					get_current_time();
				if(total_count==1000)
					get_current_time();
				total_count++;

				hostfd=it->first;
				//printf("hostfd:%d\n",hostfd);
				//printf("count:%d\n",total_count);

				sprintf(writebuffer, "host send to you :%s",readbuffer);
				int write_num = write(hostfd, writebuffer, strlen(writebuffer));
			}
		}
	}

} 

void* process_in_new_thread_when_app_accepted(void* arg){ 
	//long long_fd = (long)arg; 
	int fd = *(int *)arg; 
	char readbuffer[BUF_SIZE];  
	char writebuffer[BUF_SIZE];  
	if(fd<0){ 
		cout<<"process_in_new_thread_when_host_accepted() quit!"<<endl; 
		return 0; 
	} 
	int size = read(fd,readbuffer,BUF_SIZE);
	//	printf("read from fd=%d read:%s\n",fd,readbuffer);
	//if(size > 0) epoll_ctl(epoll_fd,EPOLL_CTL_MOD,epoll_event.data.fd,&epoll_event);
	if(size == 0)//检测到对方断开连接，清除map的fd,移除epoll监听事件
	{
		/*SockfdIter  itor;
		for (itor=AppList.begin(); itor != AppList.end();)
		{
			if(*itor == fd)
			{
				//printf("fd:%d\n",fd);
				AppList.erase(itor);
				break;
			}
			else
			{
				itor++;
			}
		}
		*/

		pthread_mutex_lock(&delepoll_lock_app);
		appinfomap.erase(fd);//清除map  
		apptestmap.erase(fd);
		app_count_id--;
		bool ret=DelEpoll(fd,APP_TYPE);//  移除 事件监听
		if(ret)
		{
			printf("clear appfd :%d success,host_count=%d\n", fd,app_count_id);
		}
		close(fd);
		pthread_mutex_unlock(&delepoll_lock_app);

	}
	else if(size > 0)
	{
		SocketTestIter itor=apptestmap.find(fd);
		int fd_tmp=itor->second;
		SocketTestIter it; 
		int appfd;
		for(it=hosttestmap.begin();it!=hosttestmap.end(); it++) 
		{ 
			if(it->second==fd_tmp)
			{

				appfd=it->first;
				//printf("hostfd:%d\n",appfd);

				sprintf(writebuffer, "host send to you :%s",readbuffer);
				int write_num = write(appfd, writebuffer, strlen(writebuffer));
			}
		}
	}

} 

void* WaitRecvData(int type){ 
	int nRet;
	SockfdList::iterator listiter;
	//int fdlist[1000001];
	int wait_fds_num;//检测有数据发送的fd个数	
	//struct epoll_event ev;
	struct epoll_event evs[MAX_EPOLL_NUM];
	//epoll_fd = epoll_create(MAX_EPOLL_NUM);  // wjj modify 
	int cur_fds = MAX_EPOLL_NUM + 100;

	while(1)
	{
		int fdnum = 0;
		/*for(listiter=HostList.begin();listiter!=HostList.end();listiter++)
		  {
		//fdlist[fdnum] = *listiter;
		fdnum++;
		//ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
		//ev.data.fd = *listiter;
		//if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,ev.data.fd,&ev) < 0)
		//{
		//printf("Epoll Error : %d\n",errno);
		//}
		}*/
		int wait_epoll_fd;
		if(type == HOST_TYPE)
			wait_epoll_fd=epoll_fd[0];
		else
			wait_epoll_fd=epoll_fd[1];
		if((wait_fds_num = epoll_wait(wait_epoll_fd,evs,cur_fds,2000)) == -1)
		{
			printf("Epoll wait Error:%d\n",errno);
		}
		//printf("fdnum=%d\n",fdnum);
		//printf("wait_fds_num=%d\n",wait_fds_num);
		for(int i=0;i < wait_fds_num;i++)
		{
			//pool_add_worker(process_in_new_thread_when_accepted,(void *)fdlist[i]);
			if(type == HOST_TYPE)
			{
				pool_add_worker(process_in_new_thread_when_host_accepted,(void *)&evs[i].data.fd);
			}
			else if(type == APP_TYPE)
			{
				pool_add_worker(process_in_new_thread_when_app_accepted,(void *)&evs[i].data.fd);
			}
			else
			{
				printf("error type ..\n");
			}

			//epoll_event = evs[i];
			//epoll_ctl(epoll_fd,EPOLL_CTL_MOD,evs[i].data.fd,&evs[i]);
			//DelEpoll(evs[i].data.fd);
		}
		/*for(int i=0;i<wait_fds;i++)
		  {
		  epoll_ctl(epoll_fd,EPOLL_CTL_MOD,evs[i].data.fd,&evs[i]);
		  }*/
	}//while(1)
} 


/*  监听主机连接线程  */
void* WaitHostConnect(void* arg)
{
	int fd=*(int *)arg;
	//----为服务器主线程绑定ip和port------------------------------ 
	struct sockaddr_in local_addr; //服务器端网络地址结构体 
	memset(&local_addr,0,sizeof(local_addr)); //数据初始化--清零 
	local_addr.sin_family=AF_INET; //设置为IP通信 
	local_addr.sin_addr.s_addr=inet_addr(SERVER_IP);//服务器IP地址 
	//local_addr.sin_addr.s_addr=htonl(INADDR_ANY);//服务器IP地址 
	local_addr.sin_port=htons(LISTEN_HOST_PORT); //服务器端口号 

	int reuseaddr_on = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on, sizeof(reuseaddr_on)); //支持端口复用
	int bind_result = bind(fd, (struct sockaddr*) &local_addr, sizeof(struct sockaddr));
	if(bind_result < 0){ 
		cout<<"Bind Error in main()"<<endl; 
		//return -1; 
	} 
	cout<<"bind_result="<<bind_result<<endl; 
	listen(fd, 10); 
	int flags = fcntl(fd, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flags);
	epoll_fd[0] = epoll_create(MAX_EPOLL_NUM);
	WaitRecvData(HOST_TYPE);
}


/*  监听APP连接线程  */
void* WaitAppConnect(void* arg)
{
	int fd=*(int *)arg;
	//----为服务器主线程绑定ip和port------------------------------ 
	struct sockaddr_in local_addr; //服务器端网络地址结构体 
	memset(&local_addr,0,sizeof(local_addr)); //数据初始化--清零 
	local_addr.sin_family=AF_INET; //设置为IP通信 
	local_addr.sin_addr.s_addr=inet_addr(SERVER_IP);//服务器IP地址 
	//local_addr.sin_addr.s_addr=htonl(INADDR_ANY);//服务器IP地址 
	local_addr.sin_port=htons(LISTEN_APP_PORT); //服务器端口号 

	int reuseaddr_on = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on, sizeof(reuseaddr_on)); //支持端口复用
	int bind_result = bind(fd, (struct sockaddr*) &local_addr, sizeof(struct sockaddr));
	if(bind_result < 0){ 
		cout<<"Bind Error in main()"<<endl; 
		//return -1; 
	} 
	cout<<"bind_result="<<bind_result<<endl; 
	listen(fd, 10); 
	int flags = fcntl(fd, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flags);
	epoll_fd[1] = epoll_create(MAX_EPOLL_NUM);
	WaitRecvData(APP_TYPE);
}




/** 
 * 每当有新连接连到server时，就通过libevent调用此函数。 
 *    每个连接对应一个新线程 
 */ 
void on_host_accept(int sock, short event, void* arg) 
{ 
	struct epoll_event ev;

	struct sockaddr_in remote_addr; 
	int sin_size=sizeof(struct sockaddr_in); 
	int new_fd = accept(sock,  (struct sockaddr*) &remote_addr, (socklen_t*)&sin_size); 
	if(new_fd < 0){ 
		cout<<"Accept error in on_host_accept()"<<endl; 
		return; 
	}
	//SocketInfoMap hostinfomap;   //wjj modify here
	hostinfomap[sock] = ntohl(remote_addr.sin_addr.s_addr);

	/*test  */
	hosttestmap[new_fd]=host_count_id;
	host_count_id++;
	//HostList.push_back(new_fd);
	cout<<"new_fd accepted is "<<new_fd<<endl;

	ev.events = EPOLLIN | EPOLLET ;//| EPOLLONESHOT;
	ev.data.fd = new_fd;
	if(epoll_ctl(epoll_fd[0],EPOLL_CTL_ADD,ev.data.fd,&ev) < 0)
	{
		printf("Epoll Error : %d\n",errno);
	}
	pthread_mutex_init(&delepoll_lock,NULL);
	//accept_new_thread(new_fd); 
	//cout<<"on_host_accept() finished for fd="<<new_fd<<endl; 
} 

void on_app_accept(int sock, short event, void* arg) 
{ 
	struct epoll_event ev;

	struct sockaddr_in remote_addr; 
	int sin_size=sizeof(struct sockaddr_in); 
	int new_fd = accept(sock,  (struct sockaddr*) &remote_addr, (socklen_t*)&sin_size); 
	if(new_fd < 0){ 
		cout<<"Accept error in on_app_accept()"<<endl; 
		return; 
	}
	appinfomap[sock] = ntohl(remote_addr.sin_addr.s_addr);

	/*test  */
	apptestmap[new_fd]=app_count_id;
	app_count_id++;


	//AppList.push_back(new_fd);
	cout<<"new_fd accepted is "<<new_fd<<endl;

	ev.events = EPOLLIN | EPOLLET ;//| EPOLLONESHOT;
	ev.data.fd = new_fd;
	if(epoll_ctl(epoll_fd[1],EPOLL_CTL_ADD,ev.data.fd,&ev) < 0)
	{
		printf("Epoll Error : %d\n",errno);
	}
	pthread_mutex_init(&delepoll_lock_app,NULL);
	//accept_new_thread(new_fd); 
	//cout<<"on_host_accept() finished for fd="<<new_fd<<endl; 
} 

int main(){
    signal(SIGPIPE,SIG_IGN);	
	listen_host_fd= getSocket(); 
	if(listen_host_fd<0){ 
		cout<<"Error in main(), fd<0"<<endl; 
	} 
	cout<<"main() fd="<<listen_host_fd<<endl; 
	listen_app_fd= getSocket(); 
	if(listen_app_fd<0){ 
		cout<<"Error in main(), fd<0"<<endl; 
	} 
	cout<<"main() fd="<<listen_app_fd<<endl;
	pool_init(1000);/*线程池中最多三个活动线程*/
	pthread_t thread;
	sleep(1);
	/* 监听  HOST 连接  */
	pthread_create(&thread,NULL,WaitHostConnect,(void*)&listen_host_fd); 

	/*  监听  APP 连接  */
	pthread_create(&thread,NULL,WaitAppConnect,(void*)&listen_app_fd); 


	//pthread_create(&thread,NULL,WaitRecvData,NULL); 
	//pthread_detach(thread);
	//-----设置libevent事件，每当socket出现可读事件，就调用on_accept()------------ 

	sleep(1);
	struct event_base* base = event_base_new(); 

	/* host连接事件 */
	struct event listen_host_ev; 
	event_set(&listen_host_ev, listen_host_fd, EV_READ|EV_PERSIST, on_host_accept, NULL); 
	event_base_set(base, &listen_host_ev); 
	event_add(&listen_host_ev, NULL); 

	/* app连接事件 */
	struct event listen_app_ev; 
	event_set(&listen_app_ev, listen_app_fd, EV_READ|EV_PERSIST, on_app_accept, NULL); 
	event_base_set(base, &listen_app_ev); 
	event_add(&listen_app_ev, NULL); 

	event_base_dispatch(base); 
	//------以下语句理论上是不会走到的--------------------------- 
	cout<<"event_base_dispatch() in main() finished"<<endl; 

	//----销毁资源------------- 
	event_del(&listen_host_ev); 
	event_del(&listen_app_ev); 

	event_base_free(base); 
	cout<<"main() finished"<<endl; 
}


