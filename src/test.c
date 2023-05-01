#include "../lib/test/presentation_tests.h"
#include "../lib/test/common_tests.h"
#include "../lib/test/command_tests.h"
#include "../lib/test/game_tests.h"
#include "../lib/test/name_set_tests.h"

int main(int argc, char** argv) {
    run_all_presentation_tests();
    run_all_common_tests();
    run_all_command_tests();
    run_all_game_tests();
    run_all_name_set_tests();

    return 0;
}