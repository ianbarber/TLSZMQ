#include "tlszmq.h"
#include <zmq.hpp>

int main(int argc, char* argv[]) {
    try {
        zmq::context_t ctx(1);
        zmq::socket_t s1(ctx,ZMQ_REQ);
        s1.connect ("tcp://localhost:5556");
        TLSZmq *tls = new TLSZmq();
        bool loop = true;
        zmq::message_t request (12);  
        memcpy(request.data(), "hello world!", 12);
        tls->send(&request);
        while (loop == true) {
            tls->update();
            
            // If we need to send to the network, do so
            if(tls->needs_write()) {
                zmq::message_t *data = tls->get_data();
                s1.send(*data);
                
                // Push message to TLS and update
                zmq::message_t response;
                s1.recv (&response);
                tls->put_data(&response);
                tls->update();
                
                delete data;
            }
                    
             // If there is app data, retrieve it
            if(tls->can_recv()) {
                zmq::message_t *data = tls->recv();
                printf("Received: %s\n", (char *)(data->data()), data->size());
                loop = false;
                delete data;
            } 
        }
        delete tls;
    }
    catch(std::exception &e) {
        printf ("An error occurred: %s\n", e.what());
        return 1;
    }
    return 0;
}
