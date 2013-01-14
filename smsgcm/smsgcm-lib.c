#include "smsgcm-lib.h"

static char *TAG = "SMSGCM-LIB";

static void smsgcm_add_buddy(struct im_connection *ic, char *name, char *phone)
{
  struct smsgcm_data *sd = ic->proto_data;

  // Check if the buddy is already in the buddy list.
  if(getenv("BITLBEE_DEBUG"))
    imcb_log(ic, "add buddy for %s", name);
  imcb_add_buddy(ic, phone, NULL);
  imcb_rename_buddy(ic, phone, name);
  imcb_buddy_nick_hint(ic, phone, name);
  imcb_buddy_status(ic, phone, OPT_LOGGED_IN, NULL, NULL);
}

static void smsgcm_buddy_msg(struct im_connection *ic, char *phone, char *msg)
{
  imcb_buddy_msg(ic, phone, msg, 0, 0);
}

void smsgcm_load_messages(struct im_connection *ic, char *recv)
{
  // look at receiveMessage and download
  //char *recv = smsgcm_download_recent_messages(ic);

  if( getenv("BITLBEE_DEBUG") ){
    fprintf(stderr, "%s: smsgcm_load_messages: parsing json from `%s`\n", TAG, recv);
  }

  json_t *root;
  json_error_t error;

  root = json_loads(recv, 0, &error);
  //g_free(recv);

  if(!json_is_array(root)){
    imcb_error(ic, "malformed json sent from server: was expecting array. (%d) %s", error.line, error.text);
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

    if(getenv("BITLBEE_DEBUG"))
      imcb_log(ic, "read message: %s (%s): %s", msg->name
                                              , msg->address
                                              , msg->message);

    smsgcm_add_buddy(ic, msg->name, msg->address);
    smsgcm_buddy_msg(ic, msg->address, msg->message);
  }

  json_decref(root);
  return;

}
