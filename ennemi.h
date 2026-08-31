#ifndef ENNEMI_H
#define ENNEMI_H
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h> // Added for audio

// Enum for enemy states
typedef enum {
    STATE_MOVE,
    STATE_HURT,
    STATE_ATTACK,
    STATE_DEATH 
} EnemyState;

// Structure to represent an enemy
typedef struct {
    SDL_Surface *moveSpritesheet;
    SDL_Surface *hurtSpritesheet;
    SDL_Surface *attackSpritesheet;
    SDL_Surface *deathSpritesheet;
    SDL_Rect position;
    SDL_Rect frame;
    int health;
    EnemyState state;
    int currentFrame;
    int moveFrames;
    int hurtFrames;
    int attackFrames;
    int deathFrames;
    int moveFrameWidth;
    int hurtFrameWidth;
    int attackFrameWidth;
    int deathFrameWidth;     
    int deadFrameHeight;
    int frameHeight;
    Uint32 lastStateSwitch;
} Ennemi;

// Function declarations
int initEnnemi(Ennemi *e, int x, int y, const char *movePath, const char *hurtPath, const char *attackPath, const char *deathPath);
void renderEnnemi(SDL_Surface *screen, Ennemi *e);
void updateEnnemiAnimation(Ennemi *e);
void handleWeaponAction(SDL_Event *event, Ennemi *enemy, SDL_Surface *weapon, SDL_Rect *weaponPos, 
                        int *weaponFrame, int *weaponAnimState, int *shotsFired, int *enemyAlive, Mix_Chunk *hitSound);
void updateEnemyHealth(SDL_Surface *screen, Ennemi *enemy, TTF_Font *font);

#endif