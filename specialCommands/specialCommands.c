#include "specialCommands.h"


#define MAX 100
extern Node * history;
extern int historySize;


/*
This method changes the working directory
*/
void changeDirectory(Node * head){
	/*First, the path needs to be concatinated if it takes up more than one node*/
	int length = 0;
	Node * cur;
	for(cur = head; cur != NULL; cur = cur->next){
		length += strlen((char *) cur->data);
	}

	char path[length + 1];
	strcpy(path,"\0");
	/*Now contatinating the path strings into path*/
	for(cur = head; cur != NULL; cur = cur->next){
		strcat(path, (char *) cur->data);
	}

	//printf("DEBUG: PATH %s\n",path);
	/*Attempting the chdir*/
	int res = chdir(path);

	if(res != 0){/*If chdir failed*/
		printf("Invalid directory\n");
	}
	char directory[1024];
	getcwd(directory, 1024);
	printf("%s\n", directory);

}

/*
This method will add an alias to the alias list.
the input should be in the form of alias=something
*/
void addAlias(char * s, Node ** aliases){
	if(strstr(s,"=") == 0){
		printf("BAD ALIAS!\n");
		return;
	}

	char * temp1, * temp2;
	temp1 = strtok(s,"=");
	temp2 = strtok(NULL,"=");

	if(temp1 == NULL || temp2 == NULL){
		printf("BAD ALIAS!\n");
		return;
	}


	Node * newNode = (Node *)calloc(1,sizeof(Node));
	char ** pair = (char **)calloc(3,sizeof(char *));

	/*Removing the old alias if it exists*/
	removeAlias(s, aliases);

	/*Adding in the key*/
	pair[0] = (char *)calloc(strlen(temp1) + 1, sizeof(char));
	strcpy(pair[0],temp1);

	/*Adding in the value*/
	pair[1] = (char *)calloc(strlen(temp2) + 1, sizeof(char));
	strcpy(pair[1], temp2);

	pair[2] = NULL;
	/*Adding the alias in*/
	newNode->data = pair;
	listAddFirst(newNode, aliases);
}

/*
This method will look through the list of aliases and remove the target if it exists
*/

int removeAlias(char * target, Node ** aliases){
	Node * cur, * prev;
	char temp[100];
	int removed = 0;

	for(cur = *aliases, prev = cur; cur != NULL; prev = cur, cur = cur->next){
		char ** tempA = (char **) cur->data;
		strcpy(temp, tempA[0]);
		if(strcmp(target,temp) == 0){/*If we have a match*/
			if(cur == *aliases){/*If the alias is at the head*/
				*aliases = (*aliases)->next;
				cur->next = NULL;
				cleanLinkedListCharArray(&cur);
				cur = *aliases;
			}
			else{
				Node * temp = cur;
				prev->next = cur->next;
				cur = prev->next;
				temp->next = NULL;
				cleanLinkedListCharArray(&temp);
			}

			removed = 1;
		}
	}

	return removed;
}

/*
This method counts the number of a particular character in a string
*/
int countChars(char * s, char target){
	if(s == NULL){
		return 0;
	}

	int len = strlen(s), num = 0;

	int i;
	for(i = 0; i < len; i++){
		if(s[i] == target){
			num ++;
		}
	}

	return num;
}


/*
This method will look at the first entry of the command for an alias
If there is one, then it is replaced
The method will loop incase there is an alias inside of the alias
*/
void deAlias(Node ** head, Node * aliases){
	if(*head == NULL){
		return;
	}

	int check = 1;
	while(check){
		check = 0;
		Node * cur;
		for(cur = aliases; cur != NULL; cur = cur->next){
			char ** alias = (char **) cur->data;
			char * data = (char *) (*head)->data;
			if(strcmp(alias[0], data) == 0){/*If there's a match*/
				char temp[100];
				strcpy(temp, alias[1]);
				if(strstr(alias[1],alias[0]) == 0){ /*If the alias doesn't include itself*/
					Node * dealiased = argsToLinkedList(temp);

					/*Freeing the first node in the list*/
					Node * tempNode = popFirst(head);
					char * tempData = (char *) tempNode->data;
					free(tempData);
					free(tempNode);
					tempNode = NULL;

					for(tempNode = dealiased; tempNode->next != NULL; tempNode = tempNode->next);

					tempNode->next = *head;
					*head = dealiased;
					check = 1;
					break;
				}
			}
		}
	}
}

/*
This method will return a char ** from a char * split on the deliminator, ending in a NULL
*/
char ** split(char * line, const char * delim){
	char * copy; /*A copy of the string to count how many sections there are*/
	char ** temp; /*The array that will be returned*/
	int count; /*The number of sections*/
	char * res; /*Variable to store the results from strtok*/

	/*copying the line into copy*/
	copy = (char *)calloc(strlen(line) + 1, sizeof(char));
	strcpy(copy,line);

	/*conting the sections*/
	count = 0;
	res = strtok(copy, delim);
	while(res != NULL){
		count ++;
		res = strtok(NULL, delim);
	}

	free(copy);
	copy = NULL;

	copy = (char *)calloc(strlen(line) + 1, sizeof(char));
	strcpy(copy,line);
	/*now creating the array*/
	res = strtok(copy, delim);
	temp = (char **)calloc(count + 1,sizeof(char*));

	temp[0] = (char *)calloc(strlen(res) + 1, sizeof(char));
	strcpy(temp[0],res);

	int i;
	for(i = 1; i < count; i ++){
		res = strtok(NULL, delim);
		temp[i] = (char *)calloc(strlen(res) + 1, sizeof(char));
		strcpy(temp[i],res);
	}

	temp[count] = NULL;

	free(copy);
	return temp;
}


