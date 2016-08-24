#include "execution.h"

#define MAX 100

extern Node * history;
extern Node * aliases;

/*
From here, the rest of the input is parsed
*/
void execute(Node ** input){
	if(*input == NULL){
		return;
	}
	/*
	Grabbing the desired input stream
	If there is a redirection in, the info in the segment after the '<' is used for the input stream and the two nodes are removed
	Otherwise stdin is returned
	*/
	int fin = getFin(input);
	/*
	Grabbing the desired output stream
	If there is a redirection out, the info after the '>' is used for the output stream and the two nodes are removed
	*/
	int fout = getFout(input);

	/*Getting the location of the pipe if there is any*/
	int pipeLoc = getPipeIndex(*input);

	/*Determining if there is an & at the end to signal to run the process in the background*/
	int wait = 1;
	char temp[MAX];
	Node * tempNode = getNode(listCount(*input) - 1,*input);
	strcpy(temp, (char*) tempNode->data);
	if(strcmp(temp,"&") == 0){
		wait = 0;
		tempNode = popLast(input);
		cleanLinkedList(&tempNode);
	}

	/*If there is a pipe*/
	if(pipeLoc != -1){
		/*getting the left half of the pipe*/
		char ** leftArgs = linkedListToArrayCut(*input, pipeLoc);

		/*itterating through the list to get the node of the right half*/
		Node * cur = *input;
		int i;
		for(i = 0; i <= pipeLoc; i++){
			cur = cur->next;
		}
		/*getting the right half, useing the regular to array function starting at cur*/
		char ** rightArgs = linkedListToArray(cur);


		/*If the last end of the pipt is empty, we need to have the users add it in*/
		if(rightArgs[0] == NULL){
			while(1){
				/*Freeing up the old array*/
				cleanCharArray(rightArgs);
				rightArgs = NULL;

				Node * head;
				char s[MAX];
				strcpy(s,"\0");
				printf("> ");
				fgets(s, MAX, stdin);

				/*Do the same stuff to this string as the others*/
				/*Stripping out the newline*/
				strip(s);

				if(strcmp(s, "") != 0){
					

					/*Convert input to LinkedList*/
					head = argsToLinkedList(s);

					/*Replacing the history recals*/
					deHistory(&head, history);

					/*De-alias*/
					deAlias(&head, aliases);

					rightArgs = linkedListToArray(head);

					cleanLinkedList(&head);
					break;
				}
			}
		}


		/*executing*/
		pipeIt(leftArgs, rightArgs, fin, fout, wait);
		/*Cleaning*/
		cleanCharArray(leftArgs);
		cleanCharArray(rightArgs);
	} /*End pipe*/

	/*Regular fork*/
	else{
		char ** args = linkedListToArray(*input);
		generalFork(args, fin, fout, wait);
		cleanCharArray(args);
	}
}

/*===================================  Redirection functions  =======================================*/

/*
This method looks for a redirection for stdin
If it exists it creates a new file pointer to there, removing it and the '<' from the linked list
Else it returns stdin
*/
int getFin(Node ** head){
	char temp[MAX]; /*Buffer*/
	Node * cur, * prev;
	
	prev = *head;
	for(cur = *head; cur != NULL; cur = cur->next){
		if(strcmp((char *) cur->data, "<") == 0){/*If there is a redirect*/
			/*copying the info to temp*/
			strcpy(temp, (char *) cur->next->data);

			/*
			Erasing the nodes in the linked list ("<" and the file pointer)
			*/
			prev->next = cur->next->next;
			cur->next->next = NULL;
			cleanLinkedList(&cur);

			int fin = open(temp, O_RDONLY, 0777);
			return fin;
		}

		prev = cur;
	}

	return 0;
}


int getFout(Node ** head){
char temp[MAX]; /*Buffer*/
	Node * cur, * prev;
	
	prev = *head;
	for(cur = *head; cur != NULL; cur = cur->next){
		if(strcmp((char *) cur->data, ">") == 0){/*If there is a redirect*/
			/*copying the info to temp*/
			strcpy(temp, (char *) cur->next->data);

			/*
			Erasing the nodes in the linked list (">" and the file pointer)
			*/
			prev->next = cur->next->next;
			cur->next->next = NULL;
			cleanLinkedList(&cur);

			int fout = open(temp, O_CREAT|O_WRONLY|O_TRUNC, 0777);
			return fout;
		}

		prev = cur;
	}

	return 1;
}
/*===================================  Parsing functions  =======================================*/


