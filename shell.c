//
//  shell.c
//  Shell
//
//  Created by LiberterianRuno on 28/03/2017.
//  Copyright © 2017 LiberterianRuno. All rights reserved.
//

#include "shell.h"
#include "parser.h"
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_COMMAND_LINE_LENGTH 512
int background_processes[80];
int subshell_pipe_pid[80];
int background_processes_number=0;
pid_t parent = -1;



void set_subshell_pid_list()
{
  int i;
  for( i=0 ; i<80 ; i++)
      subshell_pipe_pid[i]=-2;
}

void add_subshell_pipe_pid(pid_t pid)
{
    int i;
    for( i=0 ; i<80 ; i++)
    {
        if(subshell_pipe_pid[i]==-2)
        {
          subshell_pipe_pid[i]=pid;
          break;
        }
    }
}

void wait_subshell_pipe_pid()
{
    int i;
    for( i=0 ; i<80 ; i++)
    {
      if(subshell_pipe_pid[i]!=-2)
      {
          //kill(subshell_pipe_pid[i],0);
          waitpid(subshell_pipe_pid[i],NULL,0);
          subshell_pipe_pid[i]=-2;
      }
    }
}

int is_there_any_background_process()
{
    if(background_processes_number)
        return 1;

    return 0;
}

void print_background_processes()
{
    int i;
    for( i=0 ; i<80 ; i++ )
    {
        if(background_processes[i])
        {
            printf("[%d]\n",background_processes[i]);
            fflush(stdout);
        }
    }
}

void add_background_process_list(pid_t pid)
{
    for( int i=0 ; i<80 ; i++ )
    {
        if(background_processes[i]==0)
        {
            background_processes[i] = pid;
            background_processes_number++;
            break;
        }
    }
}

int check_background_process_id(pid_t pid)
{
    for( int i=0 ; i<80 ; i++ )
    {
        if(background_processes[i]==pid)
            return 1;
    }
    return 0;
}

void clear_background_process(pid_t pid)
{
    for(int i=0;i<80;i++)
    {
        if(background_processes[i]==pid){
            background_processes[i]=0;
            background_processes_number--;
			break;
		}
    }
}

void background_handler(int sig, siginfo_t *si, void *context)
{
    int status;
    pid_t pid;

    for ( pid = waitpid(-1,&status,WNOHANG) ; pid != 0 && pid != -1 ; pid = waitpid(-1,&status,WNOHANG) )
    {
        if(check_background_process_id(pid))
        {
            printf("[%d] retval: %d\n",pid,WEXITSTATUS(status));
            fflush(stdout);
            clear_background_process(pid);
        }
    }
}

int file_operations(command command)
{
    int in, out, result=0;

    if(command.output != NULL && command.input != NULL)
    {
        in = open(command.input, O_RDONLY);

        if(in == -1)
        {
            printf("%s not found\n", command.input);
            fflush(stdout);
            close(in);
            return -2;
        }


        dup2(in,STDIN_FILENO);
        close(in);

        //output file settings
        out = open(command.output, O_RDWR | O_CREAT | O_TRUNC , 0644);
        dup2( out, STDOUT_FILENO );
        close( out );

        result = 0;
    }
    else if(command.output != NULL && command.input == NULL)
    {
        out = open(command.output, O_RDWR | O_CREAT | O_TRUNC, 0644);
        dup2( out, STDOUT_FILENO );
        close( out );

        result = 0;

    }
    else if(command.output == NULL && command.input != NULL)
    {
        in = open(command.input, O_RDONLY);

        if(in == -1)
        {
            printf("%s not found\n", command.input);
            fflush(stdout);
            close(in);
            return -2;
        }

        dup2(in,STDIN_FILENO);
        close(in);

        result = 0;
    }
    return result;
}

char** generate_path(command command,int dim)
{
    int i;

    char **arr = (char**) calloc(dim, sizeof(char*));

    arr[0] = (char*) calloc(strlen(command.info.com->name), sizeof(char));
    arr[0] = command.info.com->name;

    for ( i = 1; i < dim-1; i++ )
    {
        arr[i] = (char*) calloc(strlen(command.info.com->arguments[i-1]), sizeof(char));
        arr[i] = command.info.com->arguments[i-1];
    }
    arr[dim-1] = NULL;

    return arr;
}

