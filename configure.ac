#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

CFLAGS=""

AC_ARG_VAR(WITH_MCK_SRC, "McKernel source directory"")
AS_IF([test -z $WITH_MCK_SRC], [AC_MSG_ERROR("specify WITH_MCK_SRC=<mckernel_source>")])

AC_ARG_VAR(WITH_MCK, "McKernel install directory"")
AS_IF([test -z $WITH_MCK], [AC_MSG_ERROR("specify WITH_MCK=<mckernel_install>")])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gettimeofday])

# Resolve architecture
host_cpu_x86_64=0
host_cpu_aarch64=0

AC_CANONICAL_HOST
AS_CASE([$host_cpu],
	[x86_64],
	[host_cpu_x86_64=1],
	[aarch64],
	[host_cpu_aarch64=1])

AS_IF([test "x$host_cpu_x86_64" == "x0" && test "x$host_cpu_aarch64" == "x0"],
	    [AC_MSG_ERROR([ERROR: Unknown architecture: $host_cpu])])

AC_DEFINE_UNQUOTED(HOST_CPU_X86_64,"$host_cpu_x86_64",[Architecture is x86_64])
AC_DEFINE_UNQUOTED(HOST_CPU_AARCH64,"$host_cpu_aarch64",[Architecture is aarch64])

AC_CONFIG_FILES([Makefile
                 src/Makefile
		 tools/Makefile
		 tools/util.sh
		 tools/init/Makefile
		 tools/fini/Makefile])
AC_OUTPUT
