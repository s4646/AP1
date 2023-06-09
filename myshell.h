#pragma once
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
#include "memory.h"

void sigint(int sig);

int parse(char *command, char **argv, int *amper, char **outfile);

int execute(char *command, int *status, char *prompt);