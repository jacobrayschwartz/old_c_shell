#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "./linkedlist/linkedList.h"
#include "./execution/execution.h"
#include "./specialCommands/specialCommands.h"

void strip(char* s);
Node * argsToLinkedList(char * input);
void splitSpecialChars(Node ** head);
void cleanLinkedList(Node ** head);
void cleanLinkedListCharArray(Node ** head);
void cleanCharArray(char ** s);
void printLinkedList(Node * head);
int specialCommands(Node ** input);
void readMyshrc();
void readProfile();
void readHistory();
void writeHistory();