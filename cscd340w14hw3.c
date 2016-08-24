#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions.h"

#define MAX 100

int historySize = 2;
int historyFileSize = 5;
Node * history = NULL;
Node * aliases = NULL;
char homeDirectory[MAX];

int main()
{
	/*Getting the "home directory"*/
	getcwd(homeDirectory,MAX);
	/*Reading in the preset files*/
	readProfile();
	readMyshrc();
	readHistory();

	char s[MAX];
	printf("?: ");
	fgets(s, MAX, stdin);
	Node * input = NULL;

	

 	while(1)
  	{
  		/*Stripping out the newline*/
		strip(s);

		/*Check for "exit"*/
		if(strcmp(s, "exit") == 0){
			break;
		}
		else if(strcmp(s, "") != 0){
			/*Convert input to LinkedList*/
			input = argsToLinkedList(s);

			/*Replacing the history recals*/
			deHistory(&input, history);

			/*De-alias*/
			deAlias(&input, aliases);

			//printLinkedList(input);  /*Debugging*/

			/*Check for special commands*/
			int res = specialCommands(&input);

			/*Finish the parsing and execute the commands*/
			if(res == 0){
				execute(&input);
			}

			/*Adding the command to history*/
			addHistory(s);

			/*Empty the linked lists*/
			cleanLinkedList(&input);
		}
		printf("?: ");
		fgets(s, MAX, stdin);

	}// end while
	writeHistory();

	cleanLinkedList(&history);
	cleanLinkedListCharArray(&aliases);

	return 0;

}// end main


void strip(char* s){
	int len = strlen(s);

	if(s[len - 2] == '\r'){ /*if the string has a windows style new line*/
		s[len - 2] = '\0';
	}

	else if(s[len - 1] == '\n'){/*if the string ends with \n*/
		s[len - 1] = '\0';
	}
}

