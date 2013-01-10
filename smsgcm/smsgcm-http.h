#ifndef _SMSGCM_HTTP_H_
#define _SMSGCM_HTTP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

struct write_struct {
  char *memory;
  size_t size;
};

static size_t write_callback(void *, size_t, size_t, void *);
int get(char *url, char *client_cert, char *ca_cert, char **output);

/* found at http://www.geekhideout.com/urlcode.shtml */
char from_hex(char ch);
char to_hex(char code);
char *url_encode(char *str);
char *url_decode(char *str);

#endif
