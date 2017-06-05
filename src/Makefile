all: signalonread signalonread-mt

signalonread : signalonread.c
	gcc -o signalonread signalonread.c

signalonread-mt : signalonread-mt.c
	gcc -lpthread -o signalonread-mt signalonread-mt.c
