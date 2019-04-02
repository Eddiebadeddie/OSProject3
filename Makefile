cube: cube.c wizard.c
	gcc -o cube cube.c wizard.c -lreadline -lhistory -lncurses -lpthread
