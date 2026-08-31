#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    time_t seed = time(NULL);
    srand(seed);
    printf("Random seed: %ld\n", seed);

    GameState game = {0};
    game.input[0] = '\0';
    game.inputPos = 0;
    game.correct = 0;
    game.gameover = 0;

    if (!initialize_sdl(&game)) {
        return 1;
    }
    if (!load_resources(&game)) {
        cleanup(&game);
        return 1;
    }
    if (!load_questions(&game)) {
        cleanup(&game);
        return 1;
    }

    SDL_BlitSurface(game.bg1, NULL, game.screen, NULL);
    SDL_Flip(game.screen);
    SDL_Delay(1500);

    setup_question(&game);
    game.start_time = SDL_GetTicks();
    int running = 1;
    SDL_Event event;

    while (running) {
        Uint32 current_time = SDL_GetTicks();
        Uint32 elapsed_time = current_time - game.start_time;
        if (elapsed_time >= TIMER_DURATION) {
            game.gameover = 1;
            running = 0;
        }

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            handle_input(&game, &event);
            if (game.correct || game.gameover) {
                running = 0;
            }
        }

        render_game(&game);
    }

    display_result(&game);
    cleanup(&game);
    return 0;
}