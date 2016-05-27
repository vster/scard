/* ************************************************************************

   Smart Card Digging Utilities
   PC/SC Wrappers

   Written by Ilya O. Levin, http://www.literatecode.com

   This source code released for free "as is" under the whatever ABC
   license without warranty or liability of any kind. Use it at your
   own risk any way you like for legitimate purposes only.

   ************************************************************************
*/
#pragma once
#pragma comment( lib, "winscard" )
#pragma warning (disable: 4514)     // "unref inline fn has been removed"
#pragma warning (push, 1)
#define WINDOWS_MEAN_AND_LEAN
// #include <windows.h>
#include <winscard.h>
#pragma warning (pop)

#define SC_OK		SCARD_S_SUCCESS
#define SC_BAD		0xBEDA

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  SCARDCONTEXT hCtx;
  SCARDHANDLE  hCard;
  LPCSCARD_IO_REQUEST proto;
  BYTE sw[256];              // output of the last operation
  DWORD  rdrsz;              // length of reader's name
  LPBYTE rdr;                // name of the connected reader
  BYTE CLA;                  // CLA byte
  BYTE lCLA;                 // last operation's class (CLA byte)
  unsigned short rw;         // status word of last operation
} sc_context;

char *sc_listreaders (void);
LONG sc_init (sc_context *, char *);
LONG sc_init_u (sc_context *, char *);
LONG sc_done (sc_context *, DWORD);
#define sc_finish(x) sc_done(x, SCARD_LEAVE_CARD)
LONG sc_selectfile (sc_context *, DWORD);
LONG sc_getresponse (sc_context *);
LONG sc_readdata (sc_context *, BYTE, DWORD);
LONG sc_updatedata (sc_context *, BYTE *, BYTE, DWORD);
LONG sc_rawsend (sc_context *, void *, BYTE);
char *rc_symb (LONG);
LONG sc_get_status( const void *, int );
LONG sc_verify_u (sc_context *);
LONG sc_reset_u (sc_context *);
LONG sc_verify_a (sc_context *);
LONG sc_reset_a (sc_context *);
LONG sc_createdir(sc_context *, DWORD);
LONG sc_selectdir(sc_context *, DWORD);
LONG sc_createfile (sc_context *, DWORD, int);
LONG sc_viewfileattrib (sc_context *, DWORD, DWORD );
LONG sc_showcurdir (sc_context *);
LONG sc_deletefile (sc_context *, DWORD );

#ifdef __cplusplus
}
#endif
