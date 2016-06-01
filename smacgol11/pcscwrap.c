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
// #error MBCS is not supported
#endif
#include "pcscwrap.h"
#include <reader.h>

#define NULL 0

#define SCARD_PROTOCOL_OPTIMAL 0x00000000

#define SCARD_MYPROTOSET SCARD_PROTOCOL_T0 |   \
                         SCARD_PROTOCOL_T1 |   \
                         SCARD_PROTOCOL_RAW |  \
                         SCARD_PROTOCOL_OPTIMAL

#define _DEBUG_ 1

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
char * x_strstr (const char * str1, const char * str2)
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
    rc = (LPTSTR)x_strstr((const char *)buf, (const char *)tpl);
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
  int i;

  buf[0]=ctx->CLA; ctx->lCLA=buf[0];
  buf[1] = 0xa4; buf[2] = buf[3] = buf[6] = 0; buf[4] = 2; buf[5] = 0x3f;
  if ( fID > 0 ) 
  { 
     buf[6]=(BYTE)(fID & 255); buf[5]=(BYTE)((fID >> 8 ) & 255);
  }
    
  rc=SCardTransmit(ctx->hCard, ctx->proto, buf, 7, NULL, ctx->sw, &dw);

  ctx->rw = ctx->sw[0]*256 + ctx->sw[1];

#if _DEBUG_
  printf("READ FILE DATA Length %d\n", dw);
  for(i=0;i<dw;i++)
    printf("%02X ",buf[i]);
  printf("\n");
#endif

  return ctx->rw;
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
  int i;
  DWORD dw=sizeof(ctx->sw);
  BYTE *buf= ctx->sw; 
    
  for (i = 0; i < dw; i++)
	 buf[i] = 0x00;

  buf[0]=ctx->CLA;
  ctx->lCLA=buf[0];
  buf[1] = 0xB0;
  buf[3]= (BYTE) offs & 255;
  buf[2]= (BYTE) (offs >> 8) & 255;
  // buf[4]=datsz;
  buf[4] = 0x00;


  rc=SCardTransmit(ctx->hCard, ctx->proto, buf, 5, NULL, ctx->sw, &dw);
  // ctx->rw = ctx->sw[0]*256 + ctx->sw[1];
  ctx->rw = (unsigned short)sc_get_status ( buf, dw );

#if _DEBUG_
  printf("READ FILE DATA Length %d\n", dw);
  for(i=0;i<dw;i++)
	printf("%02X ",buf[i]);
  printf("\n");
#endif

  return ctx->rw;
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

  return ctx->rw;
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

LONG 
sc_get_status( const void *buffer, int len )  
{
  const unsigned char *p = (const unsigned char *)buffer;
  if( len < 2 ) return -1;
  return (unsigned short int)(p[len-2]<<8 | p[len-1]);
}

