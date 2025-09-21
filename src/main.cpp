// TroubleTanks - Phase 2: Game Core & State Management
// This implementation extends Phase 1 with:
// 1. Game core structures (Tank, Bullet, GameState)
// 2. Fixed-time-step game loop
// 3. Tank movement and shooting mechanics
// 4. Basic collision detection
// 5. Rendering of game state

#include <windows.h>
#include <winsock2.h>
#include <mmsystem.h>
#include <gdiplus.h>
#ifndef __MINGW32__
#include <comdef.h>
#endif
#include <cmath>
#include <cstdio>
#include <cwchar>
#include <shellapi.h>
#include "resource.h"
#include "game.h"
#include "network.h"
#include "main.h"

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ws2_32.lib")

// Only include these pragmas when not using MinGW
#ifndef __MINGW32__
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "advapi32.lib")
#endif

// Global variables
HINSTANCE g_hInst = NULL;
HWND g_hWnd = NULL;
ULONG_PTR g_gdiplusToken = 0;

// Game state
GameState g_gameState;
bool g_keys[256] = { false }; // Keyboard state

// Bitmap resources
HBITMAP g_hTank1Bitmap = NULL;
HBITMAP g_hTank2Bitmap = NULL;
HBITMAP g_hBullet1Bitmap = NULL;
HBITMAP g_hBullet2Bitmap = NULL;
HBITMAP g_hWallBitmap = NULL;

// Game state management
GameStateEnum g_currentState = MENU_STATE;

// Menu variables
RECT g_hostButtonRect = { 300, 200, 500, 250 };
RECT g_joinButtonRect = { 300, 300, 500, 350 };
bool g_mousePressed = false;

// Networking variables
SOCKET g_listenSocket = INVALID_SOCKET;
SOCKET g_clientSocket = INVALID_SOCKET;
bool g_isHost = false;
char g_hostIP[256] = {0};
int g_port = 8888;

// Sound variables
bool g_soundEnabled = true;

// Game loop timing
const int TARGET_FPS = 30;
const int FRAME_DELAY = 1000 / TARGET_FPS;
bool g_gameRunning = true;

// Function prototypes
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL InitInstance(HINSTANCE, int);
BOOL InitGdiplus();
void CleanupGdiplus();
HBITMAP LoadPNGFromResource(int resourceId);
void LoadGameResources();
void UnloadGameResources();
void RenderMenu(HDC hdc);
void RenderGameState(HDC hdc);
void HandleInput();
bool BeginHosting();  // Renamed from StartHosting to avoid conflict
void StartJoining();
void UpdateNetwork();
void PlaySoundEffect(int soundId);

// Entry point
#ifdef __MINGW32__
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    // Simple wrapper that calls our wWinMain
    return wWinMain(hInstance, hPrevInstance, L"", nCmdShow);
}
#endif

