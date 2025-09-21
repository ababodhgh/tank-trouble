#include "game.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#define M_PI 3.14159265358979323846

// Tank methods
void Tank::Update() {
    // Update position based on velocity
    x += velocityX;
    y += velocityY;
    
    // Apply friction
    velocityX *= 0.9f;
    velocityY *= 0.9f;
    
    // Update cooldown
    if (cooldown > 0) {
        cooldown--;
    }
    
    // Boundary checking
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x > WINDOW_WIDTH - TANK_WIDTH) x = WINDOW_WIDTH - TANK_WIDTH;
    if (y > WINDOW_HEIGHT - TANK_HEIGHT) y = WINDOW_HEIGHT - TANK_HEIGHT;
}

void Tank::Move(float dx, float dy) {
    velocityX += dx * TANK_SPEED;
    velocityY += dy * TANK_SPEED;
    
    // Add some visual feedback - create small particles when tank moves
    if (fabs(dx) > 0.1f || fabs(dy) > 0.1f) {
        // This would create particles, but we'll handle that in the main game loop
    }
}

void Tank::Rotate(float angle) {
    rotation += angle;
}

Bullet Tank::Shoot() {
    if (cooldown <= 0) {
        cooldown = 20; // Reset cooldown
        
        // Calculate bullet starting position (center of tank)
        float bulletX = x + TANK_WIDTH / 2.0f;
        float bulletY = y + TANK_HEIGHT / 2.0f;
        
        // Calculate bullet velocity based on tank rotation
        float velX = cos(rotation) * BULLET_SPEED;
        float velY = sin(rotation) * BULLET_SPEED;
        
        return Bullet(bulletX, bulletY, velX, velY, playerID);
    }
    return Bullet(0, 0, 0, 0, 0); // Return inactive bullet
}

bool Tank::JustShot() {
    return (cooldown == 20); // Just shot if cooldown was just set
}

// Bullet methods
void Bullet::Update() {
    if (active) {
        Move();
        lifetime--;
        if (lifetime <= 0) {
            active = false;
        }
    }
}

void Bullet::Move() {
    // Store original position
    float oldX = x;
    float oldY = y;
    
    // Update position
    x += velocityX;
    y += velocityY;
    
    // Boundary checking - bounce off edges
    if (x < 0) {
        x = 0;
        velocityX = -velocityX * 0.8f; // Reverse X velocity with some energy loss
        bounceCount++;
    }
    if (y < 0) {
        y = 0;
        velocityY = -velocityY * 0.8f; // Reverse Y velocity with some energy loss
        bounceCount++;
    }
    if (x > WINDOW_WIDTH - BULLET_WIDTH) {
        x = WINDOW_WIDTH - BULLET_WIDTH;
        velocityX = -velocityX * 0.8f; // Reverse X velocity with some energy loss
        bounceCount++;
    }
    if (y > WINDOW_HEIGHT - BULLET_HEIGHT) {
        y = WINDOW_HEIGHT - BULLET_HEIGHT;
        velocityY = -velocityY * 0.8f; // Reverse Y velocity with some energy loss
        bounceCount++;
    }
    
    // Check if bullet has exceeded bounce limit
    if (bounceCount >= 4) {
        active = false;
    }
}

// GameState methods
GameState::GameState() : gameRunning(true), gameOver(false), winner(-1) {
    scores[0] = 0;
    scores[1] = 0;
    Initialize();
}

void GameState::Initialize() {
    // Initialize tanks
    tanks[0] = Tank(100, 100, 1); // Player 1
    tanks[1] = Tank(600, 400, 2); // Player 2 (for testing)
    
    // Clear bullets
    bullets.clear();
    
    // Initialize scores
    scores[0] = 0;
    scores[1] = 0;
    
    // Reset game over state
    gameOver = false;
    winner = -1;
    
    // Initialize a random maze
    srand((unsigned int)time(nullptr)); // Seed random number generator
    
    for (int x = 0; x < MAZE_WIDTH; x++) {
        for (int y = 0; y < MAZE_HEIGHT; y++) {
            // Create borders
            if (x == 0 || y == 0 || x == MAZE_WIDTH - 1 || y == MAZE_HEIGHT - 1) {
                maze[x][y] = MazeCell(true);
            }
            // Create random internal walls (30% chance)
            else if (rand() % 100 < 30) {
                maze[x][y] = MazeCell(true);
            }
            else {
                maze[x][y] = MazeCell(false);
            }
        }
    }
}

