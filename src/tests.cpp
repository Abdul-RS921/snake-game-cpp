// Tests for the game rules. Build and run:  g++ -std=c++17 src/tests.cpp -o tests && ./tests
#include <cstdio>
#include <cstdlib>
#include "snake_game.h"

static int failures = 0;

#define CHECK(cond)                                                             \
    do {                                                                        \
        if (!(cond)) {                                                          \
            std::printf("  FAILED: %s (line %d)\n", #cond, __LINE__);           \
            ++failures;                                                         \
        }                                                                       \
    } while (0)

static void test(const char* name, void (*fn)()) {
    std::printf("%s\n", name);
    fn();
}

static void startsWithThreeCells() {
    SnakeGame g(20, 10, 1);
    CHECK(g.snake().size() == 3);
    CHECK(g.score() == 0);
    CHECK(!g.isGameOver());
    CHECK(!g.isOnSnake(g.food()));
}

static void movesForward() {
    SnakeGame g(20, 10, 1);
    g.setFoodForTest({0, 0});
    Point head = g.snake().front();
    g.step();
    CHECK(g.snake().front() == (Point{head.x + 1, head.y}));
    CHECK(g.snake().size() == 3);
}

static void cannotReverse() {
    SnakeGame g(20, 10, 1);
    g.setFoodForTest({0, 0});
    g.setDirection(Direction::Left);   // opposite of the current direction (Right)
    g.step();
    CHECK(g.direction() == Direction::Right);
    CHECK(!g.isGameOver());
}

static void eatingGrowsAndScores() {
    SnakeGame g(20, 10, 1);
    Point head = g.snake().front();
    g.setFoodForTest({head.x + 1, head.y});
    g.step();
    CHECK(g.snake().size() == 4);
    CHECK(g.score() == 10);
    CHECK(!g.isOnSnake(g.food()));   // new food is on a free cell
}

static void wallEndsTheGame() {
    SnakeGame g(8, 6, 1);
    g.setFoodForTest({0, 0});
    for (int i = 0; i < 20 && !g.isGameOver(); ++i) g.step();
    CHECK(g.isGameOver());
}

static void hittingItselfEndsTheGame() {
    SnakeGame g(20, 20, 1);
    // grow the snake to 5 cells by placing food right in front of it, twice
    for (int i = 0; i < 2; ++i) {
        Point h = g.snake().front();
        g.setFoodForTest({h.x + 1, h.y});
        g.step();
    }
    CHECK(g.snake().size() == 5);
    g.setFoodForTest({0, 0});
    g.setDirection(Direction::Down);  g.step();
    g.setDirection(Direction::Left);  g.step();
    g.setDirection(Direction::Up);    g.step();   // turns into its own body
    CHECK(g.isGameOver());
}

static void tailCellIsFreeWhenNotGrowing() {
    // A snake of length 4 chasing its own tail in a 2x2 loop must not die.
    SnakeGame g(20, 20, 1);
    Point h = g.snake().front();
    g.setFoodForTest({h.x + 1, h.y});
    g.step();                         // length 4, heading right
    g.setFoodForTest({0, 0});
    g.setDirection(Direction::Down);  g.step();
    g.setDirection(Direction::Left);  g.step();
    g.setDirection(Direction::Up);    g.step();   // head moves into the cell the tail just left
    CHECK(!g.isGameOver());
}

static void foodIsNeverOnTheSnake() {
    SnakeGame g(10, 6, 7);
    for (int round = 0; round < 25 && !g.isGameOver(); ++round) {
        CHECK(!g.isOnSnake(g.food()));
        Point h = g.snake().front();
        // steer towards the food with simple rules, ignoring safety (we only test food placement)
        Point f = g.food();
        if (f.x > h.x) g.setDirection(Direction::Right);
        else if (f.x < h.x) g.setDirection(Direction::Left);
        else if (f.y > h.y) g.setDirection(Direction::Down);
        else g.setDirection(Direction::Up);
        g.step();
    }
}

static void fullBoardMeansWin() {
    SnakeGame g(4, 1, 1);             // 4 cells in a row, snake of 3 already fills 3 of them
    g.setFoodForTest({3, 0});
    g.step();                         // eats the last free cell
    CHECK(g.hasWon());
    CHECK(g.isGameOver());
    CHECK(g.score() == 10);
}

static void resetStartsOver() {
    SnakeGame g(20, 10, 1);
    Point h = g.snake().front();
    g.setFoodForTest({h.x + 1, h.y});
    g.step();
    g.reset();
    CHECK(g.snake().size() == 3);
    CHECK(g.score() == 0);
    CHECK(!g.isGameOver());
}

int main() {
    test("starts with three cells", startsWithThreeCells);
    test("moves forward", movesForward);
    test("cannot reverse into itself", cannotReverse);
    test("eating grows the snake and scores", eatingGrowsAndScores);
    test("wall ends the game", wallEndsTheGame);
    test("hitting itself ends the game", hittingItselfEndsTheGame);
    test("tail cell is free when not growing", tailCellIsFreeWhenNotGrowing);
    test("food is never placed on the snake", foodIsNeverOnTheSnake);
    test("full board means a win", fullBoardMeansWin);
    test("reset starts over", resetStartsOver);

    if (failures == 0) {
        std::printf("\nAll tests passed.\n");
        return 0;
    }
    std::printf("\n%d check(s) failed.\n", failures);
    return 1;
}
