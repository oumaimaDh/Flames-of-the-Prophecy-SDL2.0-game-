#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>  
#include <stdlib.h>
#include <stdio.h>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 786

// Struct for buttons
typedef struct {
    SDL_Rect pos;
    SDL_Surface *normal;
    SDL_Surface *hover;
    SDL_Surface *currentImage;
    int isHovered;
} Button;

SDL_Surface *screen = NULL;
SDL_Surface *bg1 = NULL, *bg2 = NULL;
Button yesButton, noButton, loadGameButton, newGameButton, backButton1, backButton2;

// 🎵 Music and sound effects
Mix_Music *bgMusic = NULL;
Mix_Chunk *buttonClick = NULL;
Mix_Chunk *buttonHover = NULL;

int gameState = 0;

// ✅ Load Assets (Images, Sounds, etc.)
void loadAssets() {
    bg1 = IMG_Load("bgr1.png");
    bg2 = IMG_Load("bgr2.png");

    // Load button images (Normal & Hover)
    yesButton.normal = IMG_Load("yesr.png");
    yesButton.hover = IMG_Load("yesr2.png");
    yesButton.currentImage = yesButton.normal;

    noButton.normal = IMG_Load("nor.png");
    noButton.hover = IMG_Load("nor2.png");
    noButton.currentImage = noButton.normal;

    loadGameButton.normal = IMG_Load("changee1.png");
    loadGameButton.hover = IMG_Load("change2.png");
    loadGameButton.currentImage = loadGameButton.normal;

    newGameButton.normal = IMG_Load("newch1.png");
    newGameButton.hover = IMG_Load("newch2.png");
    newGameButton.currentImage = newGameButton.normal;

    backButton1.normal = IMG_Load("back1.png");
    backButton1.hover = IMG_Load("back2.png");
    backButton1.currentImage = backButton1.normal;

    backButton2.normal = IMG_Load("back1.png");
    backButton2.hover = IMG_Load("back2.png");
    backButton2.currentImage = backButton2.normal;

    // ✅ Load background music (MP3)
    bgMusic = Mix_LoadMUS("break.mp3");
    if (!bgMusic) printf("Error loading music: %s\n", Mix_GetError());

    // ✅ Load button sounds (WAV format for `Mix_Chunk`)
    buttonClick = Mix_LoadWAV("bm.wav");  
    buttonHover = Mix_LoadWAV("bm.wav");

    if (!buttonClick) printf("Error loading button click sound: %s\n", Mix_GetError());
    if (!buttonHover) printf("Error loading button hover sound: %s\n", Mix_GetError());

    // ✅ Play background music in loop 🎶
    if (bgMusic) Mix_PlayMusic(bgMusic, -1);

    // Set button positions
    yesButton.pos.x = 240; yesButton.pos.y = 350;
    noButton.pos.x = 488; noButton.pos.y = 350;
    backButton1.pos.x = 310; backButton1.pos.y = 587;

    loadGameButton.pos.x = 45; loadGameButton.pos.y = 40;
    newGameButton.pos.x = 500; newGameButton.pos.y = 40;
    backButton2.pos.x = 100; backButton2.pos.y = 600;
}

// ✅ Function to check if mouse is over a button
int isMouseOver(SDL_Event event, Button *button) {
    return (event.motion.x >= button->pos.x && event.motion.x <= button->pos.x + button->normal->w &&
            event.motion.y >= button->pos.y && event.motion.y <= button->pos.y + button->normal->h);
}

// ✅ Render Function
void render() {
    SDL_Surface *bg = (gameState == 0) ? bg1 : bg2;
    SDL_BlitSurface(bg, NULL, screen, NULL);

    if (gameState == 0) {
        SDL_BlitSurface(yesButton.currentImage, NULL, screen, &yesButton.pos);
        SDL_BlitSurface(noButton.currentImage, NULL, screen, &noButton.pos);
        SDL_BlitSurface(backButton1.currentImage, NULL, screen, &backButton1.pos);
    } else if (gameState == 1) {
        SDL_BlitSurface(loadGameButton.currentImage, NULL, screen, &loadGameButton.pos);
        SDL_BlitSurface(newGameButton.currentImage, NULL, screen, &newGameButton.pos);
        SDL_BlitSurface(backButton2.currentImage, NULL, screen, &backButton2.pos);
    }

    SDL_Flip(screen);
}

// ✅ Handle Events (Mouse & Keyboard)
void handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) exit(0);

        if (event.type == SDL_MOUSEMOTION) {
            // ✅ Hover Effect (Change button image)
            Button *buttons[] = {&yesButton, &noButton, &loadGameButton, &newGameButton, &backButton1, &backButton2};
            int numButtons = 6;

            for (int i = 0; i < numButtons; i++) {
                if (isMouseOver(event, buttons[i])) {
                    if (!buttons[i]->isHovered) {
                        buttons[i]->currentImage = buttons[i]->hover;
                        buttons[i]->isHovered = 1;
                    }
                } else {
                    buttons[i]->currentImage = buttons[i]->normal;
                    buttons[i]->isHovered = 0;
                }
            }
        }

        if (event.type == SDL_MOUSEBUTTONDOWN) {
            // ✅ Play click sound
            Mix_PlayChannel(-1, buttonClick, 0);

            if (gameState == 0) {
                if (isMouseOver(event, &yesButton)) gameState = 1;
                if (isMouseOver(event, &noButton)) {
                    system("../main_menu/main_menu_app"); // ✅ Return to main menu
                    exit(0);
                }
                if (isMouseOver(event, &backButton1)) {
                    system("../actual_game/game_app"); // ✅ Resume game
                    exit(0);
                }
            } else if (gameState == 1) {
                if (isMouseOver(event, &newGameButton)) {
                    gameState = 0;  // ✅ Restart game
                }
                if (isMouseOver(event, &loadGameButton)) {
                    system("../multiplayer/multiplayer_app"); // ✅ Load Multiplayer Menu
                    exit(0);
                }
                if (isMouseOver(event, &backButton2)) {
                    gameState = 0;
                }
            }
        }
    }
}

// ✅ Main Function
int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_SWSURFACE);

    loadAssets();
    
    while (1) {
        handleEvents();
        render();
        SDL_Delay(16);
    }

    // ✅ Clean up sounds and music before exiting
    Mix_FreeMusic(bgMusic);
    Mix_FreeChunk(buttonClick);
    Mix_FreeChunk(buttonHover);
    Mix_CloseAudio();

    SDL_Quit();
    return 0;
}
