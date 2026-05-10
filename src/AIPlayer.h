#pragma once
#include "Player.h"

class AIPlayer : public Player {
public:
    explicit AIPlayer(Color color, int depth = 3);

    // Returns the best move found by Minimax + Alpha-Beta pruning
    Move chooseMove(const Board& board) override;

private:
    int searchDepth;

    // Recursive Minimax — returns a score from White's perspective
    int minimax(Board& board, int depth, int alpha, int beta, bool isMaximising);

    // Material evaluation: positive = good for White, negative = good for Black
    int evaluate(const Board& board) const;

    // Material value of each piece type
    static int pieceValue(PieceType type);
};
