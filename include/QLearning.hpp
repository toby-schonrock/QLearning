#pragma once

#include <algorithm>
#include <array>
#include <fstream>

template<std::size_t actions>
struct State : std::array<double, actions> {
    using arr = std::array<double, actions>;
    [[nodiscard]] double maxValue () const { return *maxIter(); }
    [[nodiscard]] std::size_t maxIndex () const { return maxIter() - arr::cbegin(); }

private:
   [[nodiscard]] typename arr::const_iterator maxIter() const { return std::max_element(arr::cbegin(), arr::cend()); }
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
    
    void read(const std::string& path) { 
        std::ifstream inputFile(path, std::ios::binary);
        if (inputFile.is_open()) {
            inputFile.read(reinterpret_cast<char*>(&table), sizeof(table));
        }
    }

    void write(const std::string& path) { 
        std::ofstream outputFile(path, std::ios::binary);
        outputFile.write(reinterpret_cast<char*>(&table), sizeof(table));
    }

    QTable(){
        for (State<actions>& s : table) s.fill(0.5F);
    }
};