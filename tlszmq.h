/*
 * Quick and dirty class to wrap data in TLS for use over ZeroMQ
 * Based on code from http://funcptr.net/2012/04/08/openssl-as-a-filter-%28or-non-blocking-openssl%29/
 */

#ifndef _TLSZMQ_H
#define _TLSZMQ_H

#include <openssl/ssl.h>
#include <zmq.hpp>

class TLSZmq {
    public:
		enum {SSL_CLIENT = 0, SSL_SERVER = 1};
		static SSL_CTX *ssl_ctx;
		static SSL_CTX *init_ctx(int mode);

        TLSZmq(SSL_CTX *ctx);
        TLSZmq( SSL_CTX *ctx,
        		const char *certificate,
                const char *key);
        virtual ~TLSZmq();

        bool can_recv();
        bool needs_write();
        
        zmq::message_t *read();
        void write(zmq::message_t *msg);

        zmq::message_t *get_data();
        void put_data(zmq::message_t *msg);

        void shutdown();

    private:
        void init_(SSL_CTX *ctxt);
        void update();
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

#endif /* _TLSZMQ_H */
