#pragma once
#include "Piece.h"

class Rook : public Piece {
public:
    Rook(Color color);
    std::vector<Move> getPseudoLegalMoves(Position pos, const Board& board) const override;
    Piece* clone() const override;
};
