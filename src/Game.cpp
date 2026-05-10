#include "Game.h"
#include "AIPlayer.h"
#include "Exceptions.h"
#include <SFML/Graphics.hpp>
#include <sstream>
#include <iostream>

// ─── Construction ─────────────────────────────────────────────────────────────

Game::Game(sf::RenderWindow& /*window*/, Renderer& renderer,
           LeaderBoard& leaderboard, const std::string& mode,
           const std::string& p1Name, const std::string& p2Name)
    : renderer(renderer), leaderboard(leaderboard),
      p1Name(p1Name), p2Name(p2Name),
      lastMoveSrc(-1, -1), lastMoveDst(-1, -1),
      result(GameResult::Ongoing), moveCount(0)
{
    if (mode == "hvai") {
        whitePlayer = std::make_unique<HumanPlayer>(Color::White);
        blackPlayer = std::make_unique<AIPlayer>(Color::Black, 3);
    } else {
        // Default: Human vs Human
        whitePlayer = std::make_unique<HumanPlayer>(Color::White);
        blackPlayer = std::make_unique<HumanPlayer>(Color::Black);
    }
    currentPlayer   = whitePlayer.get();
    startTime       = std::chrono::steady_clock::now();
    turnStartTime   = startTime;
}

// ─── Helpers ──────────────────────────────────────────────────────────────────

void Game::swapCurrentPlayer() {
    // Accumulate time for the player who just finished their turn
    auto now = std::chrono::steady_clock::now();
    int elapsed = static_cast<int>(
        std::chrono::duration_cast<std::chrono::seconds>(now - turnStartTime).count());
    if (currentPlayer == whitePlayer.get()) whiteElapsedSecs += elapsed;
    else                                    blackElapsedSecs += elapsed;
    turnStartTime = now;

    board.swapTurn();
    currentPlayer = (currentPlayer == whitePlayer.get())
                  ? blackPlayer.get() : whitePlayer.get();
    // Reset human selection when the turn changes
    if (auto* hp = dynamic_cast<HumanPlayer*>(whitePlayer.get())) hp->resetSelection();
    if (auto* hp = dynamic_cast<HumanPlayer*>(blackPlayer.get())) hp->resetSelection();
}

void Game::applyPlayerMove(const Move& move) {
    board.applyMove(move);
    lastMoveSrc = move.src;
    lastMoveDst = move.dst;
    ++moveCount;
}

void Game::checkTerminal() {
    Color next = board.getCurrentTurn();

    // Temporary swap to check the upcoming side
    // (board.getCurrentTurn() has already been swapped by swapCurrentPlayer before we call this,
    //  so next is the side that must move)
    if (board.isCheckmate(next)) {
        result    = (next == Color::Black) ? GameResult::WhiteWins : GameResult::BlackWins;
        statusMsg = (next == Color::Black) ? "Checkmate! White wins!"
                                           : "Checkmate! Black wins!";
        recordResult();
    } else if (board.isStalemate(next)) {
        result    = GameResult::Stalemate;
        statusMsg = "Stalemate — draw!";
        recordResult();
    } else if (board.isInCheck(next)) {
        statusMsg = (next == Color::White) ? "White is in check!"
                                           : "Black is in check!";
    } else {
        statusMsg.clear();
    }
}

void Game::recordResult() {
    auto elapsed = std::chrono::steady_clock::now() - startTime;
    int  seconds = static_cast<int>(
        std::chrono::duration_cast<std::chrono::seconds>(elapsed).count());

    std::string winner = "Draw";
    if (result == GameResult::WhiteWins) winner = "White";
    if (result == GameResult::BlackWins) winner = "Black";

    leaderboard.addRecord({p1Name, p2Name, winner, moveCount, seconds});
}

// ─── Main game loop ───────────────────────────────────────────────────────────

