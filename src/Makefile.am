#
#
#

dist_bin_SCRIPTS = config.sh \
	mck-boot.sh \
	mck-stop.sh \
	mck-mcexec.sh

bin_PROGRAMS = killit \
	signalonread \
	signalonfutex \
	simultaneousterm \
	signalonfork \
	stackcorruption \
	segviolation \
	stackoverflow

signalonread_SOURCES = signalonread.c
signalonread_LDFLAGS = -lpthread

signalonfutex_SOURCES = signalonfutex.c
signalonfutex_LDFLAGS = -lpthread

simultaneousterm_SOURCES = simultaneousterm.c
simultaneousterm_LDFLAGS = -lpthread

signalonfork_SOURCES = signalonfork.c
signalonfork_LDFLAGS = -lpthread

stackcorruption_SOURCES = stackcorruption.c
stackcorruption_LDFLAGS = -lpthread

segviolation_SOURCES = segviolation.c
segviolation_LDFLAGS = -lpthread

stackoverflow_SOURCES = stackoverflow.c
stackoverflow_LDFLAGS = -lpthread
