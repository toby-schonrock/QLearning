#!/usr/bin/bash
# g++ -std=c++20 -Wall -Wextra -Wpedantic -Wconversion -O3 -flto -march=haswell -fdiagnostics-color=always -I ../SFML-2.5.1/include -I include -L ../SFML-2.5.1/build/lib/ -l sfml-graphics-2 -l sfml-window-2 -l sfml-system-2 -o main.exe main.cpp
# g++ -std=c++20 -Wall -Wextra -Wpedantic -Wconversion -O3 -flto -fdiagnostics-color=always -I ../SFML-2.5.1/include -I include -L ../SFML-2.5.1/build/lib/ -l sfml-graphics-2 -l sfml-window-2 -l sfml-system-2 -o main.exe main.cpp
g++ -std=c++20 -Wall -Wextra -Wpedantic -Wconversion -O3 -flto -fdiagnostics-color=always -I ../SFML-2.5.1/include -I ../imgui-1.87 -I ../imgui-sfml-2.5 -I include -L lib -l ImGui-SFML -l sfml-graphics-2 -l sfml-window-2 -l sfml-system-2 -o main.exe main.cpp
