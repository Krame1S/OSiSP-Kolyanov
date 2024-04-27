#include "funcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int can_display_statistics;
int total_child_processes;
pid_t* child_pids;
PAIR value_pair;
int update_pair_values;
int count_00;
int count_01;
int count_10;
int count_11;

void read_user_command(char* command_buffer) {
    int i = 0;
    while ((command_buffer[i] = getchar()) != '\n') {
        i++;

        if (i > 3) {
            command_buffer[2] = '\0';
            i = 0;
            break;
        }
    }
    command_buffer[i] = '\0';
}

void enable_statistics_display() {
    can_display_statistics = 1;
}

void disable_statistics_display() {
    can_display_statistics = 0;
}

void add_new_child_process(pid_t child_pid) {
    child_pids = (pid_t*)realloc(child_pids, (total_child_processes + 1)* sizeof(pid_t));
    child_pids[++total_child_processes] = child_pid;

    printf("Child process added: PID %d\n", child_pid);
}

bool create_new_child_process() {
    pid_t child_pid = fork();
    
    if (child_pid > 0) add_new_child_process(child_pid);

    if (child_pid == 0) {
        signal(SIGALRM, collect_statistics);
        child_process_cycle();
    }

    return true;
}

bool terminate_last_child_process() {
    if (total_child_processes == 0) {
        printf("No child processes to terminate.\n");
        return true;
    }

    kill(child_pids[total_child_processes], SIGKILL);

    printf("Child process terminated: PID %d\n", child_pids[total_child_processes]);
    total_child_processes--;

    return true;
}

bool display_process_list() {
    if (total_child_processes == 0) {
        printf("No child processes to display.\n");
    }

    for (int i = 1; i <= total_child_processes; ++i) {
        printf("Child process PID: %d\n", child_pids[i]);
    }

    return true;
}

bool terminate_all_child_processes() {
    while (total_child_processes > 0) {
        terminate_last_child_process();
    }

    return true;
}

bool mute_statistics_by_id(int id) {
    if (id > total_child_processes || id < 0) {
        printf("Invalid process ID.\n");
        return true;
    }

    kill(child_pids[id], SIGUSR2);
    printf("Statistics for process PID %d muted.\n", child_pids[id]);

    return true;
}

bool mute_all_child_processes() {
    for (int i = 1; i <= total_child_processes; ++i) {
        mute_statistics_by_id(i);
    }

    return true;
}

bool unmute_statistics_by_id(int id) {
    if (id > total_child_processes || id < 0) {
        printf("Invalid process ID.\n");
        return true;
    }

    kill(child_pids[id], SIGUSR1);
    printf("Statistics for process PID %d unmuted.\n", child_pids[id]);

    return true;
}

bool unmute_all_child_processes() {
    for (int i = 1; i <= total_child_processes; ++i) {
        unmute_statistics_by_id(i);
    }

    return true;
}

bool exit_program() {
    terminate_all_child_processes();
    free(child_pids);

    return false;
}

bool mute_all_except_one_by_id(int id) {
    mute_all_child_processes();
    unmute_statistics_by_id(id);
    
    return true;
}

bool process_command(char* command_buffer) {
    signal(SIGUSR1, enable_statistics_display);                                                    
    signal(SIGUSR2, disable_statistics_display);

    if (!strcmp(command_buffer, "+"))
        return create_new_child_process();

    if (!strcmp(command_buffer, "-")) 
        return terminate_last_child_process();

    if (!strcmp(command_buffer, "l")) 
        return display_process_list();

    if (!strcmp(command_buffer, "k")) 
        return terminate_all_child_processes();

    if (!strcmp(command_buffer, "s")) 
        return mute_all_child_processes();

    if (!strcmp(command_buffer, "g")) 
        return unmute_all_child_processes();

    if (!strcmp(command_buffer, "q")) 
        return exit_program();

    int id = 0;
    if (sscanf(command_buffer, "s%d", &id) == 1)   
        return mute_statistics_by_id(id);
    if (sscanf(command_buffer, "g%d", &id) == 1) 
        return unmute_statistics_by_id(id);
    if (sscanf(command_buffer, "p%d", &id) == 1) 
        return mute_all_except_one_by_id(id);

    printf("Invalid command.\n");
    return true;
}

void print_statistics() {
    printf("Process PID = %d, PPID = %d, {0,0} - %d, {0,1} - %d, {1,0} - %d, {1,1} - %d\n",
           getpid(), getppid(), count_00, count_01, count_10, count_11);
}

void child_process_cycle() {
    printf("Child process started.\n\n");
    int currentRepeat = 0;
    while(1) {
        if(currentRepeat == REPEAT_COUNT) {
            currentRepeat = 0;
            if (can_display_statistics)
                print_statistics();
        }
        alarm(1);
        while (update_pair_values) {
            if (value_pair.firstVal == 0 && value_pair.secondVal == 0)
                value_pair.secondVal = 1;
            else if (value_pair.firstVal == 0 && value_pair.secondVal == 1) {
                value_pair.firstVal = 1;
                value_pair.secondVal = 0;
            } else if (value_pair.firstVal == 1 && value_pair.secondVal == 0)
                value_pair.secondVal = 1;
            else {
                value_pair.firstVal = 0;
                value_pair.secondVal = 0;
            }
        }
        update_pair_values = 1;
        currentRepeat++;
    }

    exit(0);
}

void collect_statistics() {
    if (value_pair.firstVal == 0 && value_pair.secondVal == 0)
        count_00++;
    else if (value_pair.firstVal == 0 && value_pair.secondVal == 1)
        count_01++;
    else if (value_pair.firstVal == 1 && value_pair.secondVal == 0)
        count_10++;
    else
        count_11++;
    update_pair_values = 0;
}
