#include "Renderer.h"
#include "pieces/Piece.h"
#include <sstream>
#include <iomanip>
#include <stack>
#include <algorithm>

// ── Mughlia Shatranj colour palette ──────────────────────────────────────────
// Board squares
static const sf::Color LIGHT_SQ   {235, 220, 195};   // warm beige
static const sf::Color DARK_SQ    { 50,  50,  50};   // charcoal

// Board overlay / highlights
static const sf::Color HIGHLIGHT  {255, 210,  40, 210};  // gold – selected square
static const sf::Color LEGAL_DOT  {255, 215,  70, 170};  // gold dot – legal move
static const sf::Color LAST_MOVE  {200, 165,  30, 110};  // amber tint – last move
static const sf::Color CHECK_CLR  {210,  30,  30, 190};  // crimson – king in check

// Sidebar
static const sf::Color SIDEBAR_BG { 20,  20,  20};       // near-black panel

// Fallback piece colours (used only when sprites not loaded)
static const sf::Color WHITE_PIECE{212, 160,  23};   // antique gold
static const sf::Color BLACK_PIECE{ 50, 180,  50};   // forest green

Renderer::Renderer(sf::RenderWindow& win) : window(win) {}

// ─── Font ──────────────────────────────────────────────────────────────────────
bool Renderer::loadFont(const std::string& path) {
    fontLoaded = font.openFromFile(path);
    return fontLoaded;
}

// ─── Sprite loading ───────────────────────────────────────────────────────────
bool Renderer::loadPieceSprites(const std::string& dir) {
    // Expects files named wK.png, wQ.png, wR.png, wB.png, wN.png, wP.png
    //                      bK.png, bQ.png, bR.png, bB.png, bN.png, bP.png
    static const char* keys[] = {
        "wK","wQ","wR","wB","wN","wP",
        "bK","bQ","bR","bB","bN","bP"
    };
    int loaded = 0;
    for (const char* k : keys) {
        std::string path = dir + "/" + k + ".png";
        sf::Texture tex;
        if (tex.loadFromFile(path)) {
            tex.setSmooth(true);
            textures[k] = std::move(tex);
            ++loaded;
        }
    }
    spritesLoaded = (loaded == 12);
    return spritesLoaded;
}

// ─── Sprite key ("wK", "bN", etc.) ───────────────────────────────────────────
std::string Renderer::spriteKey(Piece* piece) {
    std::string key;
    key += (piece->getColor() == Color::White) ? 'w' : 'b';
    key += pieceChar(piece->getType());
    return key;
}

char Renderer::pieceChar(PieceType t) {
    switch (t) {
        case PieceType::King:   return 'K';
        case PieceType::Queen:  return 'Q';
        case PieceType::Rook:   return 'R';
        case PieceType::Bishop: return 'B';
        case PieceType::Knight: return 'N';
        case PieceType::Pawn:   return 'P';
    }
    return '?';
}

// ─── Coordinate helpers ────────────────────────────────────────────────────────
// Row 0 = White's back rank = bottom of screen (row 7 drawn at y=0)
sf::Vector2f Renderer::toPixel(Position pos) const {
    return { static_cast<float>(pos.col * SQUARE_SIZE),
             static_cast<float>((7 - pos.row) * SQUARE_SIZE) };
}

