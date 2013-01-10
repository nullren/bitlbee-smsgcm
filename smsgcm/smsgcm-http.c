#include "smsgcm-http.h"

static size_t
write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct write_struct *mem = (struct write_struct *)userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory == NULL) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    exit(EXIT_FAILURE);
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

/* reads a uri and then puts contents into buffer */
char *get(char *url, char *client_cert, char *ca_cert)
{
  CURL *curl_handle;
  struct write_struct chunk;

  /* init stuff */
  chunk.memory = malloc(1);
  chunk.size = 0;
  curl_global_init(CURL_GLOBAL_ALL);

  /* curl */
  curl_handle = curl_easy_init();
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

  /* ssl stuff */
  if( client_cert != NULL ){
    curl_easy_setopt(curl_handle, CURLOPT_SSLCERTTYPE, "PEM");
    curl_easy_setopt(curl_handle, CURLOPT_SSLCERT, client_cert);
    curl_easy_setopt(curl_handle, CURLOPT_CAINFO, ca_cert);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 1L);
  }

  /* run curl */
  int r = curl_easy_perform(curl_handle);
  curl_easy_cleanup(curl_handle);

  if( r != 0 ){
    fprintf(stderr, "error: %s (%d)\n", curl_easy_strerror(r), r);
    return NULL;
  }

  return chunk.memory;
}
