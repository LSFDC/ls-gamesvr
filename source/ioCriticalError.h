#ifndef _ioCriticalError_h_
#define _ioCriticalError_h_

class ioCriticalError
{
private:
	static ioCriticalError *sg_Instance;

	/************************************************************************
		��Ʈ�� �˻� - ������ ��Ȳ
		- ���� ������ ����
		101. GS < - > GS ���� ����  : ���� ����� ���� IP : PORT : Last Error
		102. GS < - > MS ���� ����  : Last Error
		103. DBA < - > GS ���� ���� : ���� ����� ���� IP : PORT : Last Error
		104. BS < - > GS ���� ����  : Last Error
		105. MS - > GS ���� ����    : ���� ���� ��� �α� 

		- ���� ���� ����
		106. GS < - > GS ���� ����  : ���� ����� ���� IP : PORT
		107. GS < - > MS ���� ����  : 
		108. DBA < - > GS ���� ���� : ���� ����� ���� IP : PORT
		109. BS < - > GS ���� ����  : 

		�޸� �˻� - ������ ����
		201. Packet Pool ���� : Pool ī��Ʈ : ��Ŷ ID
		202. User Pool ����   : Pool ī��Ʈ

		ũ���� �˻� - ������ ��Ȳ
		301. ũ���� : �α� ���

		���� �ý��� �˻� - ������ ���� ( INI���� ���� ���� )
		501. �뺴 ���� �ʰ� : ���� �г��� : �뺴 ���� 
			> 300�� ����(��Ŷ 32Kbyte ���� ����)
		502. ġ�� DB ���̺� �ʰ� : ���� �г��� : DB ���̺� ����
			> 120���̺� ����(��Ŷ 32Kbyte ���� ����)
		503. Ư�� ������ DB ���̺� �ʰ� : ���� �г��� : DB ���̺� ����
			> 120���̺� ����(��Ŷ 32Kbyte ���� ����)
		504. �������� ����Ʈ DB ���̺� �ʰ� : ���� �г��� : DB ���̺� ���� 
			> 90���̺� ����(��Ŷ 32Kbyte ���� ����)
		505. �Ϸ�� ����Ʈ DB ���̺� �ʰ� : ���� �г��� : DB ���̺� ����
			> 180���̺� ����(��Ŷ 32Kbyte ���� ����)
		506. ���� DB ���̺� �ʰ� : ���� �г��� : DB ���̺� ����
			> 150���̺� ����(��Ŷ 32Kbyte ���� ����)
		507. ���� DB ���̺� �ʰ� : ���� �г��� : DB ���̺� ����
			> 280���̺� ����(��Ŷ 32Kbyte ���� ����)
		508. ��� DB ���̺� �ʰ� : ���� �г��� : DB ���̺� ����
			> 80���̺� ����(��Ŷ 32Kbyte ���� ����)
		509. �޴� DB ���̺� �ʰ� : ���� �г��� : DB ���̺� ����
			> 140���̺� ����(��Ŷ 32Kbyte ���� ����)
		510. ��� ���� ���� : ���� �г��� : ���� ���
			> 1,000�� ��� ����(���׷� ȹ���� ���ɼ�)
		511. ��� ���� ���� : ���� �г��� : ���� ���
			> 5,000�� ��� ����(���׷� ȹ���� ���ɼ�)
		PS. �ִ� ��Ŷ ������ : 32768byte

		����Ʈ ����� �˻�
		601. ����Ʈ ����� �߻�
	************************************************************************/
protected:
	PROCESS_INFORMATION m_ProcessInfo;

protected:
	int m_iSoldierLimitCount;
	int m_iDecoTableLimitCount;
	int m_iEtcItemTableLimitCount;
	int m_iProgressQuestTableLimitCount;
	int m_iCompleteQuestTableLimitCount;
	int m_iGrowthTableLimitCount;
	int m_iFishInvenTableLimitCount;
	int m_iExtraItemTableLimitCount;
	int m_iMedalTableLimitCount;
	int m_iPesoAcquiredLimitCount;
	int m_iPesoPossessionLimitCount;
	int m_iCostumeLimitCount;
	int m_iAccessoryLimitCount;

protected:
	void ExcuteProcess( char* szFileName , char* szCmdLine );

public:
	static ioCriticalError &GetInstance();
	static void ReleaseInstance();

public:
	void Initialize();

public:
	void CheckGameServerExceptionDisconnect( const ioHashString &rkDisconnectIP, DWORD dwDisconnectPort, DWORD dwLastError );
	void CheckMainServerExceptionDisconnect( DWORD dwLastError );
	void CheckDBAgentServerExceptionDisconnect( const ioHashString &rkDisconnectIP, DWORD dwDisconnectPort, DWORD dwLastError );
	void CheckBillingServerExceptionDisconnect( DWORD dwLastError );
	void CheckGameServerDisconnect( const ioHashString &rkDisconnectIP, DWORD dwDisconnectPort );
	void CheckMainServerDisconnect();
	void CheckDBAgentServerDisconnect( const ioHashString &rkDisconnectIP, DWORD dwDisconnectPort );
	void CheckBillingServerDisconnect();

	void CheckServerDownMsg( int iServerCount, int iConnectCount );
	void CheckPacketPool( int iPacketQueueCount, DWORD dwPacketID );
	void CheckUserPool( int iUserPoolCount );
	void CheckCrashLog( const ioHashString &rkCrashLog );

public:
	void CheckSoldierCount( const ioHashString &rkPublicID, int iSoldierCount );
	void CheckDecoTableCount( const ioHashString &rkPublicID, int iDecoTableCount );
	void CheckEtcItemTableCount( const ioHashString &rkPublicID, int iEtcItemTableCount );
	void CheckProgressQuestTableCount( const ioHashString &rkPublicID, int iProgressQuestTableCount );
	void CheckCompleteQuestTableCount( const ioHashString &rkPublicID, int iCompleteQuestTableCount );
	void CheckGrowthTableCount( const ioHashString &rkPublicID, int iGrowthTableCount );
	void CheckFishInvenTableCount( const ioHashString &rkPublicID, int iFishInvenTableCount );
	void CheckExtraItemTableCount( const ioHashString &rkPublicID, int iExtraItemTableCount );
	void CheckMedalTableCount( const ioHashString &rkPublicID, int iMedalTableCount );
	void CheckPesoAcquiredCount( const ioHashString &rkPublicID, int iPesoAcquiredCount );
	void CheckPesoPossessionCount( const ioHashString &rkPublicID, __int64 iPesoPossessionCount );
	void CheckQuestAbuse( const ioHashString &rkPublicID, DWORD dwMainIndex, DWORD dwSubIndex, DWORD dwGapTime, DWORD dwGapValue );

public:
	inline int GetMaxCostumeCount() { return m_iCostumeLimitCount; }
	inline int GetMaxAccessoryCount() { return m_iAccessoryLimitCount; }

private:
	ioCriticalError();
	virtual ~ioCriticalError();
};
#define g_CriticalError ioCriticalError::GetInstance()
#endif