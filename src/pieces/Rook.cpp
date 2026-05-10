#include "Rook.h"
#include "../Board.h"

Rook::Rook(Color color) : Piece(color, PieceType::Rook) {}

Piece* Rook::clone() const { return new Rook(*this); }

std::vector<Move> Rook::getPseudoLegalMoves(Position pos, const Board& board) const {
    std::vector<Move> moves;

    std::vector<Position> dirs = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}
    };

    addSlideMoves(moves, pos, board, dirs);
    return moves;
}