// ─── Square / highlight drawing ───────────────────────────────────────────────
void Renderer::drawSquares(const Board& board,
                           Position selected, const std::vector<Move>& legalMoves,
                           Position lastSrc,  Position lastDst) {

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            Position pos{r, c};
            sf::Vector2f tl = toPixel(pos);
            bool isLight = (r + c) % 2 == 0;

            sf::RectangleShape sq({(float)SQUARE_SIZE, (float)SQUARE_SIZE});
            sq.setPosition(tl);
            sq.setFillColor(isLight ? LIGHT_SQ : DARK_SQ);
            window.draw(sq);

            // Last-move tint
            if (pos == lastSrc || pos == lastDst) {
                sq.setFillColor(LAST_MOVE);
                window.draw(sq);
            }
            // Selected highlight
            if (pos == selected) {
                sq.setFillColor(HIGHLIGHT);
                window.draw(sq);
            }
            // King-in-check highlight
            Piece* p = board.getPieceAt(pos);
            if (p && p->getType() == PieceType::King) {
                Board& mb = const_cast<Board&>(board);
                if (mb.isInCheck(p->getColor())) {
                    sq.setFillColor(CHECK_CLR);
                    window.draw(sq);
                }
            }

            // ── Rank/File coordinate labels ──────────────────────────────────
            if (fontLoaded) {
                // Coord colour contrasts with its own square
                sf::Color cc = isLight ? sf::Color(100, 80, 55) : sf::Color(175, 170, 160);
                // File letters (a-h) on bottom row
                if (r == 0) {
                    sf::Text lbl(font, std::string(1, static_cast<char>('a' + c)), 14);
                    lbl.setFillColor(cc);
                    lbl.setPosition({tl.x + 5.f, tl.y + SQUARE_SIZE - 20.f});
                    window.draw(lbl);
                }
                // Rank numbers (1-8) on left column
                if (c == 0) {
                    sf::Text lbl(font, std::to_string(r + 1), 14);
                    lbl.setFillColor(cc);
                    lbl.setPosition({tl.x + 5.f, tl.y + 5.f});
                    window.draw(lbl);
                }
            }
        }
    }

    // ── Thin gold separator between board and sidebar ─────────────────────────
    sf::RectangleShape sep({2.f, (float)BOARD_PIXELS});
    sep.setPosition({(float)BOARD_PIXELS, 0.f});
    sep.setFillColor(sf::Color(180, 145, 40, 180));
    window.draw(sep);

    // ── Legal-move indicators (drawn on top of squares) ───────────────────────
    for (const Move& m : legalMoves) {
        sf::Vector2f tl = toPixel(m.dst);
        if (board.getPieceAt(m.dst)) {
            // Capture: ring
            sf::CircleShape ring(SQUARE_SIZE / 2.f - 5.f);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineThickness(6.f);
            ring.setOutlineColor(LEGAL_DOT);
            ring.setPosition(tl);
            window.draw(ring);
        } else {
            // Empty: dot in centre
            float dotR = 14.f;
            sf::CircleShape dot(dotR);
            dot.setFillColor(LEGAL_DOT);
            dot.setPosition(tl + sf::Vector2f(SQUARE_SIZE / 2.f - dotR,
                                              SQUARE_SIZE / 2.f - dotR));
            window.draw(dot);
        }
    }
}

// ─── Sprite piece renderer ────────────────────────────────────────────────────
void Renderer::drawPiece(Piece* piece, sf::Vector2f topLeft) {
    if (!piece) return;

    if (spritesLoaded) {
        auto it = textures.find(spriteKey(piece));
        if (it != textures.end()) {
            sf::Sprite sprite(it->second);
            // Scale sprite to fit the square
            sf::Vector2u ts = it->second.getSize();
            sprite.setScale({ static_cast<float>(SQUARE_SIZE) / ts.x,
                               static_cast<float>(SQUARE_SIZE) / ts.y });
            sprite.setPosition(topLeft);
            window.draw(sprite);
            return;
        }
    }
    // Fallback if sprites not loaded
    drawPieceFallback(piece, topLeft);
}

