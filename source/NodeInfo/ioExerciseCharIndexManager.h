#ifndef __ioExerciseCharIndexManager_h__
#define __ioExerciseCharIndexManager_h__

#include "../Util/Singleton.h"

#define EXERCISE_NONE                   0x00         // ���� �뺴
#define EXERCISE_GENERAL                0x01         // �Ϲ� ü�� �뺴
#define EXERCISE_PCROOM                 0x02         // PC�� ü�� �뺴
#define EXERCISE_EVENT                  0x03         // ���� ���� ���� ���ο����� ��� ������ ü�� �뺴( ExerciseSoldierEvent )
#define EXERCISE_RENTAL                 0x04         // �뿩�� �뺴
class ioExerciseCharIndexManager : public Singleton< ioExerciseCharIndexManager >
{
protected:
	enum
	{
		// ü��뺴 index�� 4284967295 ~ 4294967294 ���� õ������ ���.
		// �����뺴 index�� 4284967295 ��������� Ȯ�� ����� ��
		MAX_EXERCISE_CHAR_INDEX = 0xfffffffe,
		MIN_EXERCISE_CHAR_INDEX = 0xff67697f,
	};
protected:
	IntVec m_vIndexAdd;
	int    m_iStartAdd;
	int    m_iEndAdd;

public:
	void  Init( const DWORD dwServerIndex );
	DWORD Pop();
	void  Add(DWORD dwExerciseIndex );
	bool  IsHave();
	bool  IsRight( DWORD dwIndex );
	
public:
	ioExerciseCharIndexManager(void);
	virtual ~ioExerciseCharIndexManager(void);
};

#define g_ExerciseCharIndexMgr ioExerciseCharIndexManager::GetSingleton()

#endif // __ioExerciseCharIndexManager_h__


