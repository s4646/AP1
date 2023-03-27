#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>

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

    /* Output redirection */
    if (i >= 2 && !strcmp(argv[i - 2], ">"))
    {
        *outfile = argv[i - 1];
        argv[i - 2] = NULL;
    }

    return 0;
}

/* Execute command */
int execute(char *command)
{
    char *argv[BUFSIZ], *outfile;
    int amper, retid, status, redirect, fd;

    if (strchr(command, '>') != NULL)
    {
        redirect = 1;
    }
    else
        redirect = 0;

    parse(command, argv, &amper, &outfile);

    if (fork() == 0)
    {
        /* stdout is redirected into outfile */
        if (redirect == 1)
        {
            fd = open(outfile, O_CREAT|O_TRUNC|O_WRONLY, 0660);
            close(STDOUT_FILENO); 
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
        retid = wait(&status);

    return 0;
}

int main(int argc, char* argv[])
{
    char command[BUFSIZ], pwd[BUFSIZ] = {'\0'};

    while(1)
    {
        // https://stackoverflow.com/questions/298510/how-to-get-the-current-directory-in-a-c-program
        if (getcwd(pwd, BUFSIZ) == NULL)
        {
            perror("Error");
            exit(-1);
        }
        strcat(pwd, "$ ");
        fputs(pwd, stdout);
        bzero(pwd, BUFSIZ);

        fgets(command, BUFSIZ, stdin);
        command[strlen(command) - 1] = '\0';

        execute(command);
    }
    return 0;   
}