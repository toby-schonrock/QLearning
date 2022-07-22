#include "QLearning.hpp"
#include "Vector2.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <iterator>
#include <random>
// #include <bitset> for displaying ints in binary


constexpr bool human = false;

void bitBuilder(unsigned short& s, bool b) {
    s = static_cast<unsigned short>(static_cast<unsigned short>(s << 1U) | static_cast<unsigned short>(b));
}

class Snake {
  public:
    std::vector<Vec2I> pieces;
    std::size_t head = 0;

  private:
    static constexpr std::array<Vec2I, 4> acc2Dir {{{0, -1}, {1, 0}, {0, 1}, {-1, 0}}};
    const Vec2I gridSize;
    Vec2I              direction = Vec2I(1, 0);

  public:
    Snake(const Vec2I& pos_, const Vec2I& gridSize_) : pieces({pos_, pos_ + Vec2I(-1, 0)}), gridSize(gridSize_) {}

    void reset(std::mt19937& gen) { 
        static std::uniform_int_distribution<std::size_t> r(0, 3);
        pieces = {gridSize / 2, gridSize / 2 - acc2Dir[r(gen)]};
        head = 0;
    }
     
    void move(bool grow, const std::size_t& a) {
        direction = acc2Dir[a];
        Vec2I oldPos = pieces[head];
        if (grow) {
            pieces.insert(pieces.begin() + static_cast<std::ptrdiff_t>(head), Vec2I());
            head += 1;
        }
        if (head == 0) head = pieces.size();
        head -= 1;
        pieces[head] = oldPos + direction;
    }

    unsigned short state(const Vec2I& food) {
        const Vec2I    h = pieces[head];
        unsigned short s = static_cast<unsigned short>(danger(0));    // danger up
        bitBuilder(s, danger(1));                                     // danger right
        bitBuilder(s, danger(2));                                     // danger down
        bitBuilder(s, danger(3));                                     // danger left
        bitBuilder(s, direction.y == -1);                             // moving up
        bitBuilder(s, direction.x == 1);                              // moving right
        bitBuilder(s, food.y < h.y);                                  // food up
        bitBuilder(s, food.x > h.x);                                  // food right
        bitBuilder(s, food.y == h.y || food.x == h.x);                // food on row / column
        return s;
    }

    bool danger(const std::size_t& a) {
        Vec2I newHead = pieces[head] + acc2Dir[a];
        if (newHead.x < 0 || newHead.x > gridSize.x || newHead.y < 0 || newHead.y > gridSize.y) return true;
        return std::any_of(pieces.cbegin(), pieces.cend(),
                           [&newHead](const Vec2I& p) { return p == newHead; }); // i used a lambda and a std algorithm yey :)
    }
};

Vec2I newFood(const Vec2I& gridSize, const Snake& snake, std::mt19937& gen) {
    static std::uniform_int_distribution<int> distx(0, gridSize.x);
    static std::uniform_int_distribution<int> disty(0, gridSize.y);
    Vec2I food;
    do { food = Vec2I(distx(gen), disty(gen)); }
    while (std::any_of(snake.pieces.cbegin(), snake.pieces.cend(), [&food](const Vec2I& p) { return p == food; }));
    return food;
}

int main() {
    constexpr Vec2I gridSize(50, 50);
    const std::string path{"brain.bin"};
    std::mt19937 gen(std::random_device{}());
    Snake       snake(gridSize / 2, gridSize);
    QTable<4,512> table{};
    Vec2I foodPos(newFood(gridSize, snake, gen));

    table.read(path);

    unsigned short state1;
    unsigned short state2 = snake.state(foodPos);
    bool foodEaten = false;
    double reward = 0.0F;
    int count = 0;
    int movesSinceFood = 0;
    Vec2I prevHead;
    std::size_t action = 0;
    while (count < 1'000'000'000) {
        count++;
        state1 = state2;
        prevHead = snake.pieces[snake.head];

        if (!human) { action = table.table[state1].maxIndex(); } // ai chooses inputs

        if (snake.danger(action)) { // crash bang wollop
            reward = -5.0F;
            snake.move(false, action);
            state2 = snake.state(foodPos);
            snake.reset(gen);
            movesSinceFood = 0;
        } else { //
            reward = static_cast<double>(foodEaten) * 2.0F;
            snake.move(foodEaten, action);
            if (!foodEaten) { 
                reward = (pow(snake.pieces[snake.head].x - foodPos.x, 2) + pow(snake.pieces[snake.head].y - foodPos.y, 2)) < (pow(prevHead.x - foodPos.x, 2) + pow(prevHead.y - foodPos.y, 2)) ? 0.1F : -0.1F;
            } else {
                reward = 2.0F;
                movesSinceFood = 0;
            } 
            if (movesSinceFood > 200) reward -= 0.1F * pow(1.1F, movesSinceFood - 100);
            state2 = snake.state(foodPos);
        }

        movesSinceFood++;
        table.update(state1, state2, action, reward);

        foodEaten = snake.pieces[snake.head] == foodPos;
        if (foodEaten) { foodPos = newFood(gridSize, snake, gen); }
    }

    table.write(path);
    std::cout << "bye :) \n";
    return 0;
}