#pragma once

#include "Position.h"
#include "Enums.h"
#include <ostream>

struct Move {
    Position    src;
    Position    dst;
    MoveType    type;
    PieceType   promotionPiece;

    Move()
        : src(), dst(), type(MoveType::Normal), promotionPiece(PieceType::Queen) {}

    Move(Position s, Position d, MoveType t = MoveType::Normal,
         PieceType promo = PieceType::Queen)
        : src(s), dst(d), type(t), promotionPiece(promo) {}

    bool operator==(const Move& other) const {
        return src == other.src && dst == other.dst && type == other.type;
    }

    // Algebraic notation: e.g. "e2e4", "e7e8q" for promotion
    friend std::ostream& operator<<(std::ostream& os, const Move& m) {
        auto toFile = [](int col) -> char { return static_cast<char>('a' + col); };
        auto toRank = [](int row) -> char { return static_cast<char>('1' + row); };

        os << toFile(m.src.col) << toRank(m.src.row)
           << toFile(m.dst.col) << toRank(m.dst.row);

        if (m.type == MoveType::Promotion) {
            switch (m.promotionPiece) {
                case PieceType::Queen:  os << 'q'; break;
                case PieceType::Rook:   os << 'r'; break;
                case PieceType::Bishop: os << 'b'; break;
                case PieceType::Knight: os << 'n'; break;
                default: break;
            }
        }
        return os;
    }
};
