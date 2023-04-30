#ifndef COMMAND_TESTS_H_
#define COMMAND_TESTS_H_

void run_all_command_tests();

// parsing/translation tests
void run_get_code_tests();
void run_str_to_cmd_tests();
void run_cmd_to_str_tests();

// new command tests
void new_command_test();
void new_wait_cmd_test();
void new_rsgn_cmd_test();
void new_begn_cmd_test();
void new_begn_cmd_test();
void new_movd_cmd_test();
void new_invl_cmd_test();
void new_draw_cmd_test();
void new_over_cmd_test();
void new_play_cmd_test();
void new_move_cmd_test();

#endif