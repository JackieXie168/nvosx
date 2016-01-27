ifndef TOPDIR
TOPDIR=$(CURDIR)/
endif
ifndef top_srcdir
top_srcdir=$(CURDIR)
endif
ifndef top_builddir
top_builddir=$(CURDIR)
endif

# Marker by Wayne on 2009/09/11, 
# It's not necessary to include rules.mk in this level.
#include $(TOPDIR)/rules.mk

#include $(top_builddir)/Rules.mak

SRCOBJ =  nvram_cavium.o shutils.o devutils.o
SOURCES_PGM = nvram_main.o


EXTRA_CFLAGS+=-I$(LINUX_DIR)/include -Iinclude

CFLAGS += $(EXTRA_CFLAGS) -fPIC
#LDFLAGS += -lgcc_s -L/opt/Lantiq/work/toolchain-lantiq/toolchain-mips_r2_gcc-4.3.3+cs_uClibc-0.9.30.1_2_6_20/lib
SBINPGM = nvram
DNILIB = libnvram.so


all: $(SBINPGM) $(DNILIB)

%.o: %.c
	#$(call make-depend,$<,$@,$(subst .o,.d,$@))
	$(CC) $(CFLAGS) $(LDFLAGS) $(EXTRA_CFLAGS) -fPIC  -I. -Iinclude -c -o $@ $<
	
$(DNILIB): $(SRCOBJ)
	$(LD) $(LDFLAGS) -shared -o $@ $^ $(LIBS)

$(SBINPGM): $(SOURCES_PGM) $(SRCOBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SOURCES_PGM) $(SRCOBJ) $(LIBS)
	
ifeq ($(GPL_CODE),1)
clean:
	rm -f *.c *.a *.so nvram
else
clean:
	rm -f *.o *.a *.so nvram
endif

#ifneq "all" "clean"
#  -include .depend
#endif


#define make-depend
#	$(CC)	-MM	\
#		-MF $3	\
#		-MP	\
#		-MT $2	\
#		-E	\
#		$(CFLAGS)	\
#		$1
#	@cat $3 | while read line;do echo $$line >> .depend;done;(sort .depend | uniq) > tmp ; mv tmp .depend;rm -f $3
#endef



