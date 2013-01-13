#ifndef _SMSGCM_H_
#define _SMSGCM_H_

#include <bitlbee.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

//include "smsgcm-lib.h"
//include "smsgcm-http.h"

#define SMSGCM_API_URL "https://smsgcm.omgren.com"

#define SMSGCM_SEND_MSG_URL "/sendMessage"
#define SMSGCM_RECV_MSG_URL "/receiveMessage"

struct smsgcm_data {
  struct im_connection *ic;
  gint main_loop_id;
  struct credentials *creds;
};

struct buddy_data {
  char *name;
  char *address;
};

struct credentials {
  char *p12_file;
  char *p12_passwd;
  char *client;
  char *ca;
};

#endif
