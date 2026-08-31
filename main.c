#include "ennemi.h"
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h> // Added for audio
#include <stdlib.h>
#include <time.h>

#define MOVE_SPEED 14
#define ANIMATION_INTERVAL 150
#define HURT_ANIMATION_INTERVAL 70
#define DEATH_ANIMATION_INTERVAL 40

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    SDL_Surface *background = NULL;
    Mix_Chunk *hitSound = NULL; // Sound effect for weapon hit

    srand(time(NULL));

    // Initialize SDL video, TTF, IMG, and audio
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 || TTF_Init() < 0 || IMG_Init(IMG_INIT_PNG) < 0) {
        printf("Initialization Error: %s\n", SDL_GetError());
        return 1;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer init error: %s\n", Mix_GetError());
        SDL_Quit();
        TTF_Quit();
        IMG_Quit();
        return 1;
    }

    SDL_Surface *screen = SDL_SetVideoMode(800, 600, 32, SDL_SWSURFACE);
    if (screen == NULL) {
        printf("Error creating window: %s\n", SDL_GetError());
        SDL_Quit();
        TTF_Quit();
        IMG_Quit();
        Mix_CloseAudio();
        return 1;
    }

    // Load font
    TTF_Font *font = TTF_OpenFont("text.ttf", 24);
    if (!font) {
        printf("Error loading font: %s\n", TTF_GetError());
        SDL_Quit();
        TTF_Quit();
        IMG_Quit();
        Mix_CloseAudio();
        return 1;
    }

    // Load background
    background = IMG_Load("bg.png");
    if (background == NULL) {
        printf("Unable to load background: %s\n", IMG_GetError());
        SDL_Quit();
        TTF_Quit();
        IMG_Quit();
        Mix_CloseAudio();
        return 1;
    }

    // Load hit sound
    hitSound = Mix_LoadWAV("fight.wav");
    if (!hitSound) {
        printf("Error loading hit sound: %s\n", Mix_GetError());
        SDL_FreeSurface(background);
        TTF_CloseFont(font);
        SDL_Quit();
        TTF_Quit();
        IMG_Quit();
        Mix_CloseAudio();
        return 1;
    }

    // Initialize enemy
    Ennemi enemy;
    if (initEnnemi(&enemy, 700 - 80, 10, "Nmove.png", "Nhurt.png", "Nattack.png", "Ntrans.png") != 0) {
        printf("Failed to initialize enemy\n");
        SDL_FreeSurface(background);
        Mix_FreeChunk(hitSound);
        TTF_CloseFont(font);
        SDL_Quit();
        TTF_Quit();
        IMG_Quit();
        Mix_CloseAudio();
        return 1;
    }

    int running = 1;
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();
    int direction = 1;
    int shotsFired = 0;
    int enemyAlive = 1;
    int deathAnimationPlayed = 0;

    // Load weapon spritesheet
    SDL_Surface *weapon = IMG_Load("Attack.png");
    if (!weapon) {
        printf("Error loading weapon spritesheet: %s\n", IMG_GetError());
        SDL_FreeSurface(background);
        SDL_FreeSurface(enemy.moveSpritesheet);
        SDL_FreeSurface(enemy.hurtSpritesheet);
        SDL_FreeSurface(enemy.attackSpritesheet);
        SDL_FreeSurface(enemy.deathSpritesheet);
        Mix_FreeChunk(hitSound);
        TTF_CloseFont(font);
        SDL_Quit();
        TTF_Quit();
        IMG_Quit();
        Mix_CloseAudio();
        return 1;
    }

    SDL_Rect weaponPos = {10, 380 - weapon->h, 0, 0};
    int weaponFrame = 0;
    int weaponAnimSpeed = 50;
    Uint32 weaponLastTime = SDL_GetTicks();
    int weaponAnimState = 0;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            handleWeaponAction(&event, &enemy, weapon, &weaponPos, &weaponFrame, &weaponAnimState, 
                               &shotsFired, &enemyAlive, hitSound); // Pass hitSound
        }

        if (enemyAlive) {
            enemy.position.y += direction * MOVE_SPEED;
            if (enemy.position.y <= 0) {
                enemy.position.y = 0;
                direction = 1;
            } else if (enemy.position.y >= 600 - enemy.frame.h) {
                enemy.position.y = 600 - enemy.frame.h;
                direction = -1;
            }

            if (enemy.state == STATE_HURT) {
                if (SDL_GetTicks() - lastTime > HURT_ANIMATION_INTERVAL) {
                    updateEnnemiAnimation(&enemy);
                    lastTime = SDL_GetTicks();
                }
            } else {
                if (SDL_GetTicks() - lastTime > ANIMATION_INTERVAL) {
                    updateEnnemiAnimation(&enemy);
                    lastTime = SDL_GetTicks();
                }
            }

            if (weaponAnimState == 1) {
                if ((Uint32)(SDL_GetTicks() - weaponLastTime) > (Uint32)weaponAnimSpeed) {
                    weaponFrame = (weaponFrame + 1) % 7;
                    weaponLastTime = SDL_GetTicks();
                    if (weaponFrame == 6) {
                        weaponAnimState = 2;
                    }
                }
            } else if (weaponAnimState == 2) {
                if ((Uint32)(SDL_GetTicks() - weaponLastTime) > (Uint32)weaponAnimSpeed) {
                    weaponFrame = 0;
                    weaponLastTime = SDL_GetTicks();
                    weaponAnimState = 0;
                }
            }
        } else if (enemy.state == STATE_DEATH && !deathAnimationPlayed) {
            if (SDL_GetTicks() - lastTime > DEATH_ANIMATION_INTERVAL) {
                updateEnnemiAnimation(&enemy);
                lastTime = SDL_GetTicks();
                if (enemy.currentFrame >= enemy.deathFrames - 1) {
                    deathAnimationPlayed = 1;
                }
            }
        }

        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
        SDL_BlitSurface(background, NULL, screen, NULL);

        if (enemyAlive || (enemy.state == STATE_DEATH && !deathAnimationPlayed)) {
            renderEnnemi(screen, &enemy);
        }

        if (weapon != NULL) {
            SDL_Rect weaponFrameRect = {weaponFrame * (weapon->w / 7), 0, weapon->w / 7, weapon->h};
            SDL_BlitSurface(weapon, &weaponFrameRect, screen, &weaponPos);
        }

        updateEnemyHealth(screen, &enemy, font);

        SDL_Flip(screen);
        SDL_Delay(16);
    }

    // Free resources
    SDL_FreeSurface(background);
    SDL_FreeSurface(enemy.moveSpritesheet);
    SDL_FreeSurface(enemy.hurtSpritesheet);
    SDL_FreeSurface(enemy.attackSpritesheet);
    SDL_FreeSurface(enemy.deathSpritesheet);
    SDL_FreeSurface(weapon);
    Mix_FreeChunk(hitSound);
    TTF_CloseFont(font);
    SDL_Quit();
    TTF_Quit();
    IMG_Quit();
    Mix_CloseAudio();
    return 0;
}