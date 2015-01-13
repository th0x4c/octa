#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([OCTA], [0.1.0])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_AR

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h sys/time.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>],[union semun foo;], [mf_have_semun=1], [mf_have_semun=0])
if test $mf_have_semun = 1
then
  AC_DEFINE(HAVE_UNION_SEMUN, 1, [union semun defined in sys/ipc.h or sys/sem.h])
fi

AC_CANONICAL_HOST
case "$host_os" in
darwin*) portname=darwin;;
linux*) portname=linux;;
hpux*) portname=hpux;;
cygwin*) portname=cygwin;;
esac
AC_MSG_RESULT([$portname])
AM_CONDITIONAL(DARWIN, test x$portname = xdarwin)
AM_CONDITIONAL(LINUX, test x$portname = xlinux)
AM_CONDITIONAL(HPUX, test x$portname = xhpux)
if test x$portname = xhpux; then
  CFLAGS="$CFLAGS -Ae -z +Olibmerrno +Z +DD64"
  LDFLAGS="$LDFLAGS -Wl,+s -Wl,+n +DD64"
fi

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gettimeofday memset])

AC_ARG_ENABLE([high-precision-distribution],
              [AS_HELP_STRING([--disable-high-precision-distribution],
                              [disable high-precision distribution])],
              [if test x$enableval != xyes; then
                 AC_DEFINE(DISABLE_HIGH_PRECISION_DISTRIBUTION, 1,
                           [Disable high-precision distribution])
               fi])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/TA/Makefile
                 src/OC/Makefile
                 src/octa/Makefile
                 test/Makefile
                 test/TA/Makefile
                 test/OC/Makefile])
AC_OUTPUT
