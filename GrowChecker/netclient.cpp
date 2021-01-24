#include "netclient.h"

void ENetClient::CheckEvents(int timeout)
{
    if (!host) return;

    ENetEvent event;
    while (enet_host_service(host, &event, timeout) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            event.peer->data = _autodata;
            onConnect(event.peer);
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            onReceive(event.peer, event.packet->data, event.packet->dataLength);
            enet_packet_destroy(event.packet);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            onDisconnect(event.peer);
            event.peer->data = NULL;
        }
    }
}

ENetPeer* ENetClient::connectClient(std::string ip, enet_uint16 port)
{
    tool::LogMsg("Connecting to IP: " + ip + ":" + std::to_string(port));
    ENetAddress address;
    ENetPeer* peer;
    /* Connect to some.server.net:1234. */
    enet_address_set_host(&address, ip.c_str());
    address.port = port;
    /* Initiate the connection, allocating the two channels 0 and 1. */
    peer = enet_host_connect(host, &address, 2, 0);
    if (peer == NULL)
    {
        fprintf(stderr,
            "No available peers for initiating an ENet connection.\n");
        exit(EXIT_FAILURE);
    }

    return peer;
}

void ENetClient::createHost(int maxPeers, enet_uint8 channels, int maxInwith, int maxOutwith)
{
    host = enet_host_create(NULL /* create a client host */,
        maxPeers /* only allow 1 outgoing connection */,
        channels /* allow up 2 channels to be used, 0 and 1 */,
        maxInwith /* assume any amount of incoming bandwidth */,
        maxOutwith /* assume any amount of outgoing bandwidth */);
    if (host == NULL)
    {
        fprintf(stderr,
            "An error occurred while trying to create an ENet client host.\n");
        exit(EXIT_FAILURE);
    }
    host->checksum = enet_crc32;
    enet_host_compress_with_range_coder(host);
}

void ENetClient::setDefaultDataOnConnect(void* data)
{
    _autodata = data;
}

ENetClient::ENetClient()
{
    createHost(128, 2); // create enet host with recommended defaults.
}

ENetHost* ENetClient::GetClient()
{
    if (!host) tool::LogMsg("NOTE: host was null!");
    return host;
}

void ENetClient::onConnect(ENetPeer* peer)
{
    tool::LogMsg("Bot connected to the servers!");
}

void ENetClient::onDisconnect(ENetPeer* peer)
{
    tool::LogMsg("Bot disconnected from the servers!");
}

void ENetClient::onReceive(ENetPeer* peer, uint8_t* packet, int packetLen)
{
    if (packetLen <= 4) return; // No message type ?
    std::string text = std::string((char*)packet + 4); // packet in text form if ever needed.
    text.resize(packetLen - 5);
    
    int messageType = *(int*)packet;
    tool::LogMsg("WE RECEIVED A MSG WITH TYPE: " + std::to_string(messageType));

    switch (*(int*)packet)
    {
    case 1:
        essentials::OnServerLogonRequest(peer, packet + 4);
        break;
    case 2:
    case 3:
    {
        essentials::OnTextPacket(peer, text);
        break;
    }
    case 4:
    {
        //tool::LogMsg(std::to_string(packetLen));
        if (packetLen < 60)
        {
            tool::LogMsg("WARNING: Received raw packet with bad packet length (shorter than 60).");
            return;
        }
        
        essentials::OnRawPacket(peer, (char*)(packet + 4), packetLen, this);
        break;
    }
    case 5:
        essentials::OnTrackPacket(peer, text);
        break;
    case 6:
        essentials::OnTrackPacket(peer, text);
        break;
    case 7:
    case 8:
    case 9: // just to be sure
        break;
    default:
    {
        tool::LogMsg("Unknown message type: " + std::to_string(packet[0]));
    }
    }
}
