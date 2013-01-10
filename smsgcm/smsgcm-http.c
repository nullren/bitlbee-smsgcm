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
int get(struct credentials *creds, char *url, char *post, char **output)
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
  if( creds != NULL ){
    curl_easy_setopt(curl_handle, CURLOPT_SSLCERTTYPE, "PEM");
    curl_easy_setopt(curl_handle, CURLOPT_SSLCERT, creds->client);
    curl_easy_setopt(curl_handle, CURLOPT_CAINFO, creds->ca);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 1L);
  }

  /* post it if you got it */
  if( post != NULL ){
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, post);
  }

  /* run curl */
  int r = curl_easy_perform(curl_handle);
  curl_easy_cleanup(curl_handle);

  if( r != 0 ){
    fprintf(stderr, "error: %s (%d)\n", curl_easy_strerror(r), r);
    return r;
  }

  *output = chunk.memory;

  return r;
}

/* Converts a hex character to its integer value */
char from_hex(char ch)
{
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
char to_hex(char code)
{
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_encode(char *str)
{
  char *pstr = str, *buf = malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~') 
      *pbuf++ = *pstr;
    else if (*pstr == ' ') 
      *pbuf++ = '+';
    else {
      *pbuf++ = '%';
      *pbuf++ = to_hex(*pstr >> 4);
      *pbuf++ = to_hex(*pstr & 15);
    }
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}

/* Returns a url-decoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_decode(char *str)
{
  char *pstr = str, *buf = malloc(strlen(str) + 1), *pbuf = buf;
  while (*pstr) {
    if (*pstr == '%') {
      if (pstr[1] && pstr[2]) {
        *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
        pstr += 2;
      }
    } else if (*pstr == '+') { 
      *pbuf++ = ' ';
    } else {
      *pbuf++ = *pstr;
    }
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}

char *make_query_string(struct post_item *items, int n_items)
{
  char *buffer = malloc(1);
  int i;

  for(i = 0; i < n_items; i++){
    struct post_item *p = &items[i];
    char *ek = url_encode(p->key);
    char *ev = url_encode(p->value);

    buffer = realloc(buffer, strlen(buffer) + strlen(ek) + strlen (ev) + (i?2:1));
    sprintf(buffer, "%s%s%s=%s", buffer, (i?"&":""), ek, ev);

    free(ek);
    free(ev);
  }

  return buffer;
}

int main(int argc, char **argv)
{

  struct credentials c;
  c.client = "/home/ren/ssl/renning.pem";
  c.ca = "/home/ren/ssl/cacert.pem";

  struct post_item p[] =
      { {"address", "4154980736"}
      , {"message", "ffffffffff"}
      };
  char *post = make_query_string(p, sizeof(p)/sizeof(struct post_item));

  char *url = "https://smsgcm.omgren.com/sendMessage";
  char *out = NULL;

  int r = get(&c, url, post, &out);

  if( post != NULL )
    free(post);

  if( out != NULL ){
    printf("%s\n", out);
    free(out);
  }

  return 0;
}
