#include "smsgcm.h"

void init_plugin()
{
  struct prpl *ret = g_new0(struct prpl, 1);

  ret->options = OPT_NOOTR;
  ret->name = "smsgcm";

  register_protocol(ret);
}
