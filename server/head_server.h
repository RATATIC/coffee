#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

void begin_work (enum states state, enum signals signal);
void waiting_for_order (enum states state, enum signals signal);
void check_condition (enum states state, enum, signals signal);
void st_coffee (enum states state, enum signals signal);