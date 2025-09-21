#ifndef GAME_H
#define GAME_H

#include <windows.h>
#include <vector>

// Constants
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int TANK_WIDTH = 32;
const int TANK_HEIGHT = 32;
const int BULLET_WIDTH = 8;
const int BULLET_HEIGHT = 8;
const int WALL_SIZE = 32;
const float TANK_SPEED = 2.0f;
const float BULLET_SPEED = 5.0f;
const int BULLET_LIFETIME = 100; // frames

// Forward declarations
struct Tank;
struct Bullet;
struct GameState;

// Tank structure
struct Tank {
    float x, y;           // Position
    float rotation;       // Rotation in radians
    float velocityX, velocityY; // Movement velocity
    bool alive;           // Is the tank alive?
    int cooldown;         // Shooting cooldown timer
    int playerID;         // Player identifier (1 or 2)
    
    // Constructor
    Tank(float posX = 0, float posY = 0, int id = 1) 
        : x(posX), y(posY), rotation(0), velocityX(0), velocityY(0), 
          alive(true), cooldown(0), playerID(id) {}
    
    // Update tank state
    void Update();
    
    // Move tank
    void Move(float dx, float dy);
    
    // Rotate tank
    void Rotate(float angle);
    
    // Shoot a bullet
    Bullet Shoot();
    
    // Check if tank just shot
    bool JustShot();
};

// Bullet structure
struct Bullet {
    float x, y;           // Position
    float velocityX, velocityY; // Movement velocity
    bool active;          // Is the bullet active?
    int lifetime;         // Remaining lifetime
    int ownerID;          // Which tank fired this bullet
    int bounceCount;      // Number of times the bullet has bounced
    
    // Constructor
    Bullet(float posX = 0, float posY = 0, float velX = 0, float velY = 0, int owner = 1)
        : x(posX), y(posY), velocityX(velX), velocityY(velY), 
          active(true), lifetime(BULLET_LIFETIME), ownerID(owner), bounceCount(0) {}
    
    // Update bullet state
    void Update();
    
    // Move bullet
    void Move();
};

// Simple particle structure for effects
struct Particle {
    float x, y;
    float velocityX, velocityY;
    int lifetime;
    bool active;
    
    Particle(float posX = 0, float posY = 0, float velX = 0, float velY = 0)
        : x(posX), y(posY), velocityX(velX), velocityY(velY), 
          lifetime(20), active(true) {}
    
    void Update() {
        if (active) {
            x += velocityX;
            y += velocityY;
            velocityX *= 0.95f; // Slow down over time
            velocityY *= 0.95f;
            lifetime--;
            if (lifetime <= 0) {
                active = false;
            }
        }
    }
};

// Maze cell structure
struct MazeCell {
    bool wall;            // Is this cell a wall?
    
    // Constructor
    MazeCell(bool isWall = false) : wall(isWall) {}
};

// Game state structure
struct GameState {
    static const int MAZE_WIDTH = 25;
    static const int MAZE_HEIGHT = 19;
    
    MazeCell maze[MAZE_WIDTH][MAZE_HEIGHT]; // Maze layout
    Tank tanks[2];                          // Two tanks (player 1 and 2)
    std::vector<Bullet> bullets;            // Active bullets
    std::vector<Particle> particles;        // Particle effects
    bool gameRunning;                       // Is the game currently running?
    int scores[2];                          // Scores for each player
    bool gameOver;                          // Is the game over?
    int winner;                             // Winner player ID (0 if tie, -1 if not finished)
    
    // Constructor
    GameState();
    
    // Initialize the game state
    void Initialize();
    
    // Update the game state
    void Update();
    
    // Render the game state
    void Render(HDC hdc);
    
    // Handle input
    void HandleInput(bool keys[256]);
    
    // Check collision between a bullet and walls
    bool CheckWallCollision(float x, float y);
    
    // Check collision between a bullet and a tank
    bool CheckTankCollision(float x, float y, int ignoreTank);
    
    // Reset the game
    void Reset();
    
    // Add explosion particles
    void AddExplosion(float x, float y);
};

#endif // GAME_H