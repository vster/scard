#pragma warning (push, 1)
#include <stdlib.h>
#include <stdio.h>
// #include <conio.h>
#include <fcntl.h>
#include <sys/stat.h>
// #include <io.h>
#pragma warning (pop)
#include "pcscwrap.h"

#define MAX_MEMBUF 0xFFFF

// #define _DEBUG_ 1

static unsigned short dir[256] = {0};
static unsigned int  idr = 0;

long dump_it(sc_context *);
unsigned char rdresz(sc_context *, unsigned long);

int main()
{
  sc_context ctx = {0};
  long rc;
  char *c = "VMware Virtual USB CCID 00 00";
  unsigned short dir[256] = {0x3f00};
  BYTE *buf;

  printf("* connecting to \"%s\" - ", c);
  rc = sc_init(&ctx, c);
  if (rc == SC_OK)
  {
    printf("ok\n* Active reader: \"%s\"\n", ctx.rdr);

   int idr = 1;
	for (int i = 0; i < (int)idr; i++)
   {
     printf("/%04x", dir[i]);
     rc = sc_selectfile(&ctx, dir[i]);
     if (rc != SC_OK) { printf("- failed (%s)", rc_symb(rc)); break;}
   }

	unsigned long id = 0x1010;
	int sz = 100;
	int pri = 0;
	// rc = cr_f_40(&ctx, id, sz, pri);
	// rc = cr_f_42(&ctx, id, sz, pri );
	rc = sc_createfile(&ctx, id, sz, pri);

	sc_finish(&ctx);
  }
  else printf("failed (%s)\n", rc_symb(rc));

  return 0;
} /* main */
