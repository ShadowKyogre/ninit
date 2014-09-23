SHELL = /bin/sh
VPATH = contrib:djb:lib:misc
DESTDIR:=/

DIET=
CC=gcc $(GCC_FLAGS)
CFLAGS=-pipe -Os -Wall -W
LDFLAGS=-s
FLAG_DEBUG = no

MYARCH:=$(shell uname -m | sed -e 's/i[4-9]86/i386/' -e 's/armv[3-6]t\?e\?[lb]/arm/')
MAN_PAGES:=$(shell test -d /usr/share/man && echo usr/share/man || echo usr/man)

ifeq ($(MYARCH),i386)
include system/$(MYARCH)/Flags
endif
ifeq ($(MYARCH),x86_64)
include system/$(MYARCH)/Flags
endif

# If the assembler fail on your host change i386 bellow to XXXX
ifeq ($(MYARCH),i386)
ALL_LIB = $(shell sed -e 's/Z./S/'  -e 's/^[a-z]*\///' library_files)
else
ALL_LIB = $(shell sed -e 's/Z.//' -e 's/^[a-z]*\///' library_files)
endif

ifdef DIET
CFLAGS += -nostdinc
else
CFLAGS += $(OPTIMIZATION)
endif

N_FLAGS = -nostdlib -DINIT_SYSTEM system/$(MYARCH)/start.o
N_FLAGS += $(CFLAGS) $(LDFLAGS)
N_LIB = ninit.a system/$(MYARCH)/system.a

ifeq ($(FLAG_DEBUG),no)
CCC_ = @echo '	CC $< ' ;
CCL_ = @echo '	CL $<	-> $@ ' ;
C = @
else
CCC_ =
CCL_ =
C =
endif

CC_C = $(DIET) $(CC) $(CFLAGS)
CC_L = $(DIET) $(CC) $(CFLAGS) $(LDFLAGS)

CCC = $(CCC_) $(CC_C)
CCL = $(CCL_) $(CC_L)
STR = strip -R .comment -R .note

ALL = ninit run wait update nsvc reload pidfile reboot \
 runlevel sysvinit inittab shutdown pututmpid bootlog install-bin \
 env sleeprun conditional-init serdo argv0 ninit-mmap ninit-huge nkillall \
 remove procfs
ALL_MAN = ninit.8.gz nsvc.8.gz pututmpid.8.gz shutdown.8.gz nkillall.8.gz \
 runlevel.8.gz sysvinit.8.gz pidfile.8.gz reboot.8.gz reload.8.gz \
 bootlog.8.gz service.8.gz inittab.8.gz scan.8.gz

BYTE_C	 = $(wildcard djb/byte_*.c)
BUFFER_C = $(wildcard djb/buffer_*.c lib/err_b.c)

all: $(ALL) $(ALL_MAN) Version start_tests
ninit.a: $(ALL_LIB)
	ar cr $*.a $^

include_h_files = all_defs.h buffer_defs.h byte_defs.h utmp_defs.h \
 addprocess.h open_inout.h tryservice_nsvc.h \
 findservice.h mmap_alloca.h sighandler.h wait_services.h get_services.h \
 ninit.h t_write.h initreq.h ninitfeatures.h tryservice.h error_table.h \
 struct_root.h uid.h
help_files = $(include_h_files) int_defs.h pagesize_defs.h process_defs.h \
ninit.a

%.o: %.c $(include_h_files) process_defs.h
	$(CCC) -c -o $@ $<
S%.o: S/%.S $(include_h_files)
	$(CCC) -c -o $@ $<

printf: printf.c
	$(CCL) -o $@ $<
	$(C) $(STR) $@
bin-$(MYARCH)/%: %.c $(help_files)
	$(CCL_) $(CC) $(N_FLAGS) $(TINY_FLAGS) -o $@ $< $(N_LIB)
%: %.c $(help_files)
	$(CCL) -o $@ $< ninit.a

