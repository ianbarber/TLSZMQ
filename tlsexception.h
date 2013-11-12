#ifndef _TLS_EXCEPTION_H
#define _TLS_EXCEPTION_H
#include <stdexcept>

class TLSException : public std::runtime_error {
public:
    TLSException(const char *msg);
    TLSException(int ssl_error_code);
    virtual ~TLSException() throw() {};
};

#endif // _TLS_EXCEPTION_H
