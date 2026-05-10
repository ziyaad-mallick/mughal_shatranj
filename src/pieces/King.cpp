#include "King.h"
#include "../Board.h"

King::King(Color color) : Piece(color, PieceType::King) {}

Piece* King::clone() const { return new King(*this); }

std::vector<Move> King::getPseudoLegalMoves(Position pos, const Board& board) const {
    std::vector<Move> moves;
    Color myColor = getColor();

    // 8 one-step directions
    static const int dr[] = {-1,-1,-1, 0, 0, 1, 1, 1};
    static const int dc[] = {-1, 0, 1,-1, 1,-1, 0, 1};

    for (int i = 0; i < 8; ++i) {
        Position dst(pos.row + dr[i], pos.col + dc[i]);
        if (!dst.isValid()) continue;
        Piece* target = board.getPieceAt(dst);
        if (target == nullptr) {
            moves.emplace_back(pos, dst, MoveType::Normal);
        } else if (target->getColor() != myColor) {
            moves.emplace_back(pos, dst, MoveType::Capture);
        }
    }

    // Castling — king and rook must not have moved, squares must be empty
    // side: 0=queenside, 1=kingside
    if (!getHasMoved()) {
        int backRank = (myColor == Color::White) ? 0 : 7;

        // Kingside: rook on col 7, empty cols 5 & 6
        if (board.getCastlingRight(myColor, 1)) {
            Position rookPos(backRank, 7);
            Piece* rook = board.getPieceAt(rookPos);
            if (rook && rook->getType() == PieceType::Rook &&
                rook->getColor() == myColor && !rook->getHasMoved()) {
                if (!board.getPieceAt({backRank, 5}) &&
                    !board.getPieceAt({backRank, 6})) {
                    moves.emplace_back(pos, Position(backRank, 6), MoveType::CastleKingside);
                }
            }
        }

        // Queenside: rook on col 0, empty cols 1, 2 & 3
        if (board.getCastlingRight(myColor, 0)) {
            Position rookPos(backRank, 0);
            Piece* rook = board.getPieceAt(rookPos);
            if (rook && rook->getType() == PieceType::Rook &&
                rook->getColor() == myColor && !rook->getHasMoved()) {
                if (!board.getPieceAt({backRank, 1}) &&
                    !board.getPieceAt({backRank, 2}) &&
                    !board.getPieceAt({backRank, 3})) {
                    moves.emplace_back(pos, Position(backRank, 2), MoveType::CastleQueenside);
                }
            }
        }
    }

    return moves;
}