ninit-mmap.c: ninit.c printf
	$(C) ./printf '%s\n%s\n' '#define INIT_MMAP' '#include "ninit.c"' > $@
ninit-huge.c: ninit.c printf
	$(C) ./printf '%s\n%s\n' '#define INIT_TIMEOUT_WRITE' '#include "ninit.c"' > $@
buffer_defs.h: $(BUFFER_C)
	$(C) ./get_headers $@ $^ > $@
byte_defs.h: $(BYTE_C) djb/str_len.c
	$(C) ./get_headers $@ $^ > $@
utmp_defs.h: lib/do_wtmp.c lib/utmp_io.c
	$(C) ./get_headers $@ $^ > $@
all_defs.h: library_files printf
	$(C) ./get_headers $@ -Lint_defs.h `sed -e 's/\.o/\.c/' -e 's/Z/djb/' library_files` | sed -e '/struct.*utmp/d' -e '/buffer/d' > $@
	$(C) ./printf '\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n\n' \
 "ARCH = $(MYARCH)" "`uname -a || true`" "DIET = $(DIET)" "CC = $(CC)" \
 "CFLAGS = $(CFLAGS)" "LDFLAGS = $(LDFLAGS)" "FLAG_DEBUG = $(FLAG_DEBUG)" \

process_defs.h: misc/tryprocess.c all_defs.h
	$(C) rm -f $@Z; $(CC_L) -o $@Z $<
	./$@Z > $@
	$(C) sed -ne /_MASK_LEN/p -e /_SIZE/p -e /_NAME_/p -e /_CODED/p $@
	$(C) rm -f $@Z
int_defs.h: misc/tryulong32.c misc/try_int.h2
	$(C) ( ( rm -f $@Z; $(CC_L) -o $@Z $< && ./$@Z ) >/dev/null 2>&1 \
 && sed -e 's/Z/long/' misc/try_int.h2 || \
 sed -e 's/Z/int/' misc/try_int.h2 ) > $@
	$(C) sed -n -e '/typedef/p' $@; rm -f $@Z
pagesize_defs.h: misc/trypagesize.c
	$(C) ( ( rm -f $@Z; $(CC_L) -o $@Z $< && ./$@Z ) >/dev/null 2>&1 \
 && echo '#define page_size 4096' || echo '#define page_size 8192' ) > $@
	$(C) cat $@; rm -f $@Z
initreq: misc/initreq.c
	$(CCL) -o $@ $<

withdiet:
	make DIET='diet -Os'
nodiet:
	make DIET=

B=bin-$(MYARCH)
STATIC_FILES=$(shell echo '' $(ALL) | sed -e "s/  */ $(B)\//g")
$(MYARCH):
	test -d bin-$(MYARCH) || mkdir bin-$(MYARCH)
	make DIET=
	cd system/$(MYARCH) && make FLAG_DEBUG=$(FLAG_DEBUG)
	make system-$(MYARCH) DIET= LDFLAGS="$(TINY_LDFLAGS) $(LDFLAGS)"
$(MYARCH)-tiny: printf
	$(C) ./printf "\n%s\n\t%s\n\n" \
	'exec the following line:' \
	'make clean $(MYARCH) TINY_LDFLAGS=-Wl,-N'
system-$(MYARCH): $(STATIC_FILES) printf
	$(C) ./printf "\n\t%s\n\t%s\n\n" \
	'You can install ./'$(B)'/* static-files with:' \
	'cp ./'$(B)'/* . ; make install'

