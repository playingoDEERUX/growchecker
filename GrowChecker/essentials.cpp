#include "essentials.h"
#include "tool.h"
#include "netclient.h"
#pragma warning (disable : 4996)

using namespace std;

std::string essentials::CreateLogonPacket(std::string& username, std::string& password, int userID, int token, bool useAAPbypass)
{
    tool::LogMsg("Creating login packet for user: " + username + " with password: " + password + "...");
    std::string login_str = "tankIDName|" + username + "\n";
    login_str += "tankIDPass|" + password + "\nrequestedName|theogamer\nf|1\nprotocol|111\ngame_version|3.53\ncbits|0\nplayer_age|22\nGDPR|1\nhash2|636743872\nmeta|atnt.lines067.com\nfhash|-716928004\nplatformID|4\ndeviceVersion|0\ncountry|de\nhash|636743872\nmac|02:00:00:00:00:00";
    if (userID > 0 && token > 0) login_str += "\nuser|" + std::to_string(userID) + "\ntoken|" + std::to_string(token);
    login_str += "\nwk|NONE0\n";
    tool::LogMsg(login_str);
    return login_str;
}

void essentials::SendPacket(ENetPeer* peer, bool gamePacket, std::string text)
{
    if (!peer) return;
    if (peer->state == ENET_PEER_STATE_DISCONNECTED) return;

    int msgType = gamePacket ? 3 : 2;

    ENetPacket* v3 = enet_packet_create(0, text.length() + 5, 1);
    memcpy(v3->data, &msgType, 4);
    //*(v3->data) = (DWORD)a1;
    memcpy((v3->data) + 4, text.c_str(), text.length());

    //cout << std::hex << (int)(char)v3->data[3] << endl;
    enet_peer_send(peer, 0, v3);
}

void essentials::VarList::destroy()
{
    delete this;
}

essentials::VarList* essentials::VarList::SerializeFromMem(byte* pSrc, int bufferSize, int* pBytesReadOut, int netId)
{
	string action = "";
	BYTE* dataStruct = NULL;
	byte* startPtr = pSrc;
	byte* pStartPos = pSrc;
	byte count = pSrc[0]; pSrc++;
    if (count > VARLIST_MAX_COUNT) return nullptr;
    VarList* vList = new VarList();
    vList->varCount = count;

	bool isNetIdHandled = false;
#if DEBUG
	cout << std::to_string(count) + " entries in varlist detected." << endl;
	//for (int i = 0; i < 10; i++) cout << (int)*(pSrc + i) << endl;
#endif
    for (int i = 0; i < count; i++)
    {
        byte index = pSrc[0]; pSrc++;
        byte type = pSrc[0]; pSrc++;
        vList->v[index].type = type;

        switch (type)
        {
        case VarTypes::FLOAT:
        {
            vList->v[index].float32 = *(float*)pSrc; pSrc += 4;
            break;
        }
        case VarTypes::STRING:
        {
            int strLen = *(int*)pSrc; pSrc += 4;
            
            string v;
            v.resize(strLen);
            memcpy(&v[0], pSrc, strLen); pSrc += strLen;

            if (index == 0)
                vList->functionName = v;
            else if (index > 0)
                vList->v[index].text = v;

            break;
        }
        case VarTypes::UINT:
        {
            vList->v[index].uint32 = *(unsigned int*)pSrc; pSrc += 4;
            break;
        }
        case VarTypes::INT:
        {
            vList->v[index].int32 = *(int*)pSrc; pSrc += 4;
            break;
        }
        default:
            break;
        }
    }
    return vList;
}

void essentials::OnServerLogonRequest(ENetPeer* peer, uint8_t* data)
{
    if (peer->data) // assuming peer->data is our ClientData struct
    {
        ENetClient::PrivateData* pData = (ENetClient::PrivateData*)(peer->data);
        SendPacket(peer, false, CreateLogonPacket(pData->username, pData->password, pData->userID, pData->token));
    }
}

void essentials::OnTextPacket(ENetPeer* peer, std::string& text)
{
    tool::LogMsg("[GENERIC/GAME] MESSAGE: " + text);
}

void essentials::OnRawPacket(ENetPeer* peer, char* packet, int packetLen, void* client) // "packet" is the struct packet
{
    ENetClient* NetClient = (ENetClient*)client;
    uint8_t* extData = NULL;
    RawPacket* rawPacket = (RawPacket*)packet;
    if (rawPacket->packetFlags & 8) {
        if (packetLen < (rawPacket->packetLength + 60)) {
            tool::LogMsg("Packet too small for extended data to be valid, or extended data was null?!");
            rawPacket->extendedData = NULL;
            rawPacket->packetLength = 0;
            return;
        }
        if (rawPacket->packetLength >= 2048000) {
            tool::LogMsg("extended data is too big!");
            return;
        }
        tool::LogMsg("Extended rawpacket length is : " + std::to_string(rawPacket->packetLength));
        extData = (uint8_t*)(packet + 56);
    }
    else {
        rawPacket->packetLength = 0;
        rawPacket->extendedData = NULL;
        // assume its 0 anyway.
    }

    
    tool::LogMsg("Received a raw packet with packet type " + std::to_string(rawPacket->packetType));
    switch (rawPacket->packetType) 
    {
    case 1: // varlist
    {
        tool::LogMsg("We got variant list packet!");
        printf("arg count: %d\n", extData[0]);
		VarList* vList = VarList::SerializeFromMem(extData, rawPacket->packetLength, 0, rawPacket->netID);
        
        tool::LogMsg(vList->functionName);

        if (vList->functionName == "OnSendToServer") {
            ENetClient::PrivateData pd;
            ENetClient::PrivateData* pData = (ENetClient::PrivateData*)peer->data;

            pd.password = pData->password;
            pd.username = pData->username;
            pd.userID = vList->v[3].int32;
            pd.token = vList->v[2].int32;

            tool::LogMsg("USERID: " + std::to_string(pd.userID) + " TOKEN: " + std::to_string(pd.token));

            std::string sIP = tool::explode(vList->v[4].text, '|')[0];

            ENetPeer* currentPeer;

            for (currentPeer = peer->host->peers;
                currentPeer < &peer->host->peers[peer->host->peerCount];
                ++currentPeer)
            {
                enet_peer_disconnect(currentPeer, 0);
            }

            NetClient->setDefaultDataOnConnect(&pd);
            ENetPeer* peerd = NetClient->connectClient(sIP, vList->v[1].int32);

            if (peerd) {
                enet_peer_timeout(peerd, 1000, 2000, 3000);

                for (int i = 0; i < 5; i++)
                    NetClient->CheckEvents(200);

                enet_peer_disconnect(peerd, 0);
            }
        }
        break;
    }
    default:
        break;
    }
    
}

void essentials::OnTrackPacket(ENetPeer* peer, std::string& text)
{
    tool::LogMsg("TRACK PACKET: " + text);
    std::vector<std::string> lines = tool::explode(text, '\n');
    for (std::string line : lines) 
    {
        try {
            std::string key = line.substr(0, line.find("|"));
            std::string value = line.substr(line.find("|") + 1);

            if (key == "Authenticated") 
            {
                
            }
           
        }
        catch (...) {}
    }
}
