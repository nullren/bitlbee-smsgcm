#include "smsgcm.h"

GSList *smsgcm_connections = NULL;

/* main loop */
gboolean smsgcm_main_loop(gpointer data, gint fd, b_input_condition cond)
{
  struct im_connection *ic = data;

  // Check if we are still logged in...
  if (!g_slist_find(smsgcm_connections, ic))
    return 0;

  // Do stuff..
  smsgcm_load_messages(ic);

  // If we are still logged in run this function again after timeout.
  return (ic->flags & OPT_LOGGED_IN) == OPT_LOGGED_IN;
}

static void smsgcm_main_loop_start(struct im_connection *ic)
{
  struct smsgcm_data *sd = ic->proto_data;

  sd->main_loop_id = b_timeout_add(set_getint(&ic->acc->set, "fetch_interval") * 1000, smsgcm_main_loop, ic);
}

/* message sent to buddy - use imcb_buddy_msg(ic, handle, msg, 0, 0) to display message */
static int smsgcm_buddy_msg(struct im_connection *ic, char *who, char *message, int flags)
{
  return 0;
}

static void smsgcm_init(account_t *acc)
{
  //set some settings i guess
  set_t *s;

  char *def_url = SMSGCM_API_URL;

  s = set_add(&acc->set, "fetch_interval", "60", set_eval_int, acc);
  s->flags |= ACC_SET_OFFLINE_ONLY;

  s = set_add(&acc->set, "base_url", def_url, NULL, acc);
  s->flags |= ACC_SET_OFFLINE_ONLY;

  s = set_add(&acc->set, "client_cert", "CHANGE ME", NULL, acc);
  s->flags |= ACC_SET_OFFLINE_ONLY;

  s = set_add(&acc->set, "ca_cert", "CHANGE ME", NULL, acc);
  s->flags |= ACC_SET_OFFLINE_ONLY;

}

static void smsgcm_login(account_t *acc)
{
  struct im_connection *ic = imcb_new(acc);
  struct smsgcm_data *sd = g_new0(struct smsgcm_data, 1);
  ic->proto_data = sd;
  sd->ic = ic;

  // check that we can read or find the certs
  char *client_path = set_getstr(&ic->acc->set, "client_cert");
  char *ca_path = set_getstr(&ic->acc->set, "ca_cert");

  // try to open certs
  FILE *cl_file = fopen(client_path, "r");
  if( cl_file == NULL ){
    imcb_error(ic, "Cannot open client credentials: %s", client_path);
    imc_logout(ic, FALSE);
    return;
  } else
    fclose(cl_file);

  FILE *ca_file = fopen(ca_path, "r");
  if( ca_file == NULL ){
    imcb_error(ic, "Cannot open CA certificate: %s", ca_path);
    imc_logout(ic, FALSE);
    return;
  } else
    fclose(ca_file);

  sd->creds = g_new0(struct credentials, 1);
  sd->creds->client = client_path;
  sd->creds->ca = ca_path;

  imcb_connected(ic);
  smsgcm_connections = g_slist_append(smsgcm_connections, ic);

  smsgcm_main_loop_start(ic);
}

static void smsgcm_logout(struct im_connection *ic)
{
  // really nothing to do
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

  register_protocol(ret);
}
