#ifndef __APEXPROXY_LIB__
#define __APEXPROXY_LIB__
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include "windows.h"
#else
#include <dlfcn.h>
#endif


#define s_OK(code)		((CH_RESULT)(code) >= 0)
#define s_FAIL(code)	((CH_RESULT)(code) < 0)

#define	FLAG_NETSTATE  1003
#define ServerMaxPacketSize    20000

typedef int    SH_START_PROC();
typedef void   SH_DONE_PROC(void);

typedef int    DATA_CALLBACK_PROC( void *pMyParaData, char* pData, char cDataType, unsigned int nSendID, int nRetLen );
typedef void   SH_SENDDATA_PROC( char cMsgId, unsigned int nId, const char * pBuffer, int nLen );
typedef void*  SH_SETFUNC_PROC( void * pFunc, int nFuncFlag );
typedef int    SH_SET_CONFIG_PATH( const char *szConfigPath );
typedef int	   SH_GETDATA_CALLBACK( int nMaxGetPackets, DATA_CALLBACK_PROC *pCallBack, void *pMyParaData );

static SH_DONE_PROC *		g_sh_exit		= NULL;
static SH_SETFUNC_PROC *	g_sh_setfunc	= NULL;
static SH_START_PROC *		g_sh_start		= NULL;
static SH_SENDDATA_PROC *   g_sh_recv       = NULL; 
static SH_SET_CONFIG_PATH * g_sh_set_config_path = NULL;
static SH_GETDATA_CALLBACK * g_sh_getdata_cb = NULL;

#ifdef __cplusplus
extern "C" {
#endif

extern "C" int		s_start(const char *szConfigPath = 0);
extern "C" void		s_end();	
extern "C" void * s_setFunc( void * pFunc, int nFuncFlag );
extern "C" void		s_sendData_ToApex( char cMsgId, unsigned int nUserId, const char * pBuff, int nBuffLen );
extern "C" int		s_getData_FromApex_cb( int nMaxGetPackets, DATA_CALLBACK_PROC *pCallBack, void *pMyParaData );

#ifdef __cplusplus
}
#endif


#endif  //eof __APEXPROXY_LIB__



