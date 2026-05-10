#include "HumanPlayer.h"
#include "Board.h"
#include "pieces/Piece.h"
#include "Exceptions.h"

HumanPlayer::HumanPlayer(Color color)
    : Player(color) {}

// ── Private helpers ────────────────────────────────────────────────────────────

void HumanPlayer::selectPiece(Position sq, const Board& board) {
    selectedSquare = sq;
    Board& mb = const_cast<Board&>(board);
    legalMovesCache = mb.getLegalMoves(getColor());
    std::vector<Move> filtered;
    for (const Move& m : legalMovesCache)
        if (m.src == sq) filtered.push_back(m);
    legalMovesCache = filtered;
}

bool HumanPlayer::tryMove(Position dst) {
    for (const Move& m : legalMovesCache) {
        if (m.dst == dst) {
            storedMove     = m;
            pendingMove    = true;
            selectedSquare = {-1, -1};
            legalMovesCache.clear();
            return true;
        }
    }
    return false;
}

// ── Mouse press ────────────────────────────────────────────────────────────────
// Called when the left button goes down on a board square.
void HumanPlayer::handleMousePress(Position sq, const Board& board) {
    if (!sq.isValid()) return;
    Piece* p = board.getPieceAt(sq);

    if (p && p->getColor() == getColor()) {
        // Pressing on own piece: select it and begin drag
        selectPiece(sq, board);
        dragging   = true;
        dragFromSq = sq;
    } else if (selectedSquare.isValid()) {
        // Pressing on a destination while a piece is already selected (click-to-move)
        if (!tryMove(sq)) {
            selectedSquare = {-1, -1};
            legalMovesCache.clear();
        }
        dragging = false;
    }
    // else: pressing on empty / enemy with nothing selected → do nothing
}

// ── Mouse move ─────────────────────────────────────────────────────────────────
// Always called so the dragged piece tracks the cursor smoothly.
void HumanPlayer::handleMouseMove(int px, int py) {
    dragPx = px;
    dragPy = py;
}

// ── Mouse release ──────────────────────────────────────────────────────────────
// Completes a drag-and-drop move if the piece was released on a valid square.
void HumanPlayer::handleMouseRelease(Position dst, const Board& /*board*/) {
    if (!dragging) return;
    dragging = false;

    if (dst.isValid() && dst != dragFromSq) {
        // Dropped on a different square — try to make the move
        if (!tryMove(dst)) {
            // Not a legal destination — deselect (snap back)
            selectedSquare = {-1, -1};
            legalMovesCache.clear();
        }
    }
    // Released on same square as press: drag cancelled, selection stays for click-to-move
}

// ── Legacy shim ────────────────────────────────────────────────────────────────
void HumanPlayer::handleClick(Position square, const Board& board) {
    handleMousePress(square, board);
}

// ── Queries ────────────────────────────────────────────────────────────────────
bool HumanPlayer::hasPendingMove() const { return pendingMove; }

Move HumanPlayer::getPendingMove() {
    pendingMove = false;
    return storedMove;
}

Position HumanPlayer::getSelectedSquare() const { return selectedSquare; }
const std::vector<Move>& HumanPlayer::getLegalMovesCache() const { return legalMovesCache; }

void HumanPlayer::resetSelection() {
    selectedSquare = {-1, -1};
    legalMovesCache.clear();
    pendingMove = false;
    dragging    = false;
}

// Satisfies abstract interface — not used in interactive mode
Move HumanPlayer::chooseMove(const Board& /*board*/) { return storedMove; }
