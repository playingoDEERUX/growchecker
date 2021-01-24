#include <fstream>
#include "netclient.h"
#include <string>

#pragma warning (disable : 4996)

int enet_initialization_code = enet_initialize();
ENetClient* NetClient = new ENetClient();

int main() {
	printf("Growtopia Account Checker V1.0 by playingo/DEERUX\nPlease put your file with accounts in 'stored' folder and name it 'accs.txt', it will be automatically parsed.\nSupported format is 'tankIDName|', 'tankIDPass|', 'username:' and 'password:'\nIf your format is not supported or this checker is outdated and doesn't work anymore contact DEERUX#1551 on discord!\nIf theres any bugs also report it!\n");
	if (enet_initialization_code != 0)
	{
		printf("(ERROR): ENet could not initialize, exitting in a few seconds (exit code will be what enet_initialize will have returned)...\n");
		//Sleep(1000);
		exit(enet_initialization_code);
	}

	Sleep(3000);
	
	std::ifstream file;
	file.open("stored/accs.txt");

	ENetClient::PrivateData accTable;
	if (file.is_open()) {
		std::string line;
		
		while (std::getline(file, line)) {
			// using printf() in all tests for consistency
			//tool::LogMsg(line);
			size_t value;
			value = line.find("tankIDName|");
			if (value != std::string::npos) {
				accTable.username = line.substr(value + 11);
			}
			else
			{
				value = line.find("tankIDPass|");
				if (value != std::string::npos)
					accTable.password = line.substr(value + 11);
			}

			if (accTable.password != "" && accTable.username != "") {
				tool::LogMsg("Checking Account with USERNAME: " + accTable.username + " PASSWORD: " + accTable.password + "...");
				NetClient->setDefaultDataOnConnect(&accTable);
				ENetPeer* peer = NetClient->connectClient("213.179.209.168", 17198);
				if (peer) {
					enet_peer_timeout(peer, 1000, 2000, 3000);

					for (int i = 0; i < 8; i++)
						NetClient->CheckEvents(1000);

					enet_peer_disconnect(peer, 0);
				}
				
				accTable = ENetClient::PrivateData(); // reset
			}
		}
		file.close();
	}
	return 0;
}