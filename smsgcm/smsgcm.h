#ifndef _SMSGCM_H_
#define _SMSGCM_H_

#include <bitlbee.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "smsgcm-lib.h"
#include "smsgcm-httputils.h"
#include "smsgcm-gnutls-creds.h"

#define SMSGCM_API_URL "https://smsgcm.omgren.com"

#define SMSGCM_SEND_MSG_URL "/sendMessage"
#define SMSGCM_RECV_MSG_URL "/receiveMessage"

struct smsgcm_data {
  struct im_connection *ic;
  gint main_loop_id;
  struct credentials *creds;
  int bfd;
  struct queue_message *queued;
};

struct buddy_data {
  char *name;
  char *address;
};

struct queue_message {
  char *address;
  char *message;
};

struct credentials {
  char *p12_file;
  char *p12_passwd;
  char *client;
  char *ca;
};

#endif
