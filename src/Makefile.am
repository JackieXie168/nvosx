# $(top_srcdir)/Makefile.am
#
# AUTOMAKE_OPTIONS= foreign

if IS_DARWIN
bin_PROGRAMS= xnvram
else
bin_PROGRAMS= nvram
endif

bin_PROGRAMS += nvlist

#Build in these directories:

COMMON_FILES=\
	$(top_srcdir)/include/nvram.h \
	$(top_srcdir)/include/typedefs.h

NVRAM_FILES= $(COMMON_FILES) \
	nvram.c \
	nvram_main.c \
	shutils.c

if IS_DARWIN
xnvram_SOURCES= $(NVRAM_FILES)
else
nvram_SOURCES= $(NVRAM_FILES)
endif

AM_CFLAGS= -I$(top_srcdir)/include
#LIBS= -L/usr/lib -lpthread

h_sources = $(top_srcdir)/include/nvram.h $(top_srcdir)/include/typedefs.h
c_sources = nvram.c shutils.c

library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)
library_include_HEADERS = $(h_sources)

lib_LTLIBRARIES= libnvram-1.0.la
libnvram_1_0_la_SOURCES= $(h_sources) $(c_sources)
libnvram_1_0_la_CFLAGS = -fPIC $(AM_CFLAGS)
#libnvram_1_0_la_LIBADD= libnvram.la
libnvram_1_0_la_LDFLAGS= -version-info $(GENERIC_LIBRARY_VERSION) -release $(GENERIC_RELEASE)

nvlist_SOURCES = nvlist.c
nvlist_CPPFLAGS = -I$(top_srcdir)/include
nvlist_LDADD = $(top_srcdir)/src/libnvram-1.0.la -dlopen $(top_srcdir)/src/libnvram-1.0.la

METASOURCES = AUTO
#DISTCLEANFILES = Makefile.in
CLEANFILES = *~ *.log
