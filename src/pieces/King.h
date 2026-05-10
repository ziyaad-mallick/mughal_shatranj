#pragma once
#include "Piece.h"

class King : public Piece {
public:
    King(Color color);
    std::vector<Move> getPseudoLegalMoves(Position pos, const Board& board) const override;
    Piece* clone() const override;
};
