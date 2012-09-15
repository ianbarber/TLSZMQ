CC      = g++
CFLAGS  = -g
LDFLAGS = -lzmq -lssl -lcrypto

all: tlsserver tlsclient

tlsserver: tlsserver.cpp tlszmq.cpp
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

tlsclient: tlsclient.cpp tlszmq.cpp
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

clean:
	rm tlsclient
	rm tlsserver