unsigned long int 
rand_40(sc_context *ctx, unsigned char *rnd, int len)
{
LONG  rc=SCARD_S_SUCCESS;
DWORD dw=sizeof(ctx->sw);
BYTE *buf= ctx->sw;
#if _DEBUG_
int i;
#endif
  //
  // APDU GET CHALLENGE
  //
  buf[0] = 0; buf[1] = 0x84; buf[2] = 0; buf[3] = 0; buf[4] = (BYTE)len&0xff;
  //  
  rc=SCardTransmit(ctx->hCard, ctx->proto, buf, 5, NULL, ctx->sw, &dw);
  ctx->rw = (unsigned short)sc_get_status( buf, dw );
  //
#if _DEBUG_
  printf("GET RAND RC=%04X\n", ctx->rw);
#endif
 //
 memcpy(rnd,buf,len=dw-2);
 //
#if _DEBUG_
 for(i=0;i<len;i++)
  printf("%02X",rnd[i]);
 printf ("\n");
#endif
 //
 return ctx->rw;
}
// 
unsigned long int 
cr_f_40(sc_context *ctx, unsigned long id, int len, int pri)
{
LONG  rc=SCARD_S_SUCCESS;
DWORD dw=sizeof(ctx->sw);
BYTE *buf= ctx->sw;
#if _DEBUG_
int i;
#endif
  //
    
  BYTE crf[]=// TAG 81 - file size, 83 - file name, 86 - ACL
             "\x00\xE0\x00\x00\x1F\x6F\x1D\x81\x02\x00\x00"    
             "\x82\x03\x01\x00\x00\x83\x02\x00\x00\x85\x03\x01\x00\x00"
             "\x86\x09\x01\x01\x01\xFF\xFF\x01\x01\x01\xFF";
  
  //
	/*
  BYTE crf[]=// TAG 81 - file size, 83 - file name, 86 - ACL
             "\x00\xE0\x00\x00\x0b\x81\x02\x10\x00\x82\x01\x01"
			 "\x83\x02\x20\x00"
			 "\x86\x09\x01\x01\x01\xFF\xFF\x01\x01\x01\xFF";    
      */
  
  memcpy(buf,crf,sizeof(crf));
  
  buf[ 9]=(len>>8)&0xff;buf[10]=len&0xff;
  buf[18]=(id >>8)&0xff;buf[19]=id &0xff;
  // if(pri)
  // buf[27]=0x01;
  //
#if _DEBUG_
  printf("\nCREATING EF %04X ...\n",id);
#endif
  //
#if _DEBUG_
  printf("CREATE EF %04X OF SIZE %d APDU Length %d\n", id, len, sizeof(crf));
  for(i=0;i<sizeof(crf);i++)
  printf("%02X ",buf[i]);
  printf("\n");
#endif
  //
  rc = SCardTransmit(ctx->hCard, ctx->proto, buf, sizeof(crf), NULL, ctx->sw, &dw);
  ctx->rw = (unsigned short)sc_get_status ( buf, dw );
  //
#if _DEBUG_
  printf("CREATE EF %04X  RC=%04x\n", id, ctx->rw );
#endif
  //
  return ctx->rw;
} /* sc_createfile */
//
//
unsigned long int 
cr_d_40(sc_context *ctx, unsigned long id)
{
//dir attributes  lcyc upd  apnd deac reac del  admn crea n/a
LONG  rc=SCARD_S_SUCCESS;
#if _DEBUG_
int i;
#endif
DWORD dw=sizeof(ctx->sw);
BYTE *buf= ctx->sw;
BYTE crd[]=// TAG 83 - file name, 86 - ACL
           "\x00\xE0\x00\x00\x1F\x6F\x1D\x81\x02\x00\x00"
           "\x82\x03\x38\x00\x00\x83\x02\x00\x00\x85\x03\x01\x00\x00"
           "\x86\x09\xFF\x01\x01\xFF\xFF\x01\x01\x01\xFF";
  //
  memcpy(buf,crd,sizeof(crd));
  buf[18]=(id >>8)&0xff;
  buf[19]= id     &0xff;
  //
#if _DEBUG_
  printf("CREATE DF %04X APDU Length %d\n",id,sizeof(crd));
  for(i=0;i<sizeof(crd);i++)
  printf("%02X ",buf[i]);
  printf("\n");
#endif
  //
  rc = SCardTransmit(ctx->hCard, ctx->proto, buf, sizeof(crd), NULL, ctx->sw, &dw);
  ctx->rw = (unsigned short)sc_get_status ( buf, dw );
  //
#if _DEBUG_
  printf("CREATEDIR %04X RC=%04x\n", id, ctx->rw );
#endif
  //
  return ctx->rw;
} /* sc_createdir */
//

