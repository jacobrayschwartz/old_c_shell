#pragma once

#include <stdio.h>
#include <stdlib.h>

struct node
{
	void * data;
	struct node * next;

};

typedef struct node Node;

void listAddOrdered(Node * newNode, int (*comparator)(const void *, const void *), Node ** head);
void listAddFirst(Node * newNode, Node ** head);
void listAddLast(Node * newNode, Node ** head);
Node * popFirst(Node ** head);
Node * popLast(Node ** head);
Node * getNode(int target, Node * head);
void sort(Node ** head, int (*comparator)(const void *, const void *));
Node * removeNode(Node ** head, const void * target, int (*comparator)(const void *, const void *));
int listCount(Node * head);