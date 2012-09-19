CC      = g++
CFLAGS  += -g
LDFLAGS += -lzmq -lssl -lcrypto

all: tlsserver tlsclient device

tlsserver: tlsserver.cpp tlszmq.cpp
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

tlsclient: tlsclient.cpp tlszmq.cpp
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

device: device.cpp
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

clean:
	rm tlsclient
	rm tlsserver