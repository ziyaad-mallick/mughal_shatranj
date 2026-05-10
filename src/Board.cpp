#include "Board.h"
#include "pieces/Piece.h"
#include "pieces/King.h"
#include "pieces/Queen.h"
#include "pieces/Rook.h"
#include "pieces/Bishop.h"
#include "pieces/Knight.h"
#include "pieces/Pawn.h"
#include <iostream>

// ─── Construction ─────────────────────────────────────────────────────────────

Board::Board() : enPassantTarget(-1, -1), currentTurn(Color::White) {
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
            grid[r][c] = nullptr;

    // Both sides start with all castling rights
    castlingRights[0][0] = castlingRights[0][1] = true;
    castlingRights[1][0] = castlingRights[1][1] = true;

    initBoard();
}

void Board::initBoard() {
    auto placeBackRow = [&](int row, Color color) {
        grid[row][0] = new Rook(color);
        grid[row][1] = new Knight(color);
        grid[row][2] = new Bishop(color);
        grid[row][3] = new Queen(color);
        grid[row][4] = new King(color);
        grid[row][5] = new Bishop(color);
        grid[row][6] = new Knight(color);
        grid[row][7] = new Rook(color);
    };

    placeBackRow(0, Color::White);
    placeBackRow(7, Color::Black);

    for (int c = 0; c < 8; ++c) {
        grid[1][c] = new Pawn(Color::White);
        grid[6][c] = new Pawn(Color::Black);
    }
}

// ─── Copy / Assignment / Destructor ──────────────────────────────────────────

Piece* Board::clonePiece(Piece* p) const {
    return p ? p->clone() : nullptr;
}

void Board::copyFrom(const Board& other) {
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
            grid[r][c] = clonePiece(other.grid[r][c]);

    enPassantTarget = other.enPassantTarget;
    currentTurn     = other.currentTurn;

    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 2; ++j)
            castlingRights[i][j] = other.castlingRights[i][j];

    for (Piece* p : other.capturedPieces)
        capturedPieces.push_back(clonePiece(p));

    moveHistory = other.moveHistory;
}

Board::Board(const Board& other) {
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
            grid[r][c] = nullptr;
    copyFrom(other);
}

Board& Board::operator=(const Board& other) {
    if (this != &other) {
        clear();
        copyFrom(other);
    }
    return *this;
}

void Board::clear() {
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c) {
            delete grid[r][c];
            grid[r][c] = nullptr;
        }
    for (Piece* p : capturedPieces) delete p;
    capturedPieces.clear();
    while (!moveHistory.empty()) moveHistory.pop();
}

Board::~Board() { clear(); }

// ─── Piece access ─────────────────────────────────────────────────────────────

Piece* Board::getPieceAt(Position pos) const {
    if (!pos.isValid()) return nullptr;
    return grid[pos.row][pos.col];
}

void Board::setPieceAt(Position pos, Piece* piece) {
    if (pos.isValid()) grid[pos.row][pos.col] = piece;
}

// ─── State queries ────────────────────────────────────────────────────────────

Position Board::getEnPassantTarget()                 const { return enPassantTarget; }
bool     Board::getCastlingRight(Color c, int side)  const {
    return castlingRights[c == Color::White ? 0 : 1][side];
}
Color    Board::getCurrentTurn()                     const { return currentTurn; }
const std::vector<Piece*>& Board::getCapturedPieces()const { return capturedPieces; }
const std::stack<Move>&    Board::getMoveHistory()   const { return moveHistory; }

void Board::swapTurn() {
    currentTurn = (currentTurn == Color::White) ? Color::Black : Color::White;
}

// ─── King finder ──────────────────────────────────────────────────────────────

Position Board::findKing(Color color) const {
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
            if (grid[r][c] &&
                grid[r][c]->getType()  == PieceType::King &&
                grid[r][c]->getColor() == color)
                return {r, c};
    return {-1, -1};
}

// ─── Check detection ──────────────────────────────────────────────────────────

bool Board::isInCheck(Color color) const {
    Position kingPos = findKing(color);
    if (!kingPos.isValid()) return false;

    Color enemy = (color == Color::White) ? Color::Black : Color::White;

    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c) {
            Piece* p = grid[r][c];
            if (!p || p->getColor() != enemy) continue;
            for (const Move& m : p->getPseudoLegalMoves({r, c}, *this))
                if (m.dst == kingPos) return true;
        }
    return false;
}

