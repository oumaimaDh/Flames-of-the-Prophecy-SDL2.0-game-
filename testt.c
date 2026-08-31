#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define MAX_NAME_LENGTH 20

typedef struct {
    SDL_Rect rect;
    SDL_Surface *normal;
    SDL_Surface *selected;
    bool isSelected;
} Button;

typedef struct {
    SDL_Rect rect;
    SDL_Surface *normal;
    SDL_Surface *selected;
    bool isSelected;
    char name[MAX_NAME_LENGTH];
} AvatarInput;

// Global variables
SDL_Surface *screen = NULL;
SDL_Surface *background = NULL;
SDL_Surface *popup = NULL;
Mix_Music *bgMusic = NULL;
Mix_Chunk *buttonSound = NULL;
int gameState = 0; // 0 = Main menu, 1 = Multiplayer selection
bool typing = false;
char inputBuffer[MAX_NAME_LENGTH] = "";

// ✅ Generic draw function (now works for both Button & AvatarInput)
void drawElement(SDL_Surface *screen, void *element, bool isAvatarInput) {
    SDL_Surface *image;
    SDL_Rect *rect;

    if (isAvatarInput) {
        AvatarInput *avatar = (AvatarInput *)element;
        image = avatar->isSelected ? avatar->selected : avatar->normal;
        rect = &avatar->rect;
    } else {
        Button *button = (Button *)element;
        image = button->isSelected ? button->selected : button->normal;
        rect = &button->rect;
    }

    if (image) {
        SDL_BlitSurface(image, NULL, screen, rect);
    }
}

// Function to check if the mouse is over a button
bool isMouseOverButton(Button *button, int x, int y) {
    return x >= button->rect.x && x <= button->rect.x + button->rect.w &&
           y >= button->rect.y && y <= button->rect.y + button->rect.h;
}

// Function to save avatar names
void saveAvatarName(const char *avatarName, int avatarNumber) {
    FILE *file = fopen("avatars.txt", "a");
    if (file) {
        fprintf(file, "Avatar %d: %s\n", avatarNumber, avatarName);
        fclose(file);
    }
}

// Function to switch background
void switchBackground(const char *imagePath) {
    SDL_FreeSurface(background);
    background = IMG_Load(imagePath);
}

// Main function
int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_SWSURFACE);
    SDL_WM_SetCaption("Choose Your Play Style", NULL);

    // Load images
    background = IMG_Load("bge1.png");
    popup = IMG_Load("apprr2.png");

    Button singlePlayer = {{210, 400}, IMG_Load("S1.png"), IMG_Load("S2.png"), false};
    Button multiplayer = {{525, 405}, IMG_Load("M1.png"), IMG_Load("M2.png"), false};
    Button backButton = {{60, 570}, IMG_Load("backr2.png"), IMG_Load("backr1.png"), false};
    Button enterButton = {{820, 580}, IMG_Load("enter1.png"), IMG_Load("enter2.png"), false};

    AvatarInput avatar1 = {{120, 250}, IMG_Load("avatar1.png"), IMG_Load("AV1.png"), false, ""};
    AvatarInput avatar2 = {{120, 400}, IMG_Load("avatar2.png"), IMG_Load("AV2.png"), false, ""};
    AvatarInput input1 = {{560, 240}, IMG_Load("input1.png"), IMG_Load("IN1.png"), false, ""};
    AvatarInput input2 = {{560, 370}, IMG_Load("input2.png"), IMG_Load("IN2.png"), false, ""};

    // Initialize SDL_mixer
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
        printf("SDL_mixer initialization failed: %s\n", Mix_GetError());
        return 1;
    }

    // Load background music
    bgMusic = Mix_LoadMUS("bgar.mp3");
    if (!bgMusic) {
        printf("Error loading background music: %s\n", Mix_GetError());
    } else {
        Mix_PlayMusic(bgMusic, -1); // Loop background music
    }

    // Load button sound effect
    buttonSound = Mix_LoadWAV("bgbut.wav"); // ✅ Use WAV for button sound
    if (!buttonSound) {
        printf("Error loading button sound effect: %s\n", Mix_GetError());
    }

    bool running = true;
    int selectedAvatar = 0;
    bool showPopup = false;

    while (running) {
        SDL_Event event;
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;

            if (gameState == 0) {
                singlePlayer.isSelected = isMouseOverButton(&singlePlayer, mouseX, mouseY);
                multiplayer.isSelected = isMouseOverButton(&multiplayer, mouseX, mouseY);
                backButton.isSelected = isMouseOverButton(&backButton, mouseX, mouseY);

                if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                    Mix_PlayChannel(-1, buttonSound, 0); // ✅ Play button sound

                    if (singlePlayer.isSelected) {
                        printf("Single Player selected!\n");
                    }
                    if (multiplayer.isSelected) {
                        gameState = 1; // ✅ Switch to multiplayer menu
                        switchBackground("bge2.png");
                    }
                    if (backButton.isSelected) {
                        running = false;
                    }
                }
            } else if (gameState == 1) {
                avatar1.isSelected = isMouseOverButton((Button *)&avatar1, mouseX, mouseY);
                avatar2.isSelected = isMouseOverButton((Button *)&avatar2, mouseX, mouseY);
                input1.isSelected = isMouseOverButton((Button *)&input1, mouseX, mouseY);
                input2.isSelected = isMouseOverButton((Button *)&input2, mouseX, mouseY);
                backButton.isSelected = isMouseOverButton(&backButton, mouseX, mouseY);
                enterButton.isSelected = isMouseOverButton(&enterButton, mouseX, mouseY);

                if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                    Mix_PlayChannel(-1, buttonSound, 0); // ✅ Play button sound

                    if (avatar1.isSelected) { selectedAvatar = 1; input1.isSelected = true; typing = true; showPopup = true; }
                    if (avatar2.isSelected) { selectedAvatar = 2; input2.isSelected = true; typing = true; showPopup = true; }
                    if (input1.isSelected) { selectedAvatar = 1; typing = true; showPopup = true; }
                    if (input2.isSelected) { selectedAvatar = 2; typing = true; showPopup = true; }
                    if (backButton.isSelected) { gameState = 0; switchBackground("bge1.png"); }
                    if (enterButton.isSelected && strlen(input1.name) > 0 && strlen(input2.name) > 0) {
                        saveAvatarName(input1.name, 1);
                        saveAvatarName(input2.name, 2);
                        gameState = 0; switchBackground("bge1.png");
                    }
                }
            }
        }

        SDL_BlitSurface(background, NULL, screen, NULL);
        if (gameState == 0) {
            drawElement(screen, &singlePlayer, false);
            drawElement(screen, &multiplayer, false);
            drawElement(screen, &backButton, false);
        } else if (gameState == 1) {
            drawElement(screen, &avatar1, true);
            drawElement(screen, &avatar2, true);
            drawElement(screen, &input1, true);
            drawElement(screen, &input2, true);
            drawElement(screen, &backButton, false);
            drawElement(screen, &enterButton, false);
            if (showPopup) {
                SDL_Rect popupPos = {15, 20};
                SDL_BlitSurface(popup, NULL, screen, &popupPos);
            }
        }
        SDL_Flip(screen);
    }

    SDL_FreeSurface(background);
    SDL_Quit();
    return 0;
}
