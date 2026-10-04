# Snake Game (C++)

A classic Snake game for the Windows console, written in C++17.

## How to play

| Key | Action |
|-----|--------|
| Arrow keys or `W` `A` `S` `D` | Steer the snake |
| `P` | Pause / resume |
| `Q` | Quit |
| `R` (after game over) | Play again |

Eat the `*` to grow and score 10 points. The game speeds up as your score grows. Hitting a wall or your own body ends the game. Your best score is saved in `highscore.txt`.

## How it is built

The game rules and the screen are kept apart:

- `src/snake_game.h`: the rules only (movement, growth, collisions, food placement). No keyboard or screen code, so it can be tested.
- `src/main.cpp`: the Windows console part (keyboard input, drawing, speed, high score) and a `--demo` mode where the computer plays.
- `src/tests.cpp`: 10 tests of the rules: no reversing into yourself, eating grows and scores, walls, self-collision, the tail cell being free when the snake is not growing, food never placed on the snake, a full board counts as a win, and reset.

## Build and run (MinGW-w64 / g++ 9 or newer, Windows)

```bash
g++ -std=c++17 -O2 src/main.cpp -o snake.exe
./snake.exe              # play
./snake.exe --demo 250   # watch the computer play 250 steps

g++ -std=c++17 src/tests.cpp -o tests.exe
./tests.exe              # run the tests
```
