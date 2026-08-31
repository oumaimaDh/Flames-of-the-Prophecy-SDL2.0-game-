#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_rotozoom.h> // SDL_gfx for scaling

int main(int argc, char* args[]) {
    // Variables
    SDL_Surface* screen = NULL;
    SDL_Surface* background = NULL;
    SDL_Surface* logo = NULL;
    SDL_Surface* buttonPlay = NULL;
    SDL_Surface* buttonOptions = NULL;
    SDL_Surface* buttonScores = NULL;
    SDL_Surface* buttonQuit = NULL;
    
    SDL_Rect buttonPlayRect, buttonOptionsRect, buttonScoresRect, buttonQuitRect, logoRect;
    
    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;
    int quit = 0;

    // Initialisation SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Error: %s\n", SDL_GetError());
        return 1;
    }

    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_SWSURFACE);
    if (screen == NULL) {
        printf("Error: %s\n", SDL_GetError());
        return 1;
    }

    // Chargement des médias
    background = IMG_Load("bg1.png");
    if (background == NULL) {
        printf("Error loading background: %s\n", IMG_GetError());
        return 1;
    }
    background = SDL_ConvertSurface(background, screen->format, 0);
    
    logo = IMG_Load("logor.png");
    if (logo == NULL) {
        printf("Error loading logo: %s\n", IMG_GetError());
        return 1;
    }

    buttonPlay = IMG_Load("startr.png");
    if (buttonPlay == NULL) {
        printf("Error loading play button: %s\n", IMG_GetError());
        return 1;
    }
    buttonOptions = IMG_Load("optionsr.png");
    if (buttonOptions == NULL) {
        printf("Error loading options button: %s\n", IMG_GetError());
        return 1;
    }
    buttonScores = IMG_Load("scoresr.png");
    if (buttonScores == NULL) { 
        printf("Error loading scores button: %s\n", IMG_GetError());
        return 1;
    }
    buttonQuit = IMG_Load("quitr.png");
    if (buttonQuit == NULL) {
        printf("Error loading quit button: %s\n", IMG_GetError());
        return 1;
    }

    logoRect.x = 525; logoRect.y = 110;
    buttonPlayRect.x = 530; buttonPlayRect.y = 235;
    buttonOptionsRect.x = 525; buttonOptionsRect.y = 270;
    buttonScoresRect.x = 525; buttonScoresRect.y = 300;
    buttonQuitRect.x = 535; buttonQuitRect.y = 330;
    
    // Fix de la taille du bouton Quit
    buttonQuitRect.w = buttonQuit->w;
    buttonQuitRect.h = buttonQuit->h;

    // Boucle principale
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            } 
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                
                // Vérifie si le bouton "Quit" a été cliqué
                if (x >= buttonQuitRect.x && x <= (buttonQuitRect.x + buttonQuit->w) &&
                    y >= buttonQuitRect.y && y <= (buttonQuitRect.y + buttonQuit->h)) {
                    quit = 1;
                }
            }
        }

        // Dessin du menu
        SDL_BlitSurface(background, NULL, screen, NULL);
        SDL_BlitSurface(buttonPlay, NULL, screen, &buttonPlayRect);
        SDL_BlitSurface(buttonOptions, NULL, screen, &buttonOptionsRect);
        SDL_BlitSurface(buttonScores, NULL, screen, &buttonScoresRect);
        SDL_BlitSurface(buttonQuit, NULL, screen, &buttonQuitRect);
        SDL_BlitSurface(logo, NULL, screen, &logoRect);

        // Mise à jour de l'écran
        SDL_Flip(screen);
        SDL_Delay(16); // Délai pour rendre l'affichage plus fluide
    }

    // Libération des ressources
    SDL_FreeSurface(background);
    SDL_FreeSurface(buttonPlay);
    SDL_FreeSurface(buttonOptions);
    SDL_FreeSurface(buttonScores);
    SDL_FreeSurface(buttonQuit);
    SDL_Quit();

    return 0;
}
