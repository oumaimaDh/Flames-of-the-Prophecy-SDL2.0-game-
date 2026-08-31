#include "header.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int initialize_sdl(GameState *game) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 0;
    }
    if (TTF_Init() == -1) {
        printf("TTF could not initialize! TTF_Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 0;
    }
    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        printf("SDL_image could not initialize! IMG_Error: %s\n", IMG_GetError());
        TTF_Quit();
        SDL_Quit();
        return 0;
    }
    game->screen = SDL_SetVideoMode(WIDTH, HEIGHT, 32, SDL_HWSURFACE);
    if (!game->screen) {
        printf("Could not set video mode: %s\n", SDL_GetError());
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        return 0;
    }
    SDL_EnableUNICODE(1);
    return 1;
}

int load_resources(GameState *game) {
    game->bg1 = IMG_Load("bg1.png");
    game->bg2 = IMG_Load("bg2.png");
    game->image1 = IMG_Load("question.png");
    game->image2 = IMG_Load("answer.png");
    game->font = TTF_OpenFont("text.ttf", 20);
    game->nbr = TTF_OpenFont("nbr.ttf", 20);
    if (!game->bg1 || !game->bg2 || !game->image1 || !game->image2 || !game->font || !game->nbr) {
        printf("Failed to load resources: %s\n", IMG_GetError());
        return 0;
    }
    return 1;
}

int load_questions(GameState *game) {
    strcpy(game->answers[0], "hourglass");
    strcpy(game->answers[1], "Bou Kanoun");
    strcpy(game->answers[2], "Naaoucha");
    strcpy(game->answers[3], "Bou Chkara");
    strcpy(game->answers[4], "A Riddle");
    strcpy(game->answers[5], "Rym");
    strcpy(game->answers[6], "Bou Garnin");
    strcpy(game->answers[7], "The Medina");
    strcpy(game->answers[8], "Bou Kanoun");
    strcpy(game->answers[9], "The sand of the hourglass");

    FILE *file = fopen("questions", "r");
    if (file) {
        char line[MAX_LINE_LEN];
        int current_question = 0;
        int line_idx = 0;
        while (fgets(line, MAX_LINE_LEN, file) && current_question < MAX_QUESTIONS) {
            line[strcspn(line, "\n")] = 0;
            line[strcspn(line, "\r")] = 0;
            if (strlen(line) == 0) {
                if (line_idx > 0) {
                    current_question++;
                    game->question_count = current_question;
                    line_idx = 0;
                }
                continue;
            }
            if (line_idx < MAX_QUESTION_LINES) {
                strncpy(game->questions[current_question][line_idx], line, MAX_LINE_LEN - 1);
                game->questions[current_question][line_idx][MAX_LINE_LEN - 1] = '\0';
                line_idx++;
            } else {
                printf("Warning: Max lines (%d) reached for question %d, skipping line: %s\n", 
                       MAX_QUESTION_LINES, current_question + 1, line);
            }
        }
        if (line_idx > 0 && current_question < MAX_QUESTIONS) {
            game->question_count = current_question + 1;
        }
        fclose(file);
    } else {
        strcpy(game->questions[0][0], "I do not tick, but I hold time.");
        strcpy(game->questions[0][1], "Lose me, and history may not rhyme.");
        strcpy(game->questions[0][2], "What am I?");
        game->question_count = 1;
    }
    return 1;
}

void setup_question(GameState *game) {
    game->selected_idx = game->question_count > 0 ? rand() % game->question_count : 0;
    game->correct_answer = game->answers[game->selected_idx];
    SDL_Color blue = {184, 134, 11, 255};
    game->line_count = 0;
    for (int i = 0; i < MAX_QUESTION_LINES && game->questions[game->selected_idx][i][0] != '\0'; i++) {
        game->questionSurfaces[i] = TTF_RenderText_Solid(game->font, game->questions[game->selected_idx][i], blue);
        if (game->questionSurfaces[i]) {
            game->line_count++;
        }
    }
    game->pos1.x = 100; game->pos1.y = 10;
    game->pos2.x = 110; game->pos2.y = 280;
    game->textPos.x = 300; game->textPos.y = 470;
    game->timerPos.x = 600; game->timerPos.y = 20;
    for (int i = 0; i < game->line_count; i++) {
        game->questionTextPos[i].x = 170;
        game->questionTextPos[i].y = 70 + i * 25;
    }
}

