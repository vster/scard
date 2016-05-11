/* *************************************************************************

   Smart Card Digging Utilities
   PC/SC Wrappers

   Written by Ilya O. Levin, http://www.literatecode.com

   This source code released for free "as is" under the whatever ABC
   license without warranty or liability of any kind. Use it at your
   own risk any way you like for legitimate purposes only.

   *************************************************************************
*/
#if defined (_UNICODE)
#error Unicode is not supported
#endif
#if defined (_MBCS)
#error MBCS is not supported
#endif
#include <reader.h>
#include <pcsclite.h>
#include "pcscwrap.h"

#define SCARD_PROTOCOL_OPTIMAL  0x00000000

#define SCARD_MYPROTOSET SCARD_PROTOCOL_T0 |   \
                         SCARD_PROTOCOL_T1 |   \
                         SCARD_PROTOCOL_RAW |  \
                         SCARD_PROTOCOL_OPTIMAL

#define NULL ((void *)0)

/* -------------------------------------------------------------------------  */
char *sc_listreaders(void)
{
  SCARDCONTEXT hCtx = {0};
  LONG rc = SCARD_S_SUCCESS;
  DWORD dw = SCARD_AUTOALLOCATE;
  LPTSTR c = NULL;
  char *cc = NULL;

  rc = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hCtx);
  if (rc == SCARD_S_SUCCESS)
  {
     rc = SCardListReaders(hCtx, NULL, (LPTSTR) &c, &dw);
     if (rc == SCARD_S_SUCCESS)
     {
        cc = (char *) malloc(dw);
        if (cc != NULL)
        {
           cc[--dw]=0; cc[--dw]=0;
           while (dw--)  cc[dw] = (char)((c[dw])?c[dw]:0x0A);
        }
        SCardFreeMemory(hCtx, c);
     }
     SCardReleaseContext(hCtx);
  }

  return cc;
} /* sc_listreaders */


/* -----------------------------------------------------------------------------
   this is the exact copy of CRT's strstr() function
*/
/*
char * cdecl x_strstr (const char * str1, const char * str2)
{
  char *cp = (char *) str1;
  char *s1, *s2;

   if ( !*str2 ) return((char *)str1);
   while (*cp)
   {
      s1 = cp;
      s2 = (char *) str2;
      while ( *s1 && *s2 && !(*s1-*s2) ) s1++, s2++;
      if (!*s2) return(cp);
      cp++;
   }

   return(NULL);
} /* x_strstr */


/* -------------------------------------------------------------------------- */
LPTSTR findfirststr(char *buf, char *tpl)
{
  LPTSTR rc=NULL;

  if ((tpl == NULL) || (buf == NULL)) return (buf);

  while (*(buf))
  {
    rc = (LPTSTR)strstr((const char *)buf, (const char *)tpl);
    if (rc != NULL) {rc = buf; break;} else while (*(buf++));
  }

  return rc;
} /* findfirststr */


/* -------------------------------------------------------------------------- */
LONG sc_init (sc_context *ctx, char *rdr)
{
  SCARD_READERSTATE rSt = {0};
  LONG rc=SCARD_S_SUCCESS;
  LPTSTR c=NULL, cc=NULL;
  DWORD dw=SCARD_AUTOALLOCATE;

  rc=SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &ctx->hCtx);
  if (rc==SCARD_S_SUCCESS)
  {
     rc=SCardListReaders(ctx->hCtx, NULL, (LPTSTR) &c, &dw);
     if (rc==SCARD_S_SUCCESS)
     {
        cc=findfirststr(c, rdr);
        if (cc!=NULL)
        {
           rSt.szReader = cc;
           rSt.dwCurrentState = SCARD_STATE_UNAWARE;
           rc = SCardGetStatusChange(ctx->hCtx, 0, &rSt, 1);
           if ( (rSt.dwEventState & SCARD_STATE_EMPTY) > 0 ) rc = SC_BAD;
           else
           {
               ctx->rdr = NULL;
               ctx->rdrsz = SCARD_AUTOALLOCATE;
               ctx->CLA = 0;
               ctx->proto = SCARD_PCI_T0;
               ctx->rw = 0;
               rc=SCardConnect(ctx->hCtx, cc, SCARD_SHARE_SHARED,
                               SCARD_MYPROTOSET, &ctx->hCard, &dw);
               if (dw==SCARD_PROTOCOL_T1) ctx->proto=SCARD_PCI_T1;
               else if (dw==SCARD_PROTOCOL_RAW) ctx->proto=SCARD_PCI_RAW;
               if (rc==SCARD_S_SUCCESS)
                  SCardGetAttrib(ctx->hCard, SCARD_ATTR_DEVICE_FRIENDLY_NAME,
                             (LPBYTE)&ctx->rdr, &ctx->rdrsz);
           }
        } else rc=SC_BAD;
        SCardFreeMemory(ctx->hCtx, c);
     }
     if ( rc != SCARD_S_SUCCESS) SCardReleaseContext(ctx->hCtx);
  }

  return rc;
} /* sc_init */


