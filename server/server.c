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

#include "head_server.h"

#define BUFFER_SIZE 32

#define PORT 2000


enum states {
    st_begin_work = 0,
    st_waiting_for_order,
    st_check_equip,
    st_check_water_level,
    st_check_litter,
    st_waiting,
    st_make_coffee,
    st_end_work
};

enum signals{
	true_signal = 0,
	false_signal,
};

typedef void (*func_ptr)(enum signals* signal, int sock);

struct transition {
	enum states new_state;
	func_ptr function;
};

void begin_work (enum signals* signal, int sock);
void end_work (enum signals* signal, int sock);
void waiting_for_order (enum signals* signal, int sock);
void check_equip (enum signals* signal, int sock);
void check_water_level (enum signals* signal, int sock);
void check_litter (enum signals* signal, int sock);
void make_coffee (enum signals* signal, int sock);
void waiting (enum signals* signal, int sock);

struct transition sm_table [7][2] = {
	[st_begin_work][true_signal] = {st_waiting_for_order, waiting_for_order},
	[st_begin_work][false_signal] = {st_end_work, end_work},

	[st_waiting_for_order][true_signal] = {st_check_equip, check_equip},
	[st_waiting_for_order][false_signal] = {st_waiting_for_order, waiting_for_order},
	
	[st_check_equip][true_signal] = {st_check_water_level, check_water_level},
	[st_check_equip][false_signal] = {st_waiting, waiting},
	
	[st_check_water_level][true_signal] = {st_check_litter, check_litter},
	[st_check_water_level][false_signal] = {st_waiting, waiting},	

	[st_check_litter][true_signal] = {st_make_coffee, make_coffee},
	[st_check_litter][false_signal] = {st_waiting, waiting},

	[st_waiting][true_signal] = {st_check_equip, check_equip},
	[st_waiting][false_signal] = {st_end_work, NULL},

	[st_make_coffee][true_signal] = {st_end_work, end_work},
	[st_make_coffee][false_signal] = {st_end_work, NULL},
};

int electic_eq = 1;
int water_level = 100;
int equip = 1;
int litter = 0;
int work_coffee = 1;

char keystoke = 0;

int main () {

    int listen_sock;
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
  	
  	struct thr_listen_sock* data = (struct thr_listen_sock*)malloc (sizeof (struct thr_listen_sock));
  	
  	data->listen_sock = listen_sock;


  	pthread_create (&thr, NULL, server_accept, (void*) data);

    while (scanf("%c", &keystoke)) {
        if (keystoke == 'p')
            break;
    }

    if (pthread_join (thr, NULL)) {
        printf ("Failed join thread");
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
		coffee (data->sock);
	}

	free (data);
}

void coffee (int sock) {
	enum signals signal = true_signal;
	enum states state = st_begin_work;

	func_ptr work;

	begin_work (&signal, sock);

	while (work_coffee) {
		work = sm_table[state][signal].function;
		state = sm_table[state][signal].new_state;

		if (work != NULL) {
			work (&signal, sock);
		}
	}
}

void make_coffee (enum signals* signal, int sock) {
	char buff[BUFFER_SIZE] = "coffee done";

	water_level -= 20;
	litter += 1;
	
	if (send (sock, buff, BUFFER_SIZE, 0) < 0) {
		puts ("Failed send");
		exit (EXIT_FAILURE);
	}
	*signal = true_signal;
}

void waiting (enum signals* signal, int sock) {
	printf ("waiting \nequip : %d, water_level : %d, litter : %d\n", equip, water_level, litter);
	if (equip != 1) {
		equip = 1;
	}
	if (water_level <= 20) {
		water_level = 100;
	}
	if (litter >= 6) {
		litter = 0;
	}
	*signal = true_signal;
}

void check_litter (enum signals* signal, int sock) {
	puts ("check_litter");

	if (litter < 6) {
		*signal = true_signal;
	}
	else {
		*signal = false_signal;
	}
}

void check_water_level (enum signals* signal, int sock) {
	puts ("check_water_level");

	if (water_level > 20) {
		*signal = true_signal;
	}
	else {
		*signal = false_signal;
	}
}

void check_equip (enum signals* signal, int sock) {
	puts ("check_equip");

	if (equip == 1) {
		*signal = true_signal;
	}
	else {
		*signal = false_signal;
	}
}

void waiting_for_order (enum signals* signal, int sock) {
	puts ("waiting for order");
	char buff[BUFFER_SIZE];
	memset (buff, '\0', BUFFER_SIZE);

	int cond_recv = recv (sock, buff, BUFFER_SIZE, 0);

	if (cond_recv < 0) {
		puts ("Failed recv");
		exit (EXIT_FAILURE);
	}
	if (strcmp (buff, "A\n") == 0 || strcmp (buff, "B\n") == 0) {
		memset (buff, '\0', BUFFER_SIZE);
		strcat (buff, "order confirmed");
		
		if (send (sock, buff, BUFFER_SIZE, 0) < 0) {
			puts ("Failed send");
			exit (EXIT_FAILURE);
		}
		*signal = true_signal;
	}
	else {
		memset (buff, '\0', BUFFER_SIZE);
		strcat (buff, "order not confirmed");

		if (send (sock, buff, BUFFER_SIZE, 0) < 0) {
			puts ("Failed send");
			exit (EXIT_FAILURE);
		}
		*signal = false_signal;
	}
}

void end_work (enum signals* signal, int sock) {
	puts ("end");
	work_coffee = 0;
}

void begin_work (enum signals* signal, int sock) {
	puts("begin");
	work_coffee = 1;

	if (electic_eq == 1) {
		*signal = true_signal;
	}
	else {
		*signal = false_signal;
	}
} 

void server_accept (struct thr_listen_sock* data_listen) {
	struct thr_node* thr_top = NULL;
    struct thr_data* data = NULL;
    struct thr_node* tmp;

    int sock;

	while (keystoke != 'p') {
        if ((sock = accept (data_listen->listen_sock, NULL, NULL)) < 0) {
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

    while (thr_top != NULL) {
    	if (pthread_join (thr_top->thread, NULL)) {
    		puts ("Failed join thread");
    		exit (EXIT_FAILURE);
    	}
    	tmp = thr_top;
    	thr_top = thr_top->next;
    	free (tmp);
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