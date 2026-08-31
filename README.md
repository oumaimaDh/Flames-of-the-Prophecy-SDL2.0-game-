# Flames-of-the-Prophecy-SDL2.0-game-
Flames of the Prophecy
Reclaim the Hourglass of Time and restore the forgotten magic of Tunisia in this fast-paced 2D action and puzzle game.

(Note: This repository serves as a technical showcase and architectural archive of a first-year C/SDL project developed natively on Linux Ubuntu. Some asset files are missing, but the core systems documented below demonstrate the engine's capabilities).


<img width="1365" height="787" alt="Capture 4" src="https://github.com/user-attachments/assets/4570fc3d-4cdf-46dd-9663-27b8929deb47" />
<img width="1362" height="790" alt="Capture 3" src="https://github.com/user-attachments/assets/18ca75ae-e9d9-40c1-a48c-651d8694042b" />
<img width="1371" height="798" alt="Capture 2" src="https://github.com/user-attachments/assets/a744096f-5c96-48ff-894a-8f861712b358" />
<img width="1363" height="795" alt="Capture 1" src="https://github.com/user-attachments/assets/a6897ad7-f28c-4e63-9a94-06bd196a7f7d" />
<img width="753" height="520" alt="490993057_1840244846813062_443611577262306722_n" src="https://github.com/user-attachments/assets/b2453fa1-27d4-4a28-9c49-a7832141a5ad" />
<img width="2048" height="1529" alt="480207637_3520902104708893_350168357084808670_n" src="https://github.com/user-attachments/assets/8523f1a1-4cac-4ab6-992c-6b89db513420" />
<img width="480" height="110" alt="enemy_fight_right" src="https://github.com/user-attachments/assets/d74e7570-ef33-4b77-9e5f-878fe2136d21" />
<img width="2842" height="2068" alt="attack naaoucha_3" src="https://github.com/user-attachments/assets/2386c2c8-8ddb-4df4-b7d3-c161e319dbff" />
<img width="1340" height="746" alt="Screenshot 2025-02-20 103136" src="https://github.com/user-attachments/assets/61f05b67-e34a-4ad3-8a06-6f6937f80f37" />
<img width="4139" height="2864" alt="Sans titre 273_20250220111623" src="https://github.com/user-attachments/assets/90486153-f908-4e63-80af-d044db6c82d5" />


📜 The Legend: Flames of the Prophecy
Rym, a young woman living in modern-day Tunisia, always felt disconnected from her fast-paced world of flickering phone screens and traffic noise. She longed for the golden era of Beys and ancient magic told in her grandmother's folklore. One night, while sitting beside a glowing kanoun filled with curling bkhor smoke, a mystical entity of pure blue mist named Bou Kanoun materialized. He revealed that Bou Chkara, a towering cloaked thief, had stolen the sacred Hourglass of Time—the artifact keeping Tunisia’s ancient power alive.

Thrust backward into the year 1836, Rym awakens in the damp, fog-shrouded forest of Bou Garnin. There, she overcomes her first trial by defeating Naaoucha, an owl-faced specter serving Bou Chkara. After solving an ancient riddle inscribed upon the sealed gates of the Medina, Rym navigates the labyrinthine alleys to confront Bou Chkara. In a tense final battle, Rym breaks his hold on the artifact and catches a glimpse of his secret past: Bou Chkara was once Hakim, a noble guardian of magical relics who turned to bitterness after being betrayed by the Bey he served.

With the Hourglass reclaimed, Rym is returned to her present time. Beside the embers of the kanoun, Bou Kanoun casts an ancient incantation, illuminating the world in golden light and weaving the forgotten power of the past back into modern Tunisia.

🎮 Game Architecture & Core Systems
1. Dynamic Enemy AI & Combat Engine
The game features a robust enemy state machine that handles pursuit and combat mechanics:

State Machine Logic: Enemies transition fluidly between STATE_MOVE, STATE_HURT, STATE_ATTACK, and STATE_DEATH[cite: 2]. Transitions are timed dynamically using SDL_GetTicks() (e.g., enemies switch between attacking and moving every 2000ms)[cite: 2].

Combat System: The player attacks using the spacebar, which triggers an Axis-Aligned Bounding Box (AABB) collision check[cite: 2]. The hitbox calculation dynamically compares the enemy's y-axis center with the weapon's position to register hits[cite: 2].

Health & Audio Feedback: Enemies spawn with 4 health points[cite: 2]. Successful weapon hits instantly trigger an audio effect via Mix_PlayChannel and decrement the health pool[cite: 2].

2. Interactive Riddle System
To unlock the Medina, players must solve a riddle powered by a custom text-parsing module:

Dynamic Data Loading: Questions and answers are loaded dynamically from an external questions text file into memory arrays[cite: 3].

Live Text Input: The engine captures raw keyboard events (SDLK_BACKSPACE, SDLK_RETURN, and unicode characters) to render the player's typing on screen in real-time[cite: 3].

Timer & Visual Effects: The riddle phase includes a live countdown timer calculated via SDL_GetTicks()[cite: 3]. A correct answer triggers a smooth alpha-blended visual transition using SDL_SetAlpha to fade the background in and out[cite: 3].

3. UI, Modular Menus, and High Scores
The user interface is built to be modular and responsive:

Interactive Menus: Buttons feature dedicated normal and hover states, tracked via mouse coordinate polling (SDL_MOUSEMOTION)[cite: 5]. Clicking buttons triggers audio feedback (bm.wav) using the SDL_mixer library[cite: 5].

Modular Application Flow: The game is structured into separate executable modules. Menu buttons use system() calls to seamlessly launch other components like ../main_menu/main_menu_app or ../multiplayer/multiplayer_app[cite: 5].

High Score Tracking: A dedicated victory screen allows players to input their names[cite: 6]. Scores are automatically written to a highscores.txt file, parsed, and sorted in descending order using C's qsort before being rendered on screen[cite: 6].

🕹️ "SandCore" Custom Hardware Controller
To elevate the arcade experience, a custom physical controller was engineered to interface directly with the game via a Serial connection.

Hardware Features
Microcontroller: Driven by an Arduino processing unit that polls input data and transmits it at a 9600 baud rate[cite: 7].

Input Modules: Features an analog joystick for precise X and Y movement, alongside dedicated physical push buttons (PUSH_BUTTON, SW) for in-game actions[cite: 7].

Serial Telemetry: The controller constantly calculates the delta of the joystick's movement; if the threshold is exceeded, it formats and transmits the data payload (e.g., X:...,Y:...,B:...) to the game engine[cite: 7].

Embedded Visuals & Lighting
OLED Display: Integrated with an Adafruit_SSD1306 display module[cite: 7]. It features a non-blocking scrolling text animation that proudly displays the controller's codename, "SandCore", across the 128x32 screen[cite: 7].

Dynamic RGB Feedback: The controller features an RGB LED loop driven by PWM (analogWrite). The lighting continuously interpolates and fades between four custom color states (Red, Blue, Cyan, Yellow) based on a timer mapping, completely independent of the main delay() function[cite: 7].

Developed on Linux Ubuntu using C, SDL, SDL_image, SDL_ttf, and SDL_mixer.
