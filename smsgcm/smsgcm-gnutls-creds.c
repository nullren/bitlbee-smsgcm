/* This example code is placed in the public domain. */

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

#define MAX_BUF 1024

static int _verify_certificate_callback (gnutls_session_t session);

static gnutls_datum_t *load_file(char *fn){
  unsigned char *contents = NULL;
  FILE *fp = fopen(fn, "r");
  long bufsize = 0L;
  size_t len = 0;

  if( fp != NULL && fseek(fp, 0L, SEEK_END) == 0){
    long bufsize = ftell(fp);
    if( bufsize < 0 )
      return NULL;

    contents = malloc(sizeof(char) * (bufsize + 1));

    rewind(fp);
    len = fread(contents, sizeof(char), bufsize, fp);

    if( len == 0 )
      return NULL;

    contents[len+1] = '\0';
  }
  fclose(fp);

  gnutls_datum_t *dp12 = (gnutls_datum_t *)malloc(sizeof(gnutls_datum_t));
  dp12->data = contents;
  dp12->size = (int)len;

  return dp12;
}

void load_credentials_from_pkcs12(gpointer data, gnutls_certificate_credentials_t xcred){
  struct im_connection *ic = data;
  struct credentials *creds = ic->proto_data->creds;
  gnutls_pkcs12_t p12;
  if( gnutls_pkcs12_init(&p12) != 0 )
    exit(1);

  gnutls_datum_t *data = load_file(creds->p12_file);
  if( data == NULL )
    exit(2);

  if( gnutls_pkcs12_import(p12, data, GNUTLS_X509_FMT_DER, 0) != 0 )
    exit(3);

  gnutls_x509_privkey_t pri;
  gnutls_x509_crt_t *chain;
  unsigned int chain_len;
  gnutls_x509_crt_t *extra_certs;
  unsigned int extra_certs_len;
  gnutls_x509_crt_t crl;

  if( gnutls_pkcs12_simple_parse(p12, creds->p12_passwd, &pri
          , &chain, &chain_len, &extra_certs, &extra_certs_len, &crl
          , 0) != 0 )
    exit(4);

  gnutls_certificate_set_x509_trust (xcred, &extra_certs[0], GNUTLS_X509_FMT_PEM);
  gnutls_certificate_set_verify_function (xcred, _verify_certificate_callback);

   gnutls_certificate_set_x509_key (xcred, 
      chain, chain_len, pri);
}


/* This function will verify the peer's certificate, and check
 * if the hostname matches, as well as the activation, expiration dates.
 */
static int
_verify_certificate_callback (gnutls_session_t session)
{
  unsigned int status;
  int ret, type;
  const char *hostname;
  gnutls_datum_t out;

  /* read hostname */
  hostname = gnutls_session_get_ptr (session);

  /* This verification function uses the trusted CAs in the credentials
   * structure. So you must have installed one or more CA certificates.
   */
  ret = gnutls_certificate_verify_peers3 (session, hostname, &status);
  if (ret < 0)
  {
    printf ("Error\n");
    return GNUTLS_E_CERTIFICATE_ERROR;
  }

  type = gnutls_certificate_type_get (session);

  ret = gnutls_certificate_verification_status_print( status, type, &out, 0);
  if (ret < 0)
  {
    printf ("Error\n");
    return GNUTLS_E_CERTIFICATE_ERROR;
  }

  printf ("%s", out.data);

  gnutls_free(out.data);

  if (status != 0) /* Certificate is not trusted */
    return GNUTLS_E_CERTIFICATE_ERROR;

  /* notify gnutls to continue handshake normally */
  return 0;
}