void GameState::Update() {
    if (gameOver) {
        return;
    }
    
    // Update tanks
    for (int i = 0; i < 2; i++) {
        if (tanks[i].alive) {
            tanks[i].Update();
        }
    }
    
    // Update particles
    for (int i = particles.size() - 1; i >= 0; i--) {
        particles[i].Update();
        if (!particles[i].active) {
            particles.erase(particles.begin() + i);
        }
    }
    
    // Update bullets and check for collisions
    for (int i = bullets.size() - 1; i >= 0; i--) {
        bullets[i].Update();
        
        if (!bullets[i].active) {
            bullets.erase(bullets.begin() + i);
            continue;
        }
        
        // Check collision with walls
        if (CheckWallCollision(bullets[i].x, bullets[i].y)) {
            // Instead of removing the bullet, make it bounce
            // Calculate which side of the wall was hit
            int gridX = (int)(bullets[i].x / WALL_SIZE);
            int gridY = (int)(bullets[i].y / WALL_SIZE);
            
            // Determine bounce direction based on bullet approach
            float centerX = gridX * WALL_SIZE + WALL_SIZE / 2.0f;
            float centerY = gridY * WALL_SIZE + WALL_SIZE / 2.0f;
            
            // Simple bounce logic - reverse velocity component based on approach direction
            if (bullets[i].velocityX > 0 && bullets[i].x < centerX) {
                // Hit left side of wall
                bullets[i].velocityX = -bullets[i].velocityX * 0.8f;
                bullets[i].bounceCount++;
            } else if (bullets[i].velocityX < 0 && bullets[i].x > centerX) {
                // Hit right side of wall
                bullets[i].velocityX = -bullets[i].velocityX * 0.8f;
                bullets[i].bounceCount++;
            } else if (bullets[i].velocityY > 0 && bullets[i].y < centerY) {
                // Hit top side of wall
                bullets[i].velocityY = -bullets[i].velocityY * 0.8f;
                bullets[i].bounceCount++;
            } else if (bullets[i].velocityY < 0 && bullets[i].y > centerY) {
                // Hit bottom side of wall
                bullets[i].velocityY = -bullets[i].velocityY * 0.8f;
                bullets[i].bounceCount++;
            }
            
            // Move bullet slightly away from wall to prevent sticking
            if (bullets[i].velocityX > 0) {
                bullets[i].x += 2.0f;
            } else if (bullets[i].velocityX < 0) {
                bullets[i].x -= 2.0f;
            }
            
            if (bullets[i].velocityY > 0) {
                bullets[i].y += 2.0f;
            } else if (bullets[i].velocityY < 0) {
                bullets[i].y -= 2.0f;
            }
            
            // Add spark particles for wall hit
            AddExplosion(bullets[i].x, bullets[i].y);
            
            // Check if bullet has exceeded bounce limit
            if (bullets[i].bounceCount >= 4) {
                AddExplosion(bullets[i].x, bullets[i].y); // Add explosion when bullet expires
                bullets[i].active = false;
                bullets.erase(bullets.begin() + i);
                continue;
            }
        }
        
        // Check collision with tanks
        for (int j = 0; j < 2; j++) {
            if (tanks[j].alive && j != (bullets[i].ownerID - 1)) {
                if (bullets[i].x >= tanks[j].x && bullets[i].x <= tanks[j].x + TANK_WIDTH &&
                    bullets[i].y >= tanks[j].y && bullets[i].y <= tanks[j].y + TANK_HEIGHT) {
                    // Hit a tank
                    tanks[j].alive = false;
                    bullets[i].active = false;
                    
                    // Add explosion effect
                    AddExplosion(tanks[j].x + TANK_WIDTH/2, tanks[j].y + TANK_HEIGHT/2);
                    
                    // Remove bullet
                    bullets.erase(bullets.begin() + i);
                    
                    // Update score
                    scores[bullets[i].ownerID - 1]++;
                    
                    // Check for game over
                    int aliveCount = 0;
                    int lastAlive = -1;
                    for (int k = 0; k < 2; k++) {
                        if (tanks[k].alive) {
                            aliveCount++;
                            lastAlive = k;
                        }
                    }
                    
                    if (aliveCount <= 1) {
                        gameOver = true;
                        winner = (lastAlive >= 0) ? (lastAlive + 1) : 0; // 0 means tie
                    }
                    
                    break;
                }
            }
        }
    }
    
    // Respawn tanks if both are dead (for continuous gameplay)
    bool allDead = true;
    for (int i = 0; i < 2; i++) {
        if (tanks[i].alive) {
            allDead = false;
            break;
        }
    }
    
    if (allDead) {
        // Respawn both tanks at random positions
        tanks[0] = Tank(100, 100, 1);
        tanks[1] = Tank(600, 400, 2);
    }
}

