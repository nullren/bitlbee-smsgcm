#include "smsgcm-lib.h"


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

static void smsgcm_add_buddy(struct im_connection *ic, char *name, const char *phone)
{
  struct smsgcm_data *sd = ic->proto_data;

  // Check if the buddy is already in the buddy list.
  if (!bee_user_by_handle(ic->bee, ic, name)) {
    imcb_log(ic, "add buddy for %s", name);
    return;
    imcb_add_buddy(ic, name, NULL);
    imcb_rename_buddy(ic, name, phone);
    imcb_buddy_nick_hint(ic, name, name);
    imcb_buddy_status(ic, name, OPT_LOGGED_IN, NULL, NULL);
  }
}

void smsgcm_load_messages(struct im_connection *ic)
{
  // look at receiveMessage and download
  char *recv = smsgcm_download_recent_messages(ic);

  json_t *root;
  json_error_t error;

  root = json_loads(recv, 0, &error);
  g_free(recv);

  if(!json_is_array(root)){
    imcb_error(ic, "malformed json sent from server: was expecting array");
    return;
  }

  int i;
  for(i = 0; i < json_array_size(root); i++){
    json_t *data = json_array_get(root, i);

    if(!json_is_object(data)){
      imcb_error(ic, "malformed json sent from server: was expecting object");
      continue;
    }

    json_t *name = json_object_get(data, "name");
    if(!json_is_string(name)){
      imcb_error(ic, "malformed json sent from server: was expecting name string");
      continue;
    }

    json_t *address = json_object_get(data, "address");
    if(!json_is_string(address)){
      imcb_error(ic, "malformed json sent from server: was expecting address string");
      continue;
    }

    json_t *message = json_object_get(data, "message");
    if(!json_is_string(message)){
      imcb_error(ic, "malformed json sent from server: was expecting message string");
      continue;
    }

    struct message_data *msg = g_new0(struct message_data, 1);
    msg->name = json_string_value(name);
    msg->address = json_string_value(address);
    msg->message = json_string_value(message);

    imcb_log(ic, "read message: %s (%s): %s", msg->name
                                            , msg->address
                                            , msg->message);

    smsgcm_add_buddy(ic, msg->name, msg->address);
  }

  json_decref(root);
  return;

}
