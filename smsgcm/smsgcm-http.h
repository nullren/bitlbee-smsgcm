#ifndef _SMSGCM_HTTP_H_
#define _SMSGCM_HTTP_H_

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

struct credentials {
  char *p12_file;
  char *p12_passwd;
  char *client;
  char *ca;
};

struct post_item {
  char *key;
  char *value;
};

int get(struct credentials *creds, char *url, char *post, char **output);

/* found at http://www.geekhideout.com/urlcode.shtml */
char *url_encode(char *str);
char *url_decode(char *str);

char *make_query_string(struct post_item *items, int n_items);

#endif
