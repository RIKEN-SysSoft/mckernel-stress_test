#
#
#

dist_bin_SCRIPTS = config.sh \
	mck-boot.sh \
	mck-stop.sh \
	mck-mcexec.sh \
	run.sh

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
signalonread_CFLAGS = -g -O2

signalonfutex_SOURCES = signalonfutex.c
signalonfutex_LDFLAGS = -lpthread
signalonfutex_CFLAGS = -g -O2

simultaneousterm_SOURCES = simultaneousterm.c
simultaneousterm_LDFLAGS = -lpthread
simultaneousterm_CFLAGS = -g -O2

signalonfork_SOURCES = signalonfork.c
signalonfork_LDFLAGS = -lpthread
signalonfork_CFLAGS = -g -O2

stackcorruption_SOURCES = stackcorruption.c
stackcorruption_LDFLAGS = -lpthread
stackcorruption_CFLAGS = -O0

segviolation_SOURCES = segviolation.c
segviolation_LDFLAGS = -lpthread
segviolation_CFLAGS = -g -O2

stackoverflow_SOURCES = stackoverflow.c
stackoverflow_LDFLAGS = -lpthread
stackoverflow_CFLAGS = -g -O2

SUBDIRS = init fini
