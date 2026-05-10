#pragma once
#include "Piece.h"

class Pawn : public Piece {
public:
    Pawn(Color color);
    std::vector<Move> getPseudoLegalMoves(Position pos, const Board& board) const override;
    Piece* clone() const override;
};
