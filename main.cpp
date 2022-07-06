#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <random>
#include <bitset>

#include "SFML/Graphics.hpp"
#include "Vector2.hpp"

constexpr bool human = false;

template<size_t actions>
struct State : std::array<float, actions> {
    using arr = std::array<float, actions>;
    float maxValue () const { return *maxIter(); }
    size_t maxIndex () const { return maxIter() - arr::cbegin(); }
    size_t maxIndexRand (std::mt19937& gen) const { 
        std::vector<std::size_t> indicies;
        float max = maxValue();
        // std::cout << max << std::endl;
        indicies.reserve(actions);
        for (std::size_t i = 0; i < actions; ++i) if ((*this)[i] == max) { indicies.push_back(i); }
        if (indicies.size() == 1) { return indicies[0]; }
        std::uniform_int_distribution<int> r(0, indicies.size() - 1);
        return indicies[r(gen)];
    }

    // State () { (*this) = {}; }

private:
   arr::const_iterator maxIter() const { return std::max_element(arr::cbegin(), arr::cend()); }
};

template<size_t actions, size_t states>
class QTable{
    const float learningRate = 0.03F;
    const float discountFactor = 0.5F; // idk

public :
    std::array<State<actions>, states> table;
    void update(unsigned short state1, unsigned short state2, int action, float reward) {
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
        direction = Vec2I(1, 0);
        pieces = {gridSize / 2, gridSize / 2 + acc2Dir[r(gen)]};
        head = 0;
    }

    void updateDir(int a) { direction = acc2Dir[a]; }
     
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

    void draw(sf::RenderWindow& window, sf::RectangleShape rec) {
        for (const auto& p: pieces) {
            rec.setPosition(static_cast<float>(p.x) * 10, static_cast<float>(p.y) * 10);
            window.draw(rec);
        }
        rec.setFillColor(sf::Color::Red);
        rec.setSize(sf::Vector2f(3, 3));
        sf::RectangleShape rec2 = rec;
        Vector2<float>     headPos(static_cast<float>(pieces[head].x), static_cast<float>(pieces[head].y));
        if (direction == Vec2I(1, 0)) {
            rec.setPosition(headPos.x * 10 + 7, headPos.y * 10); // right
            rec2.setPosition(headPos.x * 10 + 7, headPos.y * 10 + 7);
        } else if (direction == Vec2I(-1, 0)) {
            rec.setPosition(headPos.x * 10, headPos.y * 10); // left
            rec2.setPosition(headPos.x * 10, headPos.y * 10 + 7);
        } else if (direction == Vec2I(0, -1)) {
            rec.setPosition(headPos.x * 10 + 7, headPos.y * 10); // up
            rec2.setPosition(headPos.x * 10, headPos.y * 10);
        } else {
            rec.setPosition(headPos.x * 10 + 7, headPos.y * 10 + 7); // down
            rec2.setPosition(headPos.x * 10, headPos.y * 10 + 7);
        }
        window.draw(rec);
        window.draw(rec2);
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

    bool danger(int a) {
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

int main() {
    constexpr Vec2I gridSize(20, 20);
    std::mt19937 gen(std::random_device{}());
    Snake       snake(gridSize / 2, gridSize);
    QTable<4,512> table{};
    Vec2I foodPos(newFood(gridSize, snake, gen));

    sf::RectangleShape body;
    body.setSize(sf::Vector2f(10, 10));
    sf::RectangleShape food = body;
    food.setFillColor(sf::Color::White);
    body.setFillColor(sf::Color::Green);

    sf::ContextSettings settings;
    // settings.antialiasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode((gridSize.x + 1) * 10, (gridSize.y + 1) * 10), "Snake", sf::Style::Default,
                            settings); // sf::VideoMode::getDesktopMode(), sf::Style::Fullscreen old one

    short state1;
    short state2 = snake.state(foodPos);
    bool foodEaten = false;
    float reward = 0.0F;
    int action = 0;
    while (window.isOpen()) {
        std::chrono::system_clock::time_point start = std::chrono::high_resolution_clock::now();

        state1 = state2;

        sf::Event event; // NOLINT
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::KeyPressed && human) { // for human inputs only
                switch (event.key.code) {
                case (sf::Keyboard::Key::W):
                    action = 0; // up
                    break;
                case (sf::Keyboard::Key::D):
                    action = 1; // right
                    break;
                case (sf::Keyboard::Key::S):
                    action = 2; // down
                    break;
                case (sf::Keyboard::Key::A):
                    action = 3; // left
                    break;
                default: {}
                }
            }
        }

        if (!human) { action = table.table[state1].maxIndexRand(gen); } // ai chooses inputs 

        snake.updateDir(action);

        if (snake.danger(action)) { // crash bang wollop
            reward = -2.0F;
            snake.move(false);
            state2 = snake.state(foodPos);
            snake.reset(gen);
        } else { //
            reward = static_cast<float>(foodEaten) * 10.0F;
            snake.move(foodEaten);
            state2 = snake.state(foodPos);
        }

        table.update(state1, state2, action, reward);

        // std::cout << static_cast<std::bitset<8>>(state1) << std::endl;

        foodEaten = snake.pieces[snake.head] == foodPos;
        if (foodEaten) { foodPos = newFood(gridSize, snake, gen); }

        window.clear();
        snake.draw(window, body);
        food.setPosition(static_cast<float>(foodPos.x) * 10.0F, static_cast<float>(foodPos.y) * 10.0F);
        window.draw(food);
        window.display();

        while ((std::chrono::high_resolution_clock::now() - start).count() < 5'000'000) {} // scuffed way of halting till time has passed
    }
    std::cout << "bye :) \n";
    return 0;
}