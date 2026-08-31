#include "header.h"
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>

int main() {
    SDL_Surface *screen = initSDL();
    if (!screen) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    initAudio();
    if (TTF_Init() == -1) {
        printf("Failed to initialize TTF: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    Ennemi ennemi;
    Character character;
    initEnnemi(&ennemi);
    initCharacter(&character);


    Mix_Music *bgMusic = Mix_LoadMUS("audio.mp3");
    if (!bgMusic) {
        printf("Failed to load background music: %s\n", Mix_GetError());
    } else {
        Mix_PlayMusic(bgMusic, -1);
    }

    TTF_Font *font = TTF_OpenFont("text.ttf", 24);
    TTF_Font *nbr = TTF_OpenFont("nbr.ttf", 24);
    if (!font || !nbr) {
        printf("Failed to load font: %s\n", TTF_GetError());
        if (font) TTF_CloseFont(font);
        if (nbr) TTF_CloseFont(nbr);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Surface *background = IMG_Load("medina.png");
    if (!background) {
        printf("Failed to load background: %s\n", IMG_GetError());
        TTF_CloseFont(font);
        TTF_CloseFont(nbr);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    Uint32 lastTime = SDL_GetTicks();
    // Run level 1
    int result = level1(screen, font, nbr, background, &character, &ennemi, lastTime);
    if (result == 1) {
        // Run level 2 once
        result = level2(screen, font, background, &character, &ennemi, lastTime);
    }

    // Cleanup
    for (int anim = 0; anim < 5; anim++) {
        for (int dir = 0; dir < 2; dir++) {
            if (ennemi.sprites[anim][dir]) {
                SDL_FreeSurface(ennemi.sprites[anim][dir]);
            }
        }
    }
    for (int i = 0; i < 5; i++) {
        if (character.sprites[i]) {
            SDL_FreeSurface(character.sprites[i]);
        }
    }
   
    SDL_FreeSurface(background);
    TTF_CloseFont(font);
    TTF_CloseFont(nbr);
    TTF_Quit();
    Mix_FreeChunk(walkSound);
    Mix_FreeChunk(fightSound);
    Mix_FreeMusic(bgMusic);
    Mix_CloseAudio();
    SDL_Quit();

    return 0;
}