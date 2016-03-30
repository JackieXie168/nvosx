# Makefile.am
AUTOMAKE_OPTIONS= foreign

bin_PROGRAMS = nvramd
if IS_DARWIN
bin_PROGRAMS+= invram
else
bin_PROGRAMS+= nvram
endif


#Build in these directories:
COMMON_FILES=-I$(top_srcdir)/include/nvram.h

NVRAM_FILES= $(COMMON_FILES) \
	mtd.c \
	nvram_defaults.c

h_sources = $(top_srcdir)/include/nvram.h
c_sources = mtd.c nvram_agent.c nvram_daemon.c nvram_defaults.c

if IS_DARWIN
invram_SOURCES= nvram_socket.c nvram_agent.c
else
nvram_SOURCES= nvram_socket.c nvram_agent.c
endif

nvramd_SOURCES = nvramd.c $(c_sources)

AM_CFLAGS= -I$(top_srcdir)/include

#LIBS= -L/usr/lib -lpthread

library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)
library_include_HEADERS = $(h_sources)

INCLUDES = -I$(top_srcdir)/include

lib_LTLIBRARIES= libnvram-1.0.la
libnvram_1_0_la_SOURCES= $(h_sources) $(c_sources)
libnvram_1_0_la_CFLAGS = -fPIC $(AM_CFLAGS)
#libnvram_1_0_la_LIBADD= libnvram.la
libnvram_1_0_la_LDFLAGS= -version-info $(GENERIC_LIBRARY_VERSION) -release $(GENERIC_RELEASE)

METASOURCES = AUTO
#DISTCLEANFILES = Makefile.in
CLEANFILES = *~ *.log
