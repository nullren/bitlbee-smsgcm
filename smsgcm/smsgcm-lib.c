#include "smsgcm-lib.h"

void smsgcm_load_messages(struct im_connection *ic)
{
  // look at receiveMessage and download

  // doesn't work - need to add buddy first, probably
  //imcb_buddy_msg(ic, "test", "lol", 0, 0);
}
