// Snake game for the Windows console.
//   snake.exe          play (arrow keys or WASD, P = pause, Q = quit)
//   snake.exe --demo   watch the computer play a short game (no keyboard needed)
#include <windows.h>
#include <conio.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

#include "snake_game.h"

static const int kWidth = 30;
static const int kHeight = 15;
static const char* kHighScoreFile = "highscore.txt";

static int loadHighScore() {
    std::ifstream in(kHighScoreFile);
    int value = 0;
    if (in >> value) return std::max(value, 0);
    return 0;
}

static void saveHighScore(int value) {
    std::ofstream out(kHighScoreFile);
    out << value << "\n";
}

// Draw the whole board into a string.
static std::string render(const SnakeGame& g, int highScore) {
    std::string out;
    out += "  SNAKE   Score: " + std::to_string(g.score()) + "   Best: " + std::to_string(highScore) + "\n";
    out += "+" + std::string(g.width(), '-') + "+\n";
    for (int y = 0; y < g.height(); ++y) {
        out += '|';
        for (int x = 0; x < g.width(); ++x) {
            Point p{x, y};
            if (p == g.snake().front()) out += 'O';
            else if (g.isOnSnake(p)) out += 'o';
            else if (p == g.food()) out += '*';
            else out += ' ';
        }
        out += "|\n";
    }
    out += "+" + std::string(g.width(), '-') + "+\n";
    return out;
}

static void hideCursor() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info{};
    GetConsoleCursorInfo(h, &info);
    info.bVisible = FALSE;
    SetConsoleCursorInfo(h, &info);
}

static void moveCursorHome() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(h, COORD{0, 0});
}

static void clearScreen() {
    std::system("cls");
}

// Time between steps in milliseconds: the game speeds up as the score grows.
static int delayFor(int score) {
    return std::max(55, 150 - score / 2);
}

// Simple computer player for --demo: among safe moves, go towards the food.
static Direction chooseDemoMove(const SnakeGame& g) {
    const Direction all[] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
    Point head = g.snake().front();
    Point food = g.food();
    Direction best = g.direction();
    int bestDistance = 1 << 30;
    for (Direction d : all) {
        Point next = head;
        if (d == Direction::Up) next.y--;
        if (d == Direction::Down) next.y++;
        if (d == Direction::Left) next.x--;
        if (d == Direction::Right) next.x++;
        bool opposite = (d == Direction::Up && g.direction() == Direction::Down) ||
                        (d == Direction::Down && g.direction() == Direction::Up) ||
                        (d == Direction::Left && g.direction() == Direction::Right) ||
                        (d == Direction::Right && g.direction() == Direction::Left);
        if (opposite) continue;
        if (next.x < 0 || next.y < 0 || next.x >= g.width() || next.y >= g.height()) continue;
        if (g.isOnSnake(next)) continue;
        int distance = std::abs(next.x - food.x) + std::abs(next.y - food.y);
        if (distance < bestDistance) { bestDistance = distance; best = d; }
    }
    return best;
}

static int runDemo(int steps) {
    SnakeGame g(kWidth, kHeight, 2026);
    int i = 0;
    for (; i < steps && !g.isGameOver(); ++i) {
        g.setDirection(chooseDemoMove(g));
        g.step();
        if (i == 0 || i == steps / 2) std::printf("%s\n", render(g, 0).c_str());
    }
    std::printf("%s\n", render(g, 0).c_str());
    std::printf("Demo finished after %d steps. Score: %d, length: %d, game over: %s\n",
                i, g.score(), static_cast<int>(g.snake().size()), g.isGameOver() ? "yes" : "no");
    return 0;
}

static bool readDirectionKey(int key, Direction& out) {
    switch (key) {
        case 'w': case 'W': out = Direction::Up; return true;
        case 's': case 'S': out = Direction::Down; return true;
        case 'a': case 'A': out = Direction::Left; return true;
        case 'd': case 'D': out = Direction::Right; return true;
        default: return false;
    }
}

static int play() {
    SnakeGame g(kWidth, kHeight, static_cast<unsigned>(GetTickCount()));
    int highScore = loadHighScore();
    hideCursor();
    clearScreen();
    bool paused = false;

    while (true) {
        // read all keys waiting in the buffer
        while (_kbhit()) {
            int key = _getch();
            if (key == 0 || key == 224) {            // arrow keys come as two codes
                int code = _getch();
                if (code == 72) g.setDirection(Direction::Up);
                else if (code == 80) g.setDirection(Direction::Down);
                else if (code == 75) g.setDirection(Direction::Left);
                else if (code == 77) g.setDirection(Direction::Right);
                continue;
            }
            Direction d;
            if (readDirectionKey(key, d)) g.setDirection(d);
            else if (key == 'p' || key == 'P') paused = !paused;
            else if (key == 'q' || key == 'Q') { clearScreen(); return 0; }
        }

        if (!paused) g.step();

        moveCursorHome();
        std::string frame = render(g, std::max(highScore, g.score()));
        frame += paused ? "  PAUSED - press P to continue          \n"
                        : "  Arrows / WASD to move, P pause, Q quit\n";
        std::fputs(frame.c_str(), stdout);

        if (g.isGameOver()) {
            if (g.score() > highScore) {
                highScore = g.score();
                saveHighScore(highScore);
            }
            std::printf("\n  %s Final score: %d   Best: %d\n", g.hasWon() ? "YOU WIN!" : "GAME OVER.", g.score(), highScore);
            std::printf("  Press R to play again or Q to quit.\n");
            while (true) {
                int key = _getch();
                if (key == 'q' || key == 'Q') { clearScreen(); return 0; }
                if (key == 'r' || key == 'R') { g.reset(); clearScreen(); break; }
            }
            continue;
        }
        Sleep(static_cast<DWORD>(delayFor(g.score())));
    }
}

int main(int argc, char** argv) {
    if (argc > 1 && std::strcmp(argv[1], "--demo") == 0) {
        int steps = argc > 2 ? std::atoi(argv[2]) : 200;
        return runDemo(steps > 0 ? steps : 200);
    }
    return play();
}