void Game::run() {
    sf::RenderWindow& window = renderer.getWindow();

    while (window.isOpen()) {
        // ── Event processing ──────────────────────────────────────────────
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) { window.close(); return; }
            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape) { window.close(); return; }
            }

            if (result != GameResult::Ongoing || !currentPlayer->isHuman()) continue;
            auto* hp = dynamic_cast<HumanPlayer*>(currentPlayer);
            if (!hp) continue;

            // Mouse move — always update drag position
            if (const auto* mv = event->getIf<sf::Event::MouseMoved>()) {
                hp->handleMouseMove(mv->position.x, mv->position.y);
            }

            // Mouse press — select piece + start drag
            if (const auto* press = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (press->button == sf::Mouse::Button::Left) {
                    sf::Vector2i mp = press->position;
                    hp->handleMouseMove(mp.x, mp.y);  // prime drag coords
                    if (mp.x >= 0 && mp.x < BOARD_PIXELS &&
                        mp.y >= 0 && mp.y < WINDOW_H) {
                        Position sq{7 - mp.y / SQUARE_SIZE, mp.x / SQUARE_SIZE};
                        try { hp->handleMousePress(sq, board); }
                        catch (const InvalidMoveException& e) { statusMsg = e.what(); }
                    }
                }
            }

            // Mouse release — complete drag-and-drop move
            if (const auto* rel = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (rel->button == sf::Mouse::Button::Left) {
                    sf::Vector2i mp = rel->position;
                    Position dst{-1, -1};
                    if (mp.x >= 0 && mp.x < BOARD_PIXELS &&
                        mp.y >= 0 && mp.y < WINDOW_H)
                        dst = Position{7 - mp.y / SQUARE_SIZE, mp.x / SQUARE_SIZE};
                    try { hp->handleMouseRelease(dst, board); }
                    catch (const InvalidMoveException& e) { statusMsg = e.what(); }
                }
            }
        }

        // ── Apply moves ───────────────────────────────────────────────────
        if (result == GameResult::Ongoing) {
            if (currentPlayer->isHuman()) {
                auto* hp = dynamic_cast<HumanPlayer*>(currentPlayer);
                if (hp && hp->hasPendingMove()) {
                    applyPlayerMove(hp->getPendingMove());
                    swapCurrentPlayer();
                    checkTerminal();
                }
            } else {
                // AI turn — compute move (may take a moment)
                renderer.render(board, {-1,-1}, {}, lastMoveSrc, lastMoveDst,
                                "AI is thinking...", whiteElapsedSecs, blackElapsedSecs);
                window.display();

                Move aiMove = currentPlayer->chooseMove(board);
                applyPlayerMove(aiMove);
                swapCurrentPlayer();
                checkTerminal();
            }
        }

        // ── Render ────────────────────────────────────────────────────────
        Position selSq{-1, -1};
        std::vector<Move> legalCache;

        if (currentPlayer->isHuman()) {
            auto* hp = dynamic_cast<HumanPlayer*>(currentPlayer);
            if (hp) {
                selSq      = hp->getSelectedSquare();
                legalCache = hp->getLegalMovesCache();
            }
        }

        // Compute live display times — add ongoing turn time to the active player
        auto now = std::chrono::steady_clock::now();
        int liveSecs = static_cast<int>(
            std::chrono::duration_cast<std::chrono::seconds>(now - turnStartTime).count());
        bool wTurn = (board.getCurrentTurn() == Color::White);
        int wDisp = whiteElapsedSecs + (wTurn  ? liveSecs : 0);
        int bDisp = blackElapsedSecs + (!wTurn ? liveSecs : 0);

        // Drag state from current human player
        bool     dragActive = false;
        Position dragFrom   {-1, -1};
        int      dragPx     = 0, dragPy = 0;
        if (auto* hp = dynamic_cast<HumanPlayer*>(currentPlayer)) {
            dragActive = hp->isDragging();
            dragFrom   = hp->getDragFrom();
            dragPx     = hp->getDragPx();
            dragPy     = hp->getDragPy();
        }

        renderer.render(board, selSq, legalCache,
                        lastMoveSrc, lastMoveDst, statusMsg,
                        wDisp, bDisp,
                        dragActive, dragFrom, dragPx, dragPy);

        // After game over: display result message overlay and wait for a key
        if (result != GameResult::Ongoing) {
            // Status is already part of statusMsg rendered by Renderer
            // Just add a "Press any key" prompt
            renderer.drawMessage(statusMsg + "\n\nPress any key to return to menu");

            bool waiting = true;
            while (window.isOpen() && waiting) {
                while (const auto ev = window.pollEvent()) {
                    if (ev->is<sf::Event::Closed>()) { window.close(); return; }
                    if (ev->is<sf::Event::KeyPressed>() ||
                        ev->is<sf::Event::MouseButtonPressed>()) {
                        waiting = false;
                    }
                }
            }
            return; // Return to main menu
        }

        window.display();
    }
}
