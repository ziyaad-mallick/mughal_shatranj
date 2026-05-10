#include "Bishop.h"
#include "../Board.h"

Bishop::Bishop(Color color) : Piece(color, PieceType::Bishop) {}

Piece* Bishop::clone() const { return new Bishop(*this); }

std::vector<Move> Bishop::getPseudoLegalMoves(Position pos, const Board& board) const {
    std::vector<Move> moves;

    std::vector<Position> dirs = {
        {-1,-1}, {-1, 1}, {1,-1}, {1, 1}
    };

    addSlideMoves(moves, pos, board, dirs);
    return moves;
}