//
unsigned long int 
cr_f_42(sc_context *ctx, unsigned long id, int len, int pri)
{
LONG  rc=SCARD_S_SUCCESS;
DWORD dw=sizeof(ctx->sw);
BYTE *buf= ctx->sw;
#if _DEBUG_
int i;
#endif
  //
  BYTE crf[]=// TAG 80 - file size, 83 - file name, 86 - ACL
             "\x00\xE0\x00\x00\x1B\x62\x19\x80\x02\x00\x00"
             "\x82\x01\x01\x83\x02\x00\x00\x85\x01\x00"
             "\x86\x09\x00\x01\x01\xFF\xFF\x01\x01\x01\xFF";
  //
  memcpy(buf,crf,sizeof(crf));
  buf[ 9]=(len>>8)&0xff;buf[10]=len&0xff;
  buf[16]=(id >>8)&0xff;buf[17]=id &0xff;
  if(pri)
  buf[23]=0x01;
  //
#if _DEBUG_
  printf("CREATING EF %04X ...\n",id);
#endif
  //
#if _DEBUG_
  printf("CREATE EF %04X OF SIZE %d APDU Length %d\n", id, len, sizeof(crf)-1);
  for(i=0;i<sizeof(crf)-1;i++)
  printf("%02X ",buf[i]);
  printf("\n");
#endif
  //
  rc = SCardTransmit(ctx->hCard, ctx->proto, buf, sizeof(crf), NULL, ctx->sw, &dw);
  ctx->rw = (unsigned short)sc_get_status ( buf, dw );
  //
#if _DEBUG_
  printf("CREATE EF %04X  RC=%04x\n", id, ctx->rw );
#endif
  //
  return ctx->rw;
} /* sc_createfile */
//
unsigned long int 
cr_d_42(sc_context *ctx, unsigned long id)
{
//dir attributes  lcyc upd  apnd deac reac del  admn crea n/a
LONG  rc=SCARD_S_SUCCESS;
#if _DEBUG_
int i;
#endif
DWORD dw=sizeof(ctx->sw);
BYTE *buf= ctx->sw;
BYTE crd[]=// TAG 83 - file name, 86 - ACL
           "\x00\xE0\x00\x00\x1B\x62\x19\x81\x02\x00\x00"
           "\x82\x01\x38\x83\x02\x00\x00\x85\x01\x00"
           "\x86\x09\xFF\x01\x01\xFF\xFF\x01\x01\x01\xFF";
  //
  memcpy(buf,crd,sizeof(crd));
  buf[16]=(id >>8)&0xff;
  buf[17]= id     &0xff;
  //
#if _DEBUG_
  printf("CREATE DF %04X APDU Length %d\n",id,sizeof(crd));
  for(i=0;i<sizeof(crd);i++)
  printf("%02X ",buf[i]);
  printf("\n");
#endif
  //
  rc = SCardTransmit(ctx->hCard, ctx->proto, buf, sizeof(crd), NULL, ctx->sw, &dw);
  ctx->rw = (unsigned short)sc_get_status ( buf, dw );
  //
#if _DEBUG_
  printf("CREATEDIR %04X RC=%04X\n", id, ctx->rw );
#endif
  //
  return ctx->rw;
} /* sc_createdir */
//


LONG sc_verify_u (sc_context *ctx)
{
	LONG  rc=SCARD_S_SUCCESS;
	DWORD dw=sizeof(ctx->sw);
	BYTE *buf= ctx->sw;
	#if _DEBUG_
		int i;
	#endif
	
    BYTE vrfu[] = "\x00\x20\x00\x02\x08"
				  "\x31\x32\x33\x34\x35\x36\x37\x38";

	int sz = sizeof(vrfu) - 1;
	memcpy(buf,vrfu,sz);
	#if _DEBUG_
		printf("\nVERIFY USER\n");
		for(i=0;i<sz;i++)
			printf("%02X ",buf[i]);
		printf("\n");
	#endif

    rc = SCardTransmit(ctx->hCard, ctx->proto, buf, sz, NULL, ctx->sw, &dw);
    ctx->rw = (unsigned short)sc_get_status ( buf, dw );

	#if _DEBUG_
		printf("VERIFY USER RC=%04X\n", ctx->rw );
	#endif

	return ctx->rw;
} /* sc_verify_u */

