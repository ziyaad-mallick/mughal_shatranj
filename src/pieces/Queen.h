#pragma once
#include "Piece.h"

class Queen : public Piece {
public:
    Queen(Color color);
    std::vector<Move> getPseudoLegalMoves(Position pos, const Board& board) const override;
    Piece* clone() const override;
};
