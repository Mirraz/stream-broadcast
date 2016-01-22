#include <event2/event.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "stack.h"

static void perror_and_exit(const char *s) {
	perror(s);
	exit(EXIT_FAILURE);
}

static void everror(const char *s) {
	fprintf(stderr, "%s\n", s);
}

static void everror_and_exit(const char *s) {
	everror(s);
	exit(EXIT_FAILURE);
}

static void printf_err(const char *format, ...) {
	va_list arglist;
	va_start(arglist, format);
	vfprintf(stderr, format, arglist);
	va_end(arglist);
	fprintf(stderr, "\n");
}

static int make_fd_nonblocking(int fd) {
	int flags;
	if ((flags = fcntl(fd, F_GETFL, NULL)) < 0) {
		return -1;
	}
	if (!(flags & O_NONBLOCK)) {
		if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
			return -1;
		}
	}
	return 0;
}

/* return:
	< 0 -- -errno
	= 0 -- read returned EAGAIN
	> 0 -- read bytes
*/
static ssize_t read_wrapper(int fd, void *buf, size_t count) {
	ssize_t bytes = read(fd, buf, count);
	if (bytes < 0) {
		switch (errno) {
			case EAGAIN:
#if EAGAIN != EWOULDBLOCK
			case EWOULDBLOCK:
#endif
				return 0;
			case ECONNRESET:
				return -errno;
			default:
				perror("recv");
				return -errno;
		}
	} else if (bytes == 0) {
		return -ECONNRESET;
	} else {
		return bytes;
	}
}

/* return:
	< 0 -- -errno
	= 0 -- write returned EAGAIN
	> 0 -- write bytes
*/
static ssize_t write_wrapper(int fd, void *buf, size_t count) {
	ssize_t bytes = send(fd, buf, count, MSG_NOSIGNAL);
	if (bytes < 0) {
		switch (errno) {
			case EAGAIN:
#if EAGAIN != EWOULDBLOCK
			case EWOULDBLOCK:
#endif
				return 0;
			case ECONNRESET:
			case EPIPE:
				return -errno;
			default:
				perror("send");
				return -errno;
		}
	} else if (bytes == 0) {
		return -ECONNRESET;
	} else {
		return bytes;
	}
}

#define INTERRUPT_SIGNAL SIGINT

static void signal_cb(evutil_socket_t signum, short ev_flag, void *arg) {
	assert(ev_flag == EV_SIGNAL);
	(void)ev_flag;
	struct event_base *base = (struct event_base *)arg;
	switch(signum) {
		case INTERRUPT_SIGNAL: {
			if (event_base_loopbreak(base)) {everror("event_base_loopbreak"); return;}
			break;
		}
		default:
			assert(0);
	}
}

static void server_accept_cb(evutil_socket_t server_sockfd, short ev_flag, void *arg) {
	assert(ev_flag == EV_READ);
	(void)ev_flag;
	stack_struct *clients = (stack_struct *)arg;
	
	int client_sockfd = accept(server_sockfd, NULL, NULL);
	if (client_sockfd < 0) {perror("accept"); return;}
	
	if (make_fd_nonblocking(client_sockfd)) {perror("fcntl"); goto close_client_sockfd;}
	
	assert(!stack_push(clients, client_sockfd));
	return;
	
close_client_sockfd:
	if (close(client_sockfd)) perror("close");
}

// TODO: settable buffer size
#define read_buffer_size (64*1024)
uint8_t read_buffer[read_buffer_size];

static void stdin_read_cb(evutil_socket_t stdin_fd, short ev_flag, void *arg) {
	assert(ev_flag == EV_READ);
	(void)ev_flag;
	stack_struct *clients = (stack_struct *)arg;
	
	
	ssize_t read_bytes = read_wrapper(stdin_fd, read_buffer, read_buffer_size);
	if (read_bytes < 0) exit(EXIT_FAILURE);
	if (read_bytes == 0) return;
	
	stack_iterator_struct iterator = stack_get_iterator(clients);
	while (stack_iterator_is_not_null(iterator)) {
		int client_sockfd = stack_iterator_get_element(iterator);
		ssize_t write_bytes = write_wrapper(client_sockfd , read_buffer, read_bytes);
		if (write_bytes < 0) {
			if (close(client_sockfd)) perror("close");
			iterator = stack_iterator_remove_and_next(iterator);
		} else {
			if (write_bytes != read_bytes)
				fprintf(stderr, "warn: read = %ld, write = %ld\n", read_bytes, write_bytes);
			iterator = stack_iterator_next(iterator);
		}
	}
}

#define LISTEN_BACKLOG 50

int main(int argc, char* argv[]) {
	assert(argc == 2);
	const char *server_addr_filename = argv[1];
	///////////
	stack_struct clients_data;
	stack_struct *clients = &clients_data;
	stack_new(clients);
	///////////
	int server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (server_sockfd < 0) perror_and_exit("socket");
	
	struct sockaddr_un server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sun_family = AF_UNIX;
	strncpy(server_addr.sun_path, server_addr_filename, sizeof(server_addr.sun_path) - 1);
	socklen_t server_addr_len = strlen(server_addr.sun_path) + sizeof(server_addr.sun_family);
	if (bind(server_sockfd, (struct sockaddr *)&server_addr, server_addr_len)) perror_and_exit("bind");
	///////////
	struct event_base *base = event_base_new();
	if (base == NULL) everror_and_exit("event_base_new");
	
	struct event *int_signal_event = evsignal_new(base, INTERRUPT_SIGNAL, signal_cb, base);
	if (int_signal_event == NULL) everror_and_exit("evsignal_new");
	if (event_add(int_signal_event, NULL)) everror_and_exit("event_add");
	
	struct event *server_event = event_new(base, server_sockfd, EV_READ|EV_PERSIST, server_accept_cb, clients);
	if (server_event == NULL) everror_and_exit("event_new");
	if (event_add(server_event, NULL)) everror_and_exit("event_add");
	
	struct event *stdin_read_event = event_new(base, STDIN_FILENO, EV_READ|EV_PERSIST, stdin_read_cb, clients);
	if (stdin_read_event == NULL) everror_and_exit("event_new");
	if (event_add(stdin_read_event, NULL)) everror_and_exit("event_add");
	///////////
	if (listen(server_sockfd, LISTEN_BACKLOG)) perror_and_exit("listen");
	///////////
	if (event_base_dispatch(base)) everror_and_exit("event_base_dispatch");
	///////////
	
	// TODO: close all
	if (unlink(server_addr_filename)) perror("unlink");
	
	return EXIT_SUCCESS;
}

