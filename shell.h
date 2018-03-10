//
//  shell.h
//  Shell
//
//  Created by LiberterianRuno on 28/03/2017.
//  Copyright © 2017 LiberterianRuno. All rights reserved.
//

#ifndef shell_h
#define shell_h
#include "shell.h"
#include "parser.h"
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void shell_execute(input*);

void execute_type_one(input);

void execute_type_two(input);

void exucute_command(command, int);

void exucute_subshell(command, int);

void dead_child_catcher();

void background_handler(int , siginfo_t *, void *);

void add_background_process_list(pid_t);

int check_background_process_id(pid_t);

void clear_background_process(pid_t);

void print_background_processes();

int is_there_any_background_process();

void wait_subshell_pipe_pid();

void add_subshell_pipe_pid(pid_t pid);

void execute_type_two_subshell(input args);

void set_subshell_pid_list();

#endif /* shell_h */
