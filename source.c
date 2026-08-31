#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_image.h>
#include "header.h"

Mix_Chunk *walkSound = NULL;
Mix_Chunk *fightSound = NULL;
Mix_Music *backgroundMusic = NULL;
Uint32 startTime;

SDL_Surface* initSDL() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_Surface *screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
    SDL_WM_SetCaption("Ennemi Suiveur", NULL);
    return screen;
}

void initAudio() {
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    walkSound = Mix_LoadWAV("walk.wav");
    fightSound = Mix_LoadWAV("fight.wav");
}

void initCharacter(Character *c) {
    const char* states[] = {"Idle", "Walk", "Jump", "Attack", "Hurt"};
    char path[128];
    
    for (int state = 0; state < 5; state++) {
        c->sprites[state] = NULL;
    }

    for (int state = 0; state < 5; state++) {
        snprintf(path, sizeof(path), "/home/user/Desktop/BOU/assets/%s.png", states[state]);
        c->sprites[state] = IMG_Load(path);
        if (!c->sprites[state]) {
            printf("Failed to load %s: %s\n", path, SDL_GetError());
        } 
    }

    c->pos.x = 100;
    c->pos.y = 100;
    c->pos.w = 64;
    c->pos.h = 64;
    c->frame = 0;
    c->state = 0;
    c->facing = 1;
    c->lives = 3;
    c->score = 0;
    c->clothing = 0;
}


SDL_Surface* flip_surface_horizontally(SDL_Surface* src) {
    SDL_Surface* flipped = SDL_CreateRGBSurface(src->flags, src->w, src->h, src->format->BitsPerPixel,
                                                src->format->Rmask, src->format->Gmask, src->format->Bmask, src->format->Amask);
    if (!flipped) {
        printf("Failed to create flipped surface: %s\n", SDL_GetError());
        return NULL;
    }

    Uint32* src_pixels = (Uint32*)src->pixels;
    Uint32* dst_pixels = (Uint32*)flipped->pixels;

    for (int y = 0; y < src->h; y++) {
        for (int x = 0; x < src->w; x++) {
            dst_pixels[y * flipped->w + (flipped->w - 1 - x)] = src_pixels[y * src->w + x];
        }
    }

    return flipped;
}

void update_character(Character* c) {
    static Uint32 last = 0;
    Uint32 now = SDL_GetTicks();
    static int attackFrameCount = 0; // Track frames in Attack state

    if (now - last > 100) { // 10 FPS
        last = now;
        c->frame = (c->frame + 1) % 7;

        if (c->state == 3) { // Attack state
            attackFrameCount++;
            if (attackFrameCount >= 7) { // Animation complete (7 frames)
                c->state = 0; // Reset to Idle
                attackFrameCount = 0; // Reset frame counter
            }
        } else {
            attackFrameCount = 0; // Reset when not in Attack state
        }
    }

    // Handle jump state
    if (c->state == 2) { // Jump
        static int jump_height = 0;
        if (jump_height < 50) {
            c->pos.y -= 5;
            jump_height += 5;
        } else {
            c->pos.y += 5;
            if (c->pos.y >= 100) {
                c->pos.y = 100;
                c->state = 0;
                jump_height = 0;
            }
        }
    } else if (c->state == 4) { // Hurt
        c->state = 0; // Back to idle after hurt
    }

    // Boundary checks
    if (c->pos.x < 0) c->pos.x = 0;
    if (c->pos.x + c->pos.w > SCREEN_W) c->pos.x = SCREEN_W - c->pos.w;
    if (c->pos.y < 0) c->pos.y = 0;
    if (c->pos.y + c->pos.h > SCREEN_H) c->pos.y = SCREEN_H - c->pos.h;
}

