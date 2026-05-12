#include "AIPlayer.h"
#include "Board.h"
#include "pieces/Piece.h"
#include <limits>
#include <algorithm>

AIPlayer::AIPlayer(Color color, int depth)
    : Player(color), searchDepth(depth) {}

// ─── Piece values ─────────────────────────────────────────────────────────────
int AIPlayer::pieceValue(PieceType type) {
    switch (type) {
        case PieceType::Pawn:   return 100;
        case PieceType::Knight: return 300;
        case PieceType::Bishop: return 300;
        case PieceType::Rook:   return 500;
        case PieceType::Queen:  return 900;
        case PieceType::King:   return 20000;
    }
    return 0;
}

// ─── Board evaluation (from White's perspective) ──────────────────────────────
// Positive score = good for White. Negative = good for Black.
int AIPlayer::evaluate(const Board& board) const {
    int score = 0;
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c) {
            Piece* p = board.getPieceAt({r, c});
            if (!p) continue;
            int val = pieceValue(p->getType());
            score += (p->getColor() == Color::White) ? val : -val;
        }
    return score;
}

// ─── Minimax with Alpha-Beta pruning ─────────────────────────────────────────
// White maximises, Black minimises.
// alpha = best score White can guarantee so far
// beta  = best score Black can guarantee so far
// If beta <= alpha: prune — this branch won't be chosen by the opponent
int AIPlayer::minimax(Board& board, int depth, int alpha, int beta, bool isMaximising) {
    Color side = isMaximising ? Color::White : Color::Black;

    // Terminal state checks (before depth 0 to score immediately)
    if (board.isCheckmate(side)) {
        return isMaximising ? -100000 : 100000; // losing side
    }
    if (board.isStalemate(side)) {
        return 0; // draw
    }

    // Base case: depth exhausted — return static evaluation
    if (depth == 0) return evaluate(board);

    auto moves = board.getLegalMoves(side);
    if (moves.empty()) return evaluate(board);

    if (isMaximising) {
        int best = std::numeric_limits<int>::min();
        for (const Move& m : moves) {
            auto snap = board.applyMove(m);
            best = std::max(best, minimax(board, depth - 1, alpha, beta, false));
            board.undoMove(m, snap);
            alpha = std::max(alpha, best);
            if (beta <= alpha) break; // Beta cut-off — prune
        }
        return best;
    } else {
        int best = std::numeric_limits<int>::max();
        for (const Move& m : moves) {
            auto snap = board.applyMove(m);
            best = std::min(best, minimax(board, depth - 1, alpha, beta, true));
            board.undoMove(m, snap);
            beta = std::min(beta, best);
            if (beta <= alpha) break; // Alpha cut-off — prune
        }
        return best;
    }
}

// ─── Choose best move ─────────────────────────────────────────────────────────
Move AIPlayer::chooseMove(const Board& board) {
    // Work on a mutable copy — AI must never modify the real board
    Board workBoard(board);

    bool isMaximising = (getColor() == Color::White);
    auto moves = workBoard.getLegalMoves(getColor());
    if (moves.empty()) return Move(); // no legal move (shouldn't happen in normal play)

    Move bestMove = moves[0];
    int  bestScore = isMaximising ? std::numeric_limits<int>::min()
                                  : std::numeric_limits<int>::max();

    int alpha = std::numeric_limits<int>::min();
    int beta  = std::numeric_limits<int>::max();

    for (const Move& m : moves) {
        auto snap = workBoard.applyMove(m);
        int score = minimax(workBoard, searchDepth - 1, alpha, beta, !isMaximising);
        workBoard.undoMove(m, snap);

        if (isMaximising ? (score > bestScore) : (score < bestScore)) {
            bestScore = score;
            bestMove  = m;
        }
        if (isMaximising) alpha = std::max(alpha, bestScore);
        else              beta  = std::min(beta,  bestScore);
    }

    return bestMove; 
}
