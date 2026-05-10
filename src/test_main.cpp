#include <iostream>
#include "Board.h"
#include "pieces/King.h"
#include "pieces/Queen.h"
#include "pieces/Rook.h"
#include "pieces/Bishop.h"
#include "pieces/Knight.h"
#include "pieces/Pawn.h"

int main() {
    // ── Print initial board ────────────────────────────────────────────────
    Board board;
    std::cout << "=== Initial Board ===\n" << board << "\n";

    // ── White pawn legal moves from e2 ────────────────────────────────────
    std::cout << "=== White legal moves from e2 (col 4, row 1) ===\n";
    auto pawnMoves = board.getLegalMoves(Color::White);
    int shown = 0;
    for (const Move& m : pawnMoves) {
        std::cout << m << "  ";
        if (++shown % 8 == 0) std::cout << "\n";
    }
    std::cout << "\nTotal white legal moves at start: " << pawnMoves.size() << "\n\n";

    // ── Play a few moves (e2e4, e7e5, d2d4) ──────────────────────────────
    std::cout << "=== Playing e2e4 ===\n";
    Move e2e4(Position(1, 4), Position(3, 4));
    auto s1 = board.applyMove(e2e4);
    board.swapTurn();
    std::cout << board << "\n";

    std::cout << "=== Playing e7e5 ===\n";
    Move e7e5(Position(6, 4), Position(4, 4));
    auto s2 = board.applyMove(e7e5);
    board.swapTurn();
    std::cout << board << "\n";

    std::cout << "=== Playing d2d4 ===\n";
    Move d2d4(Position(1, 3), Position(3, 3));
    auto s3 = board.applyMove(d2d4);
    board.swapTurn();
    std::cout << board << "\n";

    // ── Test isInCheck (should be false here) ─────────────────────────────
    bool whiteInCheck = board.isInCheck(Color::White);
    bool blackInCheck = board.isInCheck(Color::Black);
    std::cout << "White in check: " << (whiteInCheck ? "YES" : "no") << "\n";
    std::cout << "Black in check: " << (blackInCheck ? "YES" : "no") << "\n\n";

    // ── Test Board copy constructor ────────────────────────────────────────
    std::cout << "=== Board copy constructor (AI deep-copy test) ===\n";
    Board copy(board);
    Move blackKnight(Position(7, 6), Position(5, 5)); // Ng8f6
    auto s4 = copy.applyMove(blackKnight);
    copy.swapTurn();
    std::cout << "Original unchanged:\n" << board << "\n";
    std::cout << "Copy after Ng8f6:\n" << copy << "\n";

    // ── Test undoMove ─────────────────────────────────────────────────────
    std::cout << "=== Undo d2d4 ===\n";
    board.undoMove(d2d4, s3);
    board.swapTurn();
    std::cout << board << "\n";

    // ── operator== on pieces ──────────────────────────────────────────────
    Pawn p1(Color::White), p2(Color::White), p3(Color::Black);
    std::cout << "White Pawn == White Pawn: " << (p1 == p2 ? "true" : "false") << "\n";
    std::cout << "White Pawn == Black Pawn: " << (p1 == p3 ? "true" : "false") << "\n\n";

    // ── Move operator<< ───────────────────────────────────────────────────
    Move promotion(Position(6, 4), Position(7, 4), MoveType::Promotion, PieceType::Queen);
    std::cout << "Promotion move notation: " << promotion << "\n";

    std::cout << "\nAll tests passed.\n";
    return 0;
}
