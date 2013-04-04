#include <stdexcept>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "tlszmq.h"

TLSZmq::TLSZmq() 
{
    init_(init_ctx_(SSLv3_client_method ()));
    SSL_set_connect_state(ssl);
}

TLSZmq::TLSZmq( 
        const char *certificate,
        const char *key)
{
    // In server mode we probably want to actually create a context
    // once per run rather than per-session as in this example. 
    SSL_CTX *ctxt = init_ctx_(SSLv3_server_method ());
    // This could do with some error checking!
    SSL_CTX_use_certificate_file(ctxt, certificate, SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(ctxt, key, SSL_FILETYPE_PEM);
    init_(ctxt);
    SSL_set_accept_state(ssl);
}

TLSZmq::~TLSZmq() {
    SSL_free(ssl);
    delete ssl_to_app;
    delete app_to_ssl;
    delete zmq_to_ssl;
    delete ssl_to_zmq;
}

void TLSZmq::update() 
{
    // Copy the data from the ZMQ message to the memory BIO
    if (zmq_to_ssl->size() > 0) {
        int rc = BIO_write(rbio, zmq_to_ssl->data(), zmq_to_ssl->size());
        printf("DEBUG: %d written to BIO\n", rc);
        zmq_to_ssl->rebuild(0);
    }
    
    // If we have app data to send, push it through SSL write, which
    // will hit the memory BIO. 
    if (app_to_ssl->size() > 0) {
        int rc = SSL_write(ssl, app_to_ssl->data(), app_to_ssl->size());
        
        if (!continue_ssl_(rc)) {
             throw std::runtime_error("An SSL error occured.");
         }
         
         printf("DEBUG: %d written to SSL\n", rc);
         if( rc == app_to_ssl->size() ) {
             app_to_ssl->rebuild(0);
         }
    }
    
    net_read_();
    net_write_();
}

bool TLSZmq::can_recv() {
    return ssl_to_app->size() > 0;
}

bool TLSZmq::needs_write() {
    return ssl_to_zmq->size() > 0;
}

zmq::message_t *TLSZmq::recv() {
    zmq::message_t *msg = new zmq::message_t(ssl_to_app->size());
    memcpy (msg->data(), ssl_to_app->data(), ssl_to_app->size());
    ssl_to_app->rebuild(0);
    return msg;
}

zmq::message_t *TLSZmq::get_data() {
    zmq::message_t *msg = new zmq::message_t(ssl_to_zmq->size());
    memcpy (msg->data(), ssl_to_zmq->data(), ssl_to_zmq->size());
    ssl_to_zmq->rebuild(0);
    return msg;
}

void TLSZmq::put_data(zmq::message_t *msg) {
    zmq_to_ssl->rebuild(msg->data(), msg->size(), NULL, NULL);
}

void TLSZmq::send(zmq::message_t *msg) {
    app_to_ssl->rebuild(msg->data(), msg->size(), NULL, NULL);
}

SSL_CTX *TLSZmq::init_ctx_(const SSL_METHOD* meth) {
    OpenSSL_add_all_algorithms();
    SSL_library_init();
    SSL_load_error_strings();
    ERR_load_BIO_strings();

    SSL_CTX *ctxt = SSL_CTX_new (meth);
    if(!ctxt) {
        ERR_print_errors_fp(stderr);
    }
    
    return ctxt;
}

void TLSZmq::init_(SSL_CTX *ctxt) 
{
    ssl = SSL_new(ctxt);
    
    rbio = BIO_new(BIO_s_mem());
    wbio = BIO_new(BIO_s_mem());
    SSL_set_bio(ssl, rbio, wbio);
    
    ssl_to_app = new zmq::message_t(0);
    app_to_ssl = new zmq::message_t(0);
    zmq_to_ssl = new zmq::message_t(0);
    ssl_to_zmq = new zmq::message_t(0);
}

void TLSZmq::net_write_() {
    std::string nwrite;
    // Read any data to be written to the network from the memory BIO
    while (1) {
        char readto[1024];
        int read = BIO_read(wbio, readto, 1024);

        if (read > 0) {
            size_t cur_size = nwrite.length();
            nwrite.resize(cur_size + read);
            std::copy(readto, readto + read, nwrite.begin() + cur_size);
        } 

        if (read != 1024) break;
    }
    
    if (!nwrite.empty()) {
        printf("DEBUG: %d read from BIO\n", (int)nwrite.length());
        ssl_to_zmq->rebuild(nwrite.length());
        memcpy(ssl_to_zmq->data(), nwrite.c_str(), nwrite.length());
    }
}

void TLSZmq::net_read_() {
    std::string aread;
    // Read data for the application from the encrypted connection and place it in the string for the app to read
    while (1) {
        char readto[1024];
        int read = SSL_read(ssl, readto, 1024);

        if (!continue_ssl_(read)) {
            throw std::runtime_error("An SSL error occured.");
        }

        if (read > 0) {
            size_t cur_size = aread.length();
            aread.resize(cur_size + read);
            std::copy(readto, readto + read, aread.begin() + cur_size);
            continue;
        }

        break;
    }
    
    if (!aread.empty()) {
        ssl_to_app->rebuild(aread.length());
        memcpy(ssl_to_app->data(), aread.c_str(), aread.length());
    }
}

bool TLSZmq::continue_ssl_(int rc) {
    int err = SSL_get_error(ssl, rc);

    if (err == SSL_ERROR_NONE || err == SSL_ERROR_WANT_READ) {
        return true;
    }

    if (err == SSL_ERROR_SYSCALL) {
        ERR_print_errors_fp(stderr);
        perror("DEBUG: syscall error: ");
        return false;
    }

    if (err == SSL_ERROR_SSL) {
        ERR_print_errors_fp(stderr);
        return false;
    }
    return true;
}