Node * argsToLinkedList(char * input){
	/*First splitting on spaces*/
	char inputCopy[MAX];
	strcpy(inputCopy, input);
	char ** splitArgs = split(inputCopy, " ");
	Node * head = NULL;

	/*Itterating through the array, creating the linked list*/
	int i;
	for(i = 0; splitArgs[i] != NULL; i++){

		if(countChars(splitArgs[i],'\"') % 2 != 0){/*If the split cut a phrase with "s apart, we need to put it back together*/
			int start = i, stop;
			for(stop = i + 1; splitArgs[stop] != NULL && countChars(splitArgs[stop], '\"') % 2 == 0; stop ++); /*itterating stop until the next " is found or until the end of the list is hit*/

			if(splitArgs[stop] == NULL){ /*reposition stop if it is at the end to not segfault*/
				stop --;
			}

			/*new concating them together*/
			int j, size = 0;
			for(j = start; j <= stop; j++){
				size += strlen(splitArgs[j]) + 1;/*The +1 is added for the space character*/
			}
			char temp[size]; /*+1 is not neede because of the extra +1 for the space at the end*/
			strcpy(temp, "\0");
			for(j = start; j <= stop; j++){
				strcat(temp, splitArgs[j]);
				if(j != stop){
					strcat(temp, " "); /*Adding the space back in*/
				}
			}
			i = stop; /*Advancing i to the next token*/
			
			/*if there is only a quote in the string*/
			if(strlen(temp) < 2){
				fprintf(stderr,"BAD INPUT!\n");
				break;
			}

			/*Getting rid of the quotes*/
			char * tempNoQuote = removeAllChars(temp, '\"');

			/*Adding the new node to the list*/
			Node * newNode = (Node *)calloc(1,sizeof(Node));
			newNode->data = calloc(strlen(tempNoQuote) + 1, sizeof(char));
			strcpy(newNode->data,tempNoQuote);
			listAddLast(newNode,&head);
			free(tempNoQuote);
		}/*End " check*/

		/*Now we get to do that for 's*/
		else if(countChars(splitArgs[i],'\'') % 2 != 0){/*If the split cut a phrase with "s apart, we need to put it back together*/
			int start = i, stop;
			for(stop = i + 1; splitArgs[stop] != NULL && countChars(splitArgs[stop], '\'') % 2 == 0; stop ++); /*itterating stop until the next " is found or until the end of the list is hit*/

			if(splitArgs[stop] == NULL){ /*reposition stop if it is at the end to not segfault*/
				stop --;
			}

			/*now concating them together*/
			int j, size = 0;
			for(j = start; j <= stop; j++){
				size += strlen(splitArgs[j]) + 1; /*The +1 is added for the space character*/
			}
			char temp[size]; /*+1 is not neede because of the extra +1 for the space at the end*/
			strcpy(temp, "\0");
			for(j = start; j <= stop; j++){
				strcat(temp, splitArgs[j]);
				if(j != stop){
					strcat(temp, " "); /*Adding the space back in*/
				}
			}
			i = stop; /*Advancing i to the next token*/

			/*if there is only a quote in the string*/
			if(strlen(temp) < 2){
				fprintf(stderr,"BAD INPUT!\n");
				break;
			}

			/*Getting rid of the quotes*/
			char * tempNoQuote = removeAllChars(temp, '\'');

			/*Adding the new node to the list*/
			Node * newNode = (Node *)calloc(1,sizeof(Node));
			newNode->data = calloc(strlen(tempNoQuote) + 1, sizeof(char));
			strcpy(newNode->data,tempNoQuote);
			listAddLast(newNode,&head);
			free(tempNoQuote);
		}/*End ' check*/

		else{
			/*Removing any quotes*/
			char temp[MAX];
			strcpy(temp, splitArgs[i]);
			char * tempNoQuote = removeAllChars(temp, '\"');
			strcpy(temp,tempNoQuote);
			free(tempNoQuote);
			tempNoQuote = removeAllChars(temp,'\'');

			/*Adding the new node*/
			Node * newNode = (Node *)calloc(1,sizeof(Node));
			newNode->data = calloc(strlen(tempNoQuote) + 1, sizeof(char));
			strcpy(newNode->data,tempNoQuote);
			listAddLast(newNode,&head);

			free(tempNoQuote);
			tempNoQuote = NULL;
		}
	}

	cleanCharArray(splitArgs);

	/*Fixing any segments that contain special characters that weren't separated by a space*/
	splitSpecialChars(&head);

	return head;
}

