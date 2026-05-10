#pragma once
#include "Player.h"
#include "primitives/Position.h"
#include <vector>

class Board;

class HumanPlayer : public Player {
public:
    explicit HumanPlayer(Color color);

    // ── Mouse event handlers (called by Game each frame) ─────────────────────
    // Left button pressed on a board square
    void handleMousePress(Position sq, const Board& board);
    // Mouse moved (raw pixel coords, always called)
    void handleMouseMove(int pixelX, int pixelY);
    // Left button released (dst = {-1,-1} if off-board)
    void handleMouseRelease(Position dst, const Board& board);

    // Legacy click shim (routes to handleMousePress)
    void handleClick(Position square, const Board& board);

    // ── Pending move ─────────────────────────────────────────────────────────
    bool hasPendingMove() const;
    Move getPendingMove();

    // ── Renderer queries ─────────────────────────────────────────────────────
    Position                   getSelectedSquare()    const;
    const std::vector<Move>&   getLegalMovesCache()   const;

    // Drag-and-drop state (read each frame by Renderer)
    bool     isDragging()   const { return dragging; }
    Position getDragFrom()  const { return dragFromSq; }
    int      getDragPx()    const { return dragPx; }
    int      getDragPy()    const { return dragPy; }

    // ── Misc ─────────────────────────────────────────────────────────────────
    void resetSelection();
    bool isHuman() const override { return true; }
    Move chooseMove(const Board& board) override;

private:
    Position          selectedSquare {-1, -1};
    std::vector<Move> legalMovesCache;
    bool              pendingMove = false;
    Move              storedMove;

    // Drag state
    bool     dragging   = false;
    Position dragFromSq {-1, -1};
    int      dragPx     = 0;
    int      dragPy     = 0;

    // Helpers
    void selectPiece(Position sq, const Board& board);
    bool tryMove(Position dst);
};
