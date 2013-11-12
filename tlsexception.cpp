#include <openssl/ssl.h>
#include "tlsexception.h"

TLSException::TLSException(const char *msg) : runtime_error(msg) {
}

TLSException::TLSException(int err) : runtime_error(SSL_alert_desc_string_long(err)) {
}