/*
This method will return a copy of a string with all the occurances of a target removed
NOTE TO SELF: THE RETURN IS CALLOC'ED!
*/
char * removeAllChars(char * s, char target){
	Node * head = NULL;
	int len = strlen(s), i;
	for(i = 0; i < len; i ++){
		if(s[i] != target){/*If it is not the taget*/
			char temp[2];
			temp[0] = s[i];
			temp[1] = '\0';
			

			Node * newNode = (Node *)calloc(1,sizeof(Node));
			newNode->data = (char*)calloc(2,sizeof(char));/*Giving space for 2 chars, the char found and the terminator*/
			strcpy((char *) newNode->data, temp);

			listAddLast(newNode,&head);
		}
	}

	int newLen = listCount(head) + 1;
	char * temp = (char *)calloc(newLen,sizeof(char));
	strcpy(temp, "\0");

	for(i = 0; head != NULL; i++){
		Node * tempNode = popFirst(&head);
		char * tempData = (char *) tempNode->data;
		//printf("DEBUG: %s\n",tempData);
		strcat(temp, tempData);
		free(tempData);
		free(tempNode);
		tempNode = NULL;
	}
	return temp;
}

void printHistory(Node * head){
	int i;
	Node * cur = head;
	for(i = 0; cur != NULL; i++, cur = cur->next){
		printf("%d  %s\n",i,(char *) cur->data);
	}
}

void addHistory(char * s){
	char temp[MAX];
	strcpy(temp,s);

	if(s == NULL){
		return;
	}
	if(strcmp(s,"!!") == 0){/*If we are recalling the last command, it doesn't need to be added*/
		return;
	}
	if(s[0] == '!'){
		int target = atoi(s + 1);
		if(target >= 0 && target < listCount(history)){/*If the requested command is within the history file, the actual command is coppied*/
			/*Copying the data into the new history node*/
			Node * tNode = getNode(target, history);
			char * tempData = (char *) tNode->data;
			strcpy(temp,tempData);
		}
		else{/*If it's not in the history*/
			return;
		}
	}

	/*Adding the node into the linked list*/
	Node * newNode = (Node *)calloc(1,sizeof(Node));
	char * newHist = (char *)calloc(strlen(temp) + 1,sizeof(char));
	strcpy(newHist,temp);
	newNode->data = newHist;

	if(listCount(history) == historySize){ /*If history is full, the oldest entry is removed*/
		Node * tempNode = popFirst(&history);
		char * tempData = (char *) tempNode->data;
		free(tempData);
		free(tempNode);
		tempNode = NULL;
	}

	listAddLast(newNode, &history);

}

void deHistory(Node ** head, Node * history){
	/*If history is empty, return*/
	if(history == NULL){
		return;
	}

	Node * cur;
	Node * prev;
	cur = *head;
	prev = cur;
	while(cur != NULL){
		char temp[MAX];
		strcpy(temp,  (char *) cur->data);
		if(temp[0] == '!' && strlen(temp) > 1){/*If the first character is a ! and there are characters after that*/

			char histEntry[MAX];

			if(temp[1] == '!'){/*If the second character is another !, recall the last command*/
				/*Getting the last entry in history*/
				Node * histNode = getNode(listCount(history) - 1, history);
				strcpy(histEntry, (char *) histNode->data);
			}
			else{
				char charNum[MAX];
				strcpy(charNum, temp + 1); /*Getting the everything from the string after the !*/

				int histNum = atoi(charNum);
				if(histNum >= 0 && histNum < listCount(history)){/*If the number is valid*/
					Node * histNode = getNode(histNum, history);/*Getting the entry at that point*/
					strcpy(histEntry, (char *) histNode->data);
				}
				else{
					return;
				}
			}

			if(histEntry == NULL){
				return;
			}

			if(strcmp(histEntry, "") != 0){/*If there is something there*/
				/*Getting the linked list form of that entry*/
				Node * newPart = argsToLinkedList(histEntry);

				if(cur == *head){
					Node * tempNode = popFirst(head);
					char * tempData = (char *) tempNode->data;
					free(tempData);
					free(tempNode);

					for(tempNode = newPart; tempNode->next != NULL; tempNode = tempNode->next);
					listAddFirst(tempNode, head);
					*head = newPart;
					cur = *head;
				}
				else{
					prev->next = newPart;
					for(; newPart->next != NULL; newPart = newPart->next);
					newPart->next = cur->next;
					cur->next = NULL;
					cleanLinkedList(&cur);
					cur = newPart;
				}
			}
		}

		prev = cur;
		cur = cur->next;
	}
}


/*
This function allows the user to change the PATH in the environment
*/
void changePath(char * in){
	char newPath[MAX + strlen(getenv("PATH")) + 5];
	strcpy(newPath, "\0");

	char * res;
	res = strtok(in, "=");/*Removing the PATH= from the string*/
	if(strcmp(res,"PATH") != 0){/*Ensuring that the path statement is valid*/
		fprintf(stderr,"BAD PATH STATEMENT!\n");
		return;
	}


	res = strtok(NULL,":");
	while(res != NULL){
		if(strcmp(res,"$PATH") == 0){/*If the current path is in here*/
			strcat(newPath,getenv("PATH"));
		}
		else{
			strcat(newPath,res);
		}

		res = strtok(NULL,":");
		if(res != NULL){
			strcat(newPath,":");
		}
	}

	
	int valid = setenv("PATH",newPath,1);
	if(valid != 0){
		printf("Something went wrong!\n");
	}
}