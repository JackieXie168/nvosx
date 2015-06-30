# Makefile.am
AUTOMAKE_OPTIONS= foreign

bin_PROGRAMS = nvramd invram


#Build in these directories:
COMMON_FILES=-I$(top_srcdir)/include/nvram.h

NVRAM_FILES= $(COMMON_FILES) \
	mtd.c \
	nvram_defaults.c

h_sources = $(top_srcdir)/include/invram.h
c_sources = mtd.c nvram_agent.c nvram_daemon.c nvram_defaults.c


nvramd_SOURCES = nvramd.c $(c_sources)
invram_SOURCES = invram.c nvram_agent.c

AM_CFLAGS= -I$(top_srcdir)/include

#LIBS= -L/usr/lib -lpthread

library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)
library_include_HEADERS = $(h_sources)

INCLUDES = -I$(top_srcdir)/include

lib_LTLIBRARIES= libinvram-1.0.la
libinvram_1_0_la_SOURCES= $(h_sources) $(c_sources)
libinvram_1_0_la_CFLAGS = -fPIC $(AM_CFLAGS)
#libinvram_1_0_la_LIBADD= libinvram.la
libinvram_1_0_la_LDFLAGS= -version-info $(GENERIC_LIBRARY_VERSION) -release $(GENERIC_RELEASE)

METASOURCES = AUTO
#DISTCLEANFILES = Makefile.in
CLEANFILES = *~ *.log
