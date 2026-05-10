#pragma once

struct Position {
    int row;
    int col;

    Position() : row(-1), col(-1) {}
    Position(int r, int c) : row(r), col(c) {}

    bool isValid() const {
        return row >= 0 && row < 8 && col >= 0 && col < 8;
    }

    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }

    bool operator!=(const Position& other) const {
        return !(*this == other);
    }

    Position operator+(const Position& delta) const {
        return Position(row + delta.row, col + delta.col);
    }
};
