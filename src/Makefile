all: signalonread signalonfutex simultaneousterm

killit : killit.c
	gcc -o killit killit.c

signalonread : signalonread-mt.c killit
	gcc -lpthread -o signalonread-single -DSINGLE signalonread-mt.c
	gcc -lpthread -o signalonread-multi -DMULTI signalonread-mt.c
	gcc -lpthread -o signalonread-nosignal -DNOSIGNAL signalonread-mt.c

signalonfutex : signalonfutex-mt.c killit
	gcc -lpthread -o signalonfutex-single -DSINGLE signalonfutex-mt.c
	gcc -lpthread -o signalonfutex-multi -DMULTI signalonfutex-mt.c
	gcc -lpthread -o signalonfutex-nosignal -DNOSIGNAL signalonfutex-mt.c

simultaneousterm : simultaneousterm.c
	gcc -lpthread -o simultaneousterm simultaneousterm.c
