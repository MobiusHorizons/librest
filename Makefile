CCFLAGS := -ggdb
ifeq ($(CC),cc)
SL :=.so
CCFLAGS := $(CCFLAGS) -fPIC
PKG_CONFIG :=pkg-config
else
CCFLAGS := $(CCFLAGS) -Icurl/include -DCURL_STATICLIB -shared
SL:=.dll
EXE:=.exe
LIBS:= $(shell $(PKG_CONFIG) --static --libs libcurl )
endif
all : librest$(SL)
#dropbox.exe: dropbox_api.o dropbox_main.o
#	$(CC) -o dropbox.exe dropbox_api.o dropbox_main.o -lpthread -static -ljson-c -Lcurl/lib -lrtmp $(LIBS) -liconv -lrtmp -lssl -lwinmm -lws2_32 

librest.so: rest.o buffer.o
	$(LD) -shared -o librest.so rest.o buffer.o

#librest.dll: rest.o buffer.o
#	$(LD) --no-undefined --enable-runtime-pseudo-reloc -shared -o librest.dll rest.o buffer.o -static -ljson-c -lpthread -Lcurl/lib -lrtmp $(LIBS) -liconv -lrtmp -lssl -lwinmm -lws2_32 --out-implib librestdll.a 

librest.dll: rest.o buffer.o
	$(CC)  -shared -o librest.dll rest.o buffer.o -Wl,--out-implib,librestdll.a# -static curl/lib/libcurl.a curl/lib/libssl.a curl/lib/libidn.a curl/lib/libcrypto.a curl/lib/librtmp.a curl/lib/libssh2.a curl/lib/libssl.a curl/lib/libz.a -lpthread -lws2_32 -lwinmm -lgdi32 -lwldap32 -Wl,--out-implib,librestdll.a,--no-undefined,--enable-runtime-pseudo-reloc

rest.o : rest.c rest.h
	$(CC) $(CCFLAGS) -c rest.c

buffer.o : buffer.c buffer.h
	$(CC) $(CCFLAGS) -c buffer.c
