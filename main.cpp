#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <corecrt.h>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "SFML/Graphics.hpp"
#include "Vector2.hpp"
#include "imgui-SFML.h"
#include "imgui.h"

template<size_t actions, size_t states>
class QTable{
    const float learningRate = 0.1F;
    const float discountFactor = 0; // idk
    std::array<std::array<float, actions>, states> table;

    void update(unsigned short state1, unsigned short state2, int action, float reward) {

    }
};

class Snake {
  public:
    std::vector<Vec2I> pieces;
    Vec2I              direction = Vec2I(1, 0);

  private:
    const Vec2I gridSize;
    std::size_t head = 0;

  public:
    Snake(const Vec2I& pos_, const Vec2I& gridSize_) : pieces({pos_}), gridSize(gridSize_) {}

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

    //unsigned short nextState(const Vec2I& direction) {
    
    //}

    unsigned short state(const Vec2I& food) {
        const Vec2I    h = pieces[head];
        unsigned short s = static_cast<unsigned short>(danger(Vec2I(0, -1)));                                                   // danger up
        s                = static_cast<unsigned short>(s << 1U) | static_cast<unsigned short>(danger(Vec2I(1, 0)));             // danger right
        s                = static_cast<unsigned short>(s << 1U) | static_cast<unsigned short>(danger(Vec2I(0, 1)));             // danger down
        s                = static_cast<unsigned short>(s << 1U) | static_cast<unsigned short>(danger(Vec2I(-1, 0)));            // danger left
        s                = static_cast<unsigned short>(s << 1U) | static_cast<unsigned short>(direction.y == -1);               // moving up
        s                = static_cast<unsigned short>(s << 1U) | static_cast<unsigned short>(direction.x == 1);                // moving right
        s                = static_cast<unsigned short>(s << 1U) | static_cast<unsigned short>(food.y < h.y);                    // food up
        s                = static_cast<unsigned short>(s << 1U) | static_cast<unsigned short>(food.x > h.x);                    // food right
        s                = static_cast<unsigned short>(s << 1U) | static_cast<unsigned short>(food.y == h.y || food.x == h.x);  // food on row / column
        return s;
    }

    bool danger(const Vec2I& d) {
        Vec2I newHead = pieces[head] + d;
        if (newHead.x < 0 || newHead.x > gridSize.x || newHead.y < 0 || newHead.y > gridSize.y) return true;
        return std::any_of(pieces.cbegin(), pieces.cend(),
                           [newHead](const Vec2I& p) { return p == newHead; }); // i used a lambda and a std algorithm yey :)
    }
};

int main() {
    const Vec2I gridSize(49, 49);
    Snake       snake(Vec2I(1, 1), gridSize);

    sf::RectangleShape body;
    body.setSize(sf::Vector2f(10, 10));
    body.setFillColor(sf::Color::Green);

    sf::ContextSettings settings;
    // settings.antialiasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode(500, 500), "Snake", sf::Style::Default,
                            settings); // sf::VideoMode::getDesktopMode(), sf::Style::Fullscreen old one
    ImGui::SFML::Init(window);

    while (window.isOpen()) {
        std::chrono::_V2::system_clock::time_point start = std::chrono::high_resolution_clock::now();

        sf::Event event; // NOLINT
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                case (sf::Keyboard::Key::W):
                    snake.direction = Vec2I(0, -1); // up
                    break;
                case (sf::Keyboard::Key::A):
                    snake.direction = Vec2I(-1, 0); // left
                    break;
                case (sf::Keyboard::Key::S):
                    snake.direction = Vec2I(0, 1); // down
                    break;
                case (sf::Keyboard::Key::D):
                    snake.direction = Vec2I(1, 0); // right
                    break;
                default: {
                }
                }
            }
        }

        if (snake.danger(snake.direction)) window.close();
        snake.move(sf::Keyboard::isKeyPressed(sf::Keyboard::Space));
        window.clear();
        snake.draw(window, body);
        window.display();

        while ((std::chrono::high_resolution_clock::now() - start).count() < 50'000'000) {
        } // scuffed way of halting till time has passed
    }
    std::cout << "bye :)";
    return 0;
}