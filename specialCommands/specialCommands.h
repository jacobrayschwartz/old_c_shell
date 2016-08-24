#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../functions.h"


void changeDirectory(Node * head);
void addAlias(char * s, Node ** aliases);
int removeAlias(char * target, Node ** aliases);
int countChars(char * s, char target);
void deAlias(Node ** head, Node * aliases);
char ** split(char * line, const char * delim);
char * removeAllChars(char * s, char target);
void printHistory(Node * head);
void addHistory(char * s);
void deHistory(Node ** head, Node * history);
void changePath(char * in);