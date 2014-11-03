if NON_LINUX
SYSCONFDIR = /var/conf
else
SYSCONFDIR = /var/OpenAgentV2/conf
endif

all:
	@echo "Compile done";

install-data-am:
	@echo "Run 'make install-conf' if you want to install initial configuration file";

install-conf:
	install -d ${SYSCONFDIR}
	test -e ${SYSCONFDIR}/nvram.conf || install -m 600 $(top_srcdir)/nvram/conf/nvram.conf ${SYSCONFDIR}
	test -e ${SYSCONFDIR}/nvram.default || install -m 600 $(top_srcdir)/nvram/conf/nvram.default ${SYSCONFDIR}

