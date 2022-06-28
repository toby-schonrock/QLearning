#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <numbers>
#include <string>
#include <vector>

#include "SFML/System/Vector2.hpp"
#include "SFML/Window/WindowStyle.hpp"
#include "Vector2.hpp"
#include "SFML/Graphics.hpp"
#include "imgui-SFML.h"
#include "imgui.h"

class Snake {
    public:
        std::vector<Vec2I> pieces;

    private:
        const Vec2I gridSize;
        std::size_t head = 0;
        Vec2I direction = Vec2I(1, 0);

    public:
        Snake(const Vec2I& pos_, const Vec2I& gridSize_) : pieces({pos_}), gridSize(gridSize_) {}

        void move(){
            pieces[head] += direction;
    }
};

void displaySnake(const Snake& snake, sf::RenderWindow& window){
    sf::RectangleShape rec(sf::Vector2f(10, 10));
    rec.setFillColor(sf::Color::Green);
    for(const auto& p: snake.pieces) {
        rec.setPosition(p.x * 10, p.y * 10);
        window.draw(sf::RectangleShape());
        // std::cout << "square drawn \n";
    }
}

int main() {
    const Vec2I gridSize(50,50);
    Snake snake(Vec2I(10, 10), gridSize);

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    sf::RenderWindow window(sf::VideoMode(500,500), "Snake", sf::Style::Default, settings); // sf::VideoMode::getDesktopMode(), sf::Style::Fullscreen old one
    ImGui::SFML::Init(window);

    while (window.isOpen()) {
        sf::Event event; //NOLINT
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sf::Event::Closed) window.close();
        }
        window.clear();
        displaySnake(snake, window);
        window.display();
    }

    

    std::cout << "hello world \n";
    return 0;
}