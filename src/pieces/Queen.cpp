#include "Queen.h"
#include "../Board.h"

Queen::Queen(Color color) : Piece(color, PieceType::Queen) {}

Piece* Queen::clone() const { return new Queen(*this); }

std::vector<Move> Queen::getPseudoLegalMoves(Position pos, const Board& board) const {
    std::vector<Move> moves;

    // All 8 directions (horizontal, vertical, diagonal)
    std::vector<Position> dirs = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1},   // rook directions
        {-1,-1}, {-1, 1}, {1,-1}, {1, 1}     // bishop directions
    };

    addSlideMoves(moves, pos, board, dirs);
    return moves;
}
