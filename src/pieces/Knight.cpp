#include "Knight.h"
#include "../Board.h"

Knight::Knight(Color color) : Piece(color, PieceType::Knight) {}

Piece* Knight::clone() const { return new Knight(*this); }

std::vector<Move> Knight::getPseudoLegalMoves(Position pos, const Board& board) const {
    std::vector<Move> moves;
    Color myColor = getColor();

    // 8 L-shape offsets — no slide loop
    static const int dr[] = {-2,-2,-1,-1, 1, 1, 2, 2};
    static const int dc[] = {-1, 1,-2, 2,-2, 2,-1, 1};

    for (int i = 0; i < 8; ++i) {
        Position dst(pos.row + dr[i], pos.col + dc[i]);
        if (!dst.isValid()) continue;
        Piece* target = board.getPieceAt(dst);
        if (target == nullptr) {
            moves.emplace_back(pos, dst, MoveType::Normal);
        } else if (target->getColor() != myColor) {
            moves.emplace_back(pos, dst, MoveType::Capture);
        }
    }

    return moves;
}
