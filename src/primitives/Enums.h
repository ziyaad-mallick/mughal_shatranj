#pragma once

enum class Color { White, Black };

enum class PieceType { King, Queen, Rook, Bishop, Knight, Pawn };

enum class MoveType {
    Normal,
    Capture,
    EnPassant,
    CastleKingside,
    CastleQueenside,
    Promotion
};