void showYouWon(SDL_Surface *screen, TTF_Font *font) {
    const char *winText = "YOU WON";
    SDL_Color green = {0, 255, 0, 0};
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, winText, green);
    if (textSurface == NULL) {
        printf("Erreur lors du rendu du texte You Won: %s\n", TTF_GetError());
        return;
    }
    SDL_Rect textRect = {(screen->w - textSurface->w) / 2, (screen->h - textSurface->h) / 2, 0, 0};
    SDL_BlitSurface(textSurface, NULL, screen, &textRect);
    SDL_Flip(screen);
    SDL_FreeSurface(textSurface);
    SDL_Event e;
    int quit = 0;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_KEYDOWN || e.type == SDL_QUIT) {
                quit = 1;
            }
        }
        SDL_Delay(100);
    }
}

void updateCharacterMovement(Character* c, float deltaTime, bool moveUp, bool moveDown, bool moveLeft, bool moveRight, bool isRunning) {
    SDL_Rect newPos = c->pos;
    float speed = isRunning ? 600.0f * deltaTime : 300.0f * deltaTime;

    float dx = 0.0f, dy = 0.0f;
    if (moveRight) dx += speed;
    if (moveLeft) dx -= speed;
    if (moveUp) dy -= speed;
    if (moveDown) dy += speed;

    newPos.x += dx;
    newPos.y += dy;

    // Plus de collision avec rock
    c->pos = newPos;

    // Garde le personnage dans les limites de l’écran
    if (c->pos.x < 0) c->pos.x = 0;
    if (c->pos.x + c->pos.w > SCREEN_W) c->pos.x = SCREEN_W - c->pos.w;
    if (c->pos.y < 0) c->pos.y = 0;
    if (c->pos.y + c->pos.h > SCREEN_H) c->pos.y = SCREEN_H - c->pos.h;
}

void deplacerCharacter(Character* c, SDL_Event event, Ennemi *e, bool *moveUp, bool *moveDown, bool *moveLeft, bool *moveRight, bool *isRunning) {
    static Uint32 lastAttackTime = 0;
    const Uint32 attackCooldown = 500; // Reduced to 0.5 seconds for testing

    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_RIGHT) {
            *moveRight = true;
            c->facing = 1;
        } else if (event.key.keysym.sym == SDLK_LEFT) {
            *moveLeft = true;
            c->facing = 0;
        } else if (event.key.keysym.sym == SDLK_UP) {
            *moveUp = true;
        } else if (event.key.keysym.sym == SDLK_DOWN) {
            *moveDown = true;
        } else if (event.key.keysym.sym == SDLK_j) {
            c->state = 2; // Jump
        } else if (event.key.keysym.sym == SDLK_SPACE && SDL_GetTicks() - lastAttackTime > attackCooldown) {
            c->state = 3; // Attack
            c->score += 10;
            float dx = c->pos.x + c->pos.w / 2 - (e->pos.x + e->pos.w / 2);
            float dy = c->pos.y + c->pos.h / 2 - (e->pos.y + e->pos.h / 2);
            float distance = sqrt(dx * dx + dy * dy);
            printf("Attack attempted: distance=%.2f, frame=%d, collision=%d\n", distance, c->frame, checkCollision(e, c));
            if (distance < 100.0f) { // Increased range to 100.0f
                e->health--;
                printf("Enemy hit: health=%d\n", e->health);
                if (fightSound) Mix_PlayChannel(-1, fightSound, 0); // Play sound on hit
            } else {
                printf("Attack missed: distance too large (%.2f > 100.0f)\n", distance);
            }
            lastAttackTime = SDL_GetTicks();
        } else if (event.key.keysym.sym == SDLK_LSHIFT) {
            *isRunning = true;
        }
    } else if (event.type == SDL_KEYUP) {
        if (event.key.keysym.sym == SDLK_RIGHT) {
            *moveRight = false;
        } else if (event.key.keysym.sym == SDLK_LEFT) {
            *moveLeft = false;
        } else if (event.key.keysym.sym == SDLK_UP) {
            *moveUp = false;
        } else if (event.key.keysym.sym == SDLK_DOWN) {
            *moveDown = false;
        } else if (event.key.keysym.sym == SDLK_LSHIFT) {
            *isRunning = false;
            if (c->state == 5) c->state = 1;
        }
    }

    if (*moveUp || *moveDown || *moveLeft || *moveRight) {
        if (c->state != 3 && c->state != 2 && c->state != 4) { // Preserve Attack, Jump, Hurt
            c->state = *isRunning ? 5 : 1;
        }
    } else if (c->state != 2 && c->state != 3 && c->state != 4) {
        c->state = 0; // Only revert to Idle if not attacking, jumping, or hurt
    }
}

