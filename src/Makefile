
CC		= gcc
CFLAGS		= -lpthread
OPT_SINGLE	= -DSINGLE
OPT_MULTI	= -DMULTI
OPT_NOSIGNAL	= -DNOSIGNAL

#all: signalonread signalonfutex simultaneousterm signalonfork stackcorruption

all: stackcorruption


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

signalonfork : signalonfork.c
	gcc -lpthread -o signalonfork-single -DSINGLE signalonfork.c
	gcc -lpthread -o signalonfork-nosignal -DNOSIGNAL signalonfork.c

stackcorruption : stackcorruption-single stackcorruption-multi
	@:

stackcorruption-single : stackcorruption.c
	$(CC) $(CFLAGS) $(OPT_SINGLE) -o $@ $<

stackcorruption-multi : stackcorruption.c
	$(CC) $(CFLAGS) $(OPT_MULTI) -o $@ $<

