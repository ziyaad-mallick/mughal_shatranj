#pragma once

#include "Board.h"
#include "Renderer.h"
#include "Player.h"
#include "HumanPlayer.h"
#include "LeaderBoard.h"
#include <memory>
#include <string>
#include <chrono>

enum class GameResult { Ongoing, WhiteWins, BlackWins, Stalemate };

class Game {
public:
    // mode: "hvh" = Human vs Human, "hvai" = Human vs AI (AI plays Black)
    Game(sf::RenderWindow& window, Renderer& renderer,
         LeaderBoard& leaderboard, const std::string& mode,
         const std::string& p1Name = "White",
         const std::string& p2Name = "Black");

    // Main game loop — blocks until the game ends or window is closed
    void run();

private:
    Board    board;
    Renderer& renderer;
    LeaderBoard& leaderboard;

    std::unique_ptr<Player> whitePlayer;
    std::unique_ptr<Player> blackPlayer;
    Player* currentPlayer; // points to whitePlayer or blackPlayer

    std::string p1Name, p2Name;

    // State for the Renderer
    Position lastMoveSrc, lastMoveDst;
    std::string statusMsg;
    GameResult  result;

    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point turnStartTime;  // when current turn began
    int whiteElapsedSecs = 0;   // cumulative seconds white has spent thinking
    int blackElapsedSecs = 0;   // cumulative seconds black has spent thinking
    int moveCount;

    // Internal helpers
    void applyPlayerMove(const Move& move);
    void checkTerminal();
    void swapCurrentPlayer();
    void recordResult();

    // How many half-moves have been made (for the leaderboard)
    int halfMoveCount() const;
};