void updateEnemyHealth(SDL_Surface *screen, Ennemi *enemy, TTF_Font *font) {
    char healthText[50];
    const char *healthStatus;
    if (enemy->health > 2) {
        healthStatus = "Alive";
    } else if (enemy->health == 2) {
        healthStatus = "Wounded";
    } else if (enemy->health == 1) {
        healthStatus = "Gravely wounded";
    } else {
        healthStatus = "Dead";
    }
    snprintf(healthText, sizeof(healthText), "Enemy Health: %s", healthStatus);
    SDL_Color white = {255, 255, 255, 0};
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, healthText, white);
    if (textSurface == NULL) {
        printf("Error rendering text: %s\n", TTF_GetError());
        return;
    }
    SDL_Rect textRect = {screen->w - textSurface->w - 10, 10, 0, 0};
    SDL_BlitSurface(textSurface, NULL, screen, &textRect);
    SDL_FreeSurface(textSurface);
}

void updateCharacterHealth(SDL_Surface *screen, Character *character, TTF_Font *font) {
    char healthText[50];
    const char *healthStatus;
    if (character->lives > 2) {
        healthStatus = "Alive";
    } else if (character->lives == 2) {
        healthStatus = "Wounded";
    } else if (character->lives == 1) {
        healthStatus = "Gravely wounded";
    } else {
        healthStatus = "Dead";
    }
    snprintf(healthText, sizeof(healthText), "Player Health: %s (Score: %d)", healthStatus, character->score);
    SDL_Color white = {255, 255, 255, 0};
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, healthText, white);
    if (textSurface == NULL) {
        printf("Erreur lors du rendu du texte: %s\n", TTF_GetError());
        return;
    }
    SDL_Rect textRect = {10, 10, 0, 0};
    SDL_BlitSurface(textSurface, NULL, screen, &textRect);
    SDL_FreeSurface(textSurface);
}

void jouerSon(Animation anim) {
    static Animation lastAnim = -1;
    static int currentChannel = -1;

    if (anim != lastAnim) {
        if (currentChannel != -1) {
            Mix_HaltChannel(currentChannel);
            currentChannel = -1;
        }

        if (anim == WALK && walkSound) {
            currentChannel = Mix_PlayChannel(-1, walkSound, -1);
        } else if (anim == FIGHT && fightSound) {
            currentChannel = Mix_PlayChannel(-1, fightSound, -1);
        } else if (anim == IDLE && currentChannel != -1) {
            Mix_HaltChannel(currentChannel);
            currentChannel = -1;
        }

        lastAnim = anim;
    }
}

void showGameOver(SDL_Surface *screen, TTF_Font *font) {
    const char *gameOverText = "GAME OVER";
    SDL_Color red = {255, 0, 0, 0};
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, gameOverText, red);
    if (textSurface == NULL) {
        printf("Erreur lors du rendu du texte Game Over: %s\n", TTF_GetError());
        return;
    }
    SDL_Rect textRect = {(screen->w - textSurface->w) / 2, (screen->h - textSurface->h) / 2, 0, 0};
    SDL_BlitSurface(textSurface, NULL, screen, &textRect);
    SDL_Flip(screen);
    SDL_FreeSurface(textSurface);
    SDL_Event e;
    int quit = 0;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_KEYDOWN || e.type == SDL_QUIT) {
                quit = 1;
            }
        }
        SDL_Delay(100);
    }
}

