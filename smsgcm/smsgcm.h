#ifndef _SMSGCM_H_
#define _SMSGCM_H_

#include <bitlbee.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "smsgcm-lib.h"
#include "smsgcm-http.h"

struct smsgcm_data {
  struct im_connection *ic;
  gint main_loop_id;
  struct credentials *creds;
};

struct buddy_data {
  char *name;
  char *address;
};

#endif