void render_game(GameState *game) {
    Uint32 current_time = SDL_GetTicks();
    Uint32 elapsed_time = current_time - game->start_time;
    int seconds_left = (TIMER_DURATION - elapsed_time) / 1000;

    SDL_FillRect(game->screen, NULL, SDL_MapRGB(game->screen->format, 0, 0, 0));
    SDL_BlitSurface(game->image1, NULL, game->screen, &game->pos1);
    SDL_BlitSurface(game->image2, NULL, game->screen, &game->pos2);
    for (int i = 0; i < game->line_count; i++) {
        if (game->questionSurfaces[i]) {
            SDL_BlitSurface(game->questionSurfaces[i], NULL, game->screen, &game->questionTextPos[i]);
        }
    }
    SDL_Color blue = {184, 134, 11, 255};
    SDL_Surface *textSurface = TTF_RenderText_Solid(game->font, game->input, blue);
    if (textSurface) {
        SDL_BlitSurface(textSurface, NULL, game->screen, &game->textPos);
        SDL_FreeSurface(textSurface);
    }
    char timer_text[16];
    snprintf(timer_text, sizeof(timer_text), "Time: %d", seconds_left);
    SDL_Surface *timerSurface = TTF_RenderText_Solid(game->nbr, timer_text, blue);
    if (timerSurface) {
        SDL_BlitSurface(timerSurface, NULL, game->screen, &game->timerPos);
        SDL_FreeSurface(timerSurface);
    }
    game->last_second = seconds_left;
    SDL_Flip(game->screen);
}

void handle_input(GameState *game, SDL_Event *event) {
    if (event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_BACKSPACE && game->inputPos > 0) {
            game->input[--game->inputPos] = '\0';
        } else if (event->key.keysym.sym == SDLK_RETURN) {
            if (strcmp(game->input, game->correct_answer) == 0) {
                game->correct = 1;
                game->gameover = 0;
            } else {
                game->gameover = 1;
            }
        } else if (game->inputPos < MAX_ANSWER_LEN - 1 && event->key.keysym.unicode >= 32 && event->key.keysym.unicode <= 126) {
            game->input[game->inputPos++] = event->key.keysym.unicode;
            game->input[game->inputPos] = '\0';
        }
    }
}

void display_result(GameState *game) {
    SDL_FillRect(game->screen, NULL, SDL_MapRGB(game->screen->format, 0, 0, 0));
    if (game->correct) {
        int alpha;
        SDL_SetAlpha(game->bg1, SDL_SRCALPHA, 255);
        for (alpha = 255; alpha >= 0; alpha -= 5) {
            SDL_SetAlpha(game->bg1, SDL_SRCALPHA, alpha);
            SDL_FillRect(game->screen, NULL, SDL_MapRGB(game->screen->format, 0, 0, 0));
            SDL_BlitSurface(game->bg1, NULL, game->screen, NULL);
            SDL_Flip(game->screen);
            SDL_Delay(5);
        }
        SDL_SetAlpha(game->bg1, SDL_SRCALPHA, 255);
        SDL_SetAlpha(game->bg2, SDL_SRCALPHA, 0);
        for (alpha = 0; alpha <= 255; alpha += 5) {
            SDL_SetAlpha(game->bg1, SDL_SRCALPHA, 255 - alpha);
            SDL_SetAlpha(game->bg2, SDL_SRCALPHA, alpha);
            SDL_FillRect(game->screen, NULL, SDL_MapRGB(game->screen->format, 0, 0, 0));
            SDL_BlitSurface(game->bg1, NULL, game->screen, NULL);
            SDL_BlitSurface(game->bg2, NULL, game->screen, NULL);
            SDL_Flip(game->screen);
            SDL_Delay(5);
        }
    } else if (game->gameover) {
        SDL_Color blue = {184, 134, 11, 255};
        SDL_Surface *message = TTF_RenderText_Solid(game->font, "GAME OVER", blue);
        if (message) {
            SDL_BlitSurface(message, NULL, game->screen, &game->textPos);
            SDL_FreeSurface(message);
        }
    }
    SDL_Flip(game->screen);
    SDL_Delay(1000);
}

void cleanup(GameState *game) {
    for (int i = 0; i < game->line_count; i++) {
        SDL_FreeSurface(game->questionSurfaces[i]);
    }
    SDL_FreeSurface(game->bg1);
    SDL_FreeSurface(game->bg2);
    SDL_FreeSurface(game->image1);
    SDL_FreeSurface(game->image2);
    TTF_CloseFont(game->font);
    TTF_CloseFont(game->nbr);
    SDL_FreeSurface(game->screen);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}