#pragma once

#ifdef SRC_LATIN

#define MAX_APEX_PACKET	2048


#pragma pack(1)
typedef struct tagApexPacket
{
	int	nType;
	char byBuffer[MAX_APEX_PACKET];
	short nLength;
} ApexPacket;

struct nUserIp_st 
	{
		char	         Cmd_UserIpFlag;
		uint32           nClientIp;
	};
#pragma pack ()

#include "ApexProxyLib.h"

class ioApex
{
public:
	enum 
	{
		CHECK_TIME       = 300000,         // 1∫– ±«¿Â
	};

protected:
	static ioApex *sg_Instance;


protected:

	struct KillDataSt
	{
		int nOriData;
	};

	bool  m_bUse;
	DWORD m_dwProcessTime;

public:
	bool Start( bool bUse );
	void End();

	void GetDataFromAS();
	void NoticeApexProxy_UserLogIn(const DWORD dwUserIdx, const ioHashString &szPrivateID, const ioHashString &szPublicIP);
	int NoticeApexProxy_UserLogout(const DWORD dwUserIdx, const ioHashString &szPrivateID);
	int NoticeApexProxy_UserData(const DWORD dwUserIdx, const char *pBuf, int nBufLen);

public:
	static ioApex &GetInstance();
	static void ReleaseInstance();

private: // Singleton class
	ioApex(void);
	virtual ~ioApex(void);

};

#define g_ioApex ioApex::GetInstance()

#endif