/*
This method converts the linked list into an array to be executed
Adds a NULL to the end of the list
*/
char ** linkedListToArray(Node * head){
	Node * cur;
	int length = listCount(head);
	char ** args = (char**)calloc(length + 1, sizeof(char*));

	/*Now itterating through the list, adding each char * to the array*/
	cur = head;
	int i;
	for(i = 0; cur != NULL; i ++){
		/*Allocating the array*/
		args[i] = (char *)calloc(strlen((char *) cur->data) + 1, sizeof(char));
		strcpy(args[i],(char *) cur->data);

		/*Advancing the linked list pointer*/
		cur = cur->next;
	}

	return args;
}

/*
This method works like the regular linkedListToArray, but it stops after so many itterations
Also, like the top one, it does not have to start with the actual head of the linked list
The end of the array is NULL
This method DOES NOT check the index, so a segfault will happen if the stop int is beyond the end of the list
*/
char ** linkedListToArrayCut(Node * head, int stopBefore){
	Node * cur = head;
	char ** args = (char**)calloc(stopBefore + 1,sizeof(char*));

	int i;
	for(i = 0; i < stopBefore; i ++){
		args[i] = (char *)calloc(strlen((char *) cur->data) + 1, sizeof(char));
		strcpy(args[i], (char *) cur->data);

		cur = cur->next;
	}

	args[stopBefore] = NULL;
	return args;
}

/*
This method will return the index of the pipe. If there is none, -1 is returned
*/
int getPipeIndex(Node * head){
	Node * cur = head;
	int loc = -1, i;

	for(i = 0; cur != NULL; cur = cur->next){
		if(strcmp("|", (char *) cur->data) == 0){/*If the current node is "|"*/
			loc = i;
			break;
		}
		i++;
	}

	return loc;
}

/*===================================  Execution  =======================================*/


/*This method is for a general fork*/
void generalFork(char ** args, int fin, int fout, int wait){
	pid_t pid = fork(); /*Forking the process*/
	if(pid != 0){ /*If executing in parent process*/
		if(wait){
			int status;
			waitpid(pid, &status, 0);
		}
	}

	else{/*If executing in the child process*/
		/*Changing the filestreams*/
		if(fin != 0){ /*If fin is not stdin*/
			dup2(fin, 0);
			close(fin);
		}
		if(fout != 1){ /*If fout is not stdout*/
			dup2(fout, 1);
			close(fout);
		}

		execvp(args[0], args); /*exec is issued*/

		/*The following should only execute if the exec command failed*/
		cleanCharArray(args);
		fprintf(stderr,"Command not valid!\n");
		

		exit(EXIT_FAILURE);
	}
}


void pipeIt(char ** leftArgs, char ** rightArgs, int fin, int fout, int wait){

	pid_t pid = fork(); //Forking the process

	if(pid != 0){ //If executing in parent process
		if(wait){
			int status;
			waitpid(pid, &status, 0);
		}
	}

	else{//If executing in the child process
		/*Changing the filestreams*/
		if(fin != 0){ /*If fin is not stdin*/
			dup2(fin, 0);
			close(fin);
		}
		if(fout != 1){ /*If fout is not stdout*/
			dup2(fout, 1);
			close(fout);
		}

		/*Piping*/
		int pipeFd[2];
		int status;
		pipe(pipeFd);

		


		if(fork() == 0){//If we are in the child process

			close(pipeFd[0]);//Closing the read part on the pipe
			close(1); //Closing stdout
			dup2(pipeFd[1], 1);
			close(pipeFd[1]);
			execvp(leftArgs[0], leftArgs); //Executing the pre pipe

			/*The following should only execute if the exec command failed*/
			cleanCharArray(leftArgs);
			fprintf(stderr,"Command not valid!\n");
			

			exit(EXIT_FAILURE);
		}
		else{//in the parent process
			waitpid(-1, &status, 0);

			close(pipeFd[1]);
			close(0);//Close stdin
			dup(pipeFd[0]);
			close(pipeFd[0]);

			execvp(rightArgs[0], rightArgs);

			/*The following should only execute if the exec command failed*/
			cleanCharArray(rightArgs);
			fprintf(stderr,"Command not valid!\n");
			

			exit(EXIT_FAILURE);
		}
	}
}


