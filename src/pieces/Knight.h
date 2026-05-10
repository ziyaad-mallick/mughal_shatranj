#pragma once
#include "Piece.h"

class Knight : public Piece {
public:
    Knight(Color color);
    std::vector<Move> getPseudoLegalMoves(Position pos, const Board& board) const override;
    Piece* clone() const override;
};
