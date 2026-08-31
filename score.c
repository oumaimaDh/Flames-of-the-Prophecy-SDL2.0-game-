#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 786
#define MAX_NAME_LENGTH 20
#define MAX_SCORES 10

// Struct for buttons
typedef struct {
    SDL_Rect pos;
    SDL_Surface *normal;
    SDL_Surface *hover;
    SDL_Surface *currentImage;
    int isHovered;
} Button;

// Struct for storing scores
typedef struct {
    char name[MAX_NAME_LENGTH + 1];
    int score;
} PlayerScore;

// Global variables
SDL_Surface *screen = NULL;
SDL_Surface *bg3 = NULL, *bg4 = NULL;
Button validateButton, returnButton;

Mix_Music *victoryMusic = NULL;

char playerName[MAX_NAME_LENGTH + 1] = "";
TTF_Font *font = NULL;
SDL_Color textColor = {255, 255, 255};

PlayerScore highScores[MAX_SCORES];
int totalScores = 0;

const char *scoreFile = "highscores.txt";
int gameState = 0; 

// 🔹 Check if the mouse is over a button
int isMouseOver(int mouseX, int mouseY, Button *button) {
    return (mouseX >= button->pos.x && mouseX <= button->pos.x + button->normal->w &&
            mouseY >= button->pos.y && mouseY <= button->pos.y + button->normal->h);
}

// 🔹 Load scores from file
void loadScores() {
    FILE *file = fopen(scoreFile, "r");
    if (!file) {
        printf("No high score file found. Creating a new one...\n");
        return;
    }

    totalScores = 0;
    while (fscanf(file, "%20s %d", highScores[totalScores].name, &highScores[totalScores].score) == 2) {
        totalScores++;
        if (totalScores >= MAX_SCORES) break;
    }

    fclose(file);
}

// 🔹 Save scores to file
void saveScores() {
    FILE *file = fopen(scoreFile, "w");
    if (!file) {
        printf("Error saving high scores!\n");
        return;
    }

    for (int i = 0; i < totalScores && i < MAX_SCORES; i++) {
        fprintf(file, "%s %d\n", highScores[i].name, highScores[i].score);
    }

    fclose(file);
}

// 🔹 Sort scores in descending order
int compareScores(const void *a, const void *b) {
    return ((PlayerScore *)b)->score - ((PlayerScore *)a)->score;
}

// 🔹 Add new score
void addNewScore(const char *name, int score) {
    if (strlen(name) == 0) return;

    if (totalScores < MAX_SCORES) {
        strcpy(highScores[totalScores].name, name);
        highScores[totalScores].score = score;
        totalScores++;
    } else {
        if (score > highScores[MAX_SCORES - 1].score) {
            strcpy(highScores[MAX_SCORES - 1].name, name);
            highScores[MAX_SCORES - 1].score = score;
        }
    }

    qsort(highScores, totalScores, sizeof(PlayerScore), compareScores);
    saveScores();
}

// 🔹 Load Assets
void loadAssets() {
    bg3 = IMG_Load("bb1.png");
    bg4 = IMG_Load("bb2.png");

    if (!bg3 || !bg4) {
        printf("Error loading backgrounds: %s\n", IMG_GetError());
        exit(1);
    }

    validateButton.normal = IMG_Load("ent1.png");
    validateButton.hover = IMG_Load("ent2.png");
    validateButton.currentImage = validateButton.normal;
    validateButton.pos.x = 500;
    validateButton.pos.y = 400;

    returnButton.normal = IMG_Load("back1.png");
    returnButton.hover = IMG_Load("back2.png");
    returnButton.currentImage = returnButton.normal;
    returnButton.pos.x = 100;
    returnButton.pos.y = 580;

    if (!validateButton.normal || !validateButton.hover || !returnButton.normal || !returnButton.hover) {
        printf("Error loading button images: %s\n", IMG_GetError());
        exit(1);
    }

    victoryMusic = Mix_LoadMUS("victory.mp3");
    if (!victoryMusic) {
        printf("Error loading victory music: %s\n", Mix_GetError());
    }

    font = TTF_OpenFont("Roctaria-DEMO.ttf", 45);
    if (!font) {
        printf("Error loading font: %s\n", TTF_GetError());
        exit(1);
    }

    loadScores();
}

// 🎮 Render text
void renderText(const char *text, int x, int y) {
    if (!font) return;

    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, textColor);
    if (textSurface) {
        SDL_Rect textPosition = {300, 350, 0, 0};
        SDL_BlitSurface(textSurface, NULL, screen, &textPosition);
        SDL_FreeSurface(textSurface);
    }
}

// 🎮 Render screen
void render() {
    SDL_Surface *bg = (gameState == 0) ? bg3 : bg4;
    SDL_BlitSurface(bg, NULL, screen, NULL);

    if (gameState == 0) {
        SDL_BlitSurface(validateButton.currentImage, NULL, screen, &validateButton.pos);
        renderText(playerName, 400, 400);
    } else if (gameState == 1) {
        SDL_BlitSurface(returnButton.currentImage, NULL, screen, &returnButton.pos);
        
        for (int i = 0; i < 3 && i < totalScores; i++) {
            char scoreText[50];
            sprintf(scoreText, "%s - %d", highScores[i].name, highScores[i].score);
            renderText(scoreText, 400, 300 + (i * 50));
        }
    }

    SDL_Flip(screen);
}

// 🎮 Handle events
void handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            TTF_Quit();
            Mix_CloseAudio();
            SDL_Quit();
            exit(0);
        }

        if (event.type == SDL_KEYDOWN) {
            if (gameState == 0) {
                if (event.key.keysym.sym == SDLK_RETURN && strlen(playerName) > 0) {
                    int randomScore = rand() % 10000;  
                    addNewScore(playerName, randomScore);
                    gameState = 1;
                    if (victoryMusic) Mix_PlayMusic(victoryMusic, 1);
                } 
                else if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(playerName) > 0) {
                    playerName[strlen(playerName) - 1] = '\0'; 
                } 
                else if (strlen(playerName) < MAX_NAME_LENGTH) {
                    char newChar = (char)event.key.keysym.unicode;
                    if (newChar >= 32 && newChar <= 126) {
                        int len = strlen(playerName);
                        playerName[len] = newChar;
                        playerName[len + 1] = '\0';
                    }
                }
            }
        }

        if (event.type == SDL_MOUSEMOTION) {
            if (isMouseOver(event.motion.x, event.motion.y, &validateButton)) {
                validateButton.currentImage = validateButton.hover;
            } else {
                validateButton.currentImage = validateButton.normal;
            }

            if (isMouseOver(event.motion.x, event.motion.y, &returnButton)) {
                returnButton.currentImage = returnButton.hover;
            } else {
                returnButton.currentImage = returnButton.normal;
            }
        }

        if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (gameState == 0 && isMouseOver(event.button.x, event.button.y, &validateButton)) {
                gameState = 1;
            } else if (gameState == 1 && isMouseOver(event.button.x, event.button.y, &returnButton)) {
                gameState = 0;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_SWSURFACE);

    SDL_EnableUNICODE(1);
    loadAssets();
    
    while (1) {
        handleEvents();
        render();
        SDL_Delay(16);
    }

    return 0;
}

