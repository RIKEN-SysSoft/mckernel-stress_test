#
#
#

bin_PROGRAMS = killit \
	signalonreadsingle signalonreadmulti signalonreadnosignal \
	signalonfutexsingle signalonfutexmulti signalonfutexnosignal \
	simultaneousterm \
	signalonforksingle signalonforknosignal \
	stackcorruptionsingle stackcorruptionmulti

signalonreadsingle_SOURCES = signalonread.c
signalonreadsingle_LDFLAGS = -lpthread
signalonreadsingle_CFLAGS = -DSINGLE

signalonreadmulti_SOURCES = signalonread.c
signalonreadmulti_LDFLAGS = -lpthread
signalonreadmulti_CFLAGS = -DMULTI

signalonreadnosignal_SOURCES = signalonread.c
signalonreadnosignal_LDFLAGS = -lpthread
signalonreadnosignal_CFLAGS = -DNOSIGNAL

signalonfutexsingle_SOURCES = signalonfutex.c
signalonfutexsingle_LDFLAGS = -lpthread
signalonfutexsingle_CFLAGS = -DSINGLE

signalonfutexmulti_SOURCES = signalonfutex.c
signalonfutexmulti_LDFLAGS = -lpthread
signalonfutexmulti_CFLAGS = -DMULTI

signalonfutexnosignal_SOURCES = signalonfutex.c
signalonfutexnosignal_LDFLAGS = -lpthread
signalonfutexnosignal_CFLAGS = -DNOSIGNAL

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
