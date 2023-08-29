#ifndef __ioNProtect_h__
#define __ioNProtect_h__

#ifdef NPROTECT

#ifdef NPROTECT_CSAUTH3
//#pragma pack(1) 
typedef struct tagNProtectPacket
{
	enum 
	{
		MAX_PACKET_BUF = 4096,
	};
	DWORD m_dwStructSize;
	BYTE  m_byNProtectPacket[MAX_PACKET_BUF];

	tagNProtectPacket()
	{
		m_dwStructSize = 0;
		ZeroMemory( m_byNProtectPacket, MAX_PACKET_BUF );
	}
}NProtectPacket;
//#pragma pack()
#endif // NPROTECT_CSAUTH3

class ioNProtect
{
public:
	enum 
	{
		CHECK_TIME       = 240000,         // 3~5�� ����  4������...
		FIRST_CHECK_TIME = 180000,         // 3�� ����
		UPDATE_ALGORITHM_TIME = 10800000,  // ���� 3�ð�
	};
protected:
	static ioNProtect *sg_Instance;

protected:
	bool  m_bUse;
	DWORD m_dwProcessTime;
	
public:
	bool Start( bool bUse );
	void End();
	void Process();

	bool IsUse() const { return m_bUse; }

public:
	static ioNProtect &GetInstance();
	static void ReleaseInstance();

private: // Singleton class
	ioNProtect(void);
	virtual ~ioNProtect(void);
};

#define g_ioNProtect ioNProtect::GetInstance()

#endif // NPROTECT

#endif // __ioNProtect_h__