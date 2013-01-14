#ifndef _SMSGCM_HTTP_H_
#define _SMSGCM_HTTP_H_

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct post_item {
  char *key;
  char *value;
};

/* found at http://www.geekhideout.com/urlcode.shtml */
char *url_encode(char *str);
char *url_decode(char *str);

char *make_query_string(struct post_item *items, int n_items);

#endif