// ─── Pseudo-legal move collection ─────────────────────────────────────────────

std::vector<Move> Board::getPseudoLegalMoves(Color color) const {
    std::vector<Move> all;
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c) {
            Piece* p = grid[r][c];
            if (!p || p->getColor() != color) continue;
            auto pm = p->getPseudoLegalMoves({r, c}, *this);
            all.insert(all.end(), pm.begin(), pm.end());
        }
    return all;
}

// ─── Legal move generation ────────────────────────────────────────────────────

std::vector<Move> Board::getLegalMoves(Color color) {
    std::vector<Move> legal;

    for (const Move& m : getPseudoLegalMoves(color)) {
        // Castling: king may not be in check or pass through an attacked square
        if (m.type == MoveType::CastleKingside || m.type == MoveType::CastleQueenside) {
            if (isInCheck(color)) continue;

            int backRank = (color == Color::White) ? 0 : 7;
            int passCol  = (m.type == MoveType::CastleKingside) ? 5 : 3;

            Piece* king = grid[backRank][4];
            grid[backRank][4]       = nullptr;
            grid[backRank][passCol] = king;
            bool throughCheck = isInCheck(color);
            grid[backRank][passCol] = nullptr;
            grid[backRank][4]       = king;

            if (throughCheck) continue;
        }

        BoardSnapshot snap = applyMove(m);
        bool stillInCheck  = isInCheck(color);
        undoMove(m, snap);

        if (!stillInCheck) legal.push_back(m);
    }
    return legal;
}

// ─── Apply move ───────────────────────────────────────────────────────────────

BoardSnapshot Board::applyMove(const Move& move) {
    BoardSnapshot snap;
    snap.enPassantTarget = enPassantTarget;
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 2; ++j)
            snap.castlingRights[i][j] = castlingRights[i][j];
    snap.rookMoved     = false;
    snap.rookPos       = {-1, -1};
    snap.capturedPiece = nullptr;

    Piece* moving = grid[move.src.row][move.src.col];
    snap.movingHasMoved = moving ? moving->getHasMoved() : false;

    // Reset en passant — will be set again if this is a double pawn push
    enPassantTarget = {-1, -1};

    // ── En passant capture ──
    if (move.type == MoveType::EnPassant) {
        // The captured pawn sits on the same row as the moving pawn, dst column
        snap.capturedPiece                  = grid[move.src.row][move.dst.col];
        grid[move.src.row][move.dst.col]    = nullptr;
    }

    // ── Normal capture ──
    if (move.type == MoveType::Capture) {
        snap.capturedPiece = grid[move.dst.row][move.dst.col];
        // Leave deletion to undoMove discard or game end — snapshot owns it
    }

    // ── Castling: slide the rook ──
    if (move.type == MoveType::CastleKingside || move.type == MoveType::CastleQueenside) {
        int backRank   = move.src.row;
        int rookSrcCol = (move.type == MoveType::CastleKingside) ? 7 : 0;
        int rookDstCol = (move.type == MoveType::CastleKingside) ? 5 : 3;
        Piece* rook    = grid[backRank][rookSrcCol];
        grid[backRank][rookSrcCol] = nullptr;
        grid[backRank][rookDstCol] = rook;
        snap.rookMoved = true;
        snap.rookPos   = {backRank, rookSrcCol};
        if (rook) rook->setHasMoved(true);
    }

    // ── Move the piece ──
    grid[move.dst.row][move.dst.col] = moving;
    grid[move.src.row][move.src.col] = nullptr;

    if (moving) {
        moving->setHasMoved(true);

        // Record en passant target after a double pawn push
        if (moving->getType() == PieceType::Pawn) {
            int rowDiff = move.dst.row - move.src.row;
            if (rowDiff == 2 || rowDiff == -2)
                enPassantTarget = {(move.src.row + move.dst.row) / 2, move.src.col};
        }

        // Update castling rights if king or rook moves
        int ci = (moving->getColor() == Color::White) ? 0 : 1;
        if (moving->getType() == PieceType::King) {
            castlingRights[ci][0] = castlingRights[ci][1] = false;
        }
        if (moving->getType() == PieceType::Rook) {
            int backRank = (moving->getColor() == Color::White) ? 0 : 7;
            if (move.src.row == backRank) {
                if (move.src.col == 0) castlingRights[ci][0] = false;
                if (move.src.col == 7) castlingRights[ci][1] = false;
            }
        }
    }

    // ── Promotion: swap pawn for chosen piece ──
    if (move.type == MoveType::Promotion) {
        Color c = moving ? moving->getColor() : currentTurn;
        delete grid[move.dst.row][move.dst.col]; // delete the pawn we just moved
        Piece* promoted = nullptr;
        switch (move.promotionPiece) {
            case PieceType::Queen:  promoted = new Queen(c);  break;
            case PieceType::Rook:   promoted = new Rook(c);   break;
            case PieceType::Bishop: promoted = new Bishop(c); break;
            case PieceType::Knight: promoted = new Knight(c); break;
            default:                promoted = new Queen(c);  break;
        }
        if (promoted) promoted->setHasMoved(true);
        grid[move.dst.row][move.dst.col] = promoted;
    }

    moveHistory.push(move);
    return snap;
}