// ─── Fallback: coloured circle + letter ──────────────────────────────────────
void Renderer::drawPieceFallback(Piece* piece, sf::Vector2f topLeft) {
    bool isWhite = (piece->getColor() == Color::White);
    float radius = SQUARE_SIZE / 2.f - 8.f;
    sf::Vector2f centre = topLeft + sf::Vector2f(SQUARE_SIZE / 2.f, SQUARE_SIZE / 2.f);

    sf::CircleShape circle(radius);
    circle.setOrigin({radius, radius});
    circle.setPosition(centre);
    circle.setFillColor(isWhite ? WHITE_PIECE : BLACK_PIECE);
    circle.setOutlineThickness(2.5f);
    circle.setOutlineColor(isWhite ? sf::Color(80, 80, 80) : sf::Color(200, 200, 200));
    window.draw(circle);

    if (!fontLoaded) return;
    std::string label(1, pieceChar(piece->getType()));
    sf::Text text(font, label, static_cast<unsigned>(SQUARE_SIZE / 2 - 2));
    text.setFillColor(isWhite ? sf::Color::Black : sf::Color::White);
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin({bounds.position.x + bounds.size.x / 2.f,
                    bounds.position.y + bounds.size.y / 2.f});
    text.setPosition(centre);
    window.draw(text);
}

// ─── All pieces ───────────────────────────────────────────────────────────────
void Renderer::drawPieces(const Board& board, Position skip) {
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c) {
            Position pos{r, c};
            if (pos == skip) continue;   // dragged piece drawn separately at cursor
            Piece* p = board.getPieceAt(pos);
            if (p) drawPiece(p, toPixel(pos));
        }
}

