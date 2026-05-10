#pragma once
#include "Piece.h"

class Bishop : public Piece {
public:
    Bishop(Color color);
    std::vector<Move> getPseudoLegalMoves(Position pos, const Board& board) const override;
    Piece* clone() const override;
};
