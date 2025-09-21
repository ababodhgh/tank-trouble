#ifndef MAIN_H
#define MAIN_H

#include <windows.h>
#include <winsock2.h>

// Global variables declaration
extern HINSTANCE g_hInst;
extern HWND g_hWnd;
extern ULONG_PTR g_gdiplusToken;

// Game state
extern GameState g_gameState;
extern bool g_keys[256];

// Bitmap resources
extern HBITMAP g_hTank1Bitmap;
extern HBITMAP g_hTank2Bitmap;
extern HBITMAP g_hBullet1Bitmap;
extern HBITMAP g_hBullet2Bitmap;
extern HBITMAP g_hWallBitmap;

// Game loop timing
extern const int TARGET_FPS;
extern const int FRAME_DELAY;
extern bool g_gameRunning;

// Game state management
enum GameStateEnum {
    MENU_STATE,
    HOSTING_STATE,
    JOINING_STATE,
    GAME_STATE
};

extern GameStateEnum g_currentState;

// Menu variables
extern RECT g_hostButtonRect;
extern RECT g_joinButtonRect;
extern bool g_mousePressed;

// Networking variables
extern SOCKET g_listenSocket;
extern SOCKET g_clientSocket;
extern bool g_isHost;
extern char g_hostIP[256];
extern int g_port;

#endif // MAIN_H