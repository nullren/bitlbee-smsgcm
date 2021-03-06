#include "smsgcm-gnutls-creds.h"

#define MAX_BUF 1024

static int _verify_certificate_callback (gnutls_session_t session);
static char *TAG = "SMSGCM-GNUTLS-CREDS";

/**
 * must clean up datum after using it
 * g_free(obj->data); g_free(obj);
 */
static gnutls_datum_t *load_file(char *fn){
  unsigned char *contents = NULL;
  FILE *fp = fopen(fn, "r");
  long bufsize = 0L;
  size_t len = 0;

  if( fp != NULL && fseek(fp, 0L, SEEK_END) == 0){
    bufsize = ftell(fp);
    if( bufsize < 0 )
      return NULL;

    contents = g_new0(unsigned char, (bufsize + 1));

    rewind(fp);
    len = fread(contents, sizeof(char), bufsize, fp);

    if( len == 0 )
      return NULL;

    contents[len] = '\0';
  }
  fclose(fp);

  smsgcm_log(TAG, "load_file", "read contents of %s (%d)", fn, (int)len);

  gnutls_datum_t *dp12 = g_new0(gnutls_datum_t,1);
  dp12->data = contents;
  dp12->size = (int)len;

  return dp12;
}

/**
 */
void load_credentials_from_pkcs12(gpointer data){
  struct scd *conn = data;
  if( conn == NULL )
    exit(6);

  gnutls_certificate_credentials_t xcred = conn->xcred;
  smsgcm_log(TAG, "load_credentials_from_pkcs12", "address of xcreds: %p", xcred);

  struct smsgcm_data *sd = conn->data;
  struct im_connection *ic = sd->ic;
  struct credentials *creds = sd->creds;

  gnutls_pkcs12_t p12;
  if( gnutls_pkcs12_init(&p12) != 0 )
    exit(1);

  gnutls_datum_t *p12_data = load_file(creds->p12_file);
  if( p12_data == NULL )
    exit(2);

  if( gnutls_pkcs12_import(p12, p12_data, GNUTLS_X509_FMT_DER, 0) != 0 )
    exit(3);

  // free up data stuff
  gnutls_free(p12_data->data);
  gnutls_free(p12_data);

  // extract info from p12
  gnutls_x509_privkey_t pri;
  gnutls_x509_crt_t *chain;
  unsigned int chain_len;
  gnutls_x509_crt_t *extra_certs;
  unsigned int extra_certs_len;
  //gnutls_x509_crt_t crl;

  if( gnutls_pkcs12_simple_parse
          ( p12                    // gnutls_pkcs12_t
          , creds->p12_passwd      // char *
          , &pri                   // gnutls_x509_privkey_t *
          , &chain                 // gnutls_x509_crt_t **
          , &chain_len             // unsigned int *
          , &extra_certs           // gnutls_x509_crt_t **
          , &extra_certs_len       // unsigned int *
          , NULL //&crl                   // gnutls_x509_crt_t *
          , (unsigned int)0) != 0 )
    exit(4);

  // can free p12 somehow
  gnutls_pkcs12_deinit(p12);

  smsgcm_log(TAG, "load_credentials_from_pkcs12", "read contents of %s", creds->p12_file);

  gnutls_certificate_set_verify_function (xcred, _verify_certificate_callback);

  gnutls_certificate_set_x509_trust (xcred, &extra_certs[0], GNUTLS_X509_FMT_PEM);
  gnutls_certificate_set_x509_key (xcred, 
      chain, chain_len, pri);

  // clean up the stuff we made
  gnutls_x509_privkey_deinit(pri);
  gnutls_free(chain);
  gnutls_free(extra_certs);
  //gnutls_free(crl);
}


/**
 * This function will verify the peer's certificate, and check
 * if the hostname matches, as well as the activation, expiration dates.
 */
static int
_verify_certificate_callback (gnutls_session_t session)
{
  smsgcm_log(TAG, "_verify_certificate_callback", "called");
  unsigned int status;
  int ret, type;
  const char *hostname;
  gnutls_datum_t out;

  ret = gnutls_certificate_verify_peers2 (session, &status);
  if (ret < 0)
  {
    smsgcm_log(TAG, "_verify_certificate_callback", "failed veryify peers2");
    return GNUTLS_E_CERTIFICATE_ERROR;
  }

  type = gnutls_certificate_type_get (session);

  ret = gnutls_certificate_verification_status_print( status, type, &out, 0);
  if (ret < 0)
  {
    smsgcm_log(TAG, "_verify_certificate_callback", "failed verify status print");
    return GNUTLS_E_CERTIFICATE_ERROR;
  }

  if (status != 0){
    smsgcm_log(TAG, "_verify_certificate_callback", "status = %d; %s", status, out.data);
    return GNUTLS_E_CERTIFICATE_ERROR;
  }

  gnutls_free(out.data);

  /* notify gnutls to continue handshake normally */
  return 0;
}

