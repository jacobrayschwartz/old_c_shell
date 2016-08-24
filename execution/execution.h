#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "../functions.h"

void execute(Node ** input);
int getFin(Node ** head);
int getFout(Node ** head);
char ** linkedListToArray(Node * head);
char ** linkedListToArrayCut(Node * head, int stopBefore);
int getPipeIndex(Node * head);
void generalFork(char ** args, int fin, int fout, int wait);
void pipeIt(char ** leftArgs, char ** rightArgs, int fin, int fout, int wait);