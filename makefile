all:	main.c parser.c shell.c
	gcc main.c parser.c shell.c -o myshell
run:
	./myshell
