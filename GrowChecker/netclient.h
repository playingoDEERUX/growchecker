#pragma once
#include "enet/enet.h"
#include "tool.h"

class ENetClient {
public:
	struct PrivateData {
		// account table
		std::string username, password;
		int userID = -1, token = -1;
	};

	void CheckEvents(int timeout = 1000);
	ENetPeer* connectClient(std::string ip, enet_uint16 port);
	void createHost(int maxPeers, enet_uint8 channels, int maxInwith = 0, int maxOutwith = 0); // optional
	void setDefaultDataOnConnect(void* data);
	ENetClient();
	ENetHost* GetClient();
private:
	void onConnect(ENetPeer* peer);
	void onDisconnect(ENetPeer* peer);
	void onReceive(ENetPeer* peer, uint8_t* packet, int packetLen);

	void* _autodata;
	ENetHost* host;
};