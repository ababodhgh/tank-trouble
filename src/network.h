#ifndef NETWORK_H
#define NETWORK_H

#include <winsock2.h>
#include "game.h"

// Network packet types
enum PacketType {
    PACKET_INPUT = 1,
    PACKET_GAME_STATE,
    PACKET_BULLET,
    PACKET_DISCONNECT
};

// Input packet structure
struct InputPacket {
    PacketType type;
    bool keys[256];
    
    InputPacket() : type(PACKET_INPUT) {
        for (int i = 0; i < 256; i++) {
            keys[i] = false;
        }
    }
};

// Game state packet structure
struct GameStatePacket {
    PacketType type;
    Tank tanks[2];
    int bulletCount;
    Bullet bullets[50]; // Maximum 50 bullets
    
    GameStatePacket() : type(PACKET_GAME_STATE), bulletCount(0) {}
};

// Bullet creation packet structure
struct BulletPacket {
    PacketType type;
    Bullet bullet;
    
    BulletPacket() : type(PACKET_BULLET) {}
};

// Disconnect packet structure
struct DisconnectPacket {
    PacketType type;
    
    DisconnectPacket() : type(PACKET_DISCONNECT) {}
};

// Function prototypes
bool InitializeNetwork();
void CleanupNetwork();
bool StartHosting();
bool ConnectToHost(const char* ip);
void Disconnect();
void HandleDisconnection();
bool SendPacket(SOCKET socket, const void* data, int size);
bool ReceivePacket(SOCKET socket, void* data, int size);
bool SendInputPacket(SOCKET socket, const bool* keys);
bool SendGameStatePacket(SOCKET socket, const GameState& gameState);
bool SendBulletPacket(SOCKET socket, const Bullet& bullet);
bool ReceiveInputPacket(SOCKET socket, bool* keys);
bool ReceiveGameStatePacket(SOCKET socket, GameState& gameState);

#endif // NETWORK_H