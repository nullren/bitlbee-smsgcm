#ifndef _HTTP_UTIL_H_
#define _HTTP_UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

struct write_struct {
  char *memory;
  size_t size;
};

static size_t write_callback(void *, size_t, size_t, void *);
char *get(char *, char *, char *);


#endif /* _HTTP_UTIL_H_ */
