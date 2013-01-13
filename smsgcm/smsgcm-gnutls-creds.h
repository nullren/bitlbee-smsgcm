#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>
#include <gnutls/pkcs12.h>
#include <bitlbee.h>

#include "smsgcm.h"

void load_credentials_from_pkcs12(gpointer data, gnutls_certificate_credentials_t xcred);
