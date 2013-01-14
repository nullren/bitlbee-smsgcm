#ifndef _SMSGCM_CURL_H_
#define _SMSGCM_CURL_H_

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

struct post_item {
  char *key;
  char *value;
};

int curl_get(struct credentials *creds, char *url, char *post, char **output);

#endif
