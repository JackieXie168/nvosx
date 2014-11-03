SUBDIRS = .
bin_PROGRAMS = test
test_SOURCES = test.c
test_CPPFLAGS = -I$(top_srcdir)/include
test_LDADD = $(top_srcdir)/src/libnvram-1.0.la -dlopen $(top_srcdir)/src/libnvram-1.0.la

check_SCRIPTS = greptest.sh
TESTS = $(check_SCRIPTS)

greptest.sh:
		echo './test | grep "hello"' > greptest.sh
		chmod +x greptest.sh

CLEANFILES = greptest.sh *~ *.log test
#DISTCLEANFILES = Makefile.in