/* -------------------------------------------------------------------------- */
LONG sc_init_u (sc_context *ctx, char *rdr) /* *rdr must be a full name */
{
  SCARD_READERSTATE rSt = {0};
  LONG rc=SCARD_S_SUCCESS;
  DWORD dw=SCARD_AUTOALLOCATE;

  rc=SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &ctx->hCtx);
  if (rc == SCARD_S_SUCCESS)
  {
    rSt.szReader = rdr;
    rSt.dwCurrentState = SCARD_STATE_UNAWARE;
    rc = SCardGetStatusChange(ctx->hCtx, 0, &rSt, 1);
    if ( (rSt.dwEventState & SCARD_STATE_EMPTY) > 0 ) rc = SC_BAD;
    else
    {
       ctx->rdr = NULL;
       ctx->rdrsz = SCARD_AUTOALLOCATE;
       ctx->CLA = 0;
       ctx->proto = SCARD_PCI_T0;
       ctx->rw = 0;
       rc = SCardConnect(ctx->hCtx, rdr, SCARD_SHARE_SHARED,
                         SCARD_MYPROTOSET, &ctx->hCard, &dw);
       if (rc == SCARD_S_SUCCESS)
       {
          SCardGetAttrib(ctx->hCard, SCARD_ATTR_DEVICE_FRIENDLY_NAME,
                         (LPBYTE)&ctx->rdr, &ctx->rdrsz);
          if (dw == SCARD_PROTOCOL_T1) ctx->proto = SCARD_PCI_T1;
          else if (dw == SCARD_PROTOCOL_RAW) ctx->proto = SCARD_PCI_RAW;
       }
    }
    if ( rc != SCARD_S_SUCCESS) SCardReleaseContext(ctx->hCtx);
  }

  return rc;
} /* sc_init */


/* -------------------------------------------------------------------------- */
LONG sc_done (sc_context *ctx, DWORD lvSt)
{
  LONG rc=SCARD_S_SUCCESS;
  register BYTE i=0xFF;

  rc=SCardDisconnect(ctx->hCard, lvSt); /* SCARD_LEAVE_CARD); */
  if (rc==SCARD_S_SUCCESS)
  {
    if (ctx->rdr != NULL) SCardFreeMemory( ctx->hCtx, ctx->rdr);
    ctx->rdrsz=0; ctx->rdr = NULL;
    rc=SCardReleaseContext(ctx->hCtx);
  }
  while(i-- > 0) ctx->sw[i]=0;
  ctx->rw = 0;

  return rc;
} /* sc_finish */


/* -------------------------------------------------------------------------- */
LONG sc_selectfile (sc_context *ctx, DWORD fID)
{
  LONG rc=SCARD_S_SUCCESS;
  DWORD dw=sizeof(ctx->sw);
  BYTE *buf= ctx->sw;

  buf[0]=ctx->CLA; ctx->lCLA=buf[0];
  buf[1] = 0xa4; buf[2] = buf[3] = buf[6] = 0; buf[4] = 2; buf[5] = 0x3f;
  if ( fID > 0 )
  {
     buf[6]=(BYTE)(fID & 255); buf[5]=(BYTE)((fID >> 8 ) & 255);
  }

  rc=SCardTransmit(ctx->hCard, ctx->proto, buf, 7, NULL, ctx->sw, &dw);

  ctx->rw = ctx->sw[0]*256 + ctx->sw[1];

  return rc;
} /* sc_selectfile */


/* -------------------------------------------------------------------------- */
LONG sc_getresponse (sc_context *ctx)
{
  LONG rc=SCARD_S_SUCCESS;
  DWORD dw=sizeof(ctx->sw);
  BYTE *buf = ctx->sw;

  buf[0]=ctx->lCLA;  /* set a proper class */
  buf[4]=ctx->sw[1]; /* set data length */
  buf[1] = 0xC0;  buf[2] = buf[3] = 0;

  rc=SCardTransmit(ctx->hCard, ctx->proto, buf, 5, NULL, ctx->sw, &dw);

  ctx->rw = ctx->sw[0]*256 + ctx->sw[1];

  return rc;
} /* sc_getresponse */


