# vi: set tabstop=4 noautoindent:


VERSION = 1.3.0

#
#
SUBJ = ltictr_proxy_server ltictr_api_server
#
all: $(SUBJ)

#
CC   = gcc
AR   = ar
TH   = touch
SV   = svn

LIB_PATH = ..

LIB_DIR = $(LIB_PATH)/JunkBox_Lib
LIB_BSC_DIR = $(LIB_DIR)/Lib
LIB_EXT_DIR = $(LIB_DIR)/xLib

LIB_BSC = $(LIB_BSC_DIR)/libbasic.a
LIB_EXT = $(LIB_EXT_DIR)/libextend.a

#DEBUG = -DEBUG

CFLAGS  = -fPIC -W -Wall -DHAVE_CONFIG_H -DENABLE_SSL -I/usr/include -I$(LIB_DIR) -I$(LIB_BSC_DIR) -I$(LIB_EXT_DIR) $(DEBUG) 

SLIB = -L$(LIB_BSC_DIR) -L/usr/local/ssl/lib -lbasic -lssl -lm -lcrypto -lcrypt -lz
ELIB = -L$(LIB_EXT_DIR) -lextend
#
#
$(LIB_DIR)/config.sh : 
	(cd $(LIB_PATH) && $(SV) co http://www.nsl.tuis.ac.jp/svn/linux/JunkBox_Lib/trunk JunkBox_Lib)

$(LIB_DIR)/configure: $(LIB_DIR)/config.sh
	(cd $(LIB_DIR) && ./config.sh)

$(LIB_DIR)/Makefile: $(LIB_DIR)/configure
	(cd $(LIB_DIR) && ./configure --enable-ssl)

$(LIB_BSC): $(LIB_DIR)/Makefile
	(cd $(LIB_DIR) && make)

$(LIB_EXT): $(LIB_DIR)/Makefile
	(cd $(LIB_DIR) && make)


#
#
.h.c:
	$(TH) $@


.c.o: 
	$(CC) $< $(CFLAGS) -c -O2 

#
#
ltictr_proxy_server: $(LIB_BSC) $(LIB_EXT) ltictr_proxy_server.o ltictr_proxy.o ltictr_relay.o ltictr_rest.o ltictr_ws.o
	$(CC) $(@).o ltictr_proxy.o ltictr_relay.o ltictr_rest.o ltictr_ws.o $(ELIB) $(SLIB) -O2 -o $@ 
	strip ltictr_proxy_server
	#cp ltictr_proxy_server ../bin

ltictr_api_server: $(LIB_BSC) $(LIB_EXT) ltictr_api_server.o ltictr_api.o 
	$(CC) $(@).o ltictr_api.o $(ELIB) $(SLIB) -O2 -o $@ 
	strip ltictr_api_server
	#cp ltictr_api_server ../bin


install: ltictr_proxy_server  ltictr_api_server
	install -m 0755 ltictr_proxy_server  /usr/local/bin
	install -m 0755 ltictr_api_server    /usr/local/bin
	[ -f /usr/local/etc/ltictr_proxy.conf ]             || install -m 0640 ltictr_proxy.conf     /usr/local/etc
	[ -f /usr/lib/systemd/system/ltictr_proxy.service ] || install -m 0644 ltictr_proxy.service  /usr/lib/systemd/system
	systemctl daemon-reload


git: ltictr_proxy_server  ltictr_api_server
	cp ltictr_proxy_server ../bin
	cp ltictr_api_server ../bin


clean:
	rm -f *.o $(SUBJ) 


tgz:
	make clean
	(cd .. && tar zcf ltictr_proxy-${VERSION}.tgz ltictr_proxy)

