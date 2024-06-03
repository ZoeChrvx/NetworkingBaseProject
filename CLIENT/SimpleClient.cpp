#include <SDL.h>
#include <SDL_net.h>
#include <iostream>
#include <raylib.h>
#include <vector>
#include <string>
using namespace std;

struct Message
{
    bool fromMe = false;
    string content;
};

struct GameState
{
    float paddleLeft, paddleRight;
    Vector2 ballPosition;
    int scoreLeft, scoreRight;
};

void Draw(GameState gameState) {
    BeginDrawing();
    ClearBackground(BLACK);
    DrawRectangleRec({ 0, gameState.paddleLeft,10,100 }, WHITE);
    DrawRectangleRec({ 800-10, gameState.paddleRight,10,100 }, WHITE);
    DrawCircle(gameState.ballPosition.x, gameState.ballPosition.y, 20, WHITE);
    DrawText(std::to_string(gameState.scoreLeft).c_str(), 800 / 2 - 100, 50, 50, WHITE);
    DrawText(std::to_string(gameState.scoreRight).c_str(), 800 / 2 + 100, 50, 50, WHITE);
    EndDrawing();
}

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
    
    const int width = 800, height = 600;
    InitWindow(width, height, "My first chat window !");
    SetTargetFPS(60);
    
    GameState currentGameState = {};
    while (!WindowShouldClose())
    {
        BeginDrawing();
        Draw(currentGameState);
         if(SDLNet_CheckSockets(set, 0)!=0 && SDLNet_SocketReady(clientSocket))
         {
             int bytesRead = SDLNet_TCP_Recv(clientSocket, &currentGameState, sizeof(currentGameState));
             if (bytesRead <= 0)
             {
                 cerr << "SDLNet TCP Recv error: " << SDLNet_GetError() << endl;
                 SDLNet_TCP_Close(clientSocket);
                 SDLNet_Quit();
                 return 1;
             }             
         }        
        
        if(IsKeyDown(KEY_UP))
        {
            int8_t move = -1;
            int bytesSent = SDLNet_TCP_Send(clientSocket, &move, sizeof(move));
            if (bytesSent < sizeof(move)) {
                cerr << "SDLNet TCP Send error: " << SDLNet_GetError() << endl;
                break;
            }
        }
        if(IsKeyDown(KEY_DOWN))
        {
            int8_t move = 1;
            int bytesSent = SDLNet_TCP_Send(clientSocket, &move, sizeof(move));
            if (bytesSent < sizeof(move)) {
                cerr << "SDLNet TCP Send error: " << SDLNet_GetError() << endl;
                break;
            }
        }
        EndDrawing();
    }
    
    SDLNet_TCP_Close(clientSocket);
    SDLNet_Quit();
    return 0;
}