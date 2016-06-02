/* *************************************************************************

   Smart Card Digging Utilities.
   Objects detection tool

   Written by Ilya O. Levin, http://www.literatecode.com

   This source code released for free "as is" under the whatever ABC
   license without warranty or liability of any kind. Use it at your
   own risk any way you like for legitimate purposes only.

   *************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
// #include <conio.h>
#include <reader.h>
#include <winscard.h>
#include "pcscwrap.h"

static unsigned short dir[256] = {0};
static unsigned short startid = 1;
static unsigned int  idr = 0;

long scan_dir(sc_context *);

int main(int argc, char* argv[])
{
  sc_context ctx = {0};
  long rc, i;
  char *c, *a;

  printf("SMACDIG. Smart Card Digging Utilities. Version 0.50720 by "
         "Ilya O. Levin\nRaw and uncondition detection of DF/EF objects "
         "within smart cards\n\n");

  if (argc>1) c = argv[1]; else
  {
    printf("Usage: smacdig reader [target [initial id]]\n\n"
           "Examples:\n\t smacdig axalto\n"
           "\t smacdig \"AKS ifdh\" 3f00/6666 1FFE\n" );
    c=sc_listreaders();
    if (c)
    {
      printf("\nAvailable readers:\n");
      a = strtok(c, "\n");
      while (a != NULL) { printf("* %s\n", a); a = strtok(NULL, "\n");}
    }
    else printf("Unable to list smart card readers\n");
    free (c);
    return 0;
  }

  if (argc>2)
  {
     a = strtok(argv[2], "/\\");
     // a = strtok(argv[2], "/\\");
    while (a != NULL)
    {
      dir[idr++] = (unsigned short) strtoul(a, NULL, 16);
      if (idr == 255) break;
      a = strtok(NULL, "/\\");
    }
  }
  if (idr<1) dir[idr++] = 0x3f00;

  if (argc>3) startid = (unsigned short) strtoul(argv[3], NULL, 16);

  printf("* connecting to \"%s\" - ", c);
  rc = sc_init(&ctx, c);
  if (rc == SC_OK)
  {
    printf("ok\n* Active reader: \"%s\"\n* Target: ", ctx.rdr);
    for (rc=0;rc<(int)idr;rc++) printf("/%04x", dir[rc]); printf("\n");

    printf("* probing for commands class - ");
    rc = sc_selectfile(&ctx, 0x3f00);
    for (i=0; (rc==SC_OK)&&(ctx.sw[0]!=0x61)&&(ctx.sw[0]!=0x90)&&(i<256) ;i++)
    {
       ctx.CLA = (unsigned char) i; rc = sc_selectfile(&ctx, 0x3f00);
    }
    if ( (ctx.sw[0]!=0x61) && (ctx.sw[0]!=0x90) ) printf("failed\n"); else
    {
       printf("ok [CLA:%02x]\n* enumerating from %04x\n", ctx.CLA, startid);
       rc=scan_dir(&ctx);
       printf("* enumeration %s\n", (rc==SC_OK)?"done":"failed");
    }
    sc_finish(&ctx);
  }
  else printf("failed (%s)\n", rc_symb(rc));

  return 0;
} /* main */

/* -------------------------------------------------------------------------- */
long scan_dir(sc_context *ctx)
{
  long rc = SC_BAD;
  unsigned long ff, sz;
  register unsigned char i;

  if (idr==0) return SC_BAD;

  for (ff = startid; ff<0xFFFF; ff++)
  {
 //    if (kbhit()) break;
     if ( (ff==0x3F00) || (ff==0x3fff) ) continue;
     for (i=0;i<idr; i++) if ((rc = sc_selectfile(ctx, dir[i]))!=SC_OK) break;
     if (ff % 128 == 0 ) printf("%04x\r", ff);
     if (rc == SC_OK) rc = sc_selectfile(ctx, ff);
     if (rc != SC_OK) { printf(" failed (%s)\n", rc_symb(rc)); break;}
     if ( (ctx->sw[0] == 0x90) || (ctx->sw[0] == 0x61) )  /* object selected */
     {
        printf("+ %04x", ff);
        sz = ctx->sw[1];
        if (ctx->sw[0] == 0x61) if (sc_getresponse(ctx) == SC_OK)
        {
           printf(": "); for (i=0;i<sz;i++) printf("%02x", ctx->sw[i]);
        }
        rc = sc_readdata(ctx, 8, 0);
        if ( (rc == SC_OK) && (ctx->sw[0] == 0x69) && (ctx->sw[1] == 0x86)
           ) printf(": DIR");
        printf("\n");
     }
     else if (ctx->sw[0] != 0x6A)
             printf("%04x [SW=%02x%02x]\n", ff, ctx->sw[0], ctx->sw[1]);
  }

//  while (kbhit()) getch();

  return rc;
} /* scan_dir */
