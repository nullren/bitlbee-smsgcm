#ifndef _SMSGCM_LIB_H_
#define _SMSGCM_LIB_H_

#include "smsgcm.h"
#include <jansson.h>
#include <stdarg.h>

struct message_data {
  char *name;
  char *address;
  char *message;
  long time;
};

void smsgcm_load_messages(struct im_connection *ic, char *);
void smsgcm_lib_buddy_msg(struct im_connection *ic, char *phone, char *msg);
void smsgcm_lib_add_buddy(struct im_connection *ic, char *name, char *phone);
void smsgcm_log(char *tag, char *function, char *fmt, ...);

#endif
