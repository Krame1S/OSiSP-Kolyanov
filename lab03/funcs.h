#ifndef FUNCS_H
#define FUNCS_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define REPEAT_COUNT 5

typedef struct {
    int firstVal;
    int secondVal;
} PAIR;

extern int can_display_statistics;
extern int total_child_processes;
extern pid_t* child_pids;
extern PAIR value_pair;
extern int update_pair_values;
extern int count_00;
extern int count_01;
extern int count_10;
extern int count_11;

bool process_command(char* cmd);
void collect_statistics();
void child_process_cycle();
void print_statistics();
bool mute_all_except_one_by_id(int id);
bool exit_program();
bool unmute_all_child_processes();
bool unmute_statistics_by_id(int id);
bool mute_all_child_processes();
bool mute_statistics_by_id(int id);
bool terminate_all_child_processes();
bool display_process_list();
bool terminate_last_child_process();
bool create_new_child_process();
void add_new_child_process(pid_t child_pid);
void disable_statistics_display();
void enable_statistics_display();
void read_user_command(char* cmd);

#endif // FUNCS_H