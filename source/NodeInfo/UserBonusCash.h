#pragma once

class BonusCashInfo;
class User;

class UserBonusCash
{
public:
	UserBonusCash();
	virtual ~UserBonusCash();

	void Init(User* pUser);
	void Destroy();

public:
	void DBToData(CQueryResultData *query_data, int dwSelectType);
	void DBToExpiredData(CQueryResultData *query_data);
	void FillMoveData(SP2Packet& kPacket);
	void ApplyMoveData(SP2Packet& kPacket);

public:
	BOOL AddBonusCash(int iVal, int iExpirationDay);
	BOOL ResultAddBonusCash(int iIndex, int iVal, DWORD dwExpirationDate);

	BOOL SpendBonusCash(const DWORD dwIndex, const int iVal, int iBuyItemType, int iBuyValue1, int iBuyValue2);
	BOOL GetMoneyForConsume(IntOfTwoVec& vVal, const int iSpendVal);

	BOOL ResultSpendBonusCash(BonusCashUpdateType eType, const int iStatus, const int iIndex, const int iVal, const int iUsedAmount, const int iType, const int iValue1, const int iValue2, const ioHashString &szBillingGUID);

	int	GetAvailableBonusCash();

	void CheckExpiredBonusCash();
	
	void RelieveFlag(const DWORD dwIndex);

protected:
	BonusCashInfo* CreateCashData();
	void AddCashTable(BonusCashInfo* pInfo);

protected:
	typedef std::vector<BonusCashInfo*> CASHTABLE;
	typedef CASHTABLE::iterator CASHTABLE_ITER;

protected:
	CASHTABLE m_vUserBonusCashTable;		// ��밡�� ���ʽ� ĳ�� ����( GoldType = 1 )
	CASHTABLE m_vExpiredBonusCashTable;		// ���Ұ��� ���ʽ� ĳ�� ����( GoldType = 0 )

	User* m_pUser;
};