void initEnnemi(Ennemi *e) {
    e->sprites[IDLE][RIGHT]  = IMG_Load("enemy_idle_left.png");
    e->sprites[IDLE][LEFT]   = IMG_Load("enemy_idle_right.png");
    e->sprites[WALK][RIGHT]  = IMG_Load("enemy_walk_left.png");
    e->sprites[WALK][LEFT]   = IMG_Load("enemy_walk_right.png");
    e->sprites[FIGHT][RIGHT] = IMG_Load("enemy_fight_left.png");
    e->sprites[FIGHT][LEFT]  = IMG_Load("enemy_fight_right.png");
    for (int anim = 0; anim < 3; anim++) {
        for (int dir = 0; dir < 2; dir++) {
            if (e->sprites[anim][dir] != NULL) {
                SDL_SetColorKey(e->sprites[anim][dir], SDL_SRCCOLORKEY, SDL_MapRGB(e->sprites[anim][dir]->format, 0, 0, 0));
            } else {
                printf("Failed to load sprite: anim=%d, dir=%d\n", anim, dir);
            }
        }
    }
    e->health = 3;
    e->pos.x = SCREEN_W - 164;
    e->pos.y = 300;
    e->pos.w = 64;
    e->pos.h = 64;
    e->frame = 0;
    e->anim = WALK;
    e->dir = LEFT;
    e->speed = 2.5f;
    printf("Enemy initialized: pos(%d,%d,%d,%d)\n", e->pos.x, e->pos.y, e->pos.w, e->pos.h);
}

void renderEnnemi(Ennemi *e, SDL_Surface *screen) {
    SDL_Surface *spriteToRender = e->sprites[e->anim][e->dir];
    SDL_BlitSurface(spriteToRender, NULL, screen, &e->pos);
}

bool checkCollision(Ennemi *e, Character *c) {
    float marginX = 0.1f; // 10% margin
    float marginY = 0.1f; // 10% margin
    float eLeft = e->pos.x + (e->pos.w * marginX);
    float eRight = e->pos.x + e->pos.w - (e->pos.w * marginX);
    float eTop = e->pos.y + (e->pos.h * marginY);
    float eBottom = e->pos.y + e->pos.h - (e->pos.h * marginY);
    float cLeft = c->pos.x + (c->pos.w * marginX);
    float cRight = c->pos.x + c->pos.w - (c->pos.w * marginX);
    float cTop = c->pos.y + (c->pos.h * marginY);
    float cBottom = c->pos.y + c->pos.h - (c->pos.h * marginY);
    bool collision = (eRight >= cLeft && cRight >= eLeft && 
                      eBottom >= cTop && cBottom >= eTop);
    // Debugging output
    if (collision) {
        float dx = c->pos.x + c->pos.w / 2 - (e->pos.x + e->pos.w / 2);
        float dy = c->pos.y + c->pos.h / 2 - (e->pos.y + e->pos.h / 2);
        float distance = sqrt(dx * dx + dy * dy);
        printf("Collision detected: enemy_pos=(%d,%d), char_pos=(%d,%d), distance=%.2f\n",
               e->pos.x, e->pos.y, c->pos.x, c->pos.y, distance);
    }
    
    return collision;
}

void afficher(SDL_Surface *screen, Ennemi *e, Character *c, TTF_Font *font, SDL_Surface *background) {
    SDL_BlitSurface(background, NULL, screen, NULL);

    const int frameCounts[3] = {6,5,5};
    int currentFrameCount = frameCounts[e->anim];
    SDL_Surface *sprite = e->sprites[e->anim][e->dir];
    if (!sprite) {
        printf("Enemy sprite is NULL in afficher: anim=%d, dir=%d\n", e->anim, e->dir);
        return;
    }
    int frameWidth = sprite->w / currentFrameCount;
    SDL_Rect src = {(e->frame % currentFrameCount) * frameWidth, 0, frameWidth, sprite->h};
    // Preserve e->pos dimensions
    SDL_Rect destPos = {e->pos.x, e->pos.y, 64, 64};
    printf("Enemy rendering: pos(%d,%d,%d,%d), sprite(%dx%d), frameWidth=%d\n", 
           destPos.x, destPos.y, destPos.w, destPos.h, sprite->w, sprite->h, frameWidth);
    SDL_BlitSurface(sprite, &src, screen, &destPos);
    // Ensure e->pos is not corrupted
    e->pos.w = 64;
    e->pos.h = 64;
    renderCharacter(c,screen);
    updateEnemyHealth(screen, e, font);
    updateCharacterHealth(screen, c, font);
}