int APIENTRY wWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPWSTR    lpCmdLine,
                     int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize GDI+
    if (!InitGdiplus()) {
        MessageBox(NULL, L"Failed to initialize GDI+", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // Initialize networking
    if (!InitializeNetwork()) {
        CleanupGdiplus();
        MessageBox(NULL, L"Failed to initialize networking", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Register window class
    WNDCLASSEXW wcex = { sizeof(WNDCLASSEXW) };
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = L"TroubleTanksClass";
    wcex.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassExW(&wcex)) {
        CleanupNetwork();
        CleanupGdiplus();
        MessageBox(NULL, L"Failed to register window class", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    g_hInst = hInstance;

    // Initialize the application
    if (!InitInstance(hInstance, nCmdShow)) {
        CleanupNetwork();
        CleanupGdiplus();
        return 1;
    }

    // Load game resources
    LoadGameResources();

    // Main message loop with game loop
    MSG msg = {0};
    DWORD lastTime = GetTickCount();
    
    while (g_gameRunning) {
        // Handle Windows messages
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                g_gameRunning = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        // Calculate delta time
        DWORD currentTime = GetTickCount();
        DWORD deltaTime = currentTime - lastTime;
        
        // Update game state based on current state
        switch (g_currentState) {
            case MENU_STATE:
                // No game update needed in menu state
                break;
            case HOSTING_STATE:
                // In a real implementation, we would handle hosting logic here
                break;
            case JOINING_STATE:
                // In a real implementation, we would handle joining logic here
                break;
            case GAME_STATE:
                HandleInput();
                g_gameState.Update();
                UpdateNetwork(); // Handle networking updates
                InvalidateRect(g_hWnd, NULL, FALSE); // Trigger repaint
                break;
        }
        
        // Update at target FPS
        if (deltaTime >= FRAME_DELAY) {
            lastTime = currentTime;
        }
        
        // Small delay to prevent excessive CPU usage
        Sleep(1);
    }

    // Cleanup
    UnloadGameResources();
    CleanupNetwork();
    CleanupGdiplus();

    return (int) msg.wParam;
}

//
//  FUNCTION: InitInstance(HINSTANCE, int)
//
//  PURPOSE: Saves instance handle and creates main window
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   g_hInst = hInstance;

   // Create window with specific size
   RECT windowRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
   AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
   
   int windowWidth = windowRect.right - windowRect.left;
   int windowHeight = windowRect.bottom - windowRect.top;

   g_hWnd = CreateWindowW(L"TroubleTanksClass", L"Trouble Tanks - Phase 2", 
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, 0, windowWidth, windowHeight, 
                          nullptr, nullptr, hInstance, nullptr);

   if (!g_hWnd) {
      return FALSE;
   }

   ShowWindow(g_hWnd, nCmdShow);
   UpdateWindow(g_hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            
            // Render based on current state
            switch (g_currentState) {
                case MENU_STATE:
                    RenderMenu(hdc);
                    break;
                case HOSTING_STATE:
                case JOINING_STATE:
                    // Show connecting message
                    {
                        RECT rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
                        FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
                        SetTextColor(hdc, RGB(0, 0, 0));
                        SetBkMode(hdc, TRANSPARENT);
                        DrawText(hdc, L"Connecting...", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    }
                    break;
                case GAME_STATE:
                    RenderGameState(hdc);
                    break;
            }
            
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_LBUTTONDOWN:
        {
            int mouseX = LOWORD(lParam);
            int mouseY = HIWORD(lParam);
            
            // Handle mouse clicks only in menu state
            if (g_currentState == MENU_STATE) {
                // Check if host button was clicked
                if (mouseX >= g_hostButtonRect.left && mouseX <= g_hostButtonRect.right &&
                    mouseY >= g_hostButtonRect.top && mouseY <= g_hostButtonRect.bottom) {
                    // Start hosting
                    BeginHosting();
                }
                // Check if join button was clicked
                else if (mouseX >= g_joinButtonRect.left && mouseX <= g_joinButtonRect.right &&
                         mouseY >= g_joinButtonRect.top && mouseY <= g_joinButtonRect.bottom) {
                    // Start joining
                    StartJoining();
                }
            }
        }
        break;
    case WM_KEYDOWN:
        if (wParam >= 0 && wParam < 256) {
            g_keys[wParam] = true;
        }
        
        // Handle restart key
        if (wParam == 'R' && g_gameState.gameOver) {
            g_gameState.Reset();
        }
        break;
    case WM_KEYUP:
        if (wParam >= 0 && wParam < 256) {
            g_keys[wParam] = false;
        }
        break;
    case WM_DESTROY:
        g_gameRunning = false;
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

//
//  FUNCTION: LoadGameResources()
//
//  PURPOSE: Loads all game bitmaps from resources
//
void LoadGameResources() {
    g_hTank1Bitmap = LoadPNGFromResource(IDB_TANK1_PNG);
    g_hTank2Bitmap = LoadPNGFromResource(IDB_TANK2_PNG);
    g_hBullet1Bitmap = LoadPNGFromResource(IDB_TANK1_BULLET_PNG);
    g_hBullet2Bitmap = LoadPNGFromResource(IDB_TANK2_BULLET_PNG);
    g_hWallBitmap = LoadPNGFromResource(IDB_WALL_PNG);
}

//
//  FUNCTION: UnloadGameResources()
//
//  PURPOSE: Frees all loaded bitmaps
//
void UnloadGameResources() {
    if (g_hTank1Bitmap) {
        DeleteObject(g_hTank1Bitmap);
        g_hTank1Bitmap = NULL;
    }
    if (g_hTank2Bitmap) {
        DeleteObject(g_hTank2Bitmap);
        g_hTank2Bitmap = NULL;
    }
    if (g_hBullet1Bitmap) {
        DeleteObject(g_hBullet1Bitmap);
        g_hBullet1Bitmap = NULL;
    }
    if (g_hBullet2Bitmap) {
        DeleteObject(g_hBullet2Bitmap);
        g_hBullet2Bitmap = NULL;
    }
    if (g_hWallBitmap) {
        DeleteObject(g_hWallBitmap);
        g_hWallBitmap = NULL;
    }
}

//
//  FUNCTION: InitGdiplus()
//
//  PURPOSE: Initializes GDI+
//
BOOL InitGdiplus()
{
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    return (Gdiplus::GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL) == Gdiplus::Ok);
}

//
//  FUNCTION: CleanupGdiplus()
//
//  PURPOSE: Cleans up GDI+
//
void CleanupGdiplus()
{
    if (g_gdiplusToken) {
        Gdiplus::GdiplusShutdown(g_gdiplusToken);
        g_gdiplusToken = 0;
    }
}

//
//  FUNCTION: LoadPNGFromResource(int)
//
//  PURPOSE: Loads a PNG image from resources and returns an HBITMAP
//
HBITMAP LoadPNGFromResource(int resourceId)
{
    HRSRC hResource = FindResource(g_hInst, MAKEINTRESOURCE(resourceId), L"RCDATA");
    if (!hResource) {
        return NULL;
    }

    DWORD imageSize = SizeofResource(g_hInst, hResource);
    if (imageSize == 0) {
        return NULL;
    }

    HGLOBAL hGlobal = LoadResource(g_hInst, hResource);
    if (!hGlobal) {
        return NULL;
    }

    void* pData = LockResource(hGlobal);
    if (!pData) {
        return NULL;
    }

    // Create IStream from memory data
    HGLOBAL hGlobalStream = GlobalAlloc(GMEM_MOVEABLE, imageSize);
    if (!hGlobalStream) {
        return NULL;
    }

    void* pStreamData = GlobalLock(hGlobalStream);
    if (!pStreamData) {
        GlobalFree(hGlobalStream);
        return NULL;
    }

    CopyMemory(pStreamData, pData, imageSize);
    GlobalUnlock(hGlobalStream);

    IStream* pStream = NULL;
    if (CreateStreamOnHGlobal(hGlobalStream, FALSE, &pStream) != S_OK) {
        GlobalFree(hGlobalStream);
        return NULL;
    }

    // Create GDI+ bitmap from stream
    Gdiplus::Bitmap* pBitmap = Gdiplus::Bitmap::FromStream(pStream);
    if (!pBitmap || pBitmap->GetLastStatus() != Gdiplus::Ok) {
        pStream->Release();
        GlobalFree(hGlobalStream);
        if (pBitmap) {
            delete pBitmap;
        }
        return NULL;
    }

    // Convert to HBITMAP
    HBITMAP hBitmap = NULL;
    Gdiplus::Color color(0, 0, 0); // Black background
    pBitmap->GetHBITMAP(color, &hBitmap);

    // Cleanup
    delete pBitmap;
    pStream->Release();
    GlobalFree(hGlobalStream);

    return hBitmap;
}

//
//  FUNCTION: RenderMenu(HDC)
//
//  PURPOSE: Renders the main menu
//
void RenderMenu(HDC hdc) {
    // Create compatible DC for double buffering
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, WINDOW_WIDTH, WINDOW_HEIGHT);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);
    
    // Fill background with gradient
    RECT backgroundRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    HBRUSH hBackgroundBrush = CreateSolidBrush(RGB(50, 50, 100)); // Dark blue background
    FillRect(memDC, &backgroundRect, hBackgroundBrush);
    DeleteObject(hBackgroundBrush);
    
    // Draw title with better styling
    HFONT hTitleFont = CreateFont(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                                 OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, 
                                 VARIABLE_PITCH, TEXT("Arial"));
    HFONT hOldFont = (HFONT)SelectObject(memDC, hTitleFont);
    
    SetTextColor(memDC, RGB(255, 255, 255));
    SetBkMode(memDC, TRANSPARENT);
    
    RECT titleRect = { 0, 50, WINDOW_WIDTH, 150 };
    DrawText(memDC, L"Trouble Tanks", -1, &titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    // Draw subtitle
    HFONT hSubtitleFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                                    OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, 
                                    VARIABLE_PITCH, TEXT("Arial"));
    SelectObject(memDC, hSubtitleFont);
    
    RECT subtitleRect = { 0, 120, WINDOW_WIDTH, 180 };
    DrawText(memDC, L"Multiplayer Tank Battle", -1, &subtitleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    // Draw buttons with better styling
    HBRUSH hButtonBrush = CreateSolidBrush(RGB(0, 100, 0)); // Dark green
    HBRUSH hButtonBrushHover = CreateSolidBrush(RGB(0, 150, 0)); // Lighter green
    HPEN hButtonPen = CreatePen(PS_SOLID, 2, RGB(0, 200, 0)); // Bright green border
    
    HPEN hOldPen = (HPEN)SelectObject(memDC, hButtonPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(memDC, hButtonBrush);
    
    // Host button
    RoundRect(memDC, g_hostButtonRect.left, g_hostButtonRect.top, 
              g_hostButtonRect.right, g_hostButtonRect.bottom, 10, 10);
    
    // Join button
    RoundRect(memDC, g_joinButtonRect.left, g_joinButtonRect.top, 
              g_joinButtonRect.right, g_joinButtonRect.bottom, 10, 10);
    
    SelectObject(memDC, hOldPen);
    SelectObject(memDC, hOldBrush);
    
    DeleteObject(hButtonPen);
    DeleteObject(hButtonBrush);
    DeleteObject(hButtonBrushHover);
    
    // Draw button text with better styling
    HFONT hButtonFont = CreateFont(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                                  OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, 
                                  VARIABLE_PITCH, TEXT("Arial"));
    SelectObject(memDC, hButtonFont);
    
    SetTextColor(memDC, RGB(255, 255, 255));
    
    RECT hostTextRect = g_hostButtonRect;
    DrawText(memDC, L"Host Game", -1, &hostTextRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    RECT joinTextRect = g_joinButtonRect;
    DrawText(memDC, L"Join Game", -1, &joinTextRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    // Draw instructions
    HFONT hInstructionFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                                       OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, 
                                       VARIABLE_PITCH, TEXT("Arial"));
    SelectObject(memDC, hInstructionFont);
    
    RECT instructionRect = { 0, WINDOW_HEIGHT - 100, WINDOW_WIDTH, WINDOW_HEIGHT - 50 };
    DrawText(memDC, L"Click a button to start playing", -1, &instructionRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    // Blit the double-buffered image to the screen
    BitBlt(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, memDC, 0, 0, SRCCOPY);
    
    // Cleanup
    SelectObject(memDC, hOldFont);
    DeleteObject(hTitleFont);
    DeleteObject(hSubtitleFont);
    DeleteObject(hButtonFont);
    DeleteObject(hInstructionFont);
    SelectObject(memDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(memDC);
}

//
//  FUNCTION: RenderGameState(HDC)
//
//  PURPOSE: Renders the current game state
//
void RenderGameState(HDC hdc) {
    // Create compatible DC for double buffering
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, WINDOW_WIDTH, WINDOW_HEIGHT);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);
    
    // Fill background
    RECT backgroundRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    HBRUSH hBackgroundBrush = CreateSolidBrush(RGB(200, 200, 200)); // Light gray background
    FillRect(memDC, &backgroundRect, hBackgroundBrush);
    DeleteObject(hBackgroundBrush);
    
    // Draw maze with better visual styling
    for (int x = 0; x < GameState::MAZE_WIDTH; x++) {
        for (int y = 0; y < GameState::MAZE_HEIGHT; y++) {
            if (g_gameState.maze[x][y].wall) {
                if (g_hWallBitmap) {
                    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, g_hWallBitmap);
                    BitBlt(memDC, x * WALL_SIZE, y * WALL_SIZE, WALL_SIZE, WALL_SIZE, memDC, 0, 0, SRCCOPY);
                    SelectObject(memDC, oldBitmap);
                } else {
                    // Fallback: draw a textured rectangle
                    RECT rect = { x * WALL_SIZE, y * WALL_SIZE, (x + 1) * WALL_SIZE, (y + 1) * WALL_SIZE };
                    HBRUSH hBrush = CreateSolidBrush(RGB(100, 100, 100)); // Dark gray wall
                    FillRect(memDC, &rect, hBrush);
                    DeleteObject(hBrush);
                    
                    // Add border for better visibility
                    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(50, 50, 50));
                    HPEN hOldPen = (HPEN)SelectObject(memDC, hPen);
                    MoveToEx(memDC, x * WALL_SIZE, y * WALL_SIZE, NULL);
                    LineTo(memDC, (x + 1) * WALL_SIZE, y * WALL_SIZE);
                    LineTo(memDC, (x + 1) * WALL_SIZE, (y + 1) * WALL_SIZE);
                    LineTo(memDC, x * WALL_SIZE, (y + 1) * WALL_SIZE);
                    LineTo(memDC, x * WALL_SIZE, y * WALL_SIZE);
                    SelectObject(memDC, hOldPen);
                    DeleteObject(hPen);
                }
            }
        }
    }
    
    // Draw tanks with better positioning
    for (int i = 0; i < 2; i++) {
        if (g_gameState.tanks[i].alive) {
            HBITMAP hTankBitmap = (i == 0) ? g_hTank1Bitmap : g_hTank2Bitmap;
            
            if (hTankBitmap) {
                HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hTankBitmap);
                // Use StretchBlt for better scaling if needed
                StretchBlt(memDC, (int)g_gameState.tanks[i].x, (int)g_gameState.tanks[i].y, 
                          TANK_WIDTH, TANK_HEIGHT, memDC, 0, 0, TANK_WIDTH, TANK_HEIGHT, SRCCOPY);
                SelectObject(memDC, oldBitmap);
            } else {
                // Fallback: draw a colored rectangle with direction indicator
                RECT rect = { (int)g_gameState.tanks[i].x, (int)g_gameState.tanks[i].y, 
                              (int)g_gameState.tanks[i].x + TANK_WIDTH, (int)g_gameState.tanks[i].y + TANK_HEIGHT };
                HBRUSH hBrush = CreateSolidBrush((i == 0) ? RGB(0, 200, 0) : RGB(200, 0, 0)); // Darker colors
                FillRect(memDC, &rect, hBrush);
                DeleteObject(hBrush);
                
                // Draw direction indicator (simple line pointing in tank direction)
                HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
                HPEN hOldPen = (HPEN)SelectObject(memDC, hPen);
                int centerX = (int)g_gameState.tanks[i].x + TANK_WIDTH / 2;
                int centerY = (int)g_gameState.tanks[i].y + TANK_HEIGHT / 2;
                int endX = centerX + (int)(cos(g_gameState.tanks[i].rotation) * (TANK_WIDTH / 2));
                int endY = centerY + (int)(sin(g_gameState.tanks[i].rotation) * (TANK_HEIGHT / 2));
                MoveToEx(memDC, centerX, centerY, NULL);
                LineTo(memDC, endX, endY);
                SelectObject(memDC, hOldPen);
                DeleteObject(hPen);
            }
        }
    }
    
    // Draw bullets with visual enhancements
    for (const Bullet& bullet : g_gameState.bullets) {
        if (bullet.active) {
            HBITMAP hBulletBitmap = (bullet.ownerID == 1) ? g_hBullet1Bitmap : g_hBullet2Bitmap;
            
            if (hBulletBitmap) {
                HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hBulletBitmap);
                BitBlt(memDC, (int)bullet.x, (int)bullet.y, 
                       BULLET_WIDTH, BULLET_HEIGHT, memDC, 0, 0, SRCCOPY);
                SelectObject(memDC, oldBitmap);
            } else {
                // Fallback: draw a small circle with owner color
                HBRUSH hBrush = CreateSolidBrush((bullet.ownerID == 1) ? RGB(0, 255, 0) : RGB(255, 0, 0));
                HBRUSH hOldBrush = (HBRUSH)SelectObject(memDC, hBrush);
                HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
                HPEN hOldPen = (HPEN)SelectObject(memDC, hPen);
                
                Ellipse(memDC, (int)bullet.x, (int)bullet.y, 
                        (int)bullet.x + BULLET_WIDTH, (int)bullet.y + BULLET_HEIGHT);
                
                SelectObject(memDC, hOldBrush);
                SelectObject(memDC, hOldPen);
                DeleteObject(hBrush);
                DeleteObject(hPen);
            }
            
            // Draw bullet trail for visual effect
            if (bullet.bounceCount > 0) {
                HPEN hTrailPen = CreatePen(PS_SOLID, 1, 
                    (bullet.ownerID == 1) ? RGB(100, 255, 100) : RGB(255, 100, 100));
                HPEN hOldTrailPen = (HPEN)SelectObject(memDC, hTrailPen);
                MoveToEx(memDC, (int)bullet.x + BULLET_WIDTH/2, (int)bullet.y + BULLET_HEIGHT/2, NULL);
                LineTo(memDC, (int)bullet.x + BULLET_WIDTH/2 - bullet.velocityX*2, 
                       (int)bullet.y + BULLET_HEIGHT/2 - bullet.velocityY*2);
                SelectObject(memDC, hOldTrailPen);
                DeleteObject(hTrailPen);
            }
        }
    }
    
    // Draw particles (explosion effects)
    for (const Particle& particle : g_gameState.particles) {
        if (particle.active) {
            HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 0)); // Yellow particles
            HBRUSH hOldBrush = (HBRUSH)SelectObject(memDC, hBrush);
            
            // Make particles smaller as they age
            int size = 2 + (particle.lifetime / 3);
            Ellipse(memDC, (int)particle.x - size/2, (int)particle.y - size/2,
                    (int)particle.x + size/2, (int)particle.y + size/2);
            
            SelectObject(memDC, hOldBrush);
            DeleteObject(hBrush);
        }
    }
    
    // Draw scores with better styling
    wchar_t scoreText[256];
#ifdef __MINGW32__
    swprintf(scoreText, L"Player 1: %d    Player 2: %d", g_gameState.scores[0], g_gameState.scores[1]);
#else
    swprintf_s(scoreText, L"Player 1: %d    Player 2: %d", g_gameState.scores[0], g_gameState.scores[1]);
#endif
    
    HFONT hFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                            OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, 
                            VARIABLE_PITCH, TEXT("Arial"));
    HFONT hOldFont = (HFONT)SelectObject(memDC, hFont);
    
    SetTextColor(memDC, RGB(0, 0, 0));
    SetBkMode(memDC, TRANSPARENT);
    
    RECT scoreRect = { 10, 10, WINDOW_WIDTH - 10, 40 };
    DrawText(memDC, scoreText, -1, &scoreRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    
    SelectObject(memDC, hOldFont);
    DeleteObject(hFont);
    
    // Draw game over screen if game is over
    if (g_gameState.gameOver) {
        // Semi-transparent overlay
        RECT overlayRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
        HBRUSH hOverlayBrush = CreateSolidBrush(RGB(0, 0, 0));
        SetBkColor(memDC, RGB(0, 0, 0));
        SetBkMode(memDC, OPAQUE);
        ExtTextOut(memDC, 0, 0, ETO_OPAQUE, &overlayRect, NULL, 0, NULL);
        
        // Game over text
        hFont = CreateFont(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                          OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, 
                          VARIABLE_PITCH, TEXT("Arial"));
        hOldFont = (HFONT)SelectObject(memDC, hFont);
        
        SetTextColor(memDC, RGB(255, 255, 255));
        
        wchar_t gameOverText[256];
        if (g_gameState.winner == 0) {
#ifdef __MINGW32__
            wcscpy(gameOverText, L"Game Over - Tie!");
#else
            wcscpy_s(gameOverText, L"Game Over - Tie!");
#endif
        } else {
#ifdef __MINGW32__
            swprintf(gameOverText, L"Game Over - Player %d Wins!", g_gameState.winner);
#else
            swprintf_s(gameOverText, L"Game Over - Player %d Wins!", g_gameState.winner);
#endif
        }
        
        RECT gameOverRect = { 0, WINDOW_HEIGHT / 2 - 50, WINDOW_WIDTH, WINDOW_HEIGHT / 2 + 50 };
        DrawText(memDC, gameOverText, -1, &gameOverRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        // Restart instruction
        hFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                          OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, 
                          VARIABLE_PITCH, TEXT("Arial"));
        SelectObject(memDC, hFont);
        
        RECT restartRect = { 0, WINDOW_HEIGHT / 2 + 50, WINDOW_WIDTH, WINDOW_HEIGHT / 2 + 100 };
        DrawText(memDC, L"Press R to restart", -1, &restartRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        SelectObject(memDC, hOldFont);
        DeleteObject(hFont);
        DeleteObject(hOverlayBrush);
    }
    
    // Blit the double-buffered image to the screen
    BitBlt(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, memDC, 0, 0, SRCCOPY);
    
    // Cleanup
    SelectObject(memDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(memDC);
}

//
//  FUNCTION: BeginHosting()
//
//  PURPOSE: Starts hosting a game
//
bool BeginHosting() {
    g_currentState = HOSTING_STATE;
    InvalidateRect(g_hWnd, NULL, TRUE);
    
    // In a real implementation, we would show a "waiting for player" message
    // and start a background thread to accept connections
    if (StartHosting()) {  // Call the network library function
        // For now, we'll simulate waiting for a connection
        // In a real implementation, this would be handled in a separate thread
        g_isHost = true;
        g_currentState = GAME_STATE;
        // Initialize game as host (player 1)
        g_gameState.tanks[0].playerID = 1;
        g_gameState.tanks[1].playerID = 2;
        InvalidateRect(g_hWnd, NULL, TRUE);
        return true;
    } else {
        MessageBox(g_hWnd, L"Failed to start hosting", L"Error", MB_OK | MB_ICONERROR);
        g_currentState = MENU_STATE;
        InvalidateRect(g_hWnd, NULL, TRUE);
        return false;
    }
}

//
//  FUNCTION: StartJoining()
//
//  PURPOSE: Starts joining a game
//
void StartJoining() {
    g_currentState = JOINING_STATE;
    InvalidateRect(g_hWnd, NULL, TRUE);
    
    // In a real implementation, we would prompt for IP address
    // For now, we'll just simulate joining with localhost
    if (ConnectToHost("127.0.0.1")) {
        g_isHost = false;
        g_currentState = GAME_STATE;
        // Initialize game as client (player 2)
        g_gameState.tanks[0].playerID = 1;
        g_gameState.tanks[1].playerID = 2;
        InvalidateRect(g_hWnd, NULL, TRUE);
    } else {
        MessageBox(g_hWnd, L"Failed to connect to host", L"Error", MB_OK | MB_ICONERROR);
        g_currentState = MENU_STATE;
        InvalidateRect(g_hWnd, NULL, TRUE);
    }
}

//
//  FUNCTION: PlaySoundEffect(int)
//
//  PURPOSE: Plays a sound effect
//
void PlaySoundEffect(int soundId) {
    if (g_soundEnabled) {
        switch (soundId) {
            case IDS_SHOOT_SOUND:
                // PlaySound(MAKEINTRESOURCE(IDS_SHOOT_SOUND), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
                break;
            case IDS_EXPLOSION_SOUND:
                // PlaySound(MAKEINTRESOURCE(IDS_EXPLOSION_SOUND), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
                break;
        }
    }
}

//
//  FUNCTION: UpdateNetwork()
//
//  PURPOSE: Handles networking updates
//
void UpdateNetwork() {
    if (g_clientSocket != INVALID_SOCKET) {
        if (g_isHost) {
            // Host sends game state to client
            if (!SendGameStatePacket(g_clientSocket, g_gameState)) {
                // Handle disconnection
                HandleDisconnection();
                g_currentState = MENU_STATE;
                InvalidateRect(g_hWnd, NULL, TRUE);
                return;
            }
            
            // Host also receives input from client
            bool clientKeys[256] = { false };
            if (ReceiveInputPacket(g_clientSocket, clientKeys)) {
                // Apply client input to player 2 (client controls player 2)
                // In a real implementation, we would update the client's tank directly
                // For now, we'll just show how it would work
            }
        } else {
            // Client sends input to host
            if (!SendInputPacket(g_clientSocket, g_keys)) {
                // Handle disconnection
                HandleDisconnection();
                g_currentState = MENU_STATE;
                InvalidateRect(g_hWnd, NULL, TRUE);
                return;
            }
            
            // Client receives game state from host
            GameState receivedState;
            if (ReceiveGameStatePacket(g_clientSocket, receivedState)) {
                // Update local game state with received state
                g_gameState = receivedState;
            }
        }
    }
}

//
//  FUNCTION: HandleInput()
//
//  PURPOSE: Processes user input
//
void HandleInput() {
    // Store previous cooldowns to detect when a tank shoots
    int prevCooldown1 = g_gameState.tanks[0].cooldown;
    int prevCooldown2 = g_gameState.tanks[1].cooldown;
    
    g_gameState.HandleInput(g_keys);
    
    // Check if tanks shot bullets
    if (prevCooldown1 == 0 && g_gameState.tanks[0].cooldown > 0) {
        PlaySoundEffect(IDS_SHOOT_SOUND);
    }
    if (prevCooldown2 == 0 && g_gameState.tanks[1].cooldown > 0) {
        PlaySoundEffect(IDS_SHOOT_SOUND);
    }
}