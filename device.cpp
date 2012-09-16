#include <pthread.h>
#include <unistd.h>
#include <cassert>
#include <string>
#include <iostream>
#include <zmq.hpp>

int main ()
{
    //  Prepare our context and sockets
    zmq::context_t context (1);
    zmq::socket_t servers (context, ZMQ_DEALER);
    servers.bind ("tcp://*:5555");
    zmq::socket_t clients (context, ZMQ_ROUTER);
    clients.bind ("tcp://*:5556");

	zmq_device (ZMQ_FORWARDER, (void *)clients, (void *)servers);

    return 0;
}