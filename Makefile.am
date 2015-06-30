# adding doxygen support
# include $(top_srcdir)/aminclude.am
ACLOCAL_AMFLAGS=-I m4

SUBDIRS=src include testcase conf host

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = invram-1.0.pc
EXTRA_DIST=autogen.sh

DISTCLEANFILES = Makefile.in aclocal.m4 compile config.guess config.sub configure configure.scan ltmain.sh
CLEANFILES = *~ *.log
