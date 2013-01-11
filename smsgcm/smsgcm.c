#include "smsgcm.h"

static void smsgcm_init(account_t *acc)
{
  //set some settings i guess
  set_t *s;

  char *def_url = SMSGCM_API_URL;

  s = set_add(&acc->set, "fetch_interval", "60", set_eval_int, acc);
  s->flags |= ACC_SET_OFFLINE_ONLY;

  s = set_add(&acc->set, "base_url", def_url, NULL, acc);
  s->flags |= ACC_SET_OFFLINE_ONLY;

  s = set_add(&acc->set, "client_cert", "", NULL, acc);
  s->flags |= ACC_SET_OFFLINE_ONLY;

  s = set_add(&acc->set, "ca_cert", "", NULL, acc);
  s->flags |= ACC_SET_OFFLINE_ONLY;

}

static void smsgcm_login(account_t *acc)
{
  struct im_connection *ic = imcb_new(acc);

  // check that we can read or find the certs
  char *client_path = set_getstr(&ic->acc->set, "client_cert");
  char *ca_path = set_getstr(&ic->acc->set, "ca_cert");

  // try to open certs
  FILE *cl_file = fopen(client_path, "r");
  FILE *ca_file = fopen(ca_path, "r");

  if( cl_file == NULL ){
    imcb_error(ic, "Cannot open client credentials: %s", client_path);
    imc_logout(ic, FALSE);
  }

  if( cl_file == NULL ){
    imcb_error(ic, "Cannot open CA certificate: %s", ca_path);
    imc_logout(ic, FALSE);
  }

  fclose(cl_file);
  fclose(ca_file);
}

static void smsgcm_logout(struct im_connection *ic)
{
  // really nothing to do
}

void init_plugin()
{
  struct prpl *ret = g_new0(struct prpl, 1);

  ret->options = OPT_NOOTR;
  ret->name = "smsgcm";
  ret->init = smsgcm_init;
  ret->login = smsgcm_login;
  ret->logout = smsgcm_logout;

  register_protocol(ret);
}
