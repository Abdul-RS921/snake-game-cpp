// Game rules only: no keyboard, no screen. This makes the logic easy to test.
#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include <deque>
#include <random>
#include <vector>

struct Point {
    int x;
    int y;
    bool operator==(const Point& o) const { return x == o.x && y == o.y; }
};

enum class Direction { Up, Down, Left, Right };

class SnakeGame {
public:
    SnakeGame(int width, int height, unsigned seed)
        : width_(width), height_(height), rng_(seed) {
        reset();
    }

    void reset() {
        snake_.clear();
        int cx = width_ / 2, cy = height_ / 2;
        // start with a snake of length 3 moving right
        snake_.push_back({cx, cy});
        snake_.push_back({cx - 1, cy});
        snake_.push_back({cx - 2, cy});
        direction_ = Direction::Right;
        pendingDirection_ = Direction::Right;
        score_ = 0;
        gameOver_ = false;
        won_ = false;
        placeFood();
    }

    // The snake cannot turn straight back into itself.
    void setDirection(Direction d) {
        if (isOpposite(d, direction_)) return;
        pendingDirection_ = d;
    }

    // Advance the game by one tick.
    void step() {
        if (gameOver_) return;
        direction_ = pendingDirection_;

        Point head = snake_.front();
        switch (direction_) {
            case Direction::Up:    head.y -= 1; break;
            case Direction::Down:  head.y += 1; break;
            case Direction::Left:  head.x -= 1; break;
            case Direction::Right: head.x += 1; break;
        }

        // hitting a wall ends the game
        if (head.x < 0 || head.y < 0 || head.x >= width_ || head.y >= height_) {
            gameOver_ = true;
            return;
        }

        bool eating = (head == food_);

        // hitting itself ends the game. The tail cell is free if the snake is not growing,
        // because the tail moves away in the same step.
        for (size_t i = 0; i < snake_.size(); ++i) {
            bool isTail = (i == snake_.size() - 1);
            if (snake_[i] == head && !(isTail && !eating)) {
                gameOver_ = true;
                return;
            }
        }

        snake_.push_front(head);
        if (eating) {
            score_ += 10;
            if (!placeFood()) {   // no free cell left: the board is full, the player wins
                won_ = true;
                gameOver_ = true;
            }
        } else {
            snake_.pop_back();
        }
    }

    int width() const { return width_; }
    int height() const { return height_; }
    int score() const { return score_; }
    bool isGameOver() const { return gameOver_; }
    bool hasWon() const { return won_; }
    Point food() const { return food_; }
    Direction direction() const { return direction_; }
    const std::deque<Point>& snake() const { return snake_; }

    // For tests: put the food at a chosen free cell.
    void setFoodForTest(Point p) { food_ = p; }

    bool isOnSnake(const Point& p) const {
        for (const Point& s : snake_) if (s == p) return true;
        return false;
    }

private:
    static bool isOpposite(Direction a, Direction b) {
        return (a == Direction::Up && b == Direction::Down) ||
               (a == Direction::Down && b == Direction::Up) ||
               (a == Direction::Left && b == Direction::Right) ||
               (a == Direction::Right && b == Direction::Left);
    }

    // Choose a random free cell for the food. Returns false if there is none.
    bool placeFood() {
        std::vector<Point> freeCells;
        for (int y = 0; y < height_; ++y)
            for (int x = 0; x < width_; ++x)
                if (!isOnSnake({x, y})) freeCells.push_back({x, y});
        if (freeCells.empty()) return false;
        std::uniform_int_distribution<size_t> pick(0, freeCells.size() - 1);
        food_ = freeCells[pick(rng_)];
        return true;
    }

    int width_, height_;
    std::deque<Point> snake_;   // front = head
    Direction direction_ = Direction::Right;
    Direction pendingDirection_ = Direction::Right;
    Point food_{0, 0};
    int score_ = 0;
    bool gameOver_ = false;
    bool won_ = false;
    std::mt19937 rng_;
};

#endif
