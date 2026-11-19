#include "types.h"
#include "user.h"
#include "date.h"

int
main(void)
{
  struct rtcdate r;

  if(date(&r) < 0)
    exit();

  printf(1, "%d-%02d-%02dT%02d:%02d:%02d\n",
         r.year, r.month, r.day,
         r.hour, r.minute, r.second);

  exit();
}
