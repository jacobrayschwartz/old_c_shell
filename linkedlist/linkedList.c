#include "linkedList.h"

void listAddOrdered(Node * newNode, int (*comparator)(const void *, const void *), Node ** head){
	/*Vars to store positions in the list*/
	Node * cur = *head;
	Node * prev = cur;

	/*if the list is empty*/
	if(*head == NULL){
		*head = newNode;
	}
	else{
		/*if the new node is less than the head*/
		if(comparator((*head)->data,newNode->data) >= 0){
			newNode->next = *head;
			*head = newNode;
		}

		/*if the position for the new node lies somewhere else in the list*/
		else{
			while(cur != NULL && comparator(cur->data,newNode->data) < 0){
				prev = cur;
				cur = cur->next;
			}
			prev->next = newNode;
			newNode->next = cur;
		}
	}
}

void listAddFirst(Node * newNode, Node ** head){
	newNode->next = *head;
	*head = newNode;
}

void listAddLast(Node * newNode, Node ** head){
	if(*head == NULL){
		*head = newNode;
	}
	else{
		Node * cur = *head;
		for(;cur->next != NULL; cur = cur->next);
		cur->next = newNode;
	}
}


/*This method will remove the first node from the list, returning its pointer*/
Node * popFirst(Node ** head){
	Node * temp = *head;
	*head = (*head)->next;
	return temp;
}

Node * popLast(Node ** head){
	Node * cur, * prev;
	for(cur = *head, prev = cur; cur->next != NULL; prev=cur, cur=cur->next);
	prev->next = NULL;
	return cur;
}

Node * getNode(int target, Node * head){
	int x;
	Node * cur = head;
	for(x = 0; x < target; x++){
		cur = cur->next;
	}
	return cur;
}


/*Applying bubble sort to the linked list*/
void sort(Node ** head, int (*comparator)(const void *, const void *)){
	Node * cur = NULL;
	Node * stop = NULL;

	while(stop != *head){
		cur = *head;
		while(cur->next != stop && cur->next != NULL){
			if(comparator(cur->data,cur->next->data) > 0){
				void * temp = cur->data;
				cur->data = cur->next->data;
				cur->next->data = temp;
			}
			cur = cur->next;
		}
		stop = cur;
	}
}


Node * removeNode(Node ** head, const void * target, int (*comparator)(const void *, const void *)){
	Node * cur;
	Node * prev;

	cur = *head;
	prev = cur;
	int count = 0;
	if(*head == NULL){
		return NULL;
	}
	else if((comparator(target,cur->data)) == 0){
		*head = (*head)->next;
	}
	else{
		while((comparator(target,cur->data)) != 0){
			count ++;
			prev = cur;
			cur = cur->next;
			if(cur == NULL){
				break;
			}
		}
	}

	if(cur != NULL){
		prev->next = cur->next;
		cur->next = NULL;
	}
	return cur;
}

int listCount(Node * head){
	int count = 0;
	Node * cur = head;

	while(cur != NULL){
		count ++;
		cur = cur->next;
	}

	return count;
}