/*
This method will look for segments that contain special chars along with other text
This is needed because at the beginning I split on spaces only
*/
void splitSpecialChars(Node ** head){
	Node * cur, * prev;
	cur = *head;
	prev = cur;
	while(cur != NULL){
		char temp[100];
		char target = 0;
		strcpy(temp, (char *) cur->data);

		 /*
		 This if block will look for any of <, > or | in the string.
		 If it contains the char and its size is not 1, it needs to be split up
		 Then cur is set to the BEGINNING of the string in the linked list incase there were multiple special characters
		 That is why this is a while loop and not a for loop
		 */
		 if(strstr(temp,"<") != 0 && strlen(temp) > 1){
		 	target = '<';
		 }
		 else if(strstr(temp,">") != 0 && strlen(temp) > 1){
		 	target = '>';
		 }
		 else if(strstr(temp, "|") != 0 && strlen(temp) > 1){
		 	target = '|';
		 }
		 else if(strstr(temp, "&") != 0 && strlen(temp) > 1){
		 	target = '&';
		 }


		 if(target != 0){/*if one of the above conditions was met*/
		 	/*Splitting up the args on the target char*/
		 	char targetHolder[2];
		 	targetHolder[0] = target;
		 	targetHolder[1] = '\0';
		 	char ** newArgs = split(temp, targetHolder);
		 	
		 	/*Now creating a new linked list to insert*/
		 	Node * insert = NULL;

		 	Node * tempNode = NULL;
		 	if(temp[0] != target){/*If the segment does not start with the target, there is something before the target*/
		 		tempNode = (Node *)calloc(1,sizeof(Node));
		 		tempNode->data = (char *)calloc(strlen(newArgs[0]) + 1, sizeof(char));
		 		strcpy((char *) tempNode->data, newArgs[0]);
		 		listAddLast(tempNode, &insert);
		 		tempNode = NULL;
		 	}
		 	 /*Adding the target*/
		 	tempNode = (Node *)calloc(1,sizeof(Node));
		 	tempNode->data = (char *)calloc(2,sizeof(char));
		 	strcpy((char *) tempNode->data, targetHolder);
		 	listAddLast(tempNode, &insert);
		 	tempNode = NULL;

		 	if(temp[strlen(temp) - 1] != target){/*If the target is not at the end, there is something after it*/
		 		int arg;
		 		/*Getting the right index for this part, since there wasn't necisarrily a first half*/
		 		arg = (newArgs[1] == NULL) ? 0:1;

		 		tempNode = (Node *)calloc(1,sizeof(Node));
		 		tempNode->data = (char *)calloc(strlen(newArgs[arg]) + 1, sizeof(char));
		 		strcpy((char *) tempNode->data, newArgs[arg]);
		 		listAddLast(tempNode, &insert);
		 		tempNode = NULL;
		 	}

	 		/*Now inserting it into the input linked list*/
	 		/*If we are at the head of the linked list*/
	 		if(cur == *head){
	 			Node * toDelete = popFirst(head);
	 			cur = insert;
	 			for(;insert->next != NULL; insert = insert->next);
	 			listAddFirst(insert, head);

	 			toDelete->next = NULL;
	 			cleanLinkedList(&toDelete);
	 			*head = cur;
	 		}
	 		/*If we are after the head of the linked list*/
	 		else{
	 			Node * toDelete = cur;
	 			prev->next = insert;
	 			for(;insert->next != NULL; insert = insert->next);
	 			insert->next = cur->next;
	 			cur = prev->next;

	 			toDelete->next = NULL;
	 			cleanLinkedList(&toDelete);
	 		}

	 		cleanCharArray(newArgs);

		 }
		 /*If none of the conditions were met, it is safe to move on*/
		 else{
		 	prev = cur;
		 	cur = cur->next;
		 }
	}
}

void cleanLinkedList(Node ** head){
	while(*head != NULL){
		Node * temp = popFirst(head);
		if(temp->data != NULL){
			char * arg = (char *) temp->data;
			free(arg);
		}
		free(temp);
	}
}

void cleanLinkedListCharArray(Node ** head){
	while(*head != NULL){
		Node * temp = popFirst(head);
		if(temp->data != NULL){
			char ** arg = (char **) temp->data;
			cleanCharArray(arg);
		}
		free(temp);
	}
}

void cleanCharArray(char ** s){
	if(s == NULL){
		return;
	}

	int i;
	for(i = 0; s[i] != NULL; i++){
		free(s[i]);
		s[i] = NULL;
	}
	free(s);
	s = NULL;
}

/*
debuging method to make sure that the list is made correctly, prints the contents and then size of the list
*/
void printLinkedList(Node * head){
	Node * cur = head;
	printf("DEBUG:\n");
	while(cur != NULL){
		printf("%s\n",(char *) cur->data);
		cur = cur->next;
	}
	printf("\nCount: %d\n\n", listCount(head));
}


/*
This method will run through the special commands
cd
alias
unalias
history
! (recall)
PATH=
It returns either 1 to signal that a special command was run and that there is no execution needed
               or 0 to signal that execution is needed
*/
int specialCommands(Node ** input){
	int ex = 1;
	if(*input == NULL || (*input)->data == NULL){
		return ex;
	}

	if(strcmp("cd", (char *) (*input)->data) == 0){/*If this is a cd command*/
		changeDirectory((*input)->next);
	}
	else if(strcmp("alias", (char *) (*input)->data) == 0){/*If this is the alias command*/
		if((*input)->next == NULL){
			printf("BAD ALIAS!\n");
		}
		else{
			addAlias((char *) (*input)->next->data, &aliases);
		}
	}
	else if(strcmp("unalias", (char *) (*input)->data) == 0){/*If this is the alias command*/
		if((*input)->next == NULL){
			printf("BAD ALIAS!\n");
		}
		else{
			int removed = removeAlias((char *) (*input)->next->data, &aliases);
			if(!removed){
				printf("No alias %s!\n", (char *) (*input)->next->data);
			}
		}
	}
	else if(strcmp("history", (char *) (*input)->data) == 0 && (*input)->next == NULL){
		printHistory(history);
	}
	else if(strstr((char *) (*input)->data, "PATH=") != 0 && (*input)->next == NULL){
		changePath((char *) (*input)->data);
	}
	else{
		ex = 0;
	}

	return ex;
}

