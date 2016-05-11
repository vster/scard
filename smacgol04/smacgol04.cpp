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

#define _DEBUG_ 1

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
  WORD MF = 0x3f00;
  BYTE *dat;
  int k, szk;

  printf("* connecting to \"%s\" - ", c);
  rc = sc_init(&ctx, c);
  if (rc == SC_OK)
  {
    printf("ok\n* Active reader: \"%s\"\n", ctx.rdr);

	// Logon on user
	rc = sc_verify_u(&ctx);
	// Select root dir MF
	rc = sc_selectfile(&ctx, MF);

	// rc = sc_showcurdir (&ctx);

	// Create and select DF for test files
	DWORD did = 0x00a2;
	rc = sc_createdir (&ctx, did);

	// rc = sc_showcurdir (&ctx);

	// rc = sc_selectdir(&ctx, did);

	// rc = sc_selectfile(&ctx, MF);
	// rc = sc_selectfile(&ctx, did);

	// rc = sc_showcurdir (&ctx);

	// Create test EF
	DWORD fid = 0x0015; // Any simple name
	int len = 250;
	rc = sc_createfile (&ctx, fid, len);
	rc = sc_viewfileattrib (&ctx, did, fid );

	dat = (BYTE *) malloc(len);
	DWORD offs = 0;



	BYTE code = 0xcc; // Test code
	for (int i = 0; i < len; i++)
		dat[i] = code;
	// Write test codes to EF
	rc = sc_updatedata (&ctx, dat, len, offs);

	// Read test codes
    for (int i = 0; i < 100; i++)
		rc = sc_readdata (&ctx, len, offs);

	// Delete test EF
	rc = sc_deletefile (&ctx, fid );
	// rc = sc_viewfileattrib (&ctx, did, fid );  // Check of delete

	// Delete test DF
	rc = sc_selectfile(&ctx, MF);
	rc = sc_deletefile (&ctx, did );

	// Logout from user
	rc = sc_reset_u (&ctx);

	sc_finish(&ctx);
  }
  else printf("failed (%s)\n", rc_symb(rc));

  return 0;
} /* main */
