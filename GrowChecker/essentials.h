#pragma once
#include <string>
#include "netclient.h"
#include <vector>

#define VARLIST_MAX_COUNT 10

struct RawPacket {
	
#pragma pack(push,1)
		uint8_t packetType = 0; // offset: 0 (without having msgtype included...)
		uint16_t padding_1 = 0; // offset 1 and 2
		uint8_t padding_2 = 0; // byte at offset 3, used for fruit count
		int netID = -1; // offset: 4 (netid, can also have "no real value" which would represent negative values then.)
		int secondary_netID = -1; //offset: 8 (either padding or just secondary netid)
		uint32_t packetFlags = 0; // offset: 12 (holds "character state", may contain other data too tho)
		float waterSpeed = 125.0f; // offset: 16 (secondary padding, known for water speed/item count.)
		uint32_t value = 0; // offset: 20 (mainvalue, known for "plantingTree" or "tile") this is where stuff like tile id of punching etc is.
		float x, y; // offset 24 and 28
		float XSpeed, YSpeed; // offset 32 and 36
		uint32_t padding_3; // offset: 40 (third padding, idk. Might be used for particle size??)
		uint32_t punchX; // offset: 44 (punch X position)
		uint32_t punchY; // offset: 48 (punch Y position)
		uint32_t packetLength = 0; // offset: 52 (packet length for extended data)
		// extended data...
#pragma pack(pop)
		uint8_t* extendedData;
};


namespace essentials {
	enum VarTypes {
		NONE,
		FLOAT,
		STRING,
		VECTOR2,
		VECTOR3,
		UINT,
		INT = 9
	};

	struct Var {
		std::string text;
		char int8;
		unsigned short uint16;
		short int16;
		unsigned int uint32;
		int int32;
		float float32;
		unsigned long long uint64;
		long long int64;
		char type;
	};

	class VarList {
	public:
		char varCount = 0;
		Var v[VARLIST_MAX_COUNT];
		int netID = -1;
		int delay;

		std::string functionName;
		void destroy();
		static VarList* SerializeFromMem(byte* pSrc, int bufferSize, int* pBytesReadOut, int netId);
	};

	std::string CreateLogonPacket(std::string& username, std::string& password, int userID, int token, bool useAAPbypass = true);
	void OnServerLogonRequest(ENetPeer* peer, uint8_t* data);
	void OnTextPacket(ENetPeer* peer, std::string& text);
	void OnRawPacket(ENetPeer* peer, char* packet, int packetLen, void* client); // includes packetLength provided by ENet for security regarding trusting the 'packetLength' unpacked value in RawPacket struct.
	void OnTrackPacket(ENetPeer* peer, std::string& text);
	void SendPacket(ENetPeer* peer, bool gamePacket, std::string text);
	
}
