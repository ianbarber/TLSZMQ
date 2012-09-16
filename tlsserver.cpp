#include "tlszmq.h"
#include <string>
#include <map>
#include <zmq.hpp>

// Simple REP server
int main(int argc, char* argv[]) {
    try {
        zmq::context_t ctx(1);
        zmq::socket_t s1(ctx,ZMQ_ROUTER);
        //s1.connect ("tcp://localhost:5555");
        s1.bind ("tcp://*:5556");
        std::string s_crt("server.crt");
        std::string s_key("server.key");
        std::map<std::string, TLSZmq*> conns;
            
        while (true) {
            // Wait for a message
            zmq::message_t identifier(0);
            zmq::message_t request(0);
            std::string ident;
			size_t size;
            
            // Ignore seperator & intermediate ID stack
            do {	
            	s1.recv (&request);
				size = request.size();
            	if (size > 0) {
					ident.assign(static_cast<char*>(request.data()), request.size());
				}
            	s1.send(request, ZMQ_SNDMORE);
			} while(size > 0);
            
            // Retrieve or create the TLSZmq handler for this client
            TLSZmq *tls;
            if(conns.find(ident) == conns.end()) {
                tls = new TLSZmq(s_crt.c_str(), s_key.c_str());
                conns[ident] = tls;
            } else {
                tls = conns[ident];
            }
            
            // Push message on to TLS and update
            s1.recv (&request);
            tls->put_data(&request);
            tls->update();
            
            // If there is app data, retrieve it
            if(tls->can_recv()) {
                zmq::message_t *data = tls->recv();
                printf("Received: %s\n",static_cast<char*>(data->data()));
                zmq::message_t response(8);
                snprintf ((char *) response.data(), 8 ,"Got it!");
                tls->send(&response);
                tls->update();
                delete data;
            }
            
            // If we need to send to the network, do so
            if(tls->needs_write()) {
                zmq::message_t *data = tls->get_data();
                s1.send(*data);
            }
        }
    }
    catch(std::exception &e) {
        printf ("An error occurred: %s\n", e.what());
        return 1;
    }
    return 0;
}