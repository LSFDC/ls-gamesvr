#include "stdafx.h"
#include "ApexProxyLib.h"

static HMODULE g_hASclient			= NULL;

static void cleanup_sh()
{
	if ( g_sh_exit )
	{
		g_sh_exit();
	}
 
	if(NULL!=g_hASclient)
		FreeLibrary(g_hASclient);


	g_hASclient		= NULL;
	g_sh_exit		= NULL;
	g_sh_start		= NULL;
	g_sh_setfunc	= NULL;	
	g_sh_getdata_cb = NULL;
	g_sh_set_config_path = NULL;
}

extern "C" void		s_end()
{
	cleanup_sh();
}
extern "C" int		s_getData_FromApex_cb( int nMaxGetPackets, DATA_CALLBACK_PROC *pCallBack, void *pMyParaData )
{
	int nRet = 1;
	if ( g_sh_getdata_cb )
	{
		nRet = g_sh_getdata_cb(nMaxGetPackets, pCallBack, pMyParaData);
	}
	return nRet;
}

extern "C" void		s_sendData_ToApex( char cMsgId, unsigned int nUserId, const char * pBuff, int nBuffLen )
{
	if ( g_sh_recv )
	{
		g_sh_recv( cMsgId, nUserId, pBuff, nBuffLen );
	}
}
extern "C" void *   s_setFunc( void * pFunc, int nFuncFlag )
{
	void * pRet = NULL;
	if( g_sh_setfunc )
		pRet = g_sh_setfunc( pFunc, nFuncFlag );
	return pRet;
}

extern "C" int		s_start(const char *szConfigPath)
{
	if (!g_hASclient)
	{
		g_hASclient		= LoadLibraryA( "ApexProxy.dll" );
		if ( !g_hASclient ) 
			return -2;
		g_sh_exit		= (SH_DONE_PROC *) GetProcAddress( g_hASclient, "sh_load_exit" );
		g_sh_start		= (SH_START_PROC *) GetProcAddress( g_hASclient, "sh_load_start_v2" );
		g_sh_recv		= (SH_SENDDATA_PROC *) GetProcAddress( g_hASclient, "sh_recv" );
		g_sh_setfunc	= (SH_SETFUNC_PROC *) GetProcAddress( g_hASclient, "sh_setfunc" );
        g_sh_set_config_path = (SH_SET_CONFIG_PATH *)GetProcAddress( g_hASclient, "sh_set_config_path" );
		g_sh_getdata_cb	= (SH_GETDATA_CALLBACK *)GetProcAddress( g_hASclient, "sh_getdata_cb" );
	}
	if ( !g_sh_exit || !g_sh_start || !g_sh_recv || !g_sh_setfunc || !g_sh_set_config_path || !g_sh_getdata_cb )
	{
		if(g_hASclient)
			FreeLibrary(g_hASclient);

		g_hASclient		= NULL;
		g_sh_exit		= NULL;
		g_sh_start		= NULL;
		g_sh_recv		= NULL;
    g_sh_set_config_path = NULL;
		g_sh_getdata_cb = NULL;
		return -3;
	}
  int nRet = 0;
  nRet = g_sh_set_config_path(szConfigPath); //must be called before the g_sh_start();
  if (nRet == 0) {
      nRet = g_sh_start();
  }
	return nRet;
}
//////////////////////////////////////////////////////////////////////////
