#
# Makefile - ADS-B Beast Radar Forwarder for 1090MHz UK Aggregator
# Author: Michael J. Tubby B.Sc. MIET  mike@tubby.org
#
BASENAME=radar
BIN=${BASENAME}
BIN_DIR=/usr/sbin/
PID_NAME=${BASENAME}.pid
PID_DIR=/var/run/
PID_FILE=${PID_DIR}${PID_NAME}
INIT_NAME=${BASENAME}.init
INIT_DIR=/etc/init.d
UNIT_NAME=${BASENAME}.unit
UNIT_DIR=/etc/systemd/system
TARGET=./$(BIN)

#CFLAGS=-Wall -Werror -Wno-error=unused-but-set-variable -std=gnu11 -g -O -I../include -DBASENAME=\"${BASENAME}\" -DPID_FILE=\"${PID_FILE}\"
#CFLAGS=-Wall -Werror -std=gnu11 -g -O -I../include -DBASENAME=\"${BASENAME}\" -DPID_FILE=\"${PID_FILE}\"
CFLAGS=-Wall -Werror -Werror=unused-result -std=gnu11 -g -O2 -I../include -DBASENAME=\"${BASENAME}\" -DPID_FILE=\"${PID_FILE}\"
OBJ=radar.o banner.o beast.o udp.o dupe.o hex.o mstime.o ustime.o sha256.o sha512.o hmac-sha256.o authtag.o stats.o telemetry.o arch.o qerror.o

DEPDIR := .d
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) -c
POSTCOMPILE = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@


#
# targets
#
all : radar

#radar : CFLAGS += -DDEBUG
radar : depend $(OBJ) defs.h
	$(CC) $(CFLAGS) $(OBJ) -o $(BIN)
	@echo "Run 'make install' to install $(BIN) as $(BIN_DIR)$(BIN)"


#
# don't mess with this unless you know what it does!
#

distclean : 
	@echo "distclean"
	rm -f *.[o] core $(BASENAME) $(TARGET) *\$$\$$\$$ *~ \#* *.old *.deb *.buildinfo *.changes radar-*.*.*.tar.gz
	rm -rf radar-*.*.*

clean : 
	@echo "clean"
	rm -f *.[o] core $(BASENAME) $(TARGET) *\$$\$$\$$ *~ \#* *.old
	rm -rf radar-*.*.*

prepare :
	@echo "prepare"
	apt-get install build-essential git bind9-dnsutils lsof net-tools
	@echo "Run 'make' to compile the code"

install : all
	install -m 755 $(BIN) $(BIN_DIR)
	@echo "Run 'make setup' to configure a new installation"

setup : all
	./setup.sh

%.o : %.c
%.o : %.c $(DEPDIR)/%.d
	$(COMPILE.c) $<
	$(POSTCOMPILE)

%.o : %.cc
%.o : %.cc $(DEPDIR)/%.d
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

%.o : %.cxx
%.o : %.cxx $(DEPDIR)/%.d
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d


.dep depend : Makefile
	touch .dep

commit:
	git commit -a
	git push --tags


#include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS))))
include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(OBJ))))
