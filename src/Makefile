#
# Operating System
OS = Linux
#OS = Solaris
#OS = AIX

# CPU
CPU = Intel
#CPU = Motorola

#
#
SUBJ = unlock_podman_containers ipynb_tocsv ipynb_extract ipynb_conv json_file json2
#
all: $(SUBJ)

#
CC   = gcc
AR   = ar
TH	 = touch
SV   = svn
GT   = git

LIB_PATH = ..

LIB_DIR = $(LIB_PATH)/JunkBox_Lib
LIB_BSC_DIR = $(LIB_DIR)/Lib
LIB_EXT_DIR = $(LIB_DIR)/xLib

LIB_BSC = $(LIB_BSC_DIR)/libbasic.a
LIB_EXT = $(LIB_EXT_DIR)/libextend.a

#DEBUG = -DEBUG

CFLAGS  = -fPIC -DHAVE_CONFIG_H -I$(LIB_DIR) -I$(LIB_BSC_DIR) -I$(LIB_EXT_DIR) -D$(OS) -D$(CPU) $(DEBUG)

SLIB = -L$(LIB_BSC_DIR) -lbasic -lm
ELIB = -L$(LIB_EXT_DIR) -lextend
#
#
$(LIB_DIR)/config.sh :
	(cd $(LIB_PATH) && $(GT) clone https://github.com/Junk-Box-Library/JunkBox_Lib.git)

$(LIB_DIR)/configure: $(LIB_DIR)/config.sh
	(cd $(LIB_DIR) && ./config.sh)

$(LIB_DIR)/Makefile: $(LIB_DIR)/configure
	(cd $(LIB_DIR) && ./configure --enable-ssl)

$(LIB_BSC): $(LIB_DIR)/Makefile
	(cd $(LIB_DIR) && make)

$(LIB_EXT): $(LIB_DIR)/Makefile
	(cd $(LIB_DIR) && make)


.h.c:
	$(TH) $@


.c.o:
	$(CC) $< $(CFLAGS) -c -O2 


clean:
	rm -f *.o *~ $(SUBJ)


#
#
#
#
unlock_podman_containers: $(LIB_BSC) unlock_podman_containers.o
	$(CC) $(@).o $(ELIB) $(SLIB) -O2 -o $@   
	strip $(@)
	cp $(@) ../bin

ipynb_conv: $(LIB_BSC) ipynb_conv.o
	$(CC) $(@).o $(ELIB) $(SLIB) -O2 -o $@   
	strip $(@)
	cp $(@) ../bin

json_file: $(LIB_BSC) json_file.o
	$(CC) $(@).o $(ELIB) $(SLIB) -O2 -o $@ 
	strip $(@)
	cp $(@) ../bin

json2: $(LIB_BSC) json2.o
	$(CC) $(@).o $(ELIB) $(SLIB) -O2 -o $@ 
	strip $(@)

ipynb_extract: $(LIB_BSC) ipynb_extract.o
	$(CC) $(@).o $(ELIB) $(SLIB) -O2 -o $@ 
	strip $(@)
	cp $(@) ../bin

ipynb_tocsv: $(LIB_BSC) ipynb_tocsv.o
	$(CC) $(@).o $(ELIB) $(SLIB) -O2 -o $@ 
	strip $(@)
	cp $(@) ../bin

