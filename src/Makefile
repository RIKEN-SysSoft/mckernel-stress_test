CC		= gcc
CFLAGS		= -lpthread
OPT_SINGLE	= -DSINGLE
OPT_MULTI	= -DMULTI
OPT_NOSIGNAL	= -DNOSIGNAL

all: signalonread signalonfutex simultaneousterm signalonfork stackcorruption

killit : killit.c
	$(CC) -o $@ $<

#---- signal on read

signalonread : signalonread-single signalonread-multi signalonread-nosignal
	@:

signalonread-single : signalonread.c
	$(CC) $(CFLAGS) $(OPT_SINGLE) -o $@ $<

signalonread-multi : signalonread.c
	$(CC) $(CFLAGS) $(OPT_MULTI) -o $@ $<

signalonread-nosignal : signalonread.c killit
	$(CC) $(CFLAGS) $(OPT_NOSIGNAL) -o $@ $<

#---- signal on futex wait

signalonfutex : signalonfutex-single signalonfutex-multi signalonfutex-nosignal
	@:

signalonfutex-single : signalonfutex.c
	$(CC) $(CFLAGS) $(OPT_SINGLE) -o $@ $<

signalonfutex-multi : signalonfutex.c
	$(CC) $(CFLAGS) $(OPT_MULTI) -o $@ $<

signalonfutex-nosignal : signalonfutex.c killit
	$(CC) $(CFLAGS) $(OPT_NOSIGNAL) -o $@ $<

#---- simultaneous SIGTERM

simultaneousterm : simultaneousterm.c
	$(CC) $(CFLAGS) -o $@ $<

#---- signal on fork

signalonfork : signalonfork-single signalonfork-nosignal
	@:

signalonfork-single : signalonfork.c
	$(CC) $(CFLAGS) $(OPT_SINGLE) -o $@ $<

signalonfork-nosignal : signalonfork.c killit
	$(CC) $(CFLAGS) $(OPT_NOSIGNAL) -o $@ $<

#---- stack corruption

stackcorruption : stackcorruption-single stackcorruption-multi
	@:

stackcorruption-single : stackcorruption.c
	$(CC) $(CFLAGS) $(OPT_SINGLE) -o $@ $<

stackcorruption-multi : stackcorruption.c
	$(CC) $(CFLAGS) $(OPT_MULTI) -o $@ $<

