#include "SFML/Graphics.hpp"
#include "QLearning.hpp"
#include "Vector2.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
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

void displayCount(int count, sf::RenderWindow& window, const sf::Font& font) {
    sf::Text text;
    text.setFont(font); // font is a sf::Font
    text.setString(std::to_string(count));
    text.setCharacterSize(10); // in pixels, not points!
    text.setFillColor(sf::Color::Red);
    window.draw(text);
}

int main() {
    constexpr Vec2I gridSize(50, 50);
    const std::string path{"brain.bin"};
    std::mt19937 gen(std::random_device{}());
    Snake       snake(gridSize / 2, gridSize);
    QTable<4,512> table{};
    Vec2I foodPos(newFood(gridSize, snake, gen));

    table.read(path);

    sf::RectangleShape body;
    body.setSize(sf::Vector2f(10, 10));
    sf::RectangleShape food = body;
    food.setFillColor(sf::Color::White);
    body.setFillColor(sf::Color::Green);

    sf::ContextSettings settings;
    sf::RenderWindow window(sf::VideoMode((gridSize.x + 1) * 10, (gridSize.y + 1) * 10), "Snake", sf::Style::Default,
                            settings);

    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cout << "Font file not found";
        return (EXIT_FAILURE);
    }

    unsigned short state1;
    unsigned short state2 = snake.state(foodPos);
    bool foodEaten = false;
    double reward = 0.0F;
    int count = 0;
    int movesSinceFood = 0;
    Vec2I prevHead;
    std::size_t action = 0;
    while (window.isOpen()) {
        std::chrono::system_clock::time_point start = std::chrono::high_resolution_clock::now();
        count++;

        state1 = state2;
        prevHead = snake.pieces[snake.head];

        sf::Event event; // NOLINT
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
        }

        if (!human) { action = table.table[static_cast<std::size_t>(state1)].maxIndex(); } // ai chooses inputs 

        if (snake.danger(action)) {         // crash bang wollop
            reward = -10.0F;
            snake.move(false, action);
            state2 = snake.state(foodPos);
            snake.reset(gen);
            movesSinceFood = 0;
        } else {                           // not crash
            snake.move(foodEaten, action);
            if (!foodEaten) { 
                reward = (pow(snake.pieces[snake.head].x - foodPos.x, 2) + pow(snake.pieces[snake.head].y - foodPos.y, 2)) < (pow(prevHead.x - foodPos.x, 2) + pow(prevHead.y - foodPos.y, 2)) ? 0.1F : -0.1F;
            } else {
                reward = 2.0F;
                movesSinceFood = 0;
            } 
            if (movesSinceFood > 200) reward -= 0.1F * pow(1.1F, movesSinceFood - 200);
            state2 = snake.state(foodPos);
        }

        movesSinceFood++;
        table.update(state1, state2, action, reward);

        foodEaten = snake.pieces[snake.head] == foodPos;
        if (foodEaten) { foodPos = newFood(gridSize, snake, gen); }

        if(!sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            window.clear();
            snake.draw(window, body);
            food.setPosition(static_cast<float>(foodPos.x) * 10.0F, static_cast<float>(foodPos.y) * 10.0F);
            displayCount(count, window, font);
            window.draw(food);
            window.display();
        }

        while ((!sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && (std::chrono::high_resolution_clock::now() - start).count() < 10'000'000)) {} // scuffed way of halting till time has passed
    }
    table.write(path);
    std::cout << "bye :) \n";
    return 0;
}