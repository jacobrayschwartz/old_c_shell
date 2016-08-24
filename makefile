mysh:	cscd340w14hw3.c ./linkedlist/linkedList.c ./execution/execution.c ./specialCommands/specialCommands.c
	gcc -g -Wextra -Wall cscd340w14hw3.c ./linkedlist/linkedList.c ./execution/execution.c ./specialCommands/specialCommands.c -o mysh

clean:
	rm mysh