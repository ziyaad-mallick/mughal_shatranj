#pragma once
#include "primitives/Move.h"
#include "primitives/Enums.h"

class Board;

// Abstract base class — second polymorphic hierarchy alongside Piece
class Player {
public:
    explicit Player(Color color) : color(color) {}
    virtual ~Player() = default;

    // Pure virtual — HumanPlayer and AIPlayer each implement differently
    virtual Move chooseMove(const Board& board) = 0;

    // Lets Game distinguish human turns (for event routing) without dynamic_cast
    virtual bool isHuman() const { return false; }

    Color getColor() const { return color; }

private:
    Color color;
};