/* -------------------------------------------------------------------------- */
LONG sc_readdata (sc_context *ctx, BYTE datsz, DWORD offs)
{
  LONG rc=SCARD_S_SUCCESS;
  DWORD dw=sizeof(ctx->sw);
  BYTE *buf= ctx->sw;

  buf[0]=ctx->CLA;
  ctx->lCLA=buf[0];
  buf[1] = 0xB0;
  buf[3]= (BYTE) offs & 255;
  buf[2]= (BYTE) (offs >> 8) & 255;
  buf[4]=datsz;

  rc=SCardTransmit(ctx->hCard, ctx->proto, buf, 5, NULL, ctx->sw, &dw);
  ctx->rw = ctx->sw[0]*256 + ctx->sw[1];

  return rc;
} /* sc_readdata */

/* ------------------------------------------------------------------------- */
LONG sc_updatedata (sc_context *ctx, BYTE *dat, BYTE datsz, DWORD offs)
{
  LONG rc=SCARD_S_SUCCESS;
  DWORD dw=sizeof(ctx->sw);
  BYTE *buf = ctx->sw;
  register BYTE i;

  if ((dat != NULL) && (datsz > 0))
  {
     buf[0]=ctx->CLA;
     buf[1] = 0xD6;
     buf[3]= (BYTE) offs & 255;
     buf[2]= (BYTE) (offs >> 8) & 255;
     buf[4]=datsz;
     ctx->lCLA=buf[0];
     for(i=0;i<datsz;i++) buf[5+i]=dat[i];
     datsz+=5;

     rc=SCardTransmit(ctx->hCard, ctx->proto, buf, datsz, NULL, ctx->sw, &dw);
     ctx->rw = ctx->sw[0]*256 + ctx->sw[1];
  }

  return rc;
} /* sc_updatedata */

/* -------------------------------------------------------------------------- */
LONG sc_rawsend (sc_context *ctx, void *buf, BYTE bufsz)
{
  DWORD dw = sizeof(ctx->sw);
  LONG rc;

  if ( (buf == NULL) || (bufsz == 0) ) return SCARD_E_CARD_UNSUPPORTED;
  if (ctx != buf) memset(ctx->sw, 0, sizeof(ctx->sw));
  rc = SCardTransmit(ctx->hCard, ctx->proto, (BYTE *)buf, bufsz, NULL, ctx->sw, &dw);
  ctx->rw = ctx->sw[0]*256 + ctx->sw[1];

  return rc;
} /* sc__rawsend */