void GameState::HandleInput(bool keys[256]) {
    // Player 1 controls (Arrow keys)
    if (keys[VK_UP]) {
        tanks[0].Move(0, -1);
    }
    if (keys[VK_DOWN]) {
        tanks[0].Move(0, 1);
    }
    if (keys[VK_LEFT]) {
        tanks[0].Move(-1, 0);
    }
    if (keys[VK_RIGHT]) {
        tanks[0].Move(1, 0);
    }
    if (keys[VK_SPACE]) {
        Bullet bullet = tanks[0].Shoot();
        if (bullet.active) {
            bullets.push_back(bullet);
            // Would play sound effect here if we had access to PlaySoundEffect
        }
    }
    
    // Player 2 controls (WASD)
    if (keys['W']) {
        tanks[1].Move(0, -1);
    }
    if (keys['S']) {
        tanks[1].Move(0, 1);
    }
    if (keys['A']) {
        tanks[1].Move(-1, 0);
    }
    if (keys['D']) {
        tanks[1].Move(1, 0);
    }
    if (keys['E']) {
        Bullet bullet = tanks[1].Shoot();
        if (bullet.active) {
            bullets.push_back(bullet);
            // Would play sound effect here if we had access to PlaySoundEffect
        }
    }
}

bool GameState::CheckWallCollision(float x, float y) {
    int gridX = (int)(x / WALL_SIZE);
    int gridY = (int)(y / WALL_SIZE);
    
    if (gridX >= 0 && gridX < MAZE_WIDTH && gridY >= 0 && gridY < MAZE_HEIGHT) {
        return maze[gridX][gridY].wall;
    }
    return true; // Treat out of bounds as walls
}

bool GameState::CheckTankCollision(float x, float y, int ignoreTank) {
    for (int i = 0; i < 2; i++) {
        if (i != ignoreTank && tanks[i].alive) {
            if (x >= tanks[i].x && x <= tanks[i].x + TANK_WIDTH &&
                y >= tanks[i].y && y <= tanks[i].y + TANK_HEIGHT) {
                return true;
            }
        }
    }
    return false;
}

void GameState::Reset() {
    Initialize();
    scores[0] = 0;
    scores[1] = 0;
    gameOver = false;
    winner = -1;
    particles.clear(); // Clear particles on reset
}

void GameState::AddExplosion(float x, float y) {
    // Create several particles for explosion effect
    for (int i = 0; i < 8; i++) {
        float angle = (float)(i * M_PI * 2 / 8);
        float speed = 2.0f + (rand() % 3);
        float velX = cos(angle) * speed;
        float velY = sin(angle) * speed;
        particles.push_back(Particle(x, y, velX, velY));
    }
}

void GameState::Render(HDC hdc) {
    // This method will be implemented in main.cpp where we have access to the bitmaps
    // For now, we'll just leave it as a placeholder
}