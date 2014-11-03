# configure.ac
#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

dnl AC_PREREQ(2.61)
AC_INIT([nvram include/nvram.h],[v1.0],[Jackie.CPlusPlus@gmail.com])
AM_INIT_AUTOMAKE(nvram, v1.0)         
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

dnl LT_INIT([dlopen])

dnl -----------------------------------------------
dnl Package name and version number (user defined)
dnl -----------------------------------------------

GENERIC_LIBRARY_NAME=nvram

#release versioning
GENERIC_MAJOR_VERSION=1
GENERIC_MINOR_VERSION=0
GENERIC_MICRO_VERSION=0

#API version (often = GENERIC_MAJOR_VERSION.GENERIC_MINOR_VERSION)
GENERIC_API_VERSION=1.0
AC_SUBST(GENERIC_API_VERSION)

#shared library versioning
GENERIC_LIBRARY_VERSION=1:2:0
#                       | | |
#                +------+ | +---+
#                |        |     |
#             current:revision:age
#                |        |     |
#                |        |     +- increment if interfaces have been added
#                |        |        set to zero if interfaces have been removed
#                                  or changed
#                |        +- increment if source code has changed
#                |           set to zero if current is incremented
#                +- increment if interfaces have been added, removed or changed


dnl --------------------------------
dnl Package name and version number
dnl --------------------------------

AC_SUBST(GENERIC_LIBRARY_VERSION)

PACKAGE=$GENERIC_LIBRARY_NAME
AC_SUBST(GENERIC_LIBRARY_NAME)

GENERIC_VERSION=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION.$GENERIC_MICRO_VERSION
GENERIC_RELEASE=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION
AC_SUBST(GENERIC_RELEASE)
AC_SUBST(GENERIC_VERSION)

VERSION=$GENERIC_VERSION

AM_INIT_AUTOMAKE($PACKAGE, $VERSION, no-define)

AC_PREREQ
dnl AC_REQUIRE_AUX_FILE([ltmain.sh])

# Checks for programs.
AC_LANG_CPLUSPLUS
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AM_PROG_CC_C_O
AM_PROG_LIBTOOL
AM_SANITY_CHECK

#
#DX_HTML_FEATURE(ON)
#DX_CHM_FEATURE(OFF)
#DX_CHI_FEATURE(OFF)
#DX_MAN_FEATURE(OFF)
#DX_RTF_FEATURE(OFF)
#DX_XML_FEATURE(OFF)
#DX_PDF_FEATURE(ON)
#DX_PS_FEATURE(ON)
#
#DX_INIT_DOXYGEN($PACKAGE_NAME, doxygen.cfg)
#

AM_INIT_AUTOMAKE

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_FUNC_ALLOCA
AC_HEADER_STDC
#AC_CHECK_HEADERS([fcntl.h limits.h malloc.h stddef.h stdlib.h string.h unistd.h])
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h netinet/in.h stdlib.h string.h sys/file.h sys/ioctl.h sys/socket.h sys/time.h termios.h unistd.h])

# Checks for command line options
AC_ARG_ENABLE([async-exec],
  [AS_HELP_STRING([--disable-async-exec], 
    [disable asynchronous execution @<:@default: no@:>@])],
  [async_exec=${enableval}],
  [async_exec=yes])

if test "x${async_exec}" = xyes; then
  have_pthreads=no
  AC_SEARCH_LIBS([pthread_create], [pthread], 
    [have_pthreads=yes])

  if test "x${have_pthreads}" = xyes; then
    AC_CHECK_HEADERS([pthread.h], [], 
      [have_pthreads=no])
  fi

  if test "x${have_pthreads}" = xno; then
    echo "---------------------------------------"
    echo "Unable to find pthreads on this system."
    echo "Building a single-threaded version.    "
    echo "---------------------------------------"
    async_exec=no
  fi
fi

if test "x${async_exec}" = xyes; then
  AC_DEFINE([ASYNC_EXEC], 1, [async exec enabled])
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_INT32_T
AC_TYPE_PID_T
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT64_T
AC_C_VOLATILE
AC_CHECK_TYPES([ptrdiff_t])
AC_HEADER_STDBOOL
AC_C_INLINE
#AC_C_CONST

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([alarm bzero dup2 getdelim gettimeofday inet_ntoa memset memmove mkdir select setenv socket strchr strcspn strdup strerror strstr strncasecmp strcasecmp strspn strtol strtoul sysinfo])

