#pragma once

#include "../primitives/Enums.h"
#include "../primitives/Position.h"
#include "../primitives/Move.h"
#include <vector>

class Board; // forward declaration — avoids circular include

class Piece {
public:
    Piece(Color color, PieceType type);
    virtual ~Piece() = default;

    // Copy constructor — AI deep-copies the board, so pieces must be cloneable
    Piece(const Piece& other) = default;

    // Pure virtual — makes Piece abstract; each subclass implements its own move logic
    virtual std::vector<Move> getPseudoLegalMoves(Position pos, const Board& board) const = 0;

    // Virtual clone — lets Board copy-construct without knowing the concrete type
    virtual Piece* clone() const = 0;

    // Getters
    Color     getColor()    const;
    PieceType getType()     const;
    bool      getHasMoved() const;

    // Setter
    void setHasMoved(bool moved);

    // Operator overload: equality by color + type
    bool operator==(const Piece& other) const;
    bool operator!=(const Piece& other) const;

protected:
    // Shared slide helper used by Rook, Bishop, Queen
    void addSlideMoves(std::vector<Move>& moves, Position src,
                       const Board& board,
                       const std::vector<Position>& directions) const;

private:
    Color     color;
    PieceType type;
    bool      hasMoved;
};
