#include "myshell.h"

char prompt[BUFSIZE] = {'\0'};
List *commands, *variables;
Item *command_pointer;
pid_t parent_pid;

// https://stackoverflow.com/questions/18433585/kill-all-child-processes-of-a-parent-but-leave-the-parent-alive
void sigint(int sig)
{
    pid_t self = getpid();
    if (parent_pid != self) // kill childs
    {
        exit(9);
    }
    else
    {
        write(STDOUT_FILENO, "\nYou typed Control-C!\n", 23);
        write(STDOUT_FILENO, prompt, strlen(prompt));
        write(STDOUT_FILENO, ": ", 3);
    }
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
    char *argv[BUFSIZE], *outfile, copy[BUFSIZE];
    int amper, retid, redirect, fd, argc = 0, pipes = 0;
    memcpy(copy, command, BUFSIZE);

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
    if (argc == 0) return 0;

    /* Quit shell */ 
    if (!strcmp(argv[0], "quit"))
    {
        while(1)
        {
            if (commands->head == NULL)
                break;
            if(commands->head->next == NULL)
            {
                free(commands->head);
                break;
            }
            commands->head = commands->head->next;
            free(commands->head->prev);
        }
        while(1)
        {
            if (variables->head == NULL)
                break;
            if(variables->head->next == NULL)
            {
                free(variables->head);
                break;
            }
            variables->head = variables->head->next;
            free(variables->head->prev);
        }
        
        free(commands);
        free(variables);
        exit(0);
    }

    /* Last command */
    if (!strcmp(argv[0], "!!") && argc == 1)
    {
        if (commands->latest != NULL)
            execute(commands->latest->value ,status, prompt);
        return 0;
    }
    else if (argv[0][0] != '\033')
    {
        Item *c = calloc(1, sizeof(Item));
        memcpy(c->key, copy, strlen(copy)); memcpy(c->value, copy, strlen(copy));
        addItem(commands, c); 
    }
    
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
                printf("%d", *status);
            }
            else
            {
                char *output = getItem(variables, argv[1]);
                if (output != NULL)
                    printf("%s", output);
            }
        }
        else if (argc == 4 && !strcmp(argv[1], "Enter") && !strcmp(argv[2], "a") && !strcmp(argv[3], "string"))
        {
            char temp[3][BUFSIZE];
            char var[BUFSIZE];
            scanf("%s %s", temp[0], temp[1]);
            while (getchar() != '\n');
            if (!strcmp(temp[0], "read"))
            {
                scanf("%s", temp[2]);
                while (getchar() != '\n');
                Item *c = calloc(1, sizeof(Item));
                snprintf(var, BUFSIZE+1, "$%s", temp[1]);
                memcpy(c->key, var, strlen(var)); memcpy(c->value, temp[2], strlen(temp[2]));
                addItem(variables, c);
                return 0;
            }
        }
        else
        {
            int i = 0;
            while(argv[++i] != NULL)
            {
                printf("%s ", argv[i]);
            }
        }
        printf("\n");
        return 0;
    }

    /* Change directory */
    if (!strcmp(argv[0], "cd") && argc == 2)
        *status = chdir(argv[1]);

    /* Add variable*/
    if (argc == 3 && argv[0][0] == '$' && !strcmp(argv[1], "="))
    {
        Item *c = calloc(1, sizeof(Item));
        memcpy(c->key, argv[0], strlen(argv[0])); memcpy(c->value, argv[2], strlen(argv[2]));
        addItem(variables, c);
        return 0;
    }

    /* Command navigation */
    if (argc == 1 && argv[0][0] == '\033') // arrow detected
    {
        if (command_pointer == NULL)
            return 0;

        char new_input[BUFSIZE] = {'\0'}, full_command[BUFSIZE] = {'\0'};
        int last_command = 0;
        switch(argv[0][2]) // real value
        {
			case 'A': // arrow up
                printf("\033[1A"); // line up
                printf("\x1b[2K"); // delete line
                printf("%s%s%s", prompt, ": ", command_pointer->value);
                strcat(full_command, command_pointer->value);
                command_pointer = command_pointer->prev != NULL ? command_pointer->prev : command_pointer; // get prev command
			    break;
                
			case 'B': // arrow down
                printf("\033[1A"); // line up
                printf("\x1b[2K"); // delete line
                if (command_pointer == commands->latest)
                {
                    last_command = 1;
                    break;
                }
                command_pointer = command_pointer->next != NULL ? command_pointer->next : command_pointer; // get next command
                printf("%s%s%s", prompt, ": ", command_pointer->value);
                strcat(full_command, command_pointer->value);
			    break;
    	}
        if (last_command) return 0;

        fgets(new_input, BUFSIZE, stdin);

        if (new_input[0] == '\033')
            execute(new_input, status, prompt);
        else
        {
            strcat(full_command, new_input);
            full_command[strlen(full_command) - 1] = '\0';
            execute(full_command, status, prompt);
        }
        return 0;
    }

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

    return retid;
}

int main(int argc, char* argv[])
{
    parent_pid = getpid();
    signal(SIGINT, sigint);
    char command[BUFSIZE];
    memcpy(prompt, "hello", 6);
    int save_in = dup(STDIN_FILENO), save_out = dup(STDOUT_FILENO);
    int status;
    commands = calloc(1, sizeof(List)), variables = calloc(1, sizeof(List));

    while(1)
    {
        printf("%s%s", prompt, ": ");
        
        fgets(command, BUFSIZE, stdin);
        command[strlen(command) - 1] = '\0';

        execute(command, &status, prompt);
        command_pointer = commands->latest;

        dup2(save_in, STDIN_FILENO);
        dup2(save_out, STDOUT_FILENO);
    }
    return 0;   
}