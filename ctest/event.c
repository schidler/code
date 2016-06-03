/**************************************************************
 * Copyright (C) 2016-2016 All rights reserved.
 * @Version: 1.0
 * @Created: 2016-04-19 23:19
 * @Author: menqi - 1083734876@qq.com
 * @Description: 
 *
 * @History: 
 **************************************************************/
#include <stdio.h>

#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include <event2/event.h>
#include <event2/bufferevent.h>

#define LISTEN_PORT 8888
#define LISTEN_BACKLOG 32


void read_cb(struct bufferevent *bev, void *arg)
{
#define MAX_LINE    256
	char line[MAX_LINE+1];
	int n;
	evutil_socket_t fd = bufferevent_getfd(bev);
	while (n = bufferevent_read(bev, line, MAX_LINE), n > 0) {
		line[n] = '\0';
		printf("fd=%u, read line: %s\n", fd, line);
		bufferevent_write(bev, line, n);
	}
}

void error_cb(struct bufferevent *bev, short event, void *arg)
{
	bufferevent_free(bev);
}

//  回调函数  typedef void(* event_callback_fn)(evutil_socket_t sockfd, short event_type, void *arg)

void do_accept(evutil_socket_t listener, short event, void *arg)
{
	struct event_base *base = (struct event_base *)arg;
	evutil_socket_t fd;
	struct sockaddr_in sin;
	socklen_t slen;
	fd = accept(listener, (struct sockaddr *)&sin, &slen);
	if (fd > FD_SETSIZE) {
		perror("fd > FD_SETSIZE\n");
		return ;
	}
	printf("ACCEPT: fd = %u\n", fd);

	struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(bev, read_cb, NULL, error_cb, arg);
	bufferevent_enable(bev, EV_READ|EV_WRITE|EV_PERSIST);


}


int main()
{
	evutil_socket_t listener;
	listener=socket(AF_INET,SOCK_STREAM,0);
	assert(listener >0);
	evutil_make_listen_socket_reuseable(listener);
	struct sockaddr_in sin;
	sin.sin_addr.s_addr = 0;
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
	evutil_make_socket_nonblocking(listener);

	//  创建一个event_base 
	struct event_base *base=event_base_new();
	assert(base != NULL);

	//  创建并绑定一个 event
	struct event *listen_event;
	listen_event=event_new(base,listener,EV_READ|EV_PERSIST,do_accept,(void*)base);
	//  设置超时，  NULL表示无超时
	event_add(listen_event,NULL);

	//启动事件循环 
	event_base_dispatch(base);
	printf("The End.");


	return 0;
}