void deplacerEnnemiVersCharacter(Ennemi *e, Character *c, float deltaTime) {
    static int randomDirection = 0;
    static Uint32 lastDirectionChange = 0;
    const Uint32 randomMoveDuration = 5000;
    Uint32 currentTime = SDL_GetTicks();
    Uint32 elapsedTime = currentTime - startTime;

    float moveSpeed = e->speed * deltaTime * 60.0f;
    float dx = 0.0f, dy = 0.0f;

    // Mouvement aléatoire pendant les premières secondes
    if (elapsedTime < randomMoveDuration) {
        if (currentTime - lastDirectionChange > 1000) {
            randomDirection = rand() % 4;
            lastDirectionChange = currentTime;
        }
        switch (randomDirection) {
            case 0: dy -= moveSpeed; e->dir = LEFT; break;
            case 1: dy += moveSpeed; e->dir = LEFT; break;
            case 2: dx -= moveSpeed; e->dir = LEFT; break;
            case 3: dx += moveSpeed; e->dir = RIGHT; break;
        }
    } else {
        dx = c->pos.x + c->pos.w / 2 - (e->pos.x + e->pos.w / 2);
        dy = c->pos.y + c->pos.h / 2 - (e->pos.y + e->pos.h / 2);
        float distance = sqrt(dx * dx + dy * dy);
        const float MIN_ATTACK_DISTANCE = 50.0f;

        if (distance > MIN_ATTACK_DISTANCE) {
            dx /= distance;
            dy /= distance;
            dx *= moveSpeed;
            dy *= moveSpeed;
            e->dir = dx > 0 ? RIGHT : LEFT;
        } else {
            dx = dy = 0.0f;
        }
    }

    // Appliquer le déplacement
    e->pos.x += dx;
    e->pos.y += dy;

    // Vérification des bords de l'écran
    if (e->pos.x < 0) e->pos.x = 0;
    if (e->pos.x + e->pos.w > SCREEN_W) e->pos.x = SCREEN_W - e->pos.w;
    if (e->pos.y < 0) e->pos.y = 0;
    if (e->pos.y + e->pos.h > SCREEN_H) e->pos.y = SCREEN_H - e->pos.h;

    // Assurer les dimensions
    e->pos.w = 64;
    e->pos.h = 64;
}

void renderCharacter(Character *c, SDL_Surface *screen) {
    // Check if the sprite for the current state exists
    SDL_Surface *sprite = c->sprites[c->state];
    if (!sprite) {
        printf("Character sprite is NULL for state=%d\n", c->state);
        return;
    }

    // Define frame count for all states (Idle, Walk, Jump, Attack, Hurt)
    const int frameCount = 7; // Each sprite sheet has 7 frames
    int frameWidth = sprite->w / frameCount;
    int frameHeight = sprite->h; // Assume single row (horizontal sprite sheet)

    // Debugging output
    printf("Rendering character: state=%d, frame=%d/%d, sprite=%dx%d, frameWidth=%d, frameHeight=%d\n",
           c->state, c->frame, frameCount, sprite->w, sprite->h, frameWidth, frameHeight);

    // Calculate source rectangle for the current frame
    SDL_Rect src = {(c->frame % frameCount) * frameWidth, 0, frameWidth, frameHeight};

    // Handle horizontal flipping based on facing direction
    SDL_Surface *spriteToRender = sprite;
    SDL_Surface *flippedSprite = NULL;
    if (c->facing == 0) { // Facing left, flip the sprite
        flippedSprite = flip_surface_horizontally(sprite);
        if (!flippedSprite) {
            printf("Failed to flip character sprite: %s\n", SDL_GetError());
            return;
        }
        spriteToRender = flippedSprite;
        src.x = (frameCount - 1 - (c->frame % frameCount)) * frameWidth;
    }

    // Destination rectangle (use frame dimensions to avoid scaling issues)
    SDL_Rect dest = {c->pos.x, c->pos.y, frameWidth, frameHeight};

    // Blit the sprite to the screen
    SDL_BlitSurface(spriteToRender, &src, screen, &dest);

    // Free the flipped sprite if it was created
    if (flippedSprite) {
        SDL_FreeSurface(flippedSprite);
    }
}

