//
//  main.c
//  Shell
//
//  Created by LiberterianRuno on 28/03/2017.
//  Copyright © 2017 LiberterianRuno. All rights reserved.
//

#include "shell.h"
#include "parser.h"
#include <errno.h>
#include  <stdio.h>
#include  <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static void ctrlC_handler(int signum)
{
}

void Shell()
{
    char buffer[MAX_COMMAND_LINE_LENGTH+1];
    char temp;
    int index = 0;

    printf("> ");
    fflush(stdout);
    while(1) {


        ///CTRL+C signal
        struct sigaction ctrlC_action;
        ctrlC_action.sa_handler = ctrlC_handler;
        sigemptyset(&ctrlC_action.sa_mask);
        ctrlC_action.sa_flags = 0;
        sigaction(SIGINT, &ctrlC_action, NULL);

        ///& ampersand signal
        struct sigaction background_action;
        background_action.sa_flags = SA_SIGINFO;
        background_action.sa_sigaction = background_handler;
        sigaction(SIGCHLD, &background_action, NULL);

        if ( scanf("%c", &temp) == EOF)
        {
            if (errno == EINTR)
            {
                errno = 0;
                clearerr(stdin);
                continue;
            }
            else
            {
                break;
            }
        }

        else if ( temp == '\n')
        {
            buffer[index++] = temp;
            buffer[index] = '\0';
            index = 0;

            if(strcmp(buffer,"\n\0")==0)
            {
              printf("> ");
              fflush(stdout);
              continue;
            }
            input* inp = parse(buffer);

            if ( inp->commands->type != SUBSHELL && strcmp(inp->commands->info.com->name, "quit") == 0) {
                while(is_there_any_background_process()){}
                exit(0);
            }

            shell_execute(inp);
            clear_input(inp);
            printf("> ");
            fflush(stdout);
        }
        else
        {
            buffer[index++] = temp;
        }
    }
}


int  main(void)
{
    Shell();

    exit(0);
}
