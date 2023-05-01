#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "name_set_tests.h"
#include "../name_set_tools.h"

void run_all_name_set_tests() {
    new_name_set_test();
    add_name_test();
    rem_name_test();
    clear_name_set_test();
}

void new_name_set_test() {
    printf("================= NEW_NAME_SET TEST =================\n");
    struct name_set ns = new_name_set();
    display_name_set(ns);
    printf("** Freeing name set **\n");
    free_name_set(ns);
}

void add_name_test() {
    printf("================= ADD_NAME TEST =================\n");
    struct name_set ns = new_name_set();
    display_name_set(ns);

    printf("** Adding 'Clark' **\n");
    int retcode = add_name(&ns, "Clark");
    display_name_set(ns);
    printf("Return value: %d\n", retcode);

    printf("** Adding 'Kent' **\n");
    retcode = add_name(&ns, "Kent");
    display_name_set(ns);
    printf("Return value: %d\n", retcode);

    printf("** Adding 'Kent' **\n");
    retcode = add_name(&ns, "Kent");
    display_name_set(ns);
    printf("Return value: %d\n", retcode);

    printf("** Adding 'Annie', 'Beth', 'Dorothy', and 'Eliza' **\n");
    add_name(&ns, "Annie");
    add_name(&ns, "Beth");
    add_name(&ns, "Dorothy");
    retcode = add_name(&ns, "Eliza");
    display_name_set(ns);
    printf("Last return value: %d\n", retcode);

    printf("** Freeing name set **\n");
    free_name_set(ns);
}

void rem_name_test() {
    printf("================= REM_NAME TEST =================\n");
    struct name_set ns = new_name_set();
    display_name_set(ns);
    printf("** Adding 'Annie', 'Beth', 'Dorothy', and 'Eliza' **\n");
    add_name(&ns, "Annie");
    add_name(&ns, "Beth");
    add_name(&ns, "Dorothy");
    add_name(&ns, "Eliza");
    display_name_set(ns);

    printf("** Removing 'Beth' **\n");
    int retcode = rem_name(&ns, "Beth");
    display_name_set(ns);
    printf("Return value: %d\n", retcode);

    printf("** Removing 'Kyle' **\n");
    retcode = rem_name(&ns, "Kyle");
    display_name_set(ns);
    printf("Return value: %d\n", retcode);

    printf("** Freeing name set **\n");
    free_name_set(ns);
}

void clear_name_set_test() {
    printf("================= CLEAR_NAME_SET TEST =================\n");
    struct name_set ns = new_name_set();
    printf("** Adding 'Annie', 'Beth', 'Dorothy', and 'Eliza' **\n");
    add_name(&ns, "Annie");
    add_name(&ns, "Beth");
    add_name(&ns, "Dorothy");
    add_name(&ns, "Eliza");
    display_name_set(ns);

    printf("** Freeing name set **\n");
    free_name_set(ns);

    printf("** Clearing name set **\n");
    clear_name_set(&ns);
    display_name_set(ns);
}