void handleLevelTransition(SDL_Surface* screen, TTF_Font* font, TTF_Font* nbr) {
    Mix_HaltChannel(-1);
    bool quit = false;
    SDL_Event e;

    // Load background
    SDL_Surface* background = IMG_Load("background.png");
    if (background == NULL) {
        printf("Error loading background image: %s\n", IMG_GetError());
        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    } else {
        if (background->w != screen->w || background->h != screen->h) {
            printf("Warning: Background image size (%dx%d) does not match screen size (%dx%d)\n",
                   background->w, background->h, screen->w, screen->h);
        }
        SDL_BlitSurface(background, NULL, screen, NULL);
    }

    // Display "PASS TO NEXT LEVEL"
    SDL_Color green = {0, 255, 0, 0};
    SDL_Surface* levelUpSurface = TTF_RenderText_Solid(font, "PASS TO NEXT LEVEL", green);
    if (levelUpSurface == NULL) {
        printf("Error rendering level up text: %s\n", TTF_GetError());
        if (background) {
            SDL_FreeSurface(background);
        }
        return;
    }
    SDL_Rect levelUpRect = {(screen->w - levelUpSurface->w) / 2, (screen->h - levelUpSurface->h) / 2-50, 0, 0};
    SDL_BlitSurface(levelUpSurface, NULL, screen, &levelUpRect);
    SDL_Flip(screen);
    SDL_FreeSurface(levelUpSurface);

    // 2-second display with event checking
    Uint32 startTime = SDL_GetTicks();
    Uint32 transitionDuration = 2000; // 2 seconds
    while (!quit && (SDL_GetTicks() - startTime < transitionDuration)) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true; // Exit immediately on window close
            } else if (e.type == SDL_KEYDOWN) {
                quit = true; // Skip transition on keypress
            }
        }
        SDL_Delay(10); // Small delay to reduce CPU usage
    }

    // Proceed to countdown only if not quit
    if (!quit) {
        int countdown = 5;
        SDL_Rect countdownRect = {(screen->w - 100) / 2, (screen->h - 100) / 2 + 50, 100, 100};
        while (countdown > 0 && !quit) {
            // Clear countdown area
            if (background) {
                SDL_BlitSurface(background, &countdownRect, screen, &countdownRect);
            } else {
                SDL_FillRect(screen, &countdownRect, SDL_MapRGB(screen->format, 0, 0, 0));
            }

            // Render countdown text
            char countdownText[10];
            sprintf(countdownText, "%d", countdown);
            SDL_Surface* countdownTimeSurface = TTF_RenderText_Solid(nbr, countdownText, green);
            if (countdownTimeSurface == NULL) {
                printf("Error rendering countdown text: %s\n", TTF_GetError());
            } else {
                SDL_Rect countdownTimeRect = {(screen->w - countdownTimeSurface->w) / 2, (screen->h - countdownTimeSurface->h) / 2 + 50, 0, 0};
                SDL_BlitSurface(countdownTimeSurface, NULL, screen, &countdownTimeRect);
                SDL_Flip(screen);
                SDL_FreeSurface(countdownTimeSurface);
            }

            // 1-second delay with event checking
            startTime = SDL_GetTicks();
            while (!quit && (SDL_GetTicks() - startTime < 1000)) {
                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_QUIT) {
                        quit = true; // Exit immediately on window close
                    } else if (e.type == SDL_KEYDOWN) {
                        quit = true; // Skip countdown on keypress
                    }
                }
                SDL_Delay(10);
            }
            countdown--;
        }
    }

    // Final screen update
    if (!quit) {
        if (background) {
            SDL_BlitSurface(background, NULL, screen, NULL);
        } else {
            SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
        }
        SDL_Flip(screen);
    }

    // Free background
    if (background) {
        SDL_FreeSurface(background);
    }
}

