#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <numbers>
#include <string>
#include <vector>

#include "Vector2.hpp"
#include "SFML/Graphics.hpp"
#include "imgui-SFML.h"
#include "imgui.h"

class Snake {
    public:
        std::vector<Vec2I> pieces;
        Vec2I direction = Vec2I(1, 0);

    private:
        const Vec2I gridSize;
        std::size_t head = 0;

    public:
        Snake(const Vec2I& pos_, const Vec2I& gridSize_) : pieces({pos_}), gridSize(gridSize_) {}

    void move(bool grow) {
        Vec2I oldPos = pieces[head];
        if (grow) {
            pieces.insert(pieces.begin() + head, pieces[head]);
            head += 1; 
            // std::cout <<  pieces.size();
        }
        if (head == 0) head = pieces.size();
        head -= 1;
        pieces[head] = oldPos + direction;
    }

    void draw(sf::RenderWindow& window, sf::RectangleShape rec){
        for(const auto& p: pieces) {
            rec.setPosition(static_cast<float>(p.x) * 10, static_cast<float>(p.y) * 10);
            window.draw(rec);
        }
    }
};

int main() {
    const Vec2I gridSize(50,50);
    Snake snake(Vec2I(1, 1), gridSize);

    sf::RectangleShape body;
    body.setSize(sf::Vector2f(10, 10));
    body.setFillColor(sf::Color::Green);

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode(500,500), "Snake", sf::Style::Default, settings); // sf::VideoMode::getDesktopMode(), sf::Style::Fullscreen old one
    ImGui::SFML::Init(window);

    while (window.isOpen()) {
        std::chrono::_V2::system_clock::time_point start = std::chrono::high_resolution_clock::now();

        sf::Event event; //NOLINT
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
                if (event.type == sf::Event::KeyPressed) {
                switch(event.key.code) {
                    case(sf::Keyboard::Key::W):
                        snake.direction = Vec2I(0, -1);
                        break;
                    case(sf::Keyboard::Key::A):
                        snake.direction = Vec2I(-1, 0);
                        break;
                    case(sf::Keyboard::Key::S):
                        snake.direction = Vec2I(0, 1);
                        break;
                    case(sf::Keyboard::Key::D):
                        snake.direction = Vec2I(1, 0);
                        break;
                    default: {}
                }
            }
        }

        snake.move(sf::Keyboard::isKeyPressed(sf::Keyboard::Space));
        window.clear();
        snake.draw(window, body);
        window.display();

        while ((std::chrono::high_resolution_clock::now() - start).count() < 100'000'000) {} // scuffed way of halting till time has passed
    }
    std::cout << "bye :)";
    return 0;
}