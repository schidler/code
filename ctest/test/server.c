#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include "base.h"
#include <time.h>
#include <event.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "threadpool.h"
#define LISTEN_PORT 8888
#define LISTEN_BACKLOG 32

void do_accept_cb(evutil_socket_t listener, short event, void *arg);
void read_cb(struct bufferevent *bev, void *arg);
void error_cb(struct bufferevent *bev, short event, void *arg);

static unsigned  get_current_time(void){
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC,&now);
	unsigned mseconds = now.tv_sec * 1000+ now.tv_nsec/1000000;
	return mseconds;
}

// pthread info 
typedef struct {
	pthread_t thread_id;  //  id 
	//struct event_base *base;
	//struct event notify_event;
	//int notify_receive_fd;
	//int notify_send_fd;
	//Queue new_conn_queue;	//  

}LIBEVENT_THREAD;

//



struct threadpool* run_pool;


//for debug, show the libevent_server start time
static void start_time()
{
	time_t now = time(NULL);
	struct tm* p_tm = localtime(&now);
	LOG_PRINT("\t***************************************************************\n");
	LOG_PRINT("\t******libevent server start at the time: %d-%02d-%02d %02d:%02d:%02d******\n",
			(1900 + p_tm->tm_year), (1 + p_tm->tm_mon), p_tm->tm_mday, p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);
	LOG_PRINT("\t***************************************************************\n");
	LOG_DEBUG("libevent start: %u\n", get_current_time());
	return;
}

static void thread_libevent_process(int fd,short which,void*arg)
{

}
static void setup_thread(LIBEVENT_THREAD *me)
{
	/*
	   if(! me->base)
	   {
	   me->base=event_init();
	//  event ,fd, event type,callback func ,args   
	event_set(&me->notify_event,me->notify_receive_fd,EV_READ|EV_PERSIST,thread_libevent_process,me);
	event_base_set(me->base,&me->notify_event);
	event_add(&me->notify_event,0);
	}
	 */

}



/*global ....*/
LIBEVENT_THREAD *threads;
int sockfds[65535]={0};







void* work(void* arg)
{
	int sockfd= *(int *)arg;
	//static int count=0;
	//printf("accept: fd = %u, count=%d\n", fd,count);
	//printf("thread %ld callback fuction sockfd:%d ,count:%d\n",pthread_self(),sockfd,count);
	//sleep(1);
	//count++;
	/*  在这里执行我们的处理任务   收发数据*/
		
//#define MAX_LINE    256
//char line[MAX_LINE+1];
//int len;
//len = recv(sockfd, line, MAX_LINE,0);
//if(len>0)
//	printf("wjj hahha\n");
//while(len = recv(sockfd, line, MAX_LINE,0),len>0){
//line[len] ='\0';
//send(sockfd,line,len,0);
//printf("fd %d recv from client:%s\n",sockfd,line);
	
 //} 
	 


}


int main(int argc, char *argv[])
{
	/* thread init*/
#define THREAD_NUM 20

	run_pool=threadpool_init(THREAD_NUM, 10000);
	int i;
	for (i = 0; i < run_pool->thread_num; i++)          
	{
		printf("pool thread id:%ld\n",run_pool->pthreads[i]);
	}


	/*socket init */

	int ret;
	evutil_socket_t listener;
	listener = socket(AF_INET, SOCK_STREAM, 0);
	if(listener < 0)
	{
		printf("socket error!\n");
		return 1;
	}

	evutil_make_listen_socket_reuseable(listener);//端口重用，查看源码evutil.c中可以知道就是对setsockopt做了层封装,之所以这样做是因为为了和Win32 networking API兼容

	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	//sin.sin_addr.s_addr=htonl(INADDR_ANY);
	sin.sin_addr.s_addr=inet_addr("192.168.0.73");
	sin.sin_port = htons(LISTEN_PORT);

	if (bind(listener, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		perror("bind");
		return 1;
	}

	if (listen(listener, LISTEN_BACKLOG) < 0) {
		perror("listen");
		return 1;
	}
	printf ("Listening...\n");
	//start_time();//  current time  
	evutil_make_socket_nonblocking(listener);//设置为非阻塞模式，查看源码evutil.c中可以知道就是对fcntl做了层封装
	struct event_base *base = event_base_new();//创建一个event_base对象也既是创建了一个新的libevent实例
	if (!base) {
		fprintf(stderr, "Could not initialize libevent!\n");
		return 1;
	}
	//创建并绑定一个event
	struct event *listen_event;
	listen_event = event_new(base, listener, EV_READ|EV_PERSIST, do_accept_cb, (void*)base);
	event_add(listen_event, NULL);//注册时间，参数NULL表示无超时设置
	event_base_dispatch(base);//）程序进入无限循环，等待就绪事件并执行事件处理
	printf("The End.");
	return 0;
}
void do_accept_cb(evutil_socket_t listener, short event, void *arg)
{
	struct event_base *base = (struct event_base *)arg;
	evutil_socket_t fd;
	struct sockaddr_in sin;
	socklen_t slen;
	fd = accept(listener, (struct sockaddr *)&sin, &slen);
	if (fd < 0) {
		perror("accept");
		return;
	}
	static int conn_count=0;
	printf("conn_count:%d\n",conn_count);
	conn_count++;
	/*  将 fd 存起来  */
	sockfds[fd]=fd;
	/*sockfd write/read by sub thread*/
	//write(threads[thread].notify_send_fd,"",1);  //  通知线程池中线程

	//使用bufferevent_socket_new创建一个struct bufferevent *bev，关联该sockfd，托管给event_base
	struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);//BEV_OPT_CLOSE_ON_FREE表示释放bufferevent时关闭底层传输端口。这将关闭底层套接字，释放底层bufferevent等。
	bufferevent_setcb(bev, read_cb, NULL, error_cb, &sockfds[fd]);
	bufferevent_enable(bev, EV_READ|EV_WRITE|EV_PERSIST);//启用read/write事件
}
void read_cb(struct bufferevent *bev, void *arg)
{
	/* 有数据来时交给空闲线程处理 wjj 2016-04-26  */
#define MAX_LINE    256
char line[MAX_LINE+1];
int len;
static int read_count=0;
evutil_socket_t sockfd = bufferevent_getfd(bev);
while(len = bufferevent_read(bev, line, MAX_LINE),len>0){
	line[len] ='\0';
	//printf("fd %d recv from client:%s\n",sockfd,line);
	//printf("read_count:%d\n",read_count);
	read_count++;
	bufferevent_write(bev, line, len);
}

	threadpool_add_job(run_pool, work, arg);
	//printf("add_job ok...\n");
}
void error_cb(struct bufferevent *bev, short event, void *arg)
{
	evutil_socket_t fd = bufferevent_getfd(bev);
	printf("fd = %u, ", fd);
	if (event & BEV_EVENT_TIMEOUT)
	{
		printf("Timed out\n"); //if bufferevent_set_timeouts() called
	}
	else if (event & BEV_EVENT_EOF) {
		printf("connection closed\n");
	}
	else if (event & BEV_EVENT_ERROR) {
		printf("some other error\n");
	}
	bufferevent_free(bev);
}
