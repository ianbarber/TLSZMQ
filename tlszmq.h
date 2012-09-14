/*
 * Quick and dirty class to wrap data in TLS for use over ZeroMQ
 * Based on code from http://funcptr.net/2012/04/08/openssl-as-a-filter-%28or-non-blocking-openssl%29/
 */

#include <openssl/ssl.h>
#include <zmq.hpp>

class TLSZmq {
    public:
        TLSZmq();
        TLSZmq( const char *certificate,
                const char *key);
        virtual ~TLSZmq();

        void update();
        
        bool can_recv();
        bool needs_write();
        
        zmq::message_t *recv();
        zmq::message_t *get_data();
        void put_data(zmq::message_t *msg);
        void send(zmq::message_t *msg);

    private:
        void init_(SSL_CTX *ctxt);
        SSL_CTX *init_ctx_(SSL_METHOD *meth);
        
        bool continue_ssl_(int function_return);
        void net_read_();
        void net_write_();

        SSL * ssl;
        BIO * rbio;
        BIO * wbio;
        
        zmq::message_t *app_to_ssl;
        zmq::message_t *ssl_to_app;
        zmq::message_t *ssl_to_zmq;
        zmq::message_t *zmq_to_ssl;
};