// ─── Sidebar ───────────────────────────────────────────────────────────────────
void Renderer::drawSidebar(const Board& board, const std::string& statusMsg,
                            int whiteElapsed, int blackElapsed) {

    // ── Background ──────────────────────────────────────────────────────────
    sf::RectangleShape bg({(float)SIDEBAR_WIDTH, (float)WINDOW_H});
    bg.setPosition({(float)BOARD_PIXELS, 0.f});
    bg.setFillColor(SIDEBAR_BG);
    window.draw(bg);

    if (!fontLoaded) return;

    // Shared geometry
    const float PX  = (float)BOARD_PIXELS + 24.f;            // left text margin
    const float PXR = (float)BOARD_PIXELS + SIDEBAR_WIDTH - 24.f; // right edge
    const float W   = PXR - PX;

    // ── Helpers ─────────────────────────────────────────────────────────────

    // Draw text at (px, py), return rendered height
    auto txt = [&](const std::string& s, float px, float py,
                   unsigned sz, sf::Color col) -> float {
        if (s.empty()) return 0.f;
        sf::Text t(font, s, sz);
        t.setFillColor(col);
        t.setPosition({px, py});
        window.draw(t);
        return t.getLocalBounds().size.y;
    };

    // Centred text inside the sidebar
    auto ctxt = [&](const std::string& s, float py, unsigned sz, sf::Color col) {
        sf::Text t(font, s, sz);
        t.setFillColor(col);
        float tw = t.getLocalBounds().size.x;
        t.setPosition({(float)BOARD_PIXELS + ((float)SIDEBAR_WIDTH - tw) / 2.f, py});
        window.draw(t);
    };

    // Horizontal rule
    auto rule = [&](float py, sf::Color col = sf::Color(60, 58, 52)) {
        sf::RectangleShape line({W, 1.f});
        line.setPosition({PX, py});
        line.setFillColor(col);
        window.draw(line);
    };

    // Section label (small, dim, all-caps)
    auto section = [&](const std::string& s, float& y) {
        txt(s, PX, y, 11, sf::Color(110, 105, 90));
        y += 19.f;
    };

    // Format seconds as MM:SS
    auto fmt = [](int secs) -> std::string {
        char buf[8];
        std::snprintf(buf, sizeof(buf), "%02d:%02d", secs / 60, secs % 60);
        return buf;
    };

    // Algebraic square name
    auto alg = [](Position p) -> std::string {
        std::string s;
        s += (char)('a' + p.col);
        s += (char)('1' + p.row);
        return s;
    };

    float Y = 26.f;

    // ════════════════════════════════════════════════════════════════
    // TITLE
    // ════════════════════════════════════════════════════════════════
    ctxt("MUGHLIA SHATRANJ", Y, 18, sf::Color(212, 175, 55));
    Y += 32.f;
    rule(Y, sf::Color(90, 75, 30));
    Y += 18.f;

    // ════════════════════════════════════════════════════════════════
    // TURN INDICATOR
    // ════════════════════════════════════════════════════════════════
    bool whiteTurn = (board.getCurrentTurn() == Color::White);

    // Coloured dot
    const float dotR = 8.f;
    sf::CircleShape dot(dotR);
    dot.setOrigin({dotR, dotR});
    dot.setFillColor(whiteTurn ? sf::Color(220, 185, 50) : sf::Color(75, 200, 75));
    dot.setPosition({PX + dotR, Y + 12.f});
    window.draw(dot);

    txt(whiteTurn ? "WHITE'S TURN" : "BLACK'S TURN",
        PX + dotR * 2.f + 10.f, Y + 3.f, 17,
        sf::Color(235, 235, 235));
    Y += 34.f;

    // Status line (check / checkmate)
    if (!statusMsg.empty()) {
        txt(statusMsg, PX, Y, 13, sf::Color(255, 100, 85));
        Y += 22.f;
    }

    Y += 4.f;
    rule(Y);
    Y += 18.f;

    // ════════════════════════════════════════════════════════════════
    // CLOCKS
    // ════════════════════════════════════════════════════════════════
    section("CLOCKS", Y);

    // White row
    {
        txt("White", PX, Y, 15, sf::Color(210, 185, 80));
        std::string wt = fmt(whiteElapsed);
        sf::Text wtt(font, wt, 15);
        sf::Color wtc = whiteTurn  ? sf::Color(255, 220, 60) : sf::Color(160, 155, 130);
        wtt.setFillColor(wtc);
        wtt.setPosition({PXR - wtt.getLocalBounds().size.x, Y});
        window.draw(wtt);
        Y += 26.f;
    }
    // Black row
    {
        txt("Black", PX, Y, 15, sf::Color(80, 200, 80));
        std::string bt = fmt(blackElapsed);
        sf::Text btt(font, bt, 15);
        sf::Color btc = !whiteTurn ? sf::Color(90, 230, 90)  : sf::Color(115, 130, 115);
        btt.setFillColor(btc);
        btt.setPosition({PXR - btt.getLocalBounds().size.x, Y});
        window.draw(btt);
        Y += 28.f;
    }

    rule(Y);
    Y += 18.f;

    // ════════════════════════════════════════════════════════════════
    // LAST MOVE
    // ════════════════════════════════════════════════════════════════
    section("LAST MOVE", Y);

    {
        std::stack<Move> tmp = board.getMoveHistory();
        std::string moveStr = "--";
        if (!tmp.empty()) {
            Move lm = tmp.top();
            moveStr = alg(lm.src) + " -> " + alg(lm.dst);
        }
        txt(moveStr, PX + 4.f, Y, 17, sf::Color(215, 205, 175));
        Y += 32.f;
    }

    rule(Y);
    Y += 18.f;

    // ════════════════════════════════════════════════════════════════
    // CAPTURES
    // ════════════════════════════════════════════════════════════════
    const auto& captured = board.getCapturedPieces();
    std::string wCap, bCap;   // wCap = pieces White captured (Black's pieces)
                               // bCap = pieces Black captured (White's pieces)
    for (Piece* p : captured) {
        char ch = pieceChar(p->getType());
        if (p->getColor() == Color::Black) { wCap += ch; wCap += ' '; }
        else                               { bCap += ch; bCap += ' '; }
    }

    section("WHITE CAPTURES", Y);
    txt(wCap.empty() ? "--" : wCap, PX + 4.f, Y, 15, sf::Color(220, 215, 200));
    Y += 28.f;

    section("BLACK CAPTURES", Y);
    txt(bCap.empty() ? "--" : bCap, PX + 4.f, Y, 15, sf::Color(220, 215, 200));
}

