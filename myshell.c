#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <signal.h>

#define BUFSIZE 128

char prompt[BUFSIZE] = {'\0'};

void sigint(int sig)
{
    write(STDOUT_FILENO, "\nYou typed Control-C!\n", 23);
    write(STDOUT_FILENO, prompt, strlen(prompt));
    write(STDOUT_FILENO, ": ", 3);
    // exit(9);
}

/* Parse command */
int parse(char *command, char **argv, int *amper, char **outfile)
{
    char *token;
    int i = 0;
    
    token = strtok(command, " ");
    while(token != NULL)
    {
        argv[i] = token;
        token = strtok(NULL, " ");
        i++;
    }
    argv[i] = NULL;

    /* Command empty */ 
    if (argv[0] == NULL)
        return 0;

    /* Quit shell */ 
    if (!strcmp(argv[0], "quit"))
        exit(0);

    /* Command end with & */ 
    if (!strcmp(argv[i - 1], "&"))
    {
        *amper = 1;
        argv[i - 1] = NULL;
    }
    else 
        *amper = 0;

    /* IO redirection */
    if (i >= 2 && (!strcmp(argv[i - 2], ">") || !strcmp(argv[i - 2], "2>") || !strcmp(argv[i - 2], ">>") || !strcmp(argv[i - 2], "<")))
    {
        *outfile = argv[i - 1];
        argv[i - 2] = NULL;
    }

    return 0;
}

/* Execute command */
int execute(char *command, int *status, char *prompt)
{
    char *argv[BUFSIZE], *outfile;
    int amper, retid, redirect, fd, argc = 0, pipes = 0;

    char *pch = strchr(command, '|'); // count pipes
    while (pch != NULL)
    {
        pipes++;
        pch = strchr(pch + 1, '|');
    }

    if (strchr(command, '>') != NULL) // check for redirection
    {
        redirect = 1;
         if (strstr(command, "2>") != NULL)
            redirect = 2;
        else if (strstr(command, ">>") != NULL)
            redirect = 3;
    }
    else if (strchr(command, '<') != NULL)
    {
        redirect = -1;
    }
    else
        redirect = 0;

    /* Piping */
    if (pipes > 0)
    {
        int i = 0;
        char *commands[pipes+1];
        char *token = strtok(command, "|");
        while(token != NULL)
        {
            commands[i++] = token;
            token = strtok(NULL, "|");
        }
        // https://stackoverflow.com/questions/21914632/implementing-pipe-in-c
        for(i = 0; i < pipes; i++)
        {
            int pfd[2];
            pipe(pfd);

            if (!fork())
            {
                dup2(pfd[1], 1); // remap output back to parent
                close(pfd[1]);
                execute(commands[i], status, prompt);
                exit(0);
            }

            // remap output from previous child to input
            dup2(pfd[0], 0);
            close(pfd[1]);
            retid = wait(status);
        }
        execute(commands[i], status, prompt);

        return 0;
    }
    
    parse(command, argv, &amper, &outfile);
    while(argv[argc] != NULL) argc++; // count argc

    /* Prompt change */
    if (!strcmp(argv[0], "prompt") && !strcmp(argv[1], "=") && argc == 3)
    {
        bzero(prompt, BUFSIZE);
        memcpy(prompt, argv[2], strlen(argv[2]));
        return 0;
    }
    
    /* Echo */
    if (!strcmp(argv[0], "echo"))
    {
        if (argc == 2 && argv[1][0] == '$')
        {
            if (!strcmp(argv[1], "$?"))
            {
                char num[BUFSIZE] = {'\0'};
                sprintf(num, "%d", *status);
                write(STDOUT_FILENO, num, strlen(num));
            } 
        }
        else
        {
            int i = 0;
            while(argv[++i] != NULL)
            {
                write(STDOUT_FILENO, argv[i], strlen(argv[i]));
                write(STDOUT_FILENO, " ", 2);
            }
        }
        write(STDOUT_FILENO, "\n", 2);
        return 0;
    }

    /* Change directory */
    if (!strcmp(argv[0], "cd") && argc == 2)
        *status = chdir(argv[1]);
    
    if (fork() == 0)
    {
        /* Redirection */
        if (redirect == 1) // stdout is redirected into outfile
        {
            fd = open(outfile, O_CREAT|O_TRUNC|O_WRONLY, 0660);
            close(STDOUT_FILENO); 
            dup(fd); 
            close(fd);
        }
        else if (redirect == 2) // stderr is redirected into outfile
        {
            fd = open(outfile, O_CREAT|O_TRUNC|O_WRONLY, 0660);
            close(STDERR_FILENO); 
            dup(fd); 
            close(fd);
        }
        else if (redirect == 3) // stdout is appended into outfile
        {
            fd = open(outfile, O_CREAT|O_APPEND|O_WRONLY, 0660);
            close(STDOUT_FILENO); 
            dup(fd); 
            close(fd);
        }
        else if (redirect == -1) // stdin redirected into outfile
        {
            fd = open(outfile, O_RDONLY, 0);
            close(STDIN_FILENO);
            dup(fd);
            close(fd);
        }
        
        if (execvp(argv[0], argv) == -1)
            exit(EXIT_FAILURE);
        else
            exit(EXIT_SUCCESS);
    }
    /* parent continues here */
    if (amper == 0)
        retid = wait(status);

    return 0;
}

int main(int argc, char* argv[])
{
    signal(SIGINT, sigint);
    char command[BUFSIZE];
    memcpy(prompt, "hello", 6);
    int save_in = dup(STDIN_FILENO), save_out = dup(STDOUT_FILENO);
    int status;

    while(1)
    {
        write(STDOUT_FILENO, prompt, strlen(prompt));
        write(STDOUT_FILENO, ": ", 3);

        fgets(command, BUFSIZE, stdin);
        command[strlen(command) - 1] = '\0';

        execute(command, &status, prompt);

        dup2(save_in, STDIN_FILENO);
        dup2(save_out, STDOUT_FILENO);
    }
    return 0;   
}