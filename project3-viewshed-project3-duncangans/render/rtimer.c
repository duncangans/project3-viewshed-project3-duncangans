#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "rtimer.h"

char *rt_sprint_safe(char *buf, Rtimer rt) {
  if(rt_w_useconds(rt) == 0) {
    sprintf(buf, "[%4.2fu (%.0f%%) %4.2fs (%.0f%%) %4.2f %.1f%%]",
	    0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  } else {
    sprintf(buf, "[%4.2fu (%.0f%%) %4.2fs (%.0f%%) %4.2f %.1f%%]",
	    rt_u_useconds(rt)/1000000,
	    100.0*rt_u_useconds(rt)/rt_w_useconds(rt),
	    rt_s_useconds(rt)/1000000,
	    100.0*rt_s_useconds(rt)/rt_w_useconds(rt),
	    rt_w_useconds(rt)/1000000,
	    100.0*(rt_u_useconds(rt)+rt_s_useconds(rt)) / rt_w_useconds(rt));
  }
  return buf;
}
