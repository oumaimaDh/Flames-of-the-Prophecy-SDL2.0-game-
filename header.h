#ifndef GAME_H
#define GAME_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#define WIDTH 800
#define HEIGHT 600
#define MAX_QUESTIONS 10
#define MAX_QUESTION_LINES 4
#define MAX_LINE_LEN 256
#define MAX_ANSWER_LEN 100
#define TIMER_DURATION 25000 // 25 seconds in milliseconds

typedef struct {
    SDL_Surface *screen;
    SDL_Surface *bg1;
    SDL_Surface *bg2;
    SDL_Surface *image1;
    SDL_Surface *image2;
    TTF_Font *font;
    TTF_Font *nbr;
    char questions[MAX_QUESTIONS][MAX_QUESTION_LINES][MAX_LINE_LEN];
    char answers[MAX_QUESTIONS][MAX_ANSWER_LEN];
    int question_count;
    int selected_idx;
    char *correct_answer;
    SDL_Surface *questionSurfaces[MAX_QUESTION_LINES];
    int line_count;
    SDL_Rect pos1;
    SDL_Rect pos2;
    SDL_Rect textPos;
    SDL_Rect timerPos;
    SDL_Rect questionTextPos[MAX_QUESTION_LINES];
    char input[MAX_ANSWER_LEN];
    int inputPos;
    int correct;
    int gameover;
    Uint32 start_time;
    int last_second;
} GameState;

int initialize_sdl(GameState *game);
int load_resources(GameState *game);
int load_questions(GameState *game);
void setup_question(GameState *game);
void render_game(GameState *game);
void handle_input(GameState *game, SDL_Event *event);
void display_result(GameState *game);
void cleanup(GameState *game);

#endif