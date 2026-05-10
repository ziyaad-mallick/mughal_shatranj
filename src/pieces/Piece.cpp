#include "Piece.h"
#include "../Board.h"

Piece::Piece(Color color, PieceType type)
    : color(color), type(type), hasMoved(false) {}

Color     Piece::getColor()    const { return color; }
PieceType Piece::getType()     const { return type; }
bool      Piece::getHasMoved() const { return hasMoved; }

void Piece::setHasMoved(bool moved) { hasMoved = moved; }

bool Piece::operator==(const Piece& other) const {
    return color == other.color && type == other.type;
}

bool Piece::operator!=(const Piece& other) const {
    return !(*this == other);
}

void Piece::addSlideMoves(std::vector<Move>& moves, Position src,
                          const Board& board,
                          const std::vector<Position>& directions) const {
    for (const Position& dir : directions) {
        Position cur = src + dir;
        while (cur.isValid()) {
            Piece* target = board.getPieceAt(cur);
            if (target == nullptr) {
                moves.emplace_back(src, cur, MoveType::Normal);
            } else {
                if (target->getColor() != color) {
                    moves.emplace_back(src, cur, MoveType::Capture);
                }
                break; // blocked whether friendly or enemy
            }
            cur = cur + dir;
        }
    }
}
