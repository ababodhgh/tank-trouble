#include "network.h"
#include "main.h"
#include <iostream>
#include <cstring>

// External references to global variables in main.cpp
extern SOCKET g_listenSocket;
extern SOCKET g_clientSocket;
extern int g_port;
extern bool g_isHost;
extern GameState g_gameState;
extern bool g_isHost;
extern GameState g_gameState;

bool InitializeNetwork() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        return false;
    }
    return true;
}

void CleanupNetwork() {
    if (g_listenSocket != INVALID_SOCKET) {
        closesocket(g_listenSocket);
        g_listenSocket = INVALID_SOCKET;
    }
    
    if (g_clientSocket != INVALID_SOCKET) {
        closesocket(g_clientSocket);
        g_clientSocket = INVALID_SOCKET;
    }
    
    WSACleanup();
}

bool StartHosting() {
    // Close any existing sockets
    if (g_listenSocket != INVALID_SOCKET) {
        closesocket(g_listenSocket);
        g_listenSocket = INVALID_SOCKET;
    }
    
    if (g_clientSocket != INVALID_SOCKET) {
        closesocket(g_clientSocket);
        g_clientSocket = INVALID_SOCKET;
    }
    
    g_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_listenSocket == INVALID_SOCKET) {
        return false;
    }
    
    // Set socket options for reuse address
    int opt = 1;
    setsockopt(g_listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(g_port);
    
    if (bind(g_listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(g_listenSocket);
        g_listenSocket = INVALID_SOCKET;
        return false;
    }
    
    if (listen(g_listenSocket, 1) == SOCKET_ERROR) {
        closesocket(g_listenSocket);
        g_listenSocket = INVALID_SOCKET;
        return false;
    }
    
    // Set the socket to non-blocking mode for better UI responsiveness
    u_long mode = 1;
    ioctlsocket(g_listenSocket, FIONBIO, &mode);
    
    return true;
}

bool ConnectToHost(const char* ip) {
    // Close any existing sockets
    if (g_clientSocket != INVALID_SOCKET) {
        closesocket(g_clientSocket);
        g_clientSocket = INVALID_SOCKET;
    }
    
    g_clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_clientSocket == INVALID_SOCKET) {
        return false;
    }
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip);
    serverAddr.sin_port = htons(g_port);
    
    // Set the socket to non-blocking mode for timeout handling
    u_long mode = 1;
    ioctlsocket(g_clientSocket, FIONBIO, &mode);
    
    int result = connect(g_clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    
    // Handle non-blocking connect
    if (result == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) {
            closesocket(g_clientSocket);
            g_clientSocket = INVALID_SOCKET;
            return false;
        }
        
        // Wait for connection with timeout
        fd_set writeSet, errorSet;
        FD_ZERO(&writeSet);
        FD_ZERO(&errorSet);
        FD_SET(g_clientSocket, &writeSet);
        FD_SET(g_clientSocket, &errorSet);
        
        timeval timeout;
        timeout.tv_sec = 5;  // 5 second timeout
        timeout.tv_usec = 0;
        
        result = select(0, NULL, &writeSet, &errorSet, &timeout);
        if (result <= 0 || FD_ISSET(g_clientSocket, &errorSet)) {
            closesocket(g_clientSocket);
            g_clientSocket = INVALID_SOCKET;
            return false;
        }
    }
    
    // Set socket back to blocking mode
    mode = 0;
    ioctlsocket(g_clientSocket, FIONBIO, &mode);
    
    g_isHost = false;
    return true;
}

void Disconnect() {
    if (g_clientSocket != INVALID_SOCKET) {
        closesocket(g_clientSocket);
        g_clientSocket = INVALID_SOCKET;
    }
    
    if (g_listenSocket != INVALID_SOCKET) {
        closesocket(g_listenSocket);
        g_listenSocket = INVALID_SOCKET;
    }
}

bool SendPacket(SOCKET socket, const void* data, int size) {
    int sent = send(socket, (const char*)data, size, 0);
    return sent == size;
}

bool ReceivePacket(SOCKET socket, void* data, int size) {
    int received = recv(socket, (char*)data, size, 0);
    // Check for disconnection
    if (received == 0) {
        // Connection closed by peer
        return false;
    }
    if (received == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) {
            // Actual error occurred
            return false;
        }
        // Would block, no data available yet
        return false;
    }
    return received == size;
}

void HandleDisconnection() {
    Disconnect();
    // In a real implementation, we would show a message to the user
    // and return to the menu state
}

bool SendInputPacket(SOCKET socket, const bool* keys) {
    InputPacket packet;
    for (int i = 0; i < 256; i++) {
        packet.keys[i] = keys[i];
    }
    return SendPacket(socket, &packet, sizeof(packet));
}

bool SendGameStatePacket(SOCKET socket, const GameState& gameState) {
    GameStatePacket packet;
    
    // Copy tank data
    for (int i = 0; i < 2; i++) {
        packet.tanks[i] = gameState.tanks[i];
    }
    
    // Copy bullet data
    packet.bulletCount = (int)gameState.bullets.size();
    for (int i = 0; i < packet.bulletCount && i < 50; i++) {
        packet.bullets[i] = gameState.bullets[i];
    }
    
    return SendPacket(socket, &packet, sizeof(packet));
}

bool SendBulletPacket(SOCKET socket, const Bullet& bullet) {
    BulletPacket packet;
    packet.bullet = bullet;
    return SendPacket(socket, &packet, sizeof(packet));
}

bool ReceiveInputPacket(SOCKET socket, bool* keys) {
    InputPacket packet;
    if (ReceivePacket(socket, &packet, sizeof(packet))) {
        for (int i = 0; i < 256; i++) {
            keys[i] = packet.keys[i];
        }
        return true;
    }
    return false;
}

bool ReceiveGameStatePacket(SOCKET socket, GameState& gameState) {
    GameStatePacket packet;
    if (ReceivePacket(socket, &packet, sizeof(packet))) {
        // Copy tank data
        for (int i = 0; i < 2; i++) {
            gameState.tanks[i] = packet.tanks[i];
        }
        
        // Copy bullet data
        gameState.bullets.clear();
        for (int i = 0; i < packet.bulletCount && i < 50; i++) {
            gameState.bullets.push_back(packet.bullets[i]);
        }
        
        return true;
    }
    return false;
}