/*
This function runs through the .myrc file if it exists
It runs the commands line by line
*/
void readMyshrc(){
	FILE * myshrc = fopen(".myshrc","r");
	char s[MAX];
	strcpy(s,"\0");

	if(myshrc == NULL){/*If the file doesn't exist, return and start*/
		return;
	}

	/*From here we act just like a user entered the lines in on the command line*/
	fgets(s,MAX,myshrc);
	/*Reading in the lines one by one*/
	while(!feof(myshrc)){
		strip(s);

		if(strstr(s,"HISTCOUNT=")){/*if it is setting the hist count size*/
			char * res = strtok(s,"HISTCOUNT=");
			int i = atoi(res);

			if(i > 0){
				historySize = i;
			}
			else{
				fprintf(stderr, "BAD HISTORY COUNT!\n");
			}
		}
		else if(strstr(s,"HISTFILECOUNT=")){/*Same as above but with the history file*/
			char * res = strtok(s,"HISTFILECOUNT=");
			int i = atoi(res);

			if(i > 0){
				historyFileSize = i;
			}
			else{
				fprintf(stderr, "BAD HISTORY FILE COUNT!\n");
			}
		}

		else{
			Node * input = NULL;
			

			/*Check for "exit"*/
			if(strcmp(s, "exit") == 0){
				break;
			}
			else if(strcmp(s, "") != 0){
				/*Convert input to LinkedList*/
				input = argsToLinkedList(s);

				/*Replacing the history recals*/
				deHistory(&input, history);

				/*De-alias*/
				deAlias(&input, aliases);

				//printLinkedList(input);  /*Debugging*/

				/*Check for special commands*/
				int res = specialCommands(&input);

				/*Finish the parsing and execute the commands*/
				if(res == 0){
					execute(&input);
				}

				/*Empty the linked lists*/
				cleanLinkedList(&input);
			}

		}

		fgets(s,MAX,myshrc);
	}

	fclose(myshrc);
}

void readProfile(){
	char temp[MAX];
	FILE * profile = fopen(".profile","r");

	if(profile == NULL){
		return;
	}

	fgets(temp,MAX,profile);
	while(!feof(profile)){
		strip(temp);
		if(strcmp("",temp) != 0){
			changePath(temp);
		}
		fgets(temp,MAX,profile);
	}

	fclose(profile);
}

void readHistory(){
	char temp[100];
	char histPath[strlen(homeDirectory) + 15];
	strcpy(histPath, homeDirectory);
	strcat(histPath, "/");
	strcat(histPath, ".mysh_history");
	FILE * historyFile = fopen(histPath, "r");

	if(historyFile == NULL){
		return;
	}

	fgets(temp,MAX,historyFile);
	while(!feof(historyFile)){
		strip(temp);
		if(strcmp("",temp) != 0){
			addHistory(temp);
		}
		fgets(temp,MAX,historyFile);
	}

	fclose(historyFile);
}

void writeHistory(){
	char histPath[strlen(homeDirectory) + 15];
	strcpy(histPath, homeDirectory);
	strcat(histPath, "/");
	strcat(histPath, ".mysh_history");
	FILE * fout = fopen(histPath,"w");

	if(fout == NULL){
		fprintf(stderr, "ERROR WRITING HISTORY!\n");
		return;
	}

	Node * cur;
	int i, x;
	for(cur = history, x = 0; cur != NULL && x < listCount(history) - historyFileSize; cur = cur->next, x++);

	for(i = 0; cur != NULL && i < historyFileSize; cur = cur->next, i++){
		fprintf(fout, "%s\n", (char *) cur->data);
	}

	fclose(fout);
}

