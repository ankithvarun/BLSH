make:
	gcc -g process.h signalhandler.c process.c history.h history.c shell.h shell.c ls.c execute.c main.c

clean:
	rm process.h.gch history.h.gch shell.h.gch a.out