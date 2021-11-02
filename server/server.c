/*
* @file main.c
* @author Renat Kagal <kagal@itspartner.net>
*
* Assembling : gcc -Wall server.c log.o -pthread -o server
*
* Description : coffee machine
*
* Copyright (c) 2021, ITS Partner LLC.
* All rights reserved.
*
* This software is the confidential and proprietary information of
* ITS Partner LLC. ("Confidential Information"). You shall not
* disclose such Confidential Information and shall use it only in
* accordance with the terms of the license agreement you entered into
* with ITS Partner.
*/

#define _GNU_SOURCE

enum states {
    st_begin_work = 0,
    st_waiting_for_order,
    st_check_condition,
    st_coffee
};

enum signals{
	true_signal = 0,
	false_signal,
	singal_3,
	signal_4
};

typedef void (*func_ptr)(enum states state, enum signals signal);

struct transition {
	enum states new_state;
	func_ptr function;
};

void begin_work (enum states state, enum signals signal);
void end_work (enum states state, enum signals signal);
void waiting_for_order (enum states state, enum signals signal);
void check_condition (enum states state, enum, signals signal);
void make_coffee (enum states state, enum signals signal);


struct transion sm [4][4] = {
	[st_begin_work][true_signal] = {st_waiting_for_order, begin_work},
	[st_begin_work][false_signal] = {st_begin_work, end_work},
	[st_begin_work][singal_3] = {st_begin_work, NULL},
	[st_begin_work][singal_4] = {st_begin_work, NULL},
	[st_waiting_for_order][true_signal] = {st_check_condition, check_condition},
	[st_waiting_for_order][false_signal] = {st_begin_work, check_condition},
	[st_waiting_for_order][singal_3] = {st_begin_work, NULL},
	[st_waiting_for_order][singal_4] = {st_begin_work, NULL},
	[st_check_condition][true_signal] = {st_coffee, make_coffee},
	[st_check_condition][false_signal] = {st_coffee, make_coffee},	
}

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

char keystoke = 0;

#include "head_server.h"

#define BUFFER_SIZE 32

int main () {

    int listen_sock, sock;
    struct sockaddr_in addr;

    if ((listen_sock = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
        puts ("Failed create socket");
        exit (EXIT_FAILURE);
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons (PORT);
    addr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);

    if (bind (listen_sock, (struct sockaddr*) &addr, sizeof (addr)) < 0) {
        puts ("Failed bind sock");
        exit (EXIT_FAILURE);
    }
    listen (listen_sock, 1);
    
   	pthread_t thr;

  	struct thr_listen_sock* data;
  	data->listen_sock = listen_sock;

  	pthread_create (&thr, NULL, server_accept, (void*) data);

    while (scanf("%c", &keystoke)) {
        if (keystoke == 'p')
            break;
    }

    if (pthread_join (thr, NULL)) {
        printf ("Failed join thread : %d", i + 1);
        exit (EXIT_FAILURE);
    }

    close (listen_sock);
}

void coffee_machine(struct thr_data* data) {
	char buff[BUFFER_SIZE];

	int cond_recv = recv (data->sock, buff, BUFFER_SIZE, 0);

	if (cond_recv < 0) {
		puts ("Failed recv");
		exit (EXIT_FAILURE);
	}
	if (cond_recv == 0) {
		free (data);
		return;
	}

	if (strcmp (buff, "coffee") == 0) {
		coffee ();
	}

	free (data);
}

void coffee () {

}

void server_accept (struct thr_listen_sock* data) {
	struct thr_node* thr_top = NULL;
    struct thr_data* data = NULL;
    struct thr_node* tmp;

	while (keystoke != 'p') {
        if ((sock = accept (data->listen_sock, NULL, NULL)) < 0) {
            puts ("Failed accept connection");
            exit (EXIT_FAILURE);
        }
        data = (struct thr_data*)malloc (sizeof (struct thr_data));

        if (data == NULL) {
            puts ("Failed alloc memory data");
            exit (EXIT_FAILURE);
        }
        data->sock = sock;

        create_thread (&thr_top, data);       
    }

    while (top != NULL) {
    	if (pthread_join (top, NULL)) {
    		puts ("Failed join thread");
    		exit (EXIT_FAILURE);
    	}
    	tmp = top;
    	top = top->next;
    	free (top);
    }
}

void create_thread(struct thr_node** thr_top, struct thr_data* data) {
    if (*thr_top == NULL) {
        (*thr_top) = (struct thr_node*)malloc (sizeof (struct thr_node));
        
        if ((*thr_top) == NULL) {
            puts ("Failed alloc memory for thr_top");
            exit (EXIT_FAILURE);
        }
        (*thr_top)->id = 1;
        (*thr_top)->next = NULL;
        data->id = (*thr_top)->id;

        if (pthread_create (&((*thr_top)->thread), NULL, coffee_machine, data)) {
            puts ("Failed create thread top");
            exit (EXIT_FAILURE);
        }
    }
    else {
        struct thr_node* tmp = (struct thr_node*)malloc (sizeof (struct thr_node));
        
        if (tmp == NULL) {
            puts ("Failed alloc memory for tmp_thread");
            exit (EXIT_FAILURE);
        }
        struct thr_node* tmp2 = (*thr_top);

        while (tmp2->next != NULL)
             tmp2 = tmp2->next;
        tmp2->next = tmp;

        tmp->id = tmp2->id + 1;
        data->id = tmp->id;

        tmp->next = NULL;
        
        if (pthread_create (&(tmp->thread), NULL, coffee_machine, data)) {
            puts ("Failed create thread");
            exit (EXIT_FAILURE);
        }
    }
}