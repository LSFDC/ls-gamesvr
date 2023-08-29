#ifndef _ChannelParent_h_
#define _ChannelParent_h_

class SP2Packet;
class ChannelParent
{
	public:     // ä�� ����ȭ Ÿ��
	enum
	{
		CHANNEL_CREATE = 0,
		CHANNEL_DESTROY,
		CHANNEL_ENTER,
		CHANNEL_LEAVE,
		CHANNEL_TRANSFER,
	};


	//�����Լ��� �⿬
	public:
	virtual const DWORD GetIndex() = 0;
	virtual void EnterChannel( const DWORD dwUserIndex, const ioHashString &szUserName ) = 0;
	virtual void LeaveChannel( const DWORD dwUserIndex, const ioHashString &szUserName ) = 0;
	virtual void SendPacketTcp( SP2Packet &rkPacket, DWORD dwUserIndex = 0 ) = 0;
	virtual const bool IsChannelOriginal() = 0;

	public:
	ChannelParent();
	virtual ~ChannelParent();
};
typedef std::vector< ChannelParent * > vChannelParent;
typedef vChannelParent::iterator vChannelParent_iter;
#endif