// ─── Main render call ─────────────────────────────────────────────────────────
void Renderer::render(const Board& board,
                      Position selected, const std::vector<Move>& legalMoves,
                      Position lastSrc, Position lastDst,
                      const std::string& statusMsg,
                      int whiteElapsed, int blackElapsed,
                      bool isDragging, Position dragFrom,
                      int dragPx, int dragPy) {
    window.clear(SIDEBAR_BG);
    drawSquares(board, selected, legalMoves, lastSrc, lastDst);
    drawPieces(board, isDragging ? dragFrom : Position{-1,-1});

    // ── Dragged piece — drawn last so it floats above everything ─────────────
    if (isDragging && dragFrom.isValid()) {
        Piece* dp = board.getPieceAt(dragFrom);
        if (dp) {
            // Shadow: same sprite, dark tint, offset a few pixels
            const float halfSz = SQUARE_SIZE / 2.f;
            const float SCALE  = 1.08f;                         // slightly enlarged
            const float drawSz = SQUARE_SIZE * SCALE;

            if (spritesLoaded) {
                auto it = textures.find(spriteKey(dp));
                if (it != textures.end()) {
                    sf::Sprite shadow(it->second);
                    float scl = drawSz / it->second.getSize().x;
                    shadow.setScale({scl, scl});
                    shadow.setColor(sf::Color(0, 0, 0, 80));
                    shadow.setPosition({(float)dragPx - halfSz + 5.f,
                                        (float)dragPy - halfSz + 7.f});
                    window.draw(shadow);

                    sf::Sprite piece(it->second);
                    piece.setScale({scl, scl});
                    piece.setPosition({(float)dragPx - halfSz,
                                       (float)dragPy - halfSz});
                    window.draw(piece);
                } else {
                    drawPiece(dp, {(float)dragPx - halfSz, (float)dragPy - halfSz});
                }
            } else {
                drawPiece(dp, {(float)dragPx - halfSz, (float)dragPy - halfSz});
            }
        }
    }

    drawSidebar(board, statusMsg, whiteElapsed, blackElapsed);
}

// ─── Menu ─────────────────────────────────────────────────────────────────────
void Renderer::drawMenu(const std::string& title,
                        const std::vector<std::string>& options) {
    window.clear(sf::Color(30, 30, 30));
    if (!fontLoaded) { window.display(); return; }

    float y = 100.f;
    sf::Text titleText(font, title, 42);
    titleText.setFillColor(sf::Color(220, 180, 50));
    sf::FloatRect tb = titleText.getLocalBounds();
    titleText.setPosition({(WINDOW_W - tb.size.x) / 2.f, y});
    window.draw(titleText);
    y += 90.f;

    for (const std::string& opt : options) {
        sf::Text t(font, opt, 24);
        t.setFillColor(sf::Color::White);
        sf::FloatRect rb = t.getLocalBounds();
        t.setPosition({(WINDOW_W - rb.size.x) / 2.f, y});
        window.draw(t);
        y += 48.f;
    }
    window.display();
}

// ─── Full-screen message overlay ──────────────────────────────────────────────
void Renderer::drawMessage(const std::string& msg) {
    if (!fontLoaded) return;
    // Semi-transparent dark background over the board
    sf::RectangleShape overlay({static_cast<float>(WINDOW_W),
                                static_cast<float>(WINDOW_H)});
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    window.draw(overlay);

    sf::Text t(font, msg, 26);
    t.setFillColor(sf::Color::Yellow);
    sf::FloatRect b = t.getLocalBounds();
    t.setPosition({(WINDOW_W - b.size.x) / 2.f,
                   (WINDOW_H - b.size.y) / 2.f});
    window.draw(t);
    window.display();
}
