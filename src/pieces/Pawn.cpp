#include "Pawn.h"
#include "../Board.h"

Pawn::Pawn(Color color) : Piece(color, PieceType::Pawn) {}

Piece* Pawn::clone() const { return new Pawn(*this); }

std::vector<Move> Pawn::getPseudoLegalMoves(Position pos, const Board& board) const {
    std::vector<Move> moves;
    Color myColor = getColor();

    // White moves toward higher row indices (row 1→7), Black toward lower (row 6→0)
    int dir      = (myColor == Color::White) ? 1 : -1;
    int promRank = (myColor == Color::White) ? 7 : 0;

    auto addMove = [&](Position src, Position dst, MoveType type) {
        if (dst.row == promRank) {
            // Promotion — one move entry per promotable piece
            for (PieceType pt : {PieceType::Queen, PieceType::Rook,
                                  PieceType::Bishop, PieceType::Knight}) {
                moves.emplace_back(src, dst, MoveType::Promotion, pt);
            }
        } else {
            moves.emplace_back(src, dst, type);
        }
    };

    // Single push
    Position fwd(pos.row + dir, pos.col);
    if (fwd.isValid() && !board.getPieceAt(fwd)) {
        addMove(pos, fwd, MoveType::Normal);

        // Double push — only from starting rank, both squares must be empty
        int startRank = (myColor == Color::White) ? 1 : 6;
        if (pos.row == startRank) {
            Position fwd2(pos.row + 2 * dir, pos.col);
            if (fwd2.isValid() && !board.getPieceAt(fwd2)) {
                moves.emplace_back(pos, fwd2, MoveType::Normal);
            }
        }
    }

    // Diagonal captures (including en passant)
    for (int dc : {-1, 1}) {
        Position diag(pos.row + dir, pos.col + dc);
        if (!diag.isValid()) continue;

        Piece* target = board.getPieceAt(diag);
        if (target && target->getColor() != myColor) {
            addMove(pos, diag, MoveType::Capture);
        } else if (diag == board.getEnPassantTarget()) {
            moves.emplace_back(pos, diag, MoveType::EnPassant);
        }
    }

    return moves;
}