void execute_command(command command,int block)
{
    int child_status, file_result=0;
    char** path;
    pid_t pid;

    pid = fork();



    if (pid == 0) {
        file_result = file_operations(command);

        if(file_result== -2)
        {
            return;
        }

        path = generate_path(command, command.info.com->num_of_args+2+file_result);


        if( strcmp(command.info.com->name,"lbp")!=0 && execvp(command.info.com->name, path)==-1)
            perror("execvp error\n");

        else
            print_background_processes();

        exit(0);

    }
    else if(pid < 0){
        perror("pid < 0 error\n");
    }
    else
    {
        if(block)
        {
            add_background_process_list(pid);
            setpgid(pid,pid);
        }
        else
        {
            waitpid(pid, &child_status, 0);
        }
    }
}

void execute_subshell(command command,int block)
{

    int child_status, file_result=0;
    input* inp;
    pid_t pid;

    pid = fork();



    if (pid == 0) {

        inp = parse(command.info.subshell);
        file_result = file_operations(command);

        if(file_result== -2)
        {
            return;
        }


        shell_execute(inp);
        clear_input(inp);
        exit(0);

    }
    else if(pid < 0){
        perror("pid < 0 error\n");
    }
    else
    {

        if(block)
        {
            add_background_process_list(pid);
            setpgid(pid,pid);
        }
        else
        {
            waitpid(pid, &child_status, 0);
        }
    }

}

void execute_type_one(input args)
{
    int i;
    for ( i=0 ; i<args.num_of_commands ; i++ ) {
        if((args.commands+i)->type == NORMAL)
        {
            execute_command(*(args.commands+i), args.background);
        }
        else
        {
            execute_subshell(*(args.commands+i),args.background);
        }
    }
}

void execute_type_two(input args)
{
    int i, j=0, t;
    int pid, status, file_result;
    char** path;
    int pipe_array[2*(args.num_of_commands-1)];
    input* inp;

    for(i = 0; i < (args.num_of_commands-1); i++)
    {
        pipe(pipe_array + i*2);
    }

    for (i = 0; i < args.num_of_commands; ++i)
    {
        pid = fork();
        if(pid == 0)
        {
            //Except last command
            if(i < args.num_of_commands-1)
            {
                dup2(pipe_array[j + 1], 1);
            }

            //Except first command
            if(j != 0 )
            {
                dup2(pipe_array[j-2], 0);
            }

            for(t = 0; t < 2*(args.num_of_commands-1); t++)
            {
                close(pipe_array[t]);
            }

            file_result = file_operations(*(args.commands+i));

            if(file_result== -2)
            {
                return;
            }

            path = generate_path(*(args.commands+i), ((args.commands+i)->info.com->num_of_args)+2+file_result);


            if(execvp((args.commands+i)->info.com->name, path)<0)
            {
                perror("execvp error\n");
            }

            exit(1);
        }
        else if(pid < 0)
        {
            perror("pid < 0 error\n");
        }

        j = j+2;
    }

    //close pipes
    for(i = 0; i < 2 * (args.num_of_commands-1); i++)
    {
        close(pipe_array[i]);
    }

    //for all pipes
	  for(i = 0; i < args.num_of_commands ; i++)
	  {
		    wait(&status);
	  }

}

void execute_type_two_subshell(input args)
{
    int i, j=0, t;
    int pid, status, file_result;
    char** path;
    int pipe_array[2*(args.num_of_commands-1)];
    input* inp;

    set_subshell_pid_list();

    for(i = 0; i < (args.num_of_commands-1); i++)
    {
        pipe(pipe_array + i*2);
    }

    for (i = 0; i < args.num_of_commands; ++i)
    {
        pid = fork();
        if(pid == 0)
        {
            //Except last command
            if(i < args.num_of_commands-1)
            {
                dup2(pipe_array[j + 1], 1);
            }

            //Except first command
            if(j != 0 )
            {
                dup2(pipe_array[j-2], 0);
            }

            for(t = 0; t < 2*(args.num_of_commands-1); t++)
            {
                close(pipe_array[t]);
            }

            inp = parse((args.commands+i)->info.subshell);

            shell_execute(inp);

            clear_input(inp);

            exit(1);

        }
        else if(pid < 0)
        {
            perror("pid < 0 error\n");
        }
        else
        {
          add_subshell_pipe_pid(pid);
        }

        j = j+2;
    }

    //close pipes
    for(i = 0; i < 2 * (args.num_of_commands-1); i++)
    {
        close(pipe_array[i]);
    }

    //for all pipes
	  wait_subshell_pipe_pid();

}

void shell_execute(input* args)
{

    if(args->del==';' || args->del=='\0')
    {
        execute_type_one(*args);
    }
    else /// | deliminator
    {
        if(args->commands->type == NORMAL)
        {
          execute_type_two(*args);
        }
        else
        {
          execute_type_two_subshell(*args);
        }
    }
}
