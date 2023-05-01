#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "name_set_tools.h"

const int INIT_SET_SIZE = 5;

struct name_set new_name_set() {
    struct name_set ns;
    ns.set_size = INIT_SET_SIZE;
    ns.n_names = 0;
    ns.names = malloc( INIT_SET_SIZE * (sizeof(char*)));

    return ns;
}

void display_name_set(struct name_set ns) {
    printf("Capacity: %d\n", ns.set_size);
    printf("Names: ");
    if (ns.n_names == 0) {
        printf("none");
    } else {
        printf("{ ");
        for (int i=0; i<ns.set_size; ++i) {
            if (ns.names[i] != NULL && strlen(ns.names[i]) > 0) {
                printf("'%s' ", ns.names[i]);
            }
        }
        printf("} (%d total)", ns.n_names);
    }
    printf("\n");
}

// Attempts to add a unique name to the set. Returns 0 if a new name was added, 1 or 2 otherwise
int add_name(struct name_set* ns, char* name) {
    // Check for uniqueness
    for (int i=0; i < ns->set_size; ++i) {
        if (ns->names[i] != NULL && strcmp(ns->names[i], name) == 0) {
            return 1;
        }
    }

    // No name match was found. Resize if needed, then add the name
    if (ns->n_names == ns->set_size) {
        ns->set_size *= 2;
        ns->names = realloc(ns->names, (ns->set_size) * sizeof(char*));
    }

    // Add name to first free spot in array
    for (int i=0; i < ns->set_size; ++i) {
        if (ns->names[i] == NULL) {
            ns->names[i] = strdup(name);
            (ns->n_names)++;
            return 0;
        }
    }

    return 2;
}

// Attempts to free and remove the provided name from the set, replacing the value with NULL.
// Returns 0 if a name was removed, 1 otherwise.
int rem_name(struct name_set* ns, char* name) {
    for (int i=0; i < ns->set_size; ++i) {
        if (ns->names[i] != 0 && strcmp(ns->names[i], name) == 0) {
            free(ns->names[i]);
            ns->names[i] = NULL;
            --(ns->n_names);
            return 0;
        }
    }
    return 1;
}

void free_name_set(struct name_set ns) {
    for (int i=0; i < ns.set_size; ++i) {
        if (ns.names[i] != NULL) {
            free(ns.names[i]);
        }
    }
}

void clear_name_set(struct name_set* ns) {
    for (int i=0; i < ns->set_size; ++i) {
        if (ns->names[i] != NULL) {
            ns->names[i] = NULL;
        }
    }
    ns->n_names = 0;
}