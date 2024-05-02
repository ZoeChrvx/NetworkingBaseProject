#include <SDL.h>
#include <SDL_net.h>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

int main(int argc, char* argv[])
{
	map<_TCPsocket*, std::string> clientInfo;
	if (SDLNet_Init() == -1)
	{
		cerr << "SDLNet_Init error: " << SDLNet_GetError() << endl;
		return 1;
	}

	//Qui je suis
	IPaddress ip;
	if (SDLNet_ResolveHost(&ip, nullptr, 4242) == -1)
	{
		cerr << "Resolve Host error: " << SDLNet_GetError() << endl;
		SDLNet_Quit();
		return 1;
	}

	//J'ouvre mes "socket"
	TCPsocket serverSocket = SDLNet_TCP_Open(&ip);
	if (!serverSocket)
	{
		cerr << "TCP Open error: " << SDLNet_GetError() << endl;
		SDLNet_Quit();
		return 1;
	}

	//J'attend des clients
	SDLNet_SocketSet set = SDLNet_AllocSocketSet(8);
	vector<_TCPsocket*> tcpSockets;
	while (true)
	{
		_TCPsocket* clientSocket;

		clientSocket = SDLNet_TCP_Accept(serverSocket);
		if(clientSocket)
		{
			cout << "A client reached the server !" << endl;
			SDLNet_AddSocket(set, reinterpret_cast<SDLNet_GenericSocket>(clientSocket));
			char buffer[1024];
			int bytesRead = SDLNet_TCP_Recv(clientSocket, buffer, sizeof(buffer));
			if(bytesRead > 0 )//&& SDLNet_CheckSockets(set, 0)!=0)
			{
				if(clientInfo.find(clientSocket) == clientInfo.end())
				{
					clientInfo.try_emplace(clientSocket, buffer);
					tcpSockets.emplace_back(clientSocket);
				}
				cout << "Pseudo: " << buffer << endl;
				
			}
		}
		if(clientInfo.size() > 0 && SDLNet_CheckSockets(set, 0)!=0)
		{
			char buffer[2048];
			for(_TCPsocket* sock : tcpSockets)
			{
				if(!SDLNet_SocketReady(sock))
				{
					continue;
				}
				int bytesRead = SDLNet_TCP_Recv(sock, buffer, sizeof(buffer));
				string msg = (clientInfo[sock] + " : " + buffer);
				if(bytesRead > 0)
				{
					cout << "Incoming message from "<<clientInfo[sock]<<": " << buffer << endl;

					for(_TCPsocket* rcvSock : tcpSockets)
					{
						if(rcvSock == sock) continue;
						int bytesSent = SDLNet_TCP_Send(rcvSock, msg.c_str(), msg.length()+1);
						if(bytesSent < msg.length()+1)
						{
							//ERROR
						}
					}
				}
			}
			
		}
	}

	// cout << "Thank you for using ChArtFX !\n";
	// for(TCPsocket* sock: tcpSockets)
	// {
	// 	SDLNet_TCP_Close(*sock);
	// }
	//SDLNet_TCP_Close(set);
	SDLNet_Quit();
	return 0;
}
