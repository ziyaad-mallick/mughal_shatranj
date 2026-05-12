#include <SFML/Graphics.hpp>
#include "Renderer.h"
#include "Game.h"
#include "LeaderBoard.h"
#include <vector>
#include <string>
#include <iostream>

// Try to load a font — checks assets/ first, then system fallback
static bool tryLoadFont(Renderer& renderer) {
    // Try local assets copy first
    if (renderer.loadFont("assets/arial.ttf"))  return true;
    // Try Windows system fonts
    if (renderer.loadFont("C:/Windows/Fonts/arial.ttf")) return true;
    if (renderer.loadFont("C:/Windows/Fonts/calibri.ttf")) return true;
    std::cerr << "Warning: could not load font. Text will not be visible.\n";
    return false;
}

// Display the leaderboard screen and wait for a key
static void showLeaderboard(sf::RenderWindow& window, Renderer& renderer,
                            LeaderBoard& lb) {
    const auto& records = lb.getRecords();

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) { window.close(); return; }
            if (event->is<sf::Event::KeyPressed>() ||
                event->is<sf::Event::MouseButtonPressed>()) return;
        }

        std::vector<std::string> lines;
        lines.push_back(" Leaderboard (Top Games) ");
        lines.push_back("");
        if (records.empty()) {
            lines.push_back("No games recorded yet.");
        } else {
            lines.push_back("  Rank   White       Black       Winner      Moves");
            lines.push_back("  ----   -----       -----       ------      -----");
            int rank = 1;
            for (const GameRecord& r : records) {
                lines.push_back("  " + std::to_string(rank++) + ".     " +
                                r.player1 + "   vs   " + r.player2 +
                                "   ->  " + r.winner +
                                "   (" + std::to_string(r.moveCount) + " moves)");
                if (rank > 10) break;
            }
        }
        lines.push_back("");
        lines.push_back("Press any key to return to menu");
        renderer.drawMenu("Leaderboard", lines);
    }
}

int main() {
    // Create SFML window
    sf::RenderWindow window(
        sf::VideoMode({static_cast<unsigned>(WINDOW_W), static_cast<unsigned>(WINDOW_H)}),
        "Mughlia Shatranj",
        sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    Renderer    renderer(window);
    tryLoadFont(renderer);
    // Load piece sprite icons — falls back to circle+letter if not found
    if (!renderer.loadPieceSprites("assets"))
        std::cerr << "Note: piece sprites not found, using letter fallback.\n";

    LeaderBoard leaderboard("assets/leaderboard.txt");

    // ── Main menu loop ────────────────────────────────────────────────────────
    while (window.isOpen()) {
        // Draw menu
        renderer.drawMenu("Mughlia Shatranj",
            {"[1]  Human  vs  Human",
             "[2]  Human  vs  AI  (you play White)",
             "[3]  View Leaderboard",
             "[4]  Quit",
             "",
             "Press the number key to select"});

        // Wait for a key press
        bool chosen = false;
        while (window.isOpen() && !chosen) {
            while (const auto event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) { window.close(); break; }

                if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                    switch (key->code) {
                        case sf::Keyboard::Key::Num1:
                        case sf::Keyboard::Key::Numpad1: {
                            Game g(window, renderer, leaderboard, "hvh",
                                   "White", "Black");
                            g.run();
                            chosen = true; break;
                        }
                        case sf::Keyboard::Key::Num2:
                        case sf::Keyboard::Key::Numpad2: {
                            Game g(window, renderer, leaderboard, "hvai",
                                   "You", "AI");
                            g.run();
                            chosen = true; break;
                        }
                        case sf::Keyboard::Key::Num3:
                        case sf::Keyboard::Key::Numpad3:
                            showLeaderboard(window, renderer, leaderboard);
                            chosen = true; break;
                        case sf::Keyboard::Key::Num4:
                        case sf::Keyboard::Key::Numpad4:
                        case sf::Keyboard::Key::Escape:
                            window.close();
                            chosen = true; break;
                        default: break;
                    }
                }
            }
        }
    }

    return 0;
}
