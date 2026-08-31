#include "ennemi.h"
#include <stdio.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h> // Added for audio
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Initialize enemy with three spritesheets (PNG)
int initEnnemi(Ennemi *e, int x, int y, const char *movePath, const char *hurtPath, const char *attackPath, const char *deathPath) {
    e->moveSpritesheet = NULL;
    e->hurtSpritesheet = NULL;
    e->attackSpritesheet = NULL;
    e->deathSpritesheet = NULL;
    e->moveFrameWidth = 0;
    e->hurtFrameWidth = 0;
    e->attackFrameWidth = 0;
    e->deathFrameWidth = 0;
    e->frameHeight = 0;

    e->moveSpritesheet = IMG_Load(movePath);
    if (e->moveSpritesheet == NULL) {
        printf("Error loading move facilitatorsheet (%s): %s\n", movePath, IMG_GetError());
        return 1;
    }

    e->hurtSpritesheet = IMG_Load(hurtPath);
    if (e->hurtSpritesheet == NULL) {
        printf("Error loading hurt spritesheet (%s): %s\n", hurtPath, IMG_GetError());
        SDL_FreeSurface(e->moveSpritesheet);
        return 1;
    }

    e->attackSpritesheet = IMG_Load(attackPath);
    if (e->attackSpritesheet == NULL) {
        printf("Error loading attack spritesheet (%s): %s\n", attackPath, IMG_GetError());
        SDL_FreeSurface(e->moveSpritesheet);
        SDL_FreeSurface(e->hurtSpritesheet);
        return 1;
    }

    e->deathSpritesheet = IMG_Load(deathPath);
    if (e->deathSpritesheet == NULL) {
        printf("Error loading death spritesheet (%s): %s\n", deathPath, IMG_GetError());
        SDL_FreeSurface(e->moveSpritesheet);
        SDL_FreeSurface(e->hurtSpritesheet);
        SDL_FreeSurface(e->attackSpritesheet);
        return 1;
    }

    e->position.x = x;
    e->position.y = y;

    e->moveFrames = 5;
    e->hurtFrames = 6;
    e->attackFrames = 5;
    e->deathFrames = 11;
    e->currentFrame = 0;
    e->moveFrameWidth = e->moveSpritesheet->w / e->moveFrames;
    e->hurtFrameWidth = 183;
    e->attackFrameWidth = e->attackSpritesheet->w / e->attackFrames;
    e->deathFrameWidth = (int)round((double)e->deathSpritesheet->w / e->deathFrames);
    e->frameHeight = e->moveSpritesheet->h;
    e->frame.x = 0;
    e->frame.y = 0;
    e->frame.w = e->moveFrameWidth;
    e->frame.h = e->frameHeight;

    e->state = STATE_MOVE;
    e->lastStateSwitch = SDL_GetTicks();
    e->health = 4;

    return 0;
}

// Render enemy based on current state
void renderEnnemi(SDL_Surface *screen, Ennemi *e) {
    SDL_Surface *currentSpritesheet = NULL;
    int frameWidth = 0;

    if (e->health <= 0) {
        e->state = STATE_DEATH;
    }

    switch (e->state) {
        case STATE_MOVE:
            currentSpritesheet = e->moveSpritesheet;
            frameWidth = e->moveFrameWidth;
            break;
        case STATE_HURT:
            currentSpritesheet = e->hurtSpritesheet;
            frameWidth = e->hurtFrameWidth;
            printf("Rendering HURT: frame=%d, frameWidth=%d, frame.x=%d, spritesheet_w=%d\n", 
                   e->currentFrame, frameWidth, e->frame.x, e->hurtSpritesheet->w);
            break;
        case STATE_ATTACK:
            currentSpritesheet = e->attackSpritesheet;
            frameWidth = e->attackFrameWidth;
            break;
        case STATE_DEATH:
            currentSpritesheet = e->deathSpritesheet;
            frameWidth = e->deathFrameWidth;
            break;
    }

    if (currentSpritesheet != NULL) {
        e->frame.w = frameWidth;
        e->frame.x = e->currentFrame * frameWidth;
        e->frame.h = currentSpritesheet->h;
        SDL_BlitSurface(currentSpritesheet, &e->frame, screen, &e->position);
    } else {
        printf("Error: No spritesheet for state %d\n", e->state);
    }
}

