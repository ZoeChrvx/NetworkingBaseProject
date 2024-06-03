#include <SDL.h>
#include <SDL_net.h>
#include <iostream>
#include <raylib.h>
#include <vector>
//#include <string>
using namespace std;
#include "raylib.h"
#include <string.h>
#include <sstream>
#include <charconv>
#include <iostream>
#include <vector>

const int screenWidth = 800;
const int screenHeight = 600;
Vector2 ballPosition = { screenWidth / 2.0f ,screenHeight / 2.0f };
const float ballRadius = 20;
float ballSpeedX = -2.0f;
float ballSpeedY = 0.0f;
const float paddleWidth = 10.0f;
const float paddleHeight = 100.0f;
Rectangle paddleLeft = { 0, screenHeight / 2.0f,paddleWidth,paddleHeight };
Rectangle paddleRight = { screenWidth-paddleWidth, screenHeight / 2.0f,paddleWidth,paddleHeight };
const float textOffset = 100.0f;
int leftScore, rightScore = 0;
const float halfBlankZone = 5.0f;
const float IASpeed = 2.0f;
const int victoryScore = 3;


int Move() {
    if (CheckCollisionCircleRec(ballPosition, ballRadius, paddleLeft)) {
        ballSpeedX *= -1;
        ballSpeedY = (ballPosition.y - (paddleLeft.y + paddleHeight / 2.0f)) / (paddleHeight / 2.0f) * 3.0f; //speedY -3:3
        std::cout << ballPosition.y << "  " << paddleLeft.y << "  " << ballSpeedY << std::endl;
    }
    if (CheckCollisionCircleRec(ballPosition, ballRadius, paddleRight)) {
        ballSpeedX *= -1;
        ballSpeedY = (ballPosition.y - (paddleRight.y+paddleHeight/2.0f)) / (paddleHeight / 2.0f) * 3.0f; //speedY -3:3
        std::cout << ballPosition.y << "  " << paddleRight.y << "  " << ballSpeedY << std::endl;
    }
    if (ballPosition.x <= 0) {
        ballPosition.x = screenWidth / 2.0f;
        ballPosition.y = screenHeight / 2.0f;
        rightScore++;
        if (rightScore == victoryScore) {
            return -1;
        }
    }
    if (ballPosition.x >= screenWidth) {
        ballPosition.x = screenWidth / 2.0f;
        ballPosition.y = screenHeight / 2.0f;
        leftScore++;
        if (leftScore == victoryScore) {
            return 1;
        }
    }

    ballPosition.x += ballSpeedX;

    if (ballPosition.y <= 0 || ballPosition.y >= screenHeight) {
        ballSpeedY *= -1.0f;
    }
    ballPosition.y += ballSpeedY;
    return 0;
}

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

GameState Draw() {
    BeginDrawing();
    ClearBackground(BLACK);
    DrawRectangleRec(paddleLeft, WHITE);
    DrawRectangleRec(paddleRight, WHITE);
    DrawCircle(ballPosition.x, ballPosition.y, ballRadius, WHITE);
    DrawText(std::to_string(leftScore).c_str(), screenWidth / 2 - textOffset, 50, 50, WHITE);
    DrawText(std::to_string(rightScore).c_str(), screenWidth / 2 + textOffset, 50, 50, WHITE);
    EndDrawing();
    GameState pongState = { paddleLeft.y, paddleRight.y, ballPosition, leftScore, rightScore     
    };
    return pongState;
}


int main(int argc, char* argv[]) {

    if (SDLNet_Init() == -1) {
        cerr << "SDLNet_Init error: " << SDLNet_GetError() << endl;
        return 1;
    }
    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, nullptr, 4242) == -1) {
        cerr << "Resolve Host error: " << SDLNet_GetError() << endl;
        SDLNet_Quit();
        return 1;
    }
    TCPsocket serverSocket = SDLNet_TCP_Open(&ip);
    if (!serverSocket) {
        cerr << "TCP Open error: " << SDLNet_GetError() << endl;
        SDLNet_Quit();
        return 1;
    }else cout<<"Server open"<<endl;

    TCPsocket clientSocket=nullptr;
    while(clientSocket == nullptr)
    {
        clientSocket = SDLNet_TCP_Accept(serverSocket);
    }
        

    SDLNet_SocketSet set = SDLNet_AllocSocketSet(1);
    SDLNet_TCP_AddSocket(set, clientSocket);

    if(clientSocket)
    {
        InitWindow(screenWidth, screenHeight, "Pong");
        // TODO: Initialize all required variables and load all required data here!

        int framesCounter = 0;
        int moveResult = 0;

        SetTargetFPS(60);
        while (!WindowShouldClose()) {
            framesCounter++;
            if(IsKeyDown(KEY_UP))
            {
                paddleLeft.y -= 2.0f;
            }
            if(IsKeyDown(KEY_DOWN))
            {
                paddleLeft.y += 2.0f;
            }
            if (moveResult == 0)
            {
                moveResult = Move();
                GameState currentState = Draw();
                if(framesCounter %4==0)
                    {
                        int bytesSent = SDLNet_TCP_Send(clientSocket, &currentState, sizeof(currentState));
                        if (bytesSent < sizeof(currentState))
                        {
                            cerr << "SDLNet TCP Send error: " << SDLNet_GetError() << endl;
                            break;
                        }
                    }
                if(SDLNet_CheckSockets(set, 0) != 0)
                {
                    int8_t move;
                    int bytesRead = SDLNet_TCP_Recv(clientSocket, &move, sizeof(move));
                    if(bytesRead > 0)
                    {
                        paddleRight.y += move * 2.0f;
                    }
                }
            }
            else {
                BeginDrawing();
                ClearBackground(BLACK);
                std::string tmp;
                tmp.append((moveResult == 1 ? "Player" : "Computer") + std::string(" wins"));
                DrawText(tmp.c_str(),0,0,100,WHITE);
                EndDrawing();
            }
        }
    }
}