// ─── Undo move ────────────────────────────────────────────────────────────────

void Board::undoMove(const Move& move, const BoardSnapshot& snap) {
    if (!moveHistory.empty()) moveHistory.pop();

    // Restore en passant and castling rights exactly as before
    enPassantTarget = snap.enPassantTarget;
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 2; ++j)
            castlingRights[i][j] = snap.castlingRights[i][j];

    // ── Promotion undo: delete promoted piece, recreate pawn ──
    if (move.type == MoveType::Promotion) {
        delete grid[move.dst.row][move.dst.col];
        // Determine color from the side that moved (snapshot's moving piece is gone)
        Color c = (move.dst.row == 7) ? Color::White : Color::Black;
        Pawn* pawn = new Pawn(c);
        pawn->setHasMoved(snap.movingHasMoved);
        grid[move.src.row][move.src.col] = pawn;
        grid[move.dst.row][move.dst.col] = snap.capturedPiece; // nullptr for non-capture
        // Castling undo (promotions can't castle, so rookMoved is false here)
        return;
    }

    // Move piece back to source
    Piece* moving = grid[move.dst.row][move.dst.col];
    grid[move.src.row][move.src.col] = moving;
    grid[move.dst.row][move.dst.col] = snap.capturedPiece; // nullptr if not a capture

    // Restore hasMoved flag precisely
    if (moving) moving->setHasMoved(snap.movingHasMoved);

    // ── En passant undo: restore captured pawn to its square ──
    if (move.type == MoveType::EnPassant) {
        grid[move.dst.row][move.dst.col] = nullptr;               // dst is empty after undo
        grid[move.src.row][move.dst.col] = snap.capturedPiece;   // pawn goes back to side square
    }

    // ── Castling undo: slide rook back ──
    if (snap.rookMoved) {
        int rookDstCol = (move.type == MoveType::CastleKingside) ? 5 : 3;
        int backRank   = snap.rookPos.row;
        Piece* rook    = grid[backRank][rookDstCol];
        grid[backRank][rookDstCol]                          = nullptr;
        grid[snap.rookPos.row][snap.rookPos.col]            = rook;
        if (rook) rook->setHasMoved(false); // rook never moved before castling
    }
}

// ─── Terminal conditions ──────────────────────────────────────────────────────

bool Board::isCheckmate(Color color) {
    return getLegalMoves(color).empty() && isInCheck(color);
}

bool Board::isStalemate(Color color) {
    return getLegalMoves(color).empty() && !isInCheck(color);
}

// ─── ASCII debug output ───────────────────────────────────────────────────────

std::ostream& operator<<(std::ostream& os, const Board& b) {
    auto pieceChar = [](Piece* p) -> char {
        if (!p) return '.';
        bool w = (p->getColor() == Color::White);
        switch (p->getType()) {
            case PieceType::King:   return w ? 'K' : 'k';
            case PieceType::Queen:  return w ? 'Q' : 'q';
            case PieceType::Rook:   return w ? 'R' : 'r';
            case PieceType::Bishop: return w ? 'B' : 'b';
            case PieceType::Knight: return w ? 'N' : 'n';
            case PieceType::Pawn:   return w ? 'P' : 'p';
        }
        return '?';
    };

    os << "  a b c d e f g h\n";
    for (int r = 7; r >= 0; --r) {
        os << (r + 1) << ' ';
        for (int c = 0; c < 8; ++c)
            os << pieceChar(b.grid[r][c]) << ' ';
        os << (r + 1) << '\n';
    }
    os << "  a b c d e f g h\n";
    os << "Turn: " << (b.currentTurn == Color::White ? "White" : "Black") << '\n';
    return os;
}
