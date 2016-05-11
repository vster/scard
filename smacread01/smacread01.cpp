/* *************************************************************************

   Smart Card Digging Utilities.
   Objects dumper.

   Written by Ilya O. Levin, http://www.literatecode.com

   This source code released for free "as is" under the whatever ABC
   license without warranty or liability of any kind. Use it at your
   own risk any way you like for legitimate purposes only.

   *************************************************************************
*/

#pragma warning (push, 1)
#include <stdlib.h>
#include <stdio.h>
// #include <conio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
// #include <io.h>
#pragma warning (pop)
#include "pcscwrap.h"

#define MAX_MEMBUF 0xFFFF

// static unsigned short dir[256] = {0};
// static unsigned int  idr = 0;

// long dump_it(sc_context *);
// unsigned char rdresz(sc_context *, unsigned long);

int main(int argc, char* argv[])
{
  sc_context ctx = {0};
  long rc;
  char *c, *a;
  int code;
  BYTE *dat;
  int sz;
  long k = 0;
  long szk = 0;
  WORD MF = 0x3f00;

  printf("SMACREAD. \nInfoTeCS. Special Research Lab.\n"
         "\nReading hex-codes from smart card\n\n");

  if (argc>1) c = argv[1]; else
  {
    printf("Usage: smacread reader hex-code\n\n"
           "Examples:\n\t smacread Gemplus 55\n"
           "\t smacread \"ron SIM Re\" 00\n"
           "\n");
    c=sc_listreaders();
    if (c)
    {
      printf("Available readers:\n");
      a = strtok(c, "\n");
      while (a != NULL) { printf("* %s\n", a); a = strtok(NULL, "\n");}
    }
    else printf("Unable to list smart card readers\n");
    free (c);
    return 0;
  }

  if (argc>2)
  {
    code = strtol(argv[2],NULL,16);
  }

  printf("* connecting to \"%s\" - ", c);
  rc = sc_init(&ctx, c);
  if (rc == SC_OK)
  {
    printf("ok\n* Active reader: \"%s\"\n", ctx.rdr);

	// Logon on user
	rc = sc_verify_u(&ctx);
	// Select root dir MF
	rc = sc_selectfile(&ctx, MF);

	// Create and select DF for test files
	DWORD did = 0x00a2;
	rc = sc_createdir (&ctx, did);

	// Create test EF
	DWORD fid = 0x0015; // Any simple name

	int len = 250;
	rc = sc_createfile (&ctx, fid, len);
	// rc = sc_viewfileattrib (&ctx, did, fid );

	dat = (BYTE *) malloc(len);
	DWORD offs = 0;

	// BYTE code = 0xcc; // Test code
	for (int i = 0; i < len; i++) dat[i] = code;

	// Write test codes to EF
	rc = sc_updatedata (&ctx, dat, len, offs);

	printf ("\nWrtiting hex-code '%02x' to smart card\nrc = %2x\n", code, rc);

    printf ("\nReading hex-code '%02x' from smart card...\n ", code);

	k = 0;
//	while ( !(_kbhit() && _getch()==13)  )
    for (int i = 0; i < 100; i++)
	{
		// Read test codes
		rc = sc_readdata (&ctx, len, offs);
		k++;
	}
	szk = k * len;
	printf("\nrc = %2x; cycles = %d; bytes read = %d \n", ctx.rw, k, szk);


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


