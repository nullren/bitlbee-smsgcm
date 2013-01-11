#ifndef _SMSGCM_LIB_H_
#define _SMSGCM_LIB_H_

#define SMSGCM_API_URL "https://smsgcm.omgren.com"

#define SMSGCM_SEND_MSG_URL "/sendMessage"
#define SMSGCM_RECV_MSG_URL "/receiveMessage"

#include "smsgcm.h"
#include "smsgcm-http.h"

void smsgcm_load_messages(struct im_connection *ic);
char *smsgcm_download_recent_messages(struct im_connection *ic);

#endif
