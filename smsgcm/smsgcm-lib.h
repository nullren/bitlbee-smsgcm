#ifndef _SMSGCM_LIB_H_
#define _SMSGCM_LIB_H_

#include "smsgcm.h"
#include "smsgcm-httputils.h"
#include "smsgcm-curl.h"
#include <jansson.h>

struct message_data {
  char *name;
  char *address;
  char *message;
  long time;
};

void smsgcm_load_messages(struct im_connection *ic);
char *smsgcm_download_recent_messages(struct im_connection *ic);

#endif
