INSTALL_DIR = /usr/sbin

if NON_LINUX
COMMIT_BIN = $(top_srcdir)/nvram/host/commit.macosx
else
COMMIT_BIN = $(top_srcdir)/nvram/host/commit
endif

all: install-data-am
	@echo "Compile done";

install-data-am:
	@echo "Run 'make install-bin' if you want to install commit program";

install-bin:
	test -e ${INSTALL_DIR}/commit || install -m 755 ${COMMIT_BIN} ${INSTALL_DIR}/commit
	test -e ${INSTALL_DIR}/remove_shm.sh || install -m 755 remove_shm.sh ${INSTALL_DIR}

