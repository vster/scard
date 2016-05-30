#pragma warning (push, 1)
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
// #include <conio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
// #include <io.h>
#pragma warning (pop)
#include "pcscwrap.h"

using namespace std;

#define MAX_MEMBUF 0xFFFF

static struct termios stored_settings;
char *program_name;

// static unsigned short dir[256] = {0};
// static unsigned int  idr = 0;

// long dump_it(sc_context *);
// unsigned char rdresz(sc_context *, unsigned long);

void set_keypress(void)
{
    struct termios new_settings;
    tcgetattr(0,&stored_settings);
    new_settings = stored_settings;
    /* Disable canonical mode, and set buffer size to 1 byte */
    new_settings.c_lflag &= (~(ICANON|ECHO));
    new_settings.c_cc[VTIME] = 0;
    new_settings.c_cc[VMIN] = 1;
    tcsetattr(0,TCSANOW,&new_settings);
    return;
}

void reset_keypress(void)
{
    tcsetattr(0,TCSANOW,&stored_settings);
    return;
}

int get_one_char(void)
{
    set_keypress();
    char c=getchar();
    reset_keypress();
    return c;
}

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
}

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

  program_name = argv[0];

  printf("RTREAD v.2.0. \nInfoTeCS. Special Research Lab.\n"
         "\nReading hex-codes from Rutoken.\n\n");

  if (argc>1) c = argv[1]; else
  {
    printf("Usage: %s reader hex-code length(1-250 bytes)\n\n"
           "Examples:\n\t %s Rutoken 55 1\n"
           "\t %s Rutoken 00 250\n"
           "\n", program_name, program_name, program_name);
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

  int len = 250;

  if (argc>2)
  {
    code = strtol(argv[2],NULL,16);
  }

  if (argc>3)
  {
    len = strtol(argv[3],NULL,10);
  }

  if (len > 250)
  {
      printf("Length of block in bytes must be from 1 to 250.\n");
      return 0;
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


	rc = sc_createfile (&ctx, fid, len);
	// rc = sc_viewfileattrib (&ctx, did, fid );

	dat = (BYTE *) malloc(len);
	DWORD offs = 0;

	// BYTE code = 0xcc; // Test code
	for (int i = 0; i < len; i++) dat[i] = code;

	// Write test codes to EF
	rc = sc_updatedata (&ctx, dat, len, offs);

    printf ("\nWrtiting block length = %d bytes of hex-code '%02x' to smart card.\nrc = %2x\n", len, code, rc);

    printf ("\nReading block length = %d bytes of hex-code '%02x' from smart card.\n", len, code);
    printf ("Press any key for exit.\n", code);

	k = 0;

    set_keypress();

//     for (int i = 0; i < 100; i++)
    while(!kbhit())
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

  reset_keypress();
  return 0;
} /* main */


