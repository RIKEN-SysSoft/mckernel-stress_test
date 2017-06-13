#
#
#

bin_PROGRAMS = killit \
	signalonread \
	signalonfutex \
	simultaneousterm \
	signalonforksingle signalonforknosignal \
	stackcorruptionsingle stackcorruptionmulti

signalonread_SOURCES = signalonread.c
signalonread_LDFLAGS = -lpthread
signalonread_CFLAGS = -DSINGLE

signalonfutex_SOURCES = signalonfutex.c
signalonfutex_LDFLAGS = -lpthread
signalonfutex_CFLAGS = -DSINGLE

simultaneousterm_SOURCES = simultaneousterm.c
simultaneousterm_LDFLAGS = -lpthread

signalonforksingle_SOURCES = signalonfork.c
signalonforksingle_LDFLAGS = -lpthread
signalonforksingle_CFLAGS = -DSINGLE

signalonforknosignal_SOURCES = signalonfork.c
signalonforknosignal_LDFLAGS = -lpthread
signalonforknosignal_CFLAGS = -DNOSIGNAL

stackcorruptionsingle_SOURCES = stackcorruption.c
stackcorruptionsingle_LDFLAGS = -lpthread
stackcorruptionsingle_CFLAGS = -DSINGLE

stackcorruptionmulti_SOURCES = stackcorruption.c
stackcorruptionmulti_LDFLAGS = -lpthread
stackcorruptionmulti_CFLAGS = -DMULTI
