#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <random>
// #include <bitset> for displaying ints in binary

#include "Vector2.hpp"

constexpr bool human = false;

template<size_t actions>
struct State : std::array<double, actions> {
    using arr = std::array<double, actions>;
    double maxValue () const { return *maxIter(); }
    std::size_t maxIndex () const { return maxIter() - arr::cbegin(); }
    std::size_t maxIndexRand (std::mt19937& gen) const { 
        std::vector<std::size_t> indicies;
        indicies.reserve(actions);
        for (std::size_t i = 0; i < actions; ++i) if ((*this)[i] == maxValue()) { indicies.push_back(i); }
        if (indicies.size() == 1) { return indicies[0]; }
        std::uniform_int_distribution<> r(0, static_cast<int>(indicies.size()) - 1);
        return indicies[r(gen)];
    }

private:
   arr::const_iterator maxIter() const { return std::max_element(arr::cbegin(), arr::cend()); }
};

template<size_t actions, size_t states>
class QTable{
    const double learningRate = 0.03F;
    const double discountFactor = 0.5F; // idk

public :
    std::array<State<actions>, states> table;
    void update(unsigned short state1, unsigned short state2, const std::size_t& action, double reward) {
      table[state1][action] += learningRate * (reward + discountFactor * table[state2].maxValue() - table[state1][action]); // bellman equation https://en.wikipedia.org/wiki/Bellman_equation
    }

    QTable(){
        for (State<actions>& s : table) s.fill(0.5F);
    }
};

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
        static std::uniform_int_distribution<int> r(0, 3);
        direction = acc2Dir[r(gen)];
        pieces = {gridSize / 2, gridSize / 2 - direction};
        head = 0;
    }

    void updateDir(const std::size_t& a) { direction = acc2Dir[a]; }
     
    void move(bool grow) {
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
        unsigned short s = static_cast<unsigned short>(danger(0));                                                   // danger up
        s                = static_cast<unsigned short>(s << 1U) | static_cast<unsigned short>(danger(1));            // danger right
        s                = static_cast<unsigned short>(s << 1U) | static_cast<unsigned short>(danger(2));            // danger down
        s                = static_cast<unsigned short>(s << 1U) | static_cast<unsigned short>(danger(3));            // danger left
        s                = static_cast<unsigned short>(s << 1U) | static_cast<unsigned short>(direction.y == -1);               // moving up
        s                = static_cast<unsigned short>(s << 1U) | static_cast<unsigned short>(direction.x == 1);                // moving right
        s                = static_cast<unsigned short>(s << 1U) | static_cast<unsigned short>(food.y < h.y);                    // food up
        s                = static_cast<unsigned short>(s << 1U) | static_cast<unsigned short>(food.x > h.x);                    // food right
        s                = static_cast<unsigned short>(s << 1U) | static_cast<unsigned short>(food.y == h.y || food.x == h.x);  // food on row / column
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
};

void stop(){ return; }

int main() {
    constexpr Vec2I gridSize(50, 50);
    std::mt19937 gen(std::random_device{}());
    Snake       snake(gridSize / 2, gridSize);
    QTable<4,512> table{};
    Vec2I foodPos(newFood(gridSize, snake, gen));

    short state1;
    short state2 = snake.state(foodPos);
    bool foodEaten = false;
    double reward = 0.0F;
    int count = 0;
    Vec2I prevHead;
    std::size_t action = 0;
    while (count < 100'000'000) {
        std::chrono::system_clock::time_point start = std::chrono::high_resolution_clock::now();
        count++;

        state1 = state2;
        prevHead = snake.pieces[snake.head];

        if (!human) { action = table.table[state1].maxIndexRand(gen); } // ai chooses inputs 

        snake.updateDir(action);

        if (snake.danger(action)) { // crash bang wollop
            reward = -10.0F;
            snake.move(false);
            state2 = snake.state(foodPos);
            snake.reset(gen);
        } else { //
            reward = static_cast<double>(foodEaten) * 2.0F;
            snake.move(foodEaten);
            if (!foodEaten) { 
                reward = (pow(snake.pieces[snake.head].x - foodPos.x, 2) + pow(snake.pieces[snake.head].y - foodPos.y, 2)) < (pow(prevHead.x - foodPos.x, 2) + pow(prevHead.y - foodPos.y, 2)) ? 0.1F : -0.1F;
            } 
            state2 = snake.state(foodPos);
        }

        table.update(state1, state2, action, reward);


        foodEaten = snake.pieces[snake.head] == foodPos;
        if (foodEaten) { foodPos = newFood(gridSize, snake, gen); }
    }
    std::cout << "bye :) \n";
    return 0;
}