#
# Modules configuration
#
HostOS=`echo "$host" | sed 's/.*-//'`
os_is_macosx=false
nonLinuxOS=false
AC_SUBST(HostOS)
case ${HostOS} in
	darwin* | powerpc*-*-darwin*)
		os_is_macosx=true
		nonLinuxOS=true
		;;
	*)
		echo host="$host"
		echo HostOS="$HostOS"
		os_is_macosx=false
		nonLinuxOS=false
		echo "nonLinuxOS=false ..."
		;;
esac		

AM_CONDITIONAL([NON_LINUX], [test x$nonLinuxOS = xtrue])
AM_COND_IF([NON_LINUX],
	[AC_DEFINE([NON_LINUX], [1], [Get HostOS Type])])

AM_CONDITIONAL([IS_DARWIN], [test x$os_is_macosx = xtrue])
AM_COND_IF([IS_DARWIN],
	[AC_DEFINE([IS_DARWIN], [1], [Get HostOS Type is Darwin])])

dnl Check for function sysctl (KERN_BOOTTIME)
AC_MSG_CHECKING(for function sysctl (KERN_BOOTTIME))
AC_TRY_COMPILE(
[
	#include <sys/types.h>
	#include <sys/sysctl.h>
	#include <unistd.h>
	#include <time.h>
	#include <sys/time.h>
],
[
	struct timeval uptime;
        int     mib[2],len;
        int     now;

        mib[0]=CTL_KERN;
        mib[1]=KERN_BOOTTIME;
        len=sizeof(uptime);
        sysctl(mib,2,&uptime,&len,0,0);
],
AC_DEFINE(HAVE_FUNCTION_SYSCTL_KERN_BOOTTIME,1,[Define to 1 if 'KERN_BOOTTIME' exist.])
AC_MSG_RESULT(yes),
AC_MSG_RESULT(no))

dnl Check for function sysctl (KERN_MAXFILES)
AC_MSG_CHECKING(for function sysctl (KERN_MAXFILES))
AC_TRY_COMPILE(
[
	#include <sys/types.h>
	#include <sys/sysctl.h>
],
[
        int     mib[2],len;
        int     maxfiles;

        mib[0]=CTL_KERN;
        mib[1]=KERN_MAXFILES;
        len=sizeof(maxfiles);
        sysctl(mib,2,&maxfiles,&len,0,0);
],
AC_DEFINE(HAVE_FUNCTION_SYSCTL_KERN_MAXFILES,1,[Define to 1 if 'KERN_MAXFILES' exist.])
AC_MSG_RESULT(yes),
AC_MSG_RESULT(no))

dnl Check for function sysctl (KENR_MAXPROC)
AC_MSG_CHECKING(for function sysctl (KERN_MAXPROC))
AC_TRY_COMPILE(
[
	#include <sys/types.h>
	#include <sys/sysctl.h>
],
[
        int     mib[2],len;
        int     maxproc;

        mib[0]=CTL_KERN;
        mib[1]=KERN_MAXPROC;
        len=sizeof(maxproc);
        sysctl(mib,2,&maxproc,&len,0,0);
],
AC_DEFINE(HAVE_FUNCTION_SYSCTL_KERN_MAXPROC,1,[Define to 1 if 'KERN_MAXPROC' exist.])
AC_MSG_RESULT(yes),
AC_MSG_RESULT(no))

AC_CONFIG_FILES([Makefile
	src/Makefile
	include/Makefile
	testcase/Makefile
	conf/Makefile
	host/Makefile
])

AC_OUTPUT(nvram-1.0.pc)

echo \
"-------------------------------------------------

 ${PACKAGE_NAME} Version ${PACKAGE_VERSION}

 Prefix: '${prefix}'.
 Compiler: '${CC} ${CFLAGS} ${CPPFLAGS}'
 Libraries: '${LIBS}'

 Package features:
   Async Execution: ${async_exec}

 Now type 'make @<:@<target>@:>@'
   where the optional <target> is:
     all                - build all binaries
     install            - install everything

--------------------------------------------------"

