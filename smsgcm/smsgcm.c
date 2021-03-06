#include "smsgcm.h"

GSList *smsgcm_connections = NULL;

static void smsgcm_poll_messages(struct im_connection *ic);
static char *TAG = "SMSGCM";

/* main loop */
gboolean smsgcm_main_loop(gpointer data, gint fd, b_input_condition cond)
{
  struct im_connection *ic = data;

  // Check if we are still logged in...
  if (!g_slist_find(smsgcm_connections, ic))
    return 0;

  // Do stuff..
  smsgcm_poll_messages(ic);

  // If we are still logged in run this function again after timeout.
  return (ic->flags & OPT_LOGGED_IN) == OPT_LOGGED_IN;
}

void smsgcm_add_buddy(gpointer data, char *name, char *group){
  smsgcm_lib_add_buddy(data, NULL, name);
}

void smsgcm_set_name(gpointer data, char *handle, char *nick){
  smsgcm_lib_add_buddy(data, nick, handle);
}

gboolean smsgcm_ssl_read_cb(gpointer data, gint fd, b_input_condition cond)
{
  struct scd *ssl = data;
  struct smsgcm_data *sd = ssl->data;
  struct im_connection *ic = sd->ic;

  char buf[10240];

  if(sd == NULL || ssl_getfd(ssl) == -1)
    return FALSE;

  int st = ssl_read(ssl, buf, sizeof(buf));

  buf[st] = '\0';

  int skip = 4;
  char *end_headers = strstr(buf, "\r\n\r\n");
  if( end_headers == NULL ){
    skip = 2;
    end_headers = strstr(buf, "\n\n");
  }

  if( end_headers == NULL ){
    return FALSE;
  }

  char *body = end_headers + skip;

  if(st > 0){
    if( strstr(body, "[]") == NULL )
      smsgcm_log(TAG, "smsgcm_ssl_read_cb", "read contents: %s", body);
    if( sd->queued != NULL ){
      g_free(sd->queued);
      sd->queued = NULL;
      smsgcm_log(TAG, "smsgcm_ssl_read_cb", "there was a message queued, so we cleared it: %s", (char *)sd->queued);
    }

    smsgcm_load_messages(ic, body);
  }else{
    smsgcm_log(TAG, "smsgcm_ssl_read_cb", "did not read anything");
  }

  /* TODO: now clean up */
  g_free(sd);
  g_free(ssl);
  sd = ssl = NULL;

  return st;
}

gboolean smsgcm_ssl_connected(gpointer data, int returncode, void *source, b_input_condition cond)
{

  struct smsgcm_data *sd = data;
  struct im_connection *ic = sd->ic;
  struct scd *ssl = source;

  if(ssl == NULL){
    imcb_error(ic, "%s (%d): %s", gnutls_strerror_name(returncode), returncode, gnutls_strerror(returncode));
    return FALSE;
  }

  if(sd == NULL){
    imcb_error(ic, "no smsgcm data");
    return FALSE;
  }

  sd->bfd = b_input_add(ssl_getfd(ssl), B_EV_IO_READ, smsgcm_ssl_read_cb, ssl);

  char getstr[10240];
  char *template = "GET /%s?%s HTTP/1.0\r\n\r\n";

  if(sd->queued != NULL){
    char *addr = sd->queued->address;
    char *mesg = sd->queued->message;
    struct post_item p[] =
        { {"address", addr} , {"message", mesg} , {"dump", "1"} };

    char *qs = make_query_string(p, 3);
    if(qs == NULL){
      imcb_error(ic, "memory error making query string!");
      return FALSE;
    }
    g_sprintf(getstr, template, "send", qs);

    g_free(qs); qs = NULL;
    g_free(addr); addr = NULL;
    g_free(mesg); mesg = NULL;
    smsgcm_log(TAG, "smsgcm_ssl_connected", "sending '%s' to server", getstr);
  }else{
    g_sprintf(getstr, template, "received", "dump=1");
  }

  int s = ssl_write(ssl, getstr, strlen(getstr));

  return s;
}

static void smsgcm_poll_messages(struct im_connection *ic){
  struct smsgcm_data *sd = g_new0(struct smsgcm_data, 1);

  sd->ic = ic;

  struct smsgcm_data *sdata = ic->proto_data;
  sd->creds = sdata->creds;

  // calling this will put ssl_data in ssl->data
  struct scd *ssl = ssl_connect_with_creds("smsgcm.omgren.com", 443, TRUE
                        , smsgcm_ssl_connected, (ssl_credentials_func)load_credentials_from_pkcs12, sd);

  if( ssl == NULL ){
    imcb_error(ic, "ssl empty??");
    imc_logout(ic, TRUE);
  }
}