LONG sc_reset_u (sc_context *ctx)
{
	LONG  rc=SCARD_S_SUCCESS;
	DWORD dw=sizeof(ctx->sw);
	BYTE *buf= ctx->sw;
	#if _DEBUG_
		int i;
	#endif
	
	BYTE rstu[] = "\x80\x40\x00\x02";
	
	int sz = sizeof(rstu)-1;
	memcpy(buf,rstu,sz);
	#if _DEBUG_
		printf("\nRESET ACCESS RIGHTS USER\n");
		for(i=0;i<sz;i++)
			printf("%02X ",buf[i]);
		printf("\n");
	#endif

    rc = SCardTransmit(ctx->hCard, ctx->proto, buf, sz, NULL, ctx->sw, &dw);
    ctx->rw = (unsigned short)sc_get_status ( buf, dw );

	#if _DEBUG_
		printf("RESET ACCESS RIGHTS USER RC=%04X\n", ctx->rw );
	#endif

	return ctx->rw;
} /* sc_reset_u */
//
LONG sc_verify_a (sc_context *ctx)
{
	LONG  rc=SCARD_S_SUCCESS;
	DWORD dw=sizeof(ctx->sw);
	BYTE *buf= ctx->sw;
	
	BYTE vrfa[] = "\x00\x20\x00\x01\x08"
				  "\x38\x37\x36\x35\x34\x33\x32\x31";

	int sz = sizeof(vrfa) - 1;
	memcpy(buf,vrfa,sz);
	
    rc = SCardTransmit(ctx->hCard, ctx->proto, buf, sz, NULL, ctx->sw, &dw);
    ctx->rw = (unsigned short)sc_get_status ( buf, dw );

	return ctx->rw;
} /* sc_verify_a */
//
LONG sc_reset_a (sc_context *ctx)
{
	LONG  rc=SCARD_S_SUCCESS;
	DWORD dw=sizeof(ctx->sw);
	BYTE *buf= ctx->sw;
	#if _DEBUG_
		int i;
	#endif
	
	BYTE rsta[] = "\x80\x40\x00\x01";
	
	int sz = sizeof(rsta)-1;
	memcpy(buf,rsta,sz);
	
    rc = SCardTransmit(ctx->hCard, ctx->proto, buf, sz, NULL, ctx->sw, &dw);
    ctx->rw = (unsigned short)sc_get_status ( buf, dw );

	return ctx->rw;
} /* sc_reset_a */
//
LONG
sc_createdir(sc_context *ctx, DWORD id)
{
//create dir 
LONG  rc=SCARD_S_SUCCESS;
#if _DEBUG_
int i;
#endif
DWORD dw=sizeof(ctx->sw);
BYTE *buf= ctx->sw;
BYTE crd[]=// TAG 83 - file name, 86 - ACL
           "\x00\xE0\x00\x00\x1f\x62\x1d"
		   "\x80\x02\x00\x00"		// Tag 0x80
           "\x82\x02\x38\x00"		// Tag 0x82
		   "\x83\x02\x00\x00"		// Tag 0x83 - file name
		   "\x86\x0f\x00\x00\x00\x00\x00\x00\x00\x00"	// Tag 0x86 - access mode byte
           "\x00\x00\x00\x00\x00\x00\x00";
  
  int sz = sizeof(crd) - 1;
  memcpy(buf,crd,sz);
  buf[17]=(id >>8)&0xff;
  buf[18]= id     &0xff;
  //
#if _DEBUG_
  printf("CREATE DF %04X APDU Length %d\n",id,sz);
  for(i=0;i<sz;i++)
	printf("%02X ",buf[i]);
  printf("\n");
#endif
  //
  rc = SCardTransmit(ctx->hCard, ctx->proto, buf, sz, NULL, ctx->sw, &dw);
  ctx->rw = (unsigned short)sc_get_status ( buf, dw );
  //
#if _DEBUG_
  printf("CREATEDIR %04X RC=%04x\n", id, ctx->rw );
#endif
  //
  return ctx->rw;
} /* sc_createdir */
//
LONG 
sc_createfile(sc_context *ctx, DWORD id, int len)
{
LONG  rc=SCARD_S_SUCCESS;
DWORD dw=sizeof(ctx->sw);
BYTE *buf= ctx->sw;
#if _DEBUG_
int i;
#endif
  //
  BYTE crf[]=// TAG 80 - file size, 83 - file name, 86 - ACL
             "\x00\xE0\x00\x00\x1f\x62\x1d"
			 "\x80\x02\x00\x00"		// Tag 0x80 - file size
			 "\x82\x02\x01\x00"		// Tag 0x82
			 "\x83\x02\x00\x00"		// Tag 0x83 - file name
		     "\x86\x0f\x00\x00\x00\x00\x00\x00\x00\x00"	// Tag 0x86 - access mode byte
             "\x00\x00\x00\x00\x00\x00\x00";
  
  int sz = sizeof(crf) - 1;
  memcpy(buf,crf,sz);
  buf[ 9]=(len>>8)&0xff;buf[10]=len&0xff;
  buf[17]=(id >>8)&0xff;buf[18]=id &0xff;

#if _DEBUG_
  printf("CREATING EF %04X ...\n",id);
#endif
  //
#if _DEBUG_
  printf("CREATE EF %04X OF SIZE %d APDU Length %d\n", id, len, sz);
  for(i=0;i<sizeof(crf)-1;i++)
  printf("%02X ",buf[i]);
  printf("\n");
#endif
  //
  rc = SCardTransmit(ctx->hCard, ctx->proto, buf, sz, NULL, ctx->sw, &dw);
  ctx->rw = (unsigned short)sc_get_status ( buf, dw );
  //
#if _DEBUG_
  printf("CREATE EF %04X  RC=%04x\n", id, ctx->rw );
#endif
  //
  return ctx->rw;
} /* sc_createfile */
// 
LONG 
sc_viewfileattrib (sc_context *ctx, DWORD did, DWORD fid  )
{
	LONG  rc=SCARD_S_SUCCESS;
	DWORD dw=sizeof(ctx->sw);
	BYTE *buf= ctx->sw;
	int len;
	WORD id;
	#if _DEBUG_
		int i;
	#endif
	
	BYTE vfa[] = "\x00\xa4\x08\x00"
				 "\x04\x00\x00\x00\x00\xff";

	int sz = sizeof(vfa)-1;
	memcpy(buf,vfa,sz);
	buf[5]=(did >>8)&0xff;buf[6]=did &0xff;
	buf[7]=(fid >>8)&0xff;buf[8]=fid &0xff;
	
#if _DEBUG_
  printf("VIEW ATTRIBS OF EF %04X ...\n",fid);
#endif
  //
#if _DEBUG_
  printf("VIEW ATTRIBS %04X APDU Length %d\n", fid, sz);
  for(i=0;i<sz;i++)
  printf("%02X ",buf[i]);
  printf("\n");
#endif

    rc = SCardTransmit(ctx->hCard, ctx->proto, buf, sz, NULL, ctx->sw, &dw);
    ctx->rw = (unsigned short)sc_get_status ( buf, dw );

#if _DEBUG_
  printf("VIEW ATTRIBS %04X APDU Length %d\n", fid, dw);
  for(i=0;i<dw;i++)
  printf("%02X ",buf[i]);
  printf("\n");
#endif
	
  if ( (ctx->rw) == 0x9000  )
  {
	len =  (buf[4]<<8) + buf[5];
	id = (buf[16]<<8) + buf[17];
	printf("CURRENT FILE %04X OF SIZE %d\n", id, len);
  }

	return ctx->rw;
} /* sc_viewfileattrib */
//
LONG 
sc_deletefile (sc_context *ctx, DWORD fid )
{
	LONG  rc=SCARD_S_SUCCESS;
	DWORD dw=sizeof(ctx->sw);
	BYTE *buf= ctx->sw;
	int len;
	WORD id;
	#if _DEBUG_
		int i;
	#endif
	
	BYTE df[] = "\x00\xe4\x00\x00"
				 "\x02\x00\x00";

	int sz = sizeof(df)-1;
	memcpy(buf,df,sz);
	buf[5]=(fid >>8)&0xff;buf[6]=fid &0xff;
	
#if _DEBUG_
  printf("DELETE FILE OF EF %04X ...\n",fid);
#endif
  //
#if _DEBUG_
  printf("DELETE FILE  %04X APDU Length %d\n", fid, sz);
  for(i=0;i<sz;i++)
  printf("%02X ",buf[i]);
  printf("\n");
#endif

    rc = SCardTransmit(ctx->hCard, ctx->proto, buf, sz, NULL, ctx->sw, &dw);
    ctx->rw = (unsigned short)sc_get_status ( buf, dw );

#if _DEBUG_
  printf("DELETE FILE %04X APDU Length %d\n", fid, dw);
  for(i=0;i<dw;i++)
  printf("%02X ",buf[i]);
  printf("\n");
#endif
	
	return ctx->rw;
} /* sc_deletefile */
//
