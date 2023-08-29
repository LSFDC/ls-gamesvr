#ifndef _KickOutVoteHelp_h_
#define _KickOutVoteHelp_h_

class Mode;

class KickOutVoteHelp
{
protected:
	Mode *m_pCreator;

protected:
	struct KickVoteData
	{
		int m_iVoteType;
		TeamType m_eTeam;
		ioHashString m_szName;
		KickVoteData()
		{
			m_iVoteType = 0;
			m_eTeam     = TEAM_NONE;
		}
	};
	typedef std::vector< KickVoteData > KickVoteDataList;
	KickVoteDataList m_VoteUserList;
	
	ioHashStringVec  m_VoteProposalUserList;       // 1ȸ�� ������
	ioHashString     m_NowVoteProposalName;        // ���� ������ ����
	ioHashString     m_NowVoteTargetName;          // ���� �����	
	ioHashString	 m_NowVoteReason;			   // ���� ����
	
	DWORD            m_dwCurVoteTime;              // ��ǥ ���� �ð�

	// ���� INI
    DWORD            m_dwVoteProcessTime;          // ���� �ð�
	int              m_iKickVoteUserPool;          // ��ǥ�� �ϱ� ���� �ο�
	int              m_iKickVoteRoundWin;          // ��ǥ�� �ϱ� ���� ���� ����(�� ����)
	DWORD            m_dwKickVoteRoundTime;        // ��ǥ�� �ϱ� ���� �÷��� �ð�
	float            m_fVoteAdoptionPercent;       // ��ǥ�� ����Ǳ� ���� �ۼ�Ʈ

public:
	void InitVote();
	void LoadVoteInfo( Mode *pCreator, ioINILoader &rkLoader );

public:
	void ProcessVote();

public:
	const DWORD GetVoteProcessTime() { return m_dwVoteProcessTime; }
	const int GetKickVoteUserPool() { return m_iKickVoteUserPool; }
	const int GetKickVoteRoundWin() { return m_iKickVoteRoundWin; }
	const DWORD GetKickVoteRoundTime() { return m_dwKickVoteRoundTime; }
	const float GetVoteAdoptionPercent() { return m_fVoteAdoptionPercent; }

	const ioHashString &GetVoteProposalName() { return m_NowVoteProposalName; }
	const ioHashString &GetVoteTargetName() { return m_NowVoteTargetName; }
	const ioHashString &GetVoteReason() { return m_NowVoteReason; }
	TeamType GetVoteUserTeam( const ioHashString &rkName );

public:
	bool IsVoteProposalUser( const ioHashString &rkName );
	bool IsVoting();
	void StartVote( const ioHashString &rkProposalName, const ioHashString &rkTargetName, const ioHashString &rkReason );
	void EndVote();
	void InsertVoteUserList( const ioHashString &rkName, TeamType eTeam );
	void RemoveVoteUserList( const ioHashString &rkName );
	void SetKickVote( const ioHashString &rkName, int iVoteType );
	void CheckKickVoteResult();

public:
	KickOutVoteHelp();
	virtual ~KickOutVoteHelp();
};

#endif
