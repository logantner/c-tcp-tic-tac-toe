#ifndef NAME_SET_TOOLS_H
#define NAME_SET_TOOLS_H

#include <pthread.h>

extern pthread_mutex_t name_set_mutex;

struct name_set {
    char** names;
    int set_size;
    int n_names;
};

struct name_set new_name_set();
int add_name(struct name_set*, char*);
int rem_name(struct name_set*, char*);
void clear_name_set(struct name_set*);
void free_name_set(struct name_set);
void display_name_set(struct name_set);

#endif