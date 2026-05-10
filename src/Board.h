#pragma once

#include "primitives/Position.h"
#include "primitives/Move.h"
#include "primitives/Enums.h"
#include <vector>
#include <stack>
#include <ostream>

class Piece;

// Snapshot of extra state needed to fully undo a move
struct BoardSnapshot {
    Piece*   capturedPiece;        // piece removed from board (nullptr if none)
    Position enPassantTarget;      // en passant target before the move
    bool     castlingRights[2][2]; // [color][side]: [0]=queenside [1]=kingside
    bool     rookMoved;            // whether a rook's hasMoved changed this move
    Position rookPos;              // which rook's original square (for castling undo)
    bool     movingHasMoved;       // hasMoved state of the moving piece before this move
};

class Board {
public:
    Board();
    Board(const Board& other);           // deep copy — critical for AI search
    Board& operator=(const Board& other);
    ~Board();

    // --- Piece access ---
    Piece* getPieceAt(Position pos) const;
    void   setPieceAt(Position pos, Piece* piece);

    // --- State queries ---
    Position getEnPassantTarget() const;
    bool     getCastlingRight(Color color, int side) const; // side: 0=queenside 1=kingside
    Color    getCurrentTurn() const;
    const std::vector<Piece*>& getCapturedPieces() const;
    const std::stack<Move>&    getMoveHistory() const;

    // --- Move generation & validation ---
    std::vector<Move> getPseudoLegalMoves(Color color) const;
    std::vector<Move> getLegalMoves(Color color);
    bool              isInCheck(Color color) const;
    bool              isCheckmate(Color color);
    bool              isStalemate(Color color);

    // --- Move execution ---
    BoardSnapshot applyMove(const Move& move);
    void          undoMove(const Move& move, const BoardSnapshot& snap);

    // --- Utility ---
    void swapTurn();
    Position findKing(Color color) const;

    // ASCII debug output — operator overload
    friend std::ostream& operator<<(std::ostream& os, const Board& b);

private:
    Piece*   grid[8][8];
    Position enPassantTarget;
    bool     castlingRights[2][2]; // [White=0,Black=1][queenside=0,kingside=1]
    Color    currentTurn;
    std::vector<Piece*> capturedPieces;
    std::stack<Move>    moveHistory;

    void initBoard();
    void copyFrom(const Board& other);
    void clear();
    Piece* clonePiece(Piece* p) const;
};