// Update enemy animation and handle state switching
void updateEnnemiAnimation(Ennemi *e) {
    if (e->health <= 0) {
        e->state = STATE_DEATH;
    }

    int totalFrames = (e->state == STATE_MOVE) ? e->moveFrames :
                      (e->state == STATE_HURT) ? e->hurtFrames :
                      (e->state == STATE_ATTACK) ? e->attackFrames : e->deathFrames;
    int frameWidth = (e->state == STATE_MOVE) ? e->moveFrameWidth :
                     (e->state == STATE_HURT) ? e->hurtFrameWidth :
                     (e->state == STATE_ATTACK) ? e->attackFrameWidth : e->deathFrameWidth;

    if (e->state == STATE_DEATH) {
        if (e->currentFrame < e->deathFrames - 1) {
            e->currentFrame++;
        }
    } else {
        e->currentFrame = (e->currentFrame + 1) % totalFrames;
    }
    e->frame.w = frameWidth;
    e->frame.x = e->currentFrame * frameWidth;

    if (e->state == STATE_HURT && e->currentFrame == e->hurtFrames - 1 && SDL_GetTicks() - e->lastStateSwitch > 420) {
        e->state = (rand() % 2 == 0) ? STATE_MOVE : STATE_ATTACK;
        e->currentFrame = 0;
        e->lastStateSwitch = SDL_GetTicks();
        e->frame.w = (e->state == STATE_MOVE) ? e->moveFrameWidth : e->attackFrameWidth;
        e->frame.x = 0;
    }

    if (e->state != STATE_HURT && e->state != STATE_DEATH && SDL_GetTicks() - e->lastStateSwitch > 2000) {
        e->state = (rand() % 2 == 0) ? STATE_MOVE : STATE_ATTACK;
        e->currentFrame = 0;
        e->lastStateSwitch = SDL_GetTicks();
        e->frame.w = (e->state == STATE_MOVE) ? e->moveFrameWidth : e->attackFrameWidth;
        e->frame.x = 0;
    }
}

void handleWeaponAction(SDL_Event *event, Ennemi *enemy, SDL_Surface *weapon, SDL_Rect *weaponPos,
    int *weaponFrame, int *weaponAnimState, int *shotsFired, int *enemyAlive, Mix_Chunk *hitSound) {
    static int shotFiredThisPress = 0;
    int enemyCenterY = enemy->position.y + enemy->frame.h / 2;
    int weaponCenterY = weaponPos->y + weapon->h / 2;

    if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_SPACE) {
        if (!shotFiredThisPress) {
            shotFiredThisPress = 1;
            *weaponAnimState = 1;
            *weaponFrame = 0;

            SDL_Rect enemyHitbox = {
                enemy->position.x,
                enemy->position.y,
                enemy->frame.w,
                enemy->frame.h
            };
            SDL_Rect weaponHitbox = {
                weaponPos->x,
                weaponPos->y,
                800 - weaponPos->x, // Extend to screen width (800)
                weapon->h
            };

            SDL_bool intersect = (enemyHitbox.x < weaponHitbox.x + weaponHitbox.w &&
                        enemyHitbox.x + enemyHitbox.w > weaponHitbox.x &&
                        enemyHitbox.y < weaponHitbox.y + weaponHitbox.h &&
                        enemyHitbox.y + enemyHitbox.h > weaponHitbox.y);

            if (*enemyAlive && abs(enemyCenterY - weaponCenterY) < 100) { // Changed from 50 to 100
                printf("Hit registered! Health: %d\n", enemy->health - 1);
                (*shotsFired)++;
                enemy->health--;

                // **PLAY HIT SOUND HERE**
                Mix_PlayChannel(-1, hitSound, 0);

                if (enemy->state != STATE_HURT) {
                    enemy->state = STATE_HURT;
                    enemy->currentFrame = 0;
                    enemy->frame.x = 0;
                }
                if (enemy->health <= 0) {
                    *enemyAlive = 0;
                    enemy->state = STATE_DEATH;
                    enemy->currentFrame = 0;
                    enemy->frame.x = 0;
                }
            } else {
                printf("No hit: y-diff=%d\n", abs(enemyCenterY - weaponCenterY));
            }
        }
    }

    if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_SPACE) {
        shotFiredThisPress = 0;
    }
}


void updateEnemyHealth(SDL_Surface *screen, Ennemi *enemy, TTF_Font *font) {
    char healthText[50];
    const char *healthStatus;

    if (enemy->health > 3) {
        healthStatus = "Healthy";
    } else if (enemy->health == 3) {
        healthStatus = "Lightly Wounded";
    } else if (enemy->health == 2) {
        healthStatus = "Wounded";
    } else if (enemy->health == 1) {
        healthStatus = "Gravely Wounded";
    } else {
        healthStatus = "Dead";
    }

    snprintf(healthText, sizeof(healthText), "Health: %s", healthStatus);

    SDL_Color white = {255, 255, 255, 0};
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, healthText, white);
    if (textSurface == NULL) {
        printf("Error rendering text: %s\n", TTF_GetError());
        return;
    }

    SDL_Rect textRect = {screen->w - textSurface->w - 10, 10, 0, 0};
    SDL_FillRect(screen, &textRect, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_BlitSurface(textSurface, NULL, screen, &textRect);
    SDL_FreeSurface(textSurface);
}