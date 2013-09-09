CXX      ?= g++
CXXFLAGS ?= -g
PKGS     = libzmq openssl
MY_CXXFLAGS = $$($(PKG_CONFIG) --cflags $(PKGS))
PKG_CONFIG ?= pkg-config
LIBS     = $$($(PKG_CONFIG) --libs $(PKGS))
LIBEXT   ?= .so
LIBCOMPILECXXFLAGS ?= -fPIC
LIBNAME  ?= libtlszmq$(LIBEXT).0
LIBCXXFLAGS ?= -shared -Wl,-soname,$(LIBNAME)
LN_S     ?= ln -s

all: tlsserver tlsclient device
.SUFFIXES: .cpp .o .lo

LIBTLSZMQ = libtlszmq$(LIBEXT)
tlszmq.o tlszmq.lo: tlszmq.h
$(LIBNAME): tlszmq.lo
	$(CXX) $(LIBCXXFLAGS) $(CXXFLAGS) -o $(LIBNAME) tlszmq.lo $(LIBS)
$(LIBTLSZMQ): $(LIBNAME)
	rm -f $(LIBTLSZMQ)
	$(LN_S) $(LIBNAME) $(LIBTLSZMQ)

tlsserver.o: tlszmq.h
tlsserver: tlsserver.o $(LIBTLSZMQ)
	$(CXX) $(CPPFLAGS) $(MY_CXXFLAGS) $(CXXFLAGS) $(LDFLAGS) -o '$(@)' $(^) $(LIBS)

tlsclient.o: tlszmq.h
tlsclient: tlsclient.o $(LIBTLSZMQ)
	$(CXX) $(CPPFLAGS) $(MY_CXXFLAGS) $(CXXFLAGS) -o '$(@)' $(^) $(LIBS)

device: device.o
	$(CXX) $(CPPFLAGS) $(MY_CXXFLAGS) $(CXXFLAGS) -o '$(@)' $(^) $(LIBS)

.cpp.o:
	$(CXX) -c $(CPPFLAGS) $(MY_CXXFLAGS) $(CXXFLAGS) -o '$(@)' '$(<)'
.cpp.lo:
	$(CXX) -c $(CPPFLAGS) $(LIBCOMPILECXXFLAGS) $(CXXFLAGS) -o '$(@)' '$(<)'


clean:
	rm -f tlsclient tlsserver device *.o *.lo $(LIBNAME) $(LIBTLSZMQ)