int level1(SDL_Surface *screen, TTF_Font *font, TTF_Font *nbr, SDL_Surface *background, Character *character, Ennemi *ennemi, Uint32 lastTime) {
    bool running = true;
    SDL_Event event;
    int nombreDeFrappe = 0;
    int remainingTime;
    Uint32 startTime = SDL_GetTicks();

    enum GameState { PLAYING, LEVEL_UP, GAME_OVER };
    enum GameState gameState = PLAYING;

    Uint32 lastFrameTime = SDL_GetTicks();
    int currentLevel = 1;
    bool moveUp = false, moveDown = false, moveLeft = false, moveRight = false, isRunning = false;

    while (running) {
        Uint32 currentFrameTime = SDL_GetTicks();
        float deltaTime = (currentFrameTime - lastFrameTime) / 1000.0f;
        lastFrameTime = currentFrameTime;

        SDL_BlitSurface(background, NULL, screen, NULL);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return 0;
            }
            deplacerCharacter(character, event, ennemi, &moveUp, &moveDown, &moveLeft, &moveRight, &isRunning);
        }

        updateCharacterMovement(character, deltaTime, moveUp, moveDown, moveLeft, moveRight, isRunning);
        update_character(character);

        if (checkCollision(ennemi, character)) {
        ennemi->anim = FIGHT;
        static Uint32 dernierCoup = 0;
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - dernierCoup > 1000) { // 1-second cooldown
            float dx = character->pos.x + character->pos.w / 2 - (ennemi->pos.x + ennemi->pos.w / 2);
            float dy = character->pos.y + character->pos.h / 2 - (ennemi->pos.y + ennemi->pos.h / 2);
            float distance = sqrt(dx * dx + dy * dy);
            // Only deal damage if enemy is in FIGHT state and on a specific frame (e.g., attack frame)
            if (distance < 80.0f && ennemi->anim == FIGHT && ennemi->frame == 2) { // Example: frame 2 is the "attack" frame
                nombreDeFrappe++;
                character->state = 4; // Hurt state
                character->lives--;
                if (character->lives <= 0) {
                    Mix_HaltChannel(-1);
                    showGameOver(screen, font);
                    return 0;
                }
                dernierCoup = currentTime;
            }
        }
    } else {
        ennemi->anim = WALK;
        deplacerEnnemiVersCharacter(ennemi, character, deltaTime);
    }

        if (ennemi->health <= 0) {
            Mix_HaltChannel(-1);
            showYouWon(screen, font);
            return 0;
        }

        jouerSon(ennemi->anim);
        afficher(screen, ennemi, character, font, background);

        Uint32 now = SDL_GetTicks();
        if (now - lastTime > 100) {
            const int frameCounts[5] = {1, 6, 6, 8, 4};
            if (ennemi->anim >= 0 && ennemi->anim < 5) {
                int currentFrameCount = frameCounts[ennemi->anim];
                ennemi->frame = (ennemi->frame + 1) % currentFrameCount;
            }
            lastTime = now;
        }

        Uint32 currentTime = SDL_GetTicks();
        int elapsedSeconds = (currentTime - startTime) / 1000;
        remainingTime = 20 - elapsedSeconds;
        if (remainingTime < 0) remainingTime = 0;
        if (remainingTime == 0 && gameState == PLAYING) {
            gameState = LEVEL_UP;
        }

        char timeText[32];
        sprintf(timeText, "Time: %d", remainingTime);
        SDL_Color textColor = {255, 255, 255, 0};
        SDL_Surface *textSurface = TTF_RenderText_Solid(nbr, timeText, textColor);
        if (textSurface) {
            SDL_Rect textPosition = {320, 10, 0, 0};
            SDL_BlitSurface(textSurface, NULL, screen, &textPosition);
            SDL_FreeSurface(textSurface);
        }

        SDL_Flip(screen);

        if (gameState == LEVEL_UP && currentLevel == 1) {
            handleLevelTransition(screen, font, nbr);
            currentLevel = 2;
            gameState = PLAYING;
            running = false;
        }

        SDL_Delay(16);
    }

    return 1;
}
int level2(SDL_Surface *screen, TTF_Font *font, SDL_Surface *background, Character *character, Ennemi *ennemi, Uint32 lastTime) {
    character->pos.x = 100;
    ennemi->pos.x = SCREEN_W - 164;
    character->pos.y = 300;
    ennemi->pos.y = 300;
    bool running = true;
    SDL_Event event;
    Uint32 lastFrameTime = SDL_GetTicks();
    bool isAttacking = false;
    static Uint32 lastAttackTime = 0;
    static Uint32 lastAnimSwitchTime = 0;
    bool moveUp = false, moveDown = false, moveLeft = false, moveRight = false, isRunning = false;

    while (running) {
        Uint32 currentFrameTime = SDL_GetTicks();
        float deltaTime = (currentFrameTime - lastFrameTime) / 1000.0f;
        lastFrameTime = currentFrameTime;
        SDL_BlitSurface(background, NULL, screen, NULL);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return 0;
            }
            deplacerCharacter(character, event, ennemi, &moveUp, &moveDown, &moveLeft, &moveRight, &isRunning);
        }

        updateCharacterMovement(character, deltaTime, moveUp, moveDown, moveLeft, moveRight, isRunning);
        update_character(character);

            if (checkCollision(ennemi, character)) {
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastAnimSwitchTime > 1000) {
        isAttacking = !isAttacking;
        lastAnimSwitchTime = currentTime;
    }
    ennemi->anim = isAttacking ?: FIGHT;
    if (currentTime - lastAttackTime > 1000) {
        float dx = character->pos.x + character->pos.w / 2 - (ennemi->pos.x + ennemi->pos.w / 2);
        float dy = character->pos.y + character->pos.h / 2 - (ennemi->pos.y + ennemi->pos.h / 2);
        float distance = sqrt(dx * dx + dy * dy);
        if (distance < 80.0f && ennemi->frame == 2) { // Only attack on specific frame
            character->state = 4; // Hurt state
            character->lives--;
            if (character->lives <= 0) {
                Mix_HaltChannel(-1);
                showGameOver(screen, font);
                return 0;
            }
            lastAttackTime = currentTime;
        }
    }
} else {
    isAttacking = false;
    deplacerEnnemiVersCharacter(ennemi, character, deltaTime);
}

        if (ennemi->health <= 0) {
            Mix_HaltChannel(-1);
            showYouWon(screen, font);
            return 0;
        }

        jouerSon(ennemi->anim);
        afficher(screen, ennemi, character, font, background);

        Uint32 now = SDL_GetTicks();
        if (now - lastTime > 100) {
            const int frameCounts[5] = {1, 6, 6, 8, 4};
            if (ennemi->anim >= 0 && ennemi->anim < 5) {
                ennemi->frame = (ennemi->frame + 1) % frameCounts[ennemi->anim];
            }
            lastTime = now;
        }

        SDL_Flip(screen);
        SDL_Delay(16);
    }
    return 0;
}
