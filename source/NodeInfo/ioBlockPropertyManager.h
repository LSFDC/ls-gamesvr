#pragma once

struct CoordinateInfo
{
	int iX;
	int iY;
	int iZ;

	CoordinateInfo()
	{
		iX	= iY = iZ = 0;
	}
};

struct BlockDefaultInfo
{
	DWORD dwItemCode;
	int iXZ;
	int iY;
	int iDirection;

	BlockDefaultInfo()
	{
		dwItemCode	= 0;
		iXZ			= 0;
		iY			= 0;
		iDirection	= 0;
	}
};

typedef std::vector<CoordinateInfo> BLOCKCOORDINATEINFO;
typedef boost::unordered_map<DWORD, BLOCKCOORDINATEINFO> BLOCKPROPERTYGROUP;	// <itemCode, ioBlockProperty>

class ioBlockPropertyManager : public Singleton< ioBlockPropertyManager >
{
public:
	ioBlockPropertyManager();
	virtual ~ioBlockPropertyManager();

	void Init();
	void Destroy();

	void LoadIni();

public:
	static ioBlockPropertyManager& GetSingleton();

public:
	void GetCoordinateInfo(const DWORD dwItemCode, BLOCKCOORDINATEINFO& vInfo);

	BOOL IsValidItemCode(const DWORD dwItemCode);

protected:
	BLOCKPROPERTYGROUP m_mBlockPropertyGroup;
};

#define g_BlockPropertyMgr ioBlockPropertyManager::GetSingleton()