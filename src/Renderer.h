#pragma once

#include <SFML/Graphics.hpp>
#include "Board.h"
#include "primitives/Position.h"
#include "primitives/Move.h"
#include <vector>
#include <string>
#include <map>

// Layout constants — easy to tweak
static constexpr int SQUARE_SIZE    = 100;
static constexpr int BOARD_PIXELS   = 8 * SQUARE_SIZE;   // 720
static constexpr int SIDEBAR_WIDTH  = 380;
static constexpr int WINDOW_W       = BOARD_PIXELS + SIDEBAR_WIDTH; // 1100
static constexpr int WINDOW_H       = BOARD_PIXELS;                 // 720

class Renderer {
public:
    explicit Renderer(sf::RenderWindow& window);

    // Load the font — call once after construction
    bool loadFont(const std::string& path);

    // Load piece sprites from a directory (expects wK.png, bQ.png, etc.)
    // Returns true if all 12 loaded; falls back to circles+letter if any fail
    bool loadPieceSprites(const std::string& assetDir);

    // Main draw call — called every frame from Game::run()
    // whiteElapsed / blackElapsed: cumulative seconds each side has used
    // isDragging / dragFrom / dragPx,Py: drag-and-drop state
    void render(const Board& board,
                Position      selectedSquare,
                const std::vector<Move>& legalMoves,
                Position      lastMoveSrc,
                Position      lastMoveDst,
                const std::string& statusMsg,
                int whiteElapsed = 0,
                int blackElapsed = 0,
                bool isDragging  = false,
                Position dragFrom = Position{-1,-1},
                int dragPx = 0,
                int dragPy = 0);

    // Draw the main menu
    void drawMenu(const std::string& title,
                  const std::vector<std::string>& options);

    // Draw a full-screen message (e.g. "AI is thinking...")
    void drawMessage(const std::string& msg);

    sf::RenderWindow& getWindow() { return window; }

private:
    sf::RenderWindow& window;
    sf::Font          font;
    bool              fontLoaded   = false;
    bool              spritesLoaded = false;

    // Sprite textures keyed by "wK", "bQ", etc.
    std::map<std::string, sf::Texture> textures;

    // Board drawing helpers
    void drawSquares(const Board& board,
                     Position selected, const std::vector<Move>& legalMoves,
                     Position lastSrc, Position lastDst);
    void drawPieces(const Board& board, Position skip);  // skip = drag source square
    void drawSidebar(const Board& board, const std::string& statusMsg,
                     int whiteElapsed, int blackElapsed);

    // Coordinate conversion
    sf::Vector2f toPixel(Position pos) const;

    // Draw a single piece — uses sprite if loaded, falls back to circle+letter
    void drawPiece(Piece* piece, sf::Vector2f topLeft);

    // Fallback circle+letter renderer
    void drawPieceFallback(Piece* piece, sf::Vector2f topLeft);

    // Sprite key for a piece ("wK", "bN", etc.)
    static std::string spriteKey(Piece* piece);

    // Piece letter (K Q R B N P) — used by fallback renderer
    static char pieceChar(PieceType t);
};
