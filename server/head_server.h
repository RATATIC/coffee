#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

struct thr_listen_sock {
	int listen_sock;
};

struct thr_data {
	int id;
	int sock;
};

struct thr_node {
    int id;
    pthread_t thread;
    struct thr_node* next;
};

void create_thread(struct thr_node** thr_top, struct thr_data* data);

void server_accept (struct thr_listen_sock* data_listen);

void coffee (int sock);

void coffee_machine(struct thr_data* data);