#include "smsgcm-lib.h"

void smsgcm_load_messages(struct im_connection *ic)
{
  // look at receiveMessage and download
  char *recv = smsgcm_download_recent_messages(ic);

  imcb_log(ic, recv);

  // doesn't work - need to add buddy first, probably
  //imcb_buddy_msg(ic, "test", "lol", 0, 0);
}

char *smsgcm_download_recent_messages(struct im_connection *ic)
{
  account_t *acc = ic->acc;
  struct smsgcm_data *sd = ic->proto_data;

  // get url
  char *url = g_strdup_printf("%s%s",
      set_getstr(&acc->set, "base_url"), SMSGCM_RECV_MSG_URL);

  char *output;

  int r = get(sd->creds, url, NULL, &output);

  if( r != 0 ){
    return NULL;
  }

  return output;
}

int smsgcm_send_message(struct im_connection *ic, char *addr, char *msg)
{
  account_t *acc = ic->acc;
  struct smsgcm_data *sd = ic->proto_data;

  struct post_item p[] =
      { {"address", addr}
      , {"message", msg}
      };

  char *post = make_query_string(p, 2);
  char *url = g_strdup_printf("%s%s",
      set_getstr(&acc->set, "base_url"), SMSGCM_SEND_MSG_URL);

  int r = get(sd->creds, url, post, NULL);

  return r;
}
