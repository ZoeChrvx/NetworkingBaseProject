#include <SDL.h>
#include <SDL_net.h>
#include <iostream>
#include <raylib.h>
#include <vector>
//#include <string>
using namespace std;

struct Message
{
    bool fromMe = false;
    string content;
};

int main(int argc, char* argv[]) {

    if (SDLNet_Init() == -1) {
        cerr << "SDLNet_Init error: " << SDLNet_GetError() << endl;
        return 1;
    }
    //ip strin,g et port int
    cout << "Saisissez l'ip." << endl;
    string ipClient;
    cin >> ipClient;
    cout << "Saisissez le port" << endl;
    int host;
    cin >> host;
    cout << "Saisissez votre nom." << endl;
    string pseudoClient;
    cin >> pseudoClient;
    
    
    
    //Trouver la bonne adresse
    IPaddress ip;
    if(SDLNet_ResolveHost(&ip, ipClient.c_str(), host)==1)
    {
        cerr<< "Resolve Host Error: " << SDLNet_GetError() << endl;
        SDLNet_Quit();
        return 1;
    }

    //Creer la "socket"
    TCPsocket clientSocket = SDLNet_TCP_Open(&ip);
    if(!clientSocket)
    {
        cerr << "TCP Open error: " << SDLNet_GetError() << endl;
        SDLNet_Quit();
        return 1;
    }
    SDLNet_SocketSet set = SDLNet_AllocSocketSet(1);;
    SDLNet_TCP_AddSocket(set, clientSocket);
    int pseudoClientSend = SDLNet_TCP_Send(clientSocket, pseudoClient.c_str(), pseudoClient.length()+1);

    vector<Message> log{Message{ false, "Waiting for someone to talk to ..."}};
    
    const int width = 500, height = 750;
    InitWindow(width, height, "My first chat window !");
    SetTargetFPS(60);


    string typing;
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(GRAY);
        DrawText("Welcome to ChArtFX !", 220, 15, 25, WHITE);
        DrawRectangle(20,50,width-40,height-150, DARKBLUE);
        
        DrawRectangle(20,height-90,width-40,50, LIGHTGRAY);

        int inputChar = GetCharPressed();
        if(inputChar != 0)
        {
            typing += static_cast<char>(inputChar);
        }
        if(typing.size()>0)
        {
            if(IsKeyPressed(KEY_BACKSPACE))typing.pop_back();
            else if (IsKeyPressed(KEY_ENTER))
            {
                log.push_back(Message{true, typing});
                int bytesSent = SDLNet_TCP_Send(clientSocket, typing.c_str(), typing.length()+1);
                cout << "Sent " << bytesSent << " bytes to the server !" << std::endl;

                if(bytesSent < typing.length()+1)
                {
                    cerr << "SDLNet TCP Send error: " << SDLNet_GetError()<<endl;
                    SDLNet_TCP_Close(clientSocket);
                    SDLNet_Quit();
                    return 1;
                }
                typing.clear();
                
            }
            
        }
        char buffer[2048];
                
         if(SDLNet_CheckSockets(set, 0)!=0 && SDLNet_SocketReady(clientSocket))
         {
             int bytesRead = SDLNet_TCP_Recv(clientSocket, buffer, sizeof(buffer));
             if (bytesRead <= 0)
             {
                 cerr << "SDLNet TCP Recv error: " << SDLNet_GetError() << endl;
                 SDLNet_TCP_Close(clientSocket);
                 SDLNet_Quit();
                 return 1;
             }
             cout << "Incoming response: " << buffer << endl;
             log.emplace_back(Message{false, buffer});
         }
        for(int msg = 0; msg < log.size(); msg++)
        {
            DrawText(log[msg].content.c_str(), 30,90+(msg*30),15, log[msg].fromMe?SKYBLUE:PURPLE);
        }
        DrawText(typing.c_str(), 30, height - 75, 25, BLACK);
        EndDrawing();
    }

    //Envoyer un message

    
    SDLNet_TCP_Close(clientSocket);
    SDLNet_Quit();
	

	
    cout << "Thank you for using ChArtFX !\n";
    return 0;
}