clean:
	rm -f $(ALL) \
 system/$(MYARCH)/*.o system/$(MYARCH)/*.a system/$(MYARCH)/system_*.S \
 *.o *.a a.out contrib/*.o djb/*.o djb/*.s S/*.o misc/*.o misc/*.s \
 lib/*.o lib/*.s *.8.gz *_defs.h *_defs.hZ x y z initreq printf \
 ninit_server.sh services.sh after-reload ninit.data \
 ninit-mmap.c ninit-huge.c Version tests_log* OTHER
	rm -rf home etc bin-* mantxt

dietlibc/bin-$(MYARCH)/diet:
	cvs -d :pserver:cvs@cvs.fefe.de:/cvs -z9 co dietlibc
	cd dietlibc && make
dietbuild: dietlibc/bin-$(MYARCH)/diet
	DIETHOME=$(CURDIR)/dietlibc make DIET="$(CURDIR)/$< -Os"
%.gz: man/%
	$(C) gzip -9c $< > $@
mantxt/%: man/%.8
	$(C) test -d mantxt || mkdir mantxt
	man ./$< | col -b > $@.man
man_txt: $(patsubst %.8.gz,mantxt/%,$(ALL_MAN))

tests: scripts/tests.sh $(ALL) Version
	./bootlog -ctr 200000 tests_log ./$<
	@sleep 1; ./printf 'Above output is saved in file: \e[1;34mtests_log\e[0;39m.\r\nRead it using the programs cat/more/less.\r\n'
strip: $(patsubst printf,,$(ALL))
	$(STR) $^ bin-$(MYARCH)/* || true

D=$(DESTDIR)
install: $(ALL) $(ALL_MAN) scripts/update.sh Version printf
	./install-bin $(D) < misc/BIN
	./install-bin $(D)/$(MAN_PAGES) < misc/MAN
	./scripts/update.sh $(D)
	$(C) ./printf "\n\t%s\n\n" \
	'Install some additional programs with:  make install_other'

install_other: $(ALL)
	sed -e 's/# //' -e /600/q misc/BIN > OTHER
	./install-bin $(D) < OTHER

start_tests: $(ALL) $(ALL_MAN) Version printf
	$(C) ./printf "\n\tStart now:  make tests\n\n"
ser_vi_ces: inittab
	./inittab /etc/inittab /etc/ninit services.sh
	./services.sh
package: distro
distro:
	umask 022 && mkdir -p /tmp/ninit.distro/$(MAN_PAGES)
	make install DESTDIR=/tmp/ninit.distro
	cd /tmp/ninit.distro && tar -cjf \
	  /tmp/$(VERSION)-$(MYARCH).tar.bz2 --owner=root --group=root *

VERSION=ninit-$(shell head -n 1 CHANGES|sed 's/:.*//')
VERSION_LONG=$(shell head -n 1 CHANGES)
TIMENOW=$(shell	date -u "+%Y-%m-%d %H:%M:%S")
CURNAME=$(notdir $(CURDIR))
RRR=--owner=root --group=root
Version: CHANGES printf
	./printf '\r\n\e[1;34m%s\e[0;39m:%s\r\n%s\e[1;35m%s\e[0;39m\r\n' \
 'NINIT'  '   Version: $(VERSION_LONG)' \
 'source:  ' 'http://riemann.fmi.uni-sofia.bg/ninit/' > Version

tests.tar:
	tar -cv $(RRR) home/default/* home/env/environ home/env/run \
	  home/sh/* home/S/* home/sleep/* | gzip -9 > home.tar.gz
rename:
	if test $(CURNAME) != $(VERSION); then cd .. && mv $(CURNAME) $(VERSION); fi
TAR=tar
TAR_OPT=
tar: rename
	sed -e "1s/^\(.*:   \)\(.*\)/\1$(TIMENOW)/" CHANGES > CHANGES.tmp
	mv CHANGES.tmp CHANGES
	cd .. && $(TAR) cvjf $(VERSION).tar.bz2 $(TAR_OPT) $(RRR) \
   --exclude $(VERSION)/dietlibc $(VERSION)
packit:
	make clean tar TAR=tar.f TAR_OPT=--sort
rpm: ninit.spec
	rpmbuild -ba --clean $<