static void smsgcm_post_message(struct im_connection *ic, char *address, char *message){
  struct smsgcm_data *sd = g_new0(struct smsgcm_data, 1);

  sd->ic = ic;
  sd->queued = g_new0(struct queue_message, 1);

  sd->queued->address = g_strdup(address);
  sd->queued->message = g_strdup(message);

  struct smsgcm_data *sdata = ic->proto_data;
  sd->creds = sdata->creds;

  // calling this will put ssl_data in ssl->data
  struct scd *ssl = ssl_connect_with_creds("smsgcm.omgren.com", 443, TRUE
                        , smsgcm_ssl_connected, (ssl_credentials_func)load_credentials_from_pkcs12, sd);

  if( ssl == NULL ){
    imcb_error(ic, "ssl empty??");
    imc_logout(ic, TRUE);
  }
}

static void smsgcm_main_loop_start(struct im_connection *ic)
{
  struct smsgcm_data *sd = ic->proto_data;

  smsgcm_poll_messages(ic);

  sd->main_loop_id = b_timeout_add(set_getint(&ic->acc->set, "fetch_interval") * 1000, smsgcm_main_loop, ic);
}

/* message sent to buddy - use imcb_buddy_msg(ic, handle, msg, 0, 0) to display message */
static int smsgcm_buddy_msg(struct im_connection *ic, char *who, char *message, int flags)
{
  smsgcm_log(TAG, "smsgcm_buddy_msg", "tried to send a buddy message to %s", who);
  smsgcm_post_message(ic, who, message);
  return 0;
}

static void smsgcm_init(account_t *acc)
{
  //set some settings i guess
  set_t *s;

  char *def_url = SMSGCM_API_URL;

  s = set_add(&acc->set, "fetch_interval", "60", set_eval_int, acc);

  s = set_add(&acc->set, "base_url", def_url, NULL, acc);
  s->flags |= ACC_SET_OFFLINE_ONLY;

  s = set_add(&acc->set, "p12_file", "CHANGE ME", NULL, acc);
  s->flags |= ACC_SET_OFFLINE_ONLY;

  s = set_add(&acc->set, "p12_passwd", "CHANGE ME", NULL, acc);
  s->flags |= ACC_SET_OFFLINE_ONLY;

}

static void smsgcm_login(account_t *acc)
{
  struct im_connection *ic = imcb_new(acc);
  struct smsgcm_data *sd = g_new0(struct smsgcm_data, 1);
  ic->proto_data = sd;
  sd->ic = ic;

  ic->flags |= OPT_SLOW_LOGIN;

  // check that we can read or find the certs
  char *p12_file = set_getstr(&ic->acc->set, "p12_file");
  char *p12_passwd = set_getstr(&ic->acc->set, "p12_passwd");

  if( g_file_test(p12_file, G_FILE_TEST_EXISTS) != TRUE ){
    imcb_error(ic, "Cannot find client credentials: %s", p12_file);
    imc_logout(ic, FALSE);
    return;
  }

  sd->creds = g_new0(struct credentials, 1);
  sd->creds->p12_file = p12_file;
  sd->creds->p12_passwd = p12_passwd;

  imcb_connected(ic);
  smsgcm_connections = g_slist_append(smsgcm_connections, ic);

  smsgcm_main_loop_start(ic);
}

static void smsgcm_logout(struct im_connection *ic)
{
  struct smsgcm_data *sd = ic->proto_data;
  g_free(sd->creds);
  g_free(sd);
  smsgcm_connections = g_slist_remove(smsgcm_connections, ic);
}

void init_plugin()
{
  struct prpl *ret = g_new0(struct prpl, 1);

  ret->options = OPT_NOOTR;
  ret->name = "smsgcm";
  ret->init = smsgcm_init;
  ret->login = smsgcm_login;
  ret->logout = smsgcm_logout;
  ret->buddy_msg = smsgcm_buddy_msg;
  ret->handle_cmp = g_strcasecmp;
  ret->add_buddy = smsgcm_add_buddy;
  ret->set_name = smsgcm_set_name;

  register_protocol(ret);
}