/* -------------------------------------------------------------------------- */
char * rc_symb (LONG rc)
{
  char *c;
  switch (rc)
  {
    case SC_BAD: c="SCARD_GENERIC_ERROR"; break;
    case SCARD_E_BAD_SEEK: c="SCARD_E_BAD_SEEK"; break;
    case SCARD_E_CANCELLED: c="SCARD_E_CANCELLED"; break;
    case SCARD_E_CANT_DISPOSE: c="SCARD_E_CANT_DISPOSE"; break;
    case SCARD_E_CARD_UNSUPPORTED: c="SCARD_E_CARD_UNSUPPORTED"; break;
    case SCARD_E_CERTIFICATE_UNAVAILABLE: c="SCARD_E_CERTIFICATE_UNAVAILABLE"; break;
    case SCARD_E_COMM_DATA_LOST: c="SCARD_E_COMM_DATA_LOST"; break;
    case SCARD_E_DIR_NOT_FOUND: c="SCARD_E_DIR_NOT_FOUND"; break;
    case SCARD_E_DUPLICATE_READER: c="SCARD_E_DUPLICATE_READER"; break;
    case SCARD_E_FILE_NOT_FOUND: c="SCARD_E_FILE_NOT_FOUND"; break;
    case SCARD_E_ICC_CREATEORDER: c="SCARD_E_ICC_CREATEORDER"; break;
    case SCARD_E_ICC_INSTALLATION: c="SCARD_E_ICC_INSTALLATION"; break;
    case SCARD_E_INSUFFICIENT_BUFFER: c="SCARD_E_INSUFFICIENT_BUFFER"; break;
    case SCARD_E_INVALID_ATR: c="SCARD_E_INVALID_ATR"; break;
    case SCARD_E_INVALID_CHV: c="SCARD_E_INVALID_CHV"; break;
    case SCARD_E_INVALID_HANDLE: c="SCARD_E_INVALID_HANDLE"; break;
    case SCARD_E_INVALID_PARAMETER: c="SCARD_E_INVALID_PARAMETER"; break;
    case SCARD_E_INVALID_TARGET: c="SCARD_E_INVALID_TARGET"; break;
    case SCARD_E_INVALID_VALUE: c="SCARD_E_INVALID_VALUE"; break;
    case SCARD_E_NO_ACCESS: c="SCARD_E_NO_ACCESS"; break;
    case SCARD_E_NO_DIR: c="SCARD_E_NO_DIR"; break;
    case SCARD_E_NO_FILE: c="SCARD_E_NO_FILE"; break;
    case SCARD_E_NO_MEMORY: c="SCARD_E_NO_MEMORY"; break;
    case SCARD_E_NO_READERS_AVAILABLE: c="SCARD_E_NO_READERS_AVAILABLE"; break;
    case SCARD_E_NO_SERVICE: c="SCARD_E_NO_SERVICE"; break;
    case SCARD_E_NO_SMARTCARD: c="SCARD_E_NO_SMARTCARD"; break;
    case SCARD_E_NO_SUCH_CERTIFICATE: c="SCARD_E_NO_SUCH_CERTIFICATE"; break;
    case SCARD_E_NOT_READY: c="SCARD_E_NOT_READY"; break;
    case SCARD_E_NOT_TRANSACTED: c="SCARD_E_NOT_TRANSACTED"; break;
    case SCARD_E_PCI_TOO_SMALL: c="SCARD_E_PCI_TOO_SMALL"; break;
    case SCARD_E_PROTO_MISMATCH: c="SCARD_E_PROTO_MISMATCH"; break;
    case SCARD_E_READER_UNAVAILABLE: c="SCARD_E_READER_UNAVAILABLE"; break;
    case SCARD_E_READER_UNSUPPORTED: c="SCARD_E_READER_UNSUPPORTED"; break;
    case SCARD_E_SERVICE_STOPPED: c="SCARD_E_SERVICE_STOPPED"; break;
    case SCARD_E_SHARING_VIOLATION: c="SCARD_E_SHARING_VIOLATION"; break;
    case SCARD_E_SYSTEM_CANCELLED: c="SCARD_E_SYSTEM_CANCELLED"; break;
    case SCARD_E_TIMEOUT: c="SCARD_E_TIMEOUT"; break;
//    case SCARD_E_UNEXPECTED: c="SCARD_E_UNEXPECTED"; break;
    case SCARD_E_UNKNOWN_CARD: c="SCARD_E_UNKNOWN_CARD"; break;
    case SCARD_E_UNKNOWN_READER: c="SCARD_E_UNKNOWN_READER"; break;
    case SCARD_E_UNKNOWN_RES_MNG: c="SCARD_E_UNKNOWN_RES_MNG"; break;
//    case SCARD_E_UNSUPPORTED_FEATURE: c="SCARD_E_UNSUPPORTED_FEATURE"; break;
    case SCARD_E_WRITE_TOO_MANY: c="SCARD_E_WRITE_TOO_MANY"; break;
    case SCARD_F_COMM_ERROR: c="SCARD_F_COMM_ERROR"; break;
    case SCARD_F_INTERNAL_ERROR: c="SCARD_F_INTERNAL_ERROR"; break;
    case SCARD_F_UNKNOWN_ERROR: c="SCARD_F_UNKNOWN_ERROR"; break;
    case SCARD_F_WAITED_TOO_LONG: c="SCARD_F_WAITED_TOO_LONG"; break;
    case SCARD_P_SHUTDOWN: c="SCARD_P_SHUTDOWN"; break;
    case SCARD_S_SUCCESS: c="SCARD_S_SUCCESS"; break;
    case SCARD_W_CANCELLED_BY_USER: c="SCARD_W_CANCELLED_BY_USER"; break;
    case SCARD_W_CHV_BLOCKED: c="SCARD_W_CHV_BLOCKED"; break;
    case SCARD_W_EOF: c="SCARD_W_EOF"; break;
    case SCARD_W_REMOVED_CARD: c="SCARD_W_REMOVED_CARD"; break;
    case SCARD_W_RESET_CARD: c="SCARD_W_RESET_CARD"; break;
    case SCARD_W_SECURITY_VIOLATION: c="SCARD_W_SECURITY_VIOLATION"; break;
    case SCARD_W_UNPOWERED_CARD: c="SCARD_W_UNPOWERED_CARD"; break;
    case SCARD_W_UNRESPONSIVE_CARD: c="SCARD_W_UNRESPONSIVE_CARD"; break;
    case SCARD_W_UNSUPPORTED_CARD: c="SCARD_W_UNSUPPORTED_CARD"; break;
    case SCARD_W_WRONG_CHV: c="SCARD_W_WRONG_CHV"; break;
    default:  c="UNKNOWN CODE"; break;
  }
  return c;
} /* rc_symb */
