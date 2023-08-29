#pragma once
#include "../BoostPooler.h"
#include "Mission.h"

enum MissionState
{
	MS_NONE = 0,
	MS_PROGRESS,		// ������ - ���� ����
	MS_COMPLETED,		// �Ϸ� - ���� �޾���
	MS_RECV_COMPENSATION,
	MS_END
};

class MissionData : public BoostPooler<MissionData>
{
public:
	MissionData();
	virtual ~MissionData();

	void Init();
	void Destroy();

public:
	void Create(const MissionTypes iMissionType, const DWORD dwCode, const DWORD dwValue, const MissionState iMissionState);
	void AddValue(const DWORD dwValue);
	void ApplyData(CQueryResultData *query_data);

public:
	inline DWORD GetValue() { return m_dwValue; }
	inline int	GetState() { return m_eState; }
	inline DWORD GetCode() { return m_dwCode; }
	inline int GetType() { return m_eType; }

	inline void SetState(MissionState eState) { m_eState = eState; }
	inline void SetValue(DWORD dwValue) { m_dwValue = dwValue; }
	inline void SetCode(DWORD dwCode) { m_dwCode = dwCode; }
	inline void SetType(MissionTypes eType) { m_eType = eType; }

protected:
	DWORD m_dwCode;				//�̼� ���� ��
	DWORD m_dwValue;			//���� �޼���
	MissionState m_eState;		
	MissionTypes m_eType;		//��,��,�� �̼� ����
};