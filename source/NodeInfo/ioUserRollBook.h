#pragma once
#include "RollBookManager.h"

class User;

class ioUserRollBook
{
public:
	ioUserRollBook();
	~ioUserRollBook();

	void Destroy();
	void Init(User* pUser);

public:
	void DBToData(CQueryResultData *query_data);
	void FillMoveData(SP2Packet& kPacket);
	void ApplyMoveData(SP2Packet& kPacket);

public:
	inline void SetRollBookType(int iType) { m_eLoginRollBookType = (RollBookType)iType; }
	inline int GetRollBookType() { return m_eLoginRollBookType; }

	inline int GetCurTable() { return m_iCurPage; }
	inline void SetCurTable(int iIndex) { m_iCurPage = iIndex; }
	
	inline int GetAccumulationCount() { return m_iCurLine; }
	inline void SetAccumulationCount(int iCount) { m_iCurLine = iCount; }

	inline void SetCheckDate(DWORD dwDate) { m_dwCheckDate = dwDate; }
	inline DWORD GetCheckDate() { return m_dwCheckDate; }
	
	inline void SetSQLUpdateFlag(BOOL bFlag) { m_bUpdate = bFlag; }
	inline BOOL GetSQLUpdateFlag() { return m_bUpdate; }

	void SQLUpdateRollBook();
	void SQLUpdateLogInfo();

	void ResultSQLUpdateRollBook(CQueryResultData *query_data);

	BOOL IsRenewalDate();
	DWORD GetStandardDate(DWORD dwDate);

	void GetNextStep(int iRollBookType, int& iTableIndex, int& iAccumulationCount);

	//Test MACRO
	void ProgressRollBook(SP2Packet& kPacket);

protected:
	int m_iCurPage;
	int m_iCurLine;
	DWORD m_dwCheckDate;	//�ش� ���̺� ��ġ�� ������� ��¥����.
	BOOL m_bUpdate;			//���� DB�� ������Ʈ ��. Ŭ���̾�Ʈ���� �ߺ� ��û ���� ��� ���.

	RollBookType m_eLoginRollBookType;

	User* m_pUser;
};