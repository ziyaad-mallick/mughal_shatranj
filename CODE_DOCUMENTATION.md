# Mughlia Shatranj — Complete Code Documentation
### OOP Final Project · CSE142 · Every File, Every Line Explained

---

## Table of Contents
1. [Project Overview](#1-project-overview)
2. [Complete Folder Map](#2-complete-folder-map)
3. [Root Files](#3-root-files)
4. [assets/ Folder](#4-assets-folder)
5. [build/ Folder & DLL Reference](#5-build-folder--dll-reference)
6. [src/primitives/ — Core Data Types](#6-srcprimitives--core-data-types)
7. [src/pieces/ — Piece Class Hierarchy](#7-srcpieces--piece-class-hierarchy)
8. [src/ — Game Logic Layer](#8-src--game-logic-layer)
9. [OOP Pillars Demonstrated](#9-oop-pillars-demonstrated)
10. [Data Structures Used](#10-data-structures-used)

---

## 1. Project Overview

This is a fully playable chess engine written in **C++17** using the **SFML 3** graphics library. It supports Human vs Human and Human vs AI modes, renders a themed board with drag-and-drop interaction, tracks per-side clocks, saves game records to a leaderboard file, and includes a separate command-line test runner.

The codebase deliberately demonstrates all four OOP pillars required by the course: **encapsulation, inheritance, polymorphism, and abstraction**.

---

## 2. Complete Folder Map

```
Final Project/
├── CMakeLists.txt          Build script — tells the compiler what to compile
├── generate_pieces.py      Original fallback sprite generator (pure Python/PIL)
├── CODE_DOCUMENTATION.md   This file
├── Chess Engine - Presentation.pptx
├── Chess Engine - Technical Documentation.docx
│
├── assets/                 Source sprite images (PNGs copied into build/)
│   ├── wK/wQ/wR/wB/wN/wP.png   White (ivory) pieces
│   └── bK/bQ/bR/bB/bN/bP.png   Black (ebony) pieces
│
├── build/                  Compiled output — run chess.exe from here
│   ├── chess.exe           The game
│   ├── chess_test.exe      Week 1/2 unit tests (no GUI)
│   ├── assets/             Runtime copy of sprites, font, leaderboard
│   ├── libsfml-*.dll       SFML runtime libraries
│   ├── libgcc_s_seh-1.dll  GCC C++ runtime
│   ├── libstdc++-6.dll     C++ Standard Library
│   ├── libwinpthread-1.dll POSIX threads
│   ├── libopenal-1.dll     Audio engine (OpenAL)
│   ├── libFLAC.dll         FLAC audio codec
│   ├── libogg-0.dll        OGG container codec
│   ├── libvorbis-0.dll     Vorbis codec
│   ├── libvorbisenc-2.dll  Vorbis encoder
│   └── libvorbisfile-3.dll Vorbis file reader
│
└── src/
    ├── primitives/         Tiny data-only structs shared everywhere
    │   ├── Enums.h         Color, PieceType, MoveType
    │   ├── Position.h      Board square (row, col) with helpers
    │   └── Move.h          A single chess move (src → dst + metadata)
    ├── pieces/             One .h + .cpp per piece type
    │   ├── Piece.h/cpp     Abstract base class for all pieces
    │   ├── King/Queen/Rook/Bishop/Knight/Pawn .h + .cpp
    ├── Board.h/cpp         The 8×8 board, move execution, legal-move gen
    ├── Exceptions.h        Custom exception classes
    ├── Player.h            Abstract base for all players
    ├── HumanPlayer.h/cpp   Mouse-driven player with drag-and-drop
    ├── AIPlayer.h/cpp      Minimax + Alpha-Beta pruning AI
    ├── Game.h/cpp          Main game loop, event routing, clock
    ├── Renderer.h/cpp      All SFML drawing code
    ├── LeaderBoard.h/cpp   CSV persistence of game records
    ├── main.cpp            Entry point, menu loop
    └── test_main.cpp       Console test runner (no SFML)
```

---

## 3. Root Files

### CMakeLists.txt

CMake is the **build system generator**. It reads this file and produces the actual Makefile that `mingw32-make` runs. Think of it as a recipe that says "here are the ingredients (source files) and here's what to cook (chess.exe)."

```cmake
cmake_minimum_required(VERSION 3.10)
```
Requires CMake 3.10 or newer. If an older CMake is installed, it stops with an error immediately.

```cmake
project(ChessEngine CXX)
```
Names the project "ChessEngine" and declares it's a C++ project. CMake uses this name in generated files.

```cmake
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```
Forces C++17 mode. We use `std::optional`, `std::chrono`, structured bindings, and `if constexpr` — all C++17 features. `REQUIRED ON` means it's an error if the compiler doesn't support C++17.

```cmake
set(CMAKE_PREFIX_PATH "C:/msys64/mingw64" ${CMAKE_PREFIX_PATH})
find_package(SFML 3 COMPONENTS Graphics Window System Audio REQUIRED)
```
Tells CMake to look for SFML 3 inside the MSYS2/MinGW installation at `C:/msys64/mingw64`. It needs four specific SFML sub-libraries: Graphics (drawing), Window (OS window + input), System (clock, types), Audio (sound — linked even though unused). `REQUIRED` means the build fails immediately if SFML isn't found.

```cmake
set(SOURCES
    src/Board.cpp
    src/pieces/Piece.cpp
    ... (all .cpp files except test_main.cpp)
    src/main.cpp
)
add_executable(chess ${SOURCES})
```
Lists every `.cpp` file that should be compiled into `chess.exe`. Header files (`.h`) are NOT listed here — they're automatically found via `#include`. `add_executable` creates the build target named "chess".

```cmake
target_include_directories(chess PRIVATE src)
```
Tells the compiler to look in the `src/` folder when resolving `#include` statements. That's why every file can write `#include "Board.h"` instead of `#include "../Board.h"`.

```cmake
target_link_libraries(chess SFML::Graphics SFML::Window SFML::System SFML::Audio)
```
Links the compiled object files against the four SFML libraries. Without this the linker wouldn't know where SFML functions live.

```cmake
target_compile_options(chess PRIVATE -Wall -Wextra -Wpedantic)
```
Turns on maximum compiler warnings. `-Wall` enables common warnings, `-Wextra` adds extra checks, `-Wpedantic` enforces strict C++ standard compliance. This catches bugs at compile time.

```cmake
if(WIN32)
    add_custom_command(TARGET chess POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "C:/msys64/mingw64/bin/libsfml-graphics-3.dll"
            ...
            $<TARGET_FILE_DIR:chess>
        COMMENT "Copying SFML DLLs..."
    )
```
After `chess.exe` is compiled (`POST_BUILD`), automatically copies all required DLLs next to it. `copy_if_different` skips copying if the file hasn't changed (saves time). `$<TARGET_FILE_DIR:chess>` resolves to the folder where `chess.exe` was created (the `build/` directory).

The subsequent `add_custom_command` blocks similarly copy the Arial font, create a blank `leaderboard.txt`, and copy the sprite PNGs from `assets/` into `build/assets/`.

```cmake
set(TEST_SOURCES ...)
add_executable(chess_test ${TEST_SOURCES})
```
A second, smaller executable that compiles only the game logic (no SFML, no Renderer, no Game) plus `test_main.cpp`. Used to verify the Board and pieces work correctly without launching the window.

---

### generate_pieces.py

This is the **original** piece sprite generator — it draws simple geometric chess pieces using Python's PIL/Pillow library. It was used to bootstrap the project before real hand-drawn sprites were available. The sprites it generates are still used as a fallback if the real PNGs are missing.

```python
SIZE = 80
OUT  = os.path.join(os.path.dirname(__file__), "assets")
os.makedirs(OUT, exist_ok=True)
```
Sets the canvas size to 80×80 pixels and points output to the `assets/` folder next to the script. `exist_ok=True` means it won't error if the folder already exists.

```python
WHITE_FILL    = (255, 255, 255, 255)   # solid white
WHITE_OUTLINE = ( 40,  40,  40, 255)   # dark grey outline
BLACK_FILL    = ( 30,  30,  30, 255)   # near-black
BLACK_OUTLINE = (220, 220, 220, 255)   # light grey outline
TRANSP        = (  0,   0,   0,   0)   # fully transparent (RGBA alpha=0)
```
All colours are (Red, Green, Blue, Alpha) tuples where 255 = fully opaque, 0 = fully transparent.

```python
def new_img():
    img = Image.new("RGBA", (SIZE, SIZE), TRANSP)
    return img, ImageDraw.Draw(img)
```
Creates a blank 80×80 transparent canvas. Returns both the image object and a drawing context (d) that lets us draw shapes onto it.

The `ellipse`, `rect`, `poly`, `line` helpers are thin wrappers around PIL's draw functions so each piece drawer doesn't repeat the same long call syntax.

Each `draw_*` function (draw_pawn, draw_rook, etc.) builds a piece shape from geometric primitives:
- **Pawn**: a trapezoid base + rectangular stem + circular head
- **Rook**: a tall rectangle body + wide base + three battlements (merlons) on top
- **Bishop**: trapezoid base + tapered body polygon + small head circle + dot on tip + collar
- **Knight**: base + neck polygon + horse-head polygon + eye dot + nostril hint
- **Queen**: base + body + collar + head circle + 5 crown circles + crown band
- **King**: same as Queen but with a cross (vertical + horizontal rectangles) instead of 5 crowns

```python
for piece, drawer in DRAWERS.items():
    for color, (fill, outline, prefix) in {...}.items():
        img, d = new_img()
        drawer(d, fill, outline)
        path = os.path.join(OUT, f"{prefix}{piece}.png")
        img.save(path)
```
Nested loop: for each of the 6 piece types × 2 colours = 12 images total. Calls the appropriate drawer function, then saves as a PNG.

---

## 4. assets/ Folder

Contains 12 PNG sprite files — the visual chess piece icons. These are RGBA images (colour + transparency channel) that Renderer.cpp loads into `sf::Texture` objects.

| File | What it is |
|------|-----------|
| `wK.png` | White King — ivory piece with dark amber shadows and mahogany outline |
| `wQ.png` | White Queen |
| `wR.png` | White Rook |
| `wB.png` | White Bishop |
| `wN.png` | White Knight |
| `wP.png` | White Pawn |
| `bK.png` | Black King — near-black ebony body with warm tan outline |
| `bQ.png` | Black Queen |
| `bR.png` | Black Rook |
| `bB.png` | Black Bishop |
| `bN.png` | Black Knight |
| `bP.png` | Black Pawn |

The naming convention `w` = white, `b` = black, then `K/Q/R/B/N/P` = King/Queen/Rook/Bishop/Knight/Pawn. Renderer.cpp constructs the filename from the piece's color and type to look up the right texture.

These sprites were generated from a source chess piece image using Python Pillow: the alpha channel (transparency mask) encodes the piece silhouette, and `ImageOps.colorize()` maps it to the ivory or ebony palette. A `MaxFilter` dilation adds a built-in outline so pieces remain visible on both the beige and charcoal squares.

---

## 5. build/ Folder & DLL Reference

The `build/` folder is where the compiler puts its output. You never edit files here manually — everything is regenerated by `mingw32-make`.

### chess.exe
The game itself. Double-click to run, or launch from this directory. It looks for `assets/` in the same folder, which is why CMake copies everything there.

### chess_test.exe
A separate console-only executable. Run it in a terminal to verify the chess logic (Board, pieces, move generation, check detection) without opening a window. Useful for debugging pure logic.

### What is a DLL?

A **DLL (Dynamic Link Library)** is a compiled code file that Windows programs load at startup. Instead of copying all of SFML's code into `chess.exe` (which would make the .exe enormous), the program references these DLLs and Windows loads them into memory at runtime. The `.exe` stays small but all the listed `.dll` files must sit in the same folder — if one is missing, Windows shows "the program can't start because X.dll is not found."

### SFML DLLs

| DLL | What it provides |
|-----|-----------------|
| `libsfml-graphics-3.dll` | **Everything visual.** `sf::RenderWindow`, `sf::Sprite`, `sf::Texture`, `sf::RectangleShape`, `sf::CircleShape`, `sf::Text`, `sf::Font` all live here. Every single draw call in Renderer.cpp goes through this DLL. |
| `libsfml-window-3.dll` | **The OS window and input.** Creates the actual Windows window, reads keyboard presses, mouse clicks, mouse movement, and the window close button. `sf::Event` (mouse press, key press, etc.) comes from here. Depends on sfml-system. |
| `libsfml-system-3.dll` | **Low-level foundation.** `sf::Clock`, `sf::Time`, `sf::Vector2`, `sf::String`. Required by all other SFML modules — always loaded first. Without it nothing else works. |
| `libsfml-audio-3.dll` | **Sound playback.** `sf::Sound`, `sf::Music`, `sf::SoundBuffer`. We don't use audio in the game, but SFML is configured to link against it. If you removed the SFML::Audio line from CMakeLists.txt and recompiled, you could drop this DLL and the five codec DLLs below. |

### MinGW Runtime DLLs

These are **compiler runtime libraries** — every C++ program compiled with GCC/MinGW needs them.

| DLL | What it provides |
|-----|-----------------|
| `libgcc_s_seh-1.dll` | **GCC runtime.** Implements C++ exception handling infrastructure on Windows using SEH (Structured Exception Handling). Every `try/catch` block in the code relies on this. Without it, throwing an exception crashes the program instantly. |
| `libstdc++-6.dll` | **The entire C++ Standard Library.** Contains `std::vector`, `std::string`, `std::map`, `std::stack`, `std::sort`, `std::chrono`, `std::cout`, `std::unique_ptr`, `std::ostream` — literally every `#include <...>` standard header you use in the project. This is the biggest and most important runtime DLL. |
| `libwinpthread-1.dll` | **POSIX threading.** Provides `std::thread`, `std::mutex`, `std::condition_variable`. We don't create threads explicitly, but SFML's audio system runs its own background thread, and `libstdc++` uses it internally. |

### Audio Codec DLLs (pulled in by libsfml-audio)

| DLL | What it provides |
|-----|-----------------|
| `libopenal-1.dll` | **OpenAL.** The audio engine that handles 3D sound positioning and hardware mixing. SFML-audio requires it even for simple 2D sound. |
| `libFLAC.dll` | **FLAC codec.** SFML can load `.flac` lossless audio files. This DLL decodes them. |
| `libogg-0.dll` | **OGG container.** The OGG format is a container (like a ZIP for audio). Vorbis audio data is wrapped inside OGG files. All three Vorbis DLLs depend on this. |
| `libvorbis-0.dll` | **Vorbis core codec.** The main decompression engine for `.ogg` audio files. |
| `libvorbisenc-2.dll` | **Vorbis encoder.** Writes OGG/Vorbis files. Pulled in transitively even though we never write audio. |
| `libvorbisfile-3.dll` | **Vorbis file reader.** High-level API for reading `.ogg` files sequentially. Used by SFML's music streaming. |

**Can any DLLs be removed?** Only if you recompile SFML without the Audio component. As long as `SFML::Audio` is in `CMakeLists.txt`, all audio DLLs are required even if you never call a single sound function.

---

## 6. src/primitives/ — Core Data Types

These three files define the smallest data types used everywhere in the project. They contain no logic beyond what's needed to compare, combine, and print values.

### Enums.h

```cpp
#pragma once
```
"Include guard" — tells the compiler to skip this file if it's already been included in the current compilation unit. Prevents duplicate definition errors when multiple `.cpp` files include the same header. `#pragma once` is a non-standard but universally supported shorthand for the older `#ifndef ENUMS_H / #define ENUMS_H / #endif` pattern.

```cpp
enum class Color { White, Black };
```
**Strongly-typed enum** for which side owns a piece or whose turn it is. `enum class` (as opposed to plain `enum`) means you must write `Color::White` — you can't accidentally mix it up with an integer or another enum's values.

```cpp
enum class PieceType { King, Queen, Rook, Bishop, Knight, Pawn };
```
Identifies what kind of piece an object is. Used in `switch` statements throughout the codebase for material values, algebraic notation, sprite key lookups, and promotion choices.

```cpp
enum class MoveType {
    Normal,          // standard move, no special rules
    Capture,         // lands on and removes an enemy piece
    EnPassant,       // pawn captures a pawn that just made a double push
    CastleKingside,  // king moves g1/g8, rook slides to f1/f8
    CastleQueenside, // king moves c1/c8, rook slides to d1/d8
    Promotion        // pawn reaches the back rank, becomes another piece
};
```
Every chess move has exactly one type. Board.cpp's `applyMove` uses this to trigger the right special-case logic.

---

### Position.h

Represents a single square on the 8×8 board. Row 0 = rank 1 (White's back rank), Row 7 = rank 8. Col 0 = file a, Col 7 = file h.

```cpp
struct Position {
    int row;
    int col;
```
Plain public data. `struct` is used instead of `class` because Position is a pure data container with no invariants to protect.

```cpp
    Position() : row(-1), col(-1) {}
```
Default constructor sets both to -1, which means "no square" / "invalid". This is used as a sentinel value throughout the code (e.g., `enPassantTarget = {-1,-1}` means no en passant available; `selectedSquare = {-1,-1}` means nothing selected).

```cpp
    Position(int r, int c) : row(r), col(c) {}
```
Main constructor. Called as `Position(3, 4)` for d4 (rank 4, file e... wait, row=3 is rank 4, col=4 is file e).

```cpp
    bool isValid() const {
        return row >= 0 && row < 8 && col >= 0 && col < 8;
    }
```
Returns true only if both coordinates are on the board (0–7). Every piece's move generator calls this before adding a destination to its move list, preventing array out-of-bounds.

```cpp
    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }
    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
```
Operator overloads so you can write `pos == lastMoveDst` naturally. `!=` delegates to `==` — the standard pattern to avoid duplicating the comparison logic.

```cpp
    Position operator+(const Position& delta) const {
        return Position(row + delta.row, col + delta.col);
    }
```
Allows writing `cur = cur + dir` when sliding a piece one step at a time. For example, a rook moving up uses `dir = {1, 0}` and this operator adds it to the current position each iteration.

---

### Move.h

Represents a complete chess move — not just "from here to there" but also what kind of move it is and, for promotions, what piece to promote to.

```cpp
struct Move {
    Position  src;             // which square the piece is moving FROM
    Position  dst;             // which square it's moving TO
    MoveType  type;            // Normal / Capture / EnPassant / Castle / Promotion
    PieceType promotionPiece;  // only meaningful when type == Promotion
```

```cpp
    Move()
        : src(), dst(), type(MoveType::Normal), promotionPiece(PieceType::Queen) {}
```
Default constructor — both positions default to {-1,-1} (invalid), type defaults to Normal, promotion defaults to Queen (the most common promotion). The AI's `chooseMove` returns a default Move if it has no legal moves, which signals the game loop that the game is over.

```cpp
    Move(Position s, Position d, MoveType t = MoveType::Normal,
         PieceType promo = PieceType::Queen)
        : src(s), dst(d), type(t), promotionPiece(promo) {}
```
Main constructor. `t` and `promo` have default values so callers only need to specify them when relevant. Example: `Move(Position(1,4), Position(3,4))` is a normal pawn push e2-e4.

```cpp
    bool operator==(const Move& other) const {
        return src == other.src && dst == other.dst && type == other.type;
    }
```
Two moves are equal if they describe the same physical move. Promotion piece type is intentionally excluded — when checking whether a player's chosen move is legal, the type of promotion doesn't affect legality, only the resulting piece.

```cpp
    friend std::ostream& operator<<(std::ostream& os, const Move& m) {
        auto toFile = [](int col) -> char { return static_cast<char>('a' + col); };
        auto toRank = [](int row) -> char { return static_cast<char>('1' + row); };
        os << toFile(m.src.col) << toRank(m.src.row)
           << toFile(m.dst.col) << toRank(m.dst.row);
        if (m.type == MoveType::Promotion) { /* append piece letter */ }
        return os;
    }
```
`friend` means this is a free function that can access Move's private members (though Move has no private members — it's a struct). The `<<` overload lets `std::cout << move` print algebraic notation like "e2e4" or "e7e8q". The two lambdas convert 0-indexed integers back to letter/number notation. Used in the sidebar's move history and in test_main.cpp.

---

## 7. src/pieces/ — Piece Class Hierarchy

This folder implements the **Inheritance** and **Polymorphism** pillars of OOP. Every piece type inherits from the abstract `Piece` base class and overrides its movement logic.

### Piece.h & Piece.cpp — Abstract Base Class

```cpp
class Board;  // forward declaration
```
`Board.h` includes `Piece.h` and `Piece.h` would normally need `Board.h` for the `getPseudoLegalMoves` parameter. Forward declaration breaks the circular include by telling the compiler "Board is a class, trust me, you'll see the full definition elsewhere."

```cpp
class Piece {
public:
    Piece(Color color, PieceType type);
    virtual ~Piece() = default;
```
The destructor is `virtual` because we always hold pieces via `Piece*` (base pointer) and delete them. Without `virtual`, deleting a `Piece*` that points to a `King` would only call `Piece`'s destructor, leaking the King's resources. `= default` tells the compiler to generate the default destructor body.

```cpp
    virtual std::vector<Move> getPseudoLegalMoves(Position pos, const Board& board) const = 0;
```
**Pure virtual function** — makes `Piece` an abstract class. No `Piece` object can be created directly; only concrete subclasses (King, Queen, etc.) that provide their own implementation can be instantiated. "Pseudo-legal" means moves that follow the piece's movement rules but might leave the king in check — legality filtering happens in `Board::getLegalMoves`.

```cpp
    virtual Piece* clone() const = 0;
```
Also pure virtual. Each subclass returns `new King(*this)`, `new Queen(*this)`, etc. This is the **Prototype pattern** — it lets `Board`'s copy constructor duplicate all pieces without knowing their concrete types. The AI calls this implicitly whenever it creates a board copy for search.

```cpp
protected:
    void addSlideMoves(std::vector<Move>& moves, Position src,
                       const Board& board,
                       const std::vector<Position>& directions) const;
```
**Shared helper** available to Rook, Bishop, and Queen (the "sliding pieces") but hidden from outside code. `protected` means subclasses inherit it but external code cannot call it.

```cpp
private:
    Color     color;
    PieceType type;
    bool      hasMoved;
```
**Encapsulation** — these fields are only accessible through the public getters/setters. This prevents external code from accidentally changing a piece's colour or claiming it has moved when it hasn't.

#### Piece.cpp

```cpp
Piece::Piece(Color color, PieceType type)
    : color(color), type(type), hasMoved(false) {}
```
**Member initialiser list** — faster than assignment inside the body because it initialises members directly rather than default-constructing them first. `hasMoved` starts false: every piece starts the game without having moved (relevant for pawn double-push and castling rights).

```cpp
void Piece::addSlideMoves(std::vector<Move>& moves, Position src,
                          const Board& board,
                          const std::vector<Position>& directions) const {
    for (const Position& dir : directions) {
        Position cur = src + dir;      // start one step in this direction
        while (cur.isValid()) {        // keep going until off the board
            Piece* target = board.getPieceAt(cur);
            if (target == nullptr) {
                moves.emplace_back(src, cur, MoveType::Normal);  // empty square: add move, continue
            } else {
                if (target->getColor() != color) {
                    moves.emplace_back(src, cur, MoveType::Capture);  // enemy: can capture, then stop
                }
                break;  // friendly or enemy: can't slide further
            }
            cur = cur + dir;  // advance one more step
        }
    }
}
```
The core sliding-piece algorithm. Each direction (e.g., `{1,0}` for "up") is explored step by step until hitting the board edge or any piece. Friendly pieces block without capturing; enemy pieces allow capture and then block. This single function implements movement for Rook, Bishop, and Queen.

---

### King.cpp

```cpp
static const int dr[] = {-1,-1,-1, 0, 0, 1, 1, 1};
static const int dc[] = {-1, 0, 1,-1, 1,-1, 0, 1};
```
Two parallel arrays encoding all 8 directions (row delta, col delta). `static` means these arrays live in the program's data segment (created once) rather than being recreated every time `getPseudoLegalMoves` is called. The king can step one square in any of these 8 directions.

```cpp
for (int i = 0; i < 8; ++i) {
    Position dst(pos.row + dr[i], pos.col + dc[i]);
    if (!dst.isValid()) continue;
    Piece* target = board.getPieceAt(dst);
    if (target == nullptr)
        moves.emplace_back(pos, dst, MoveType::Normal);
    else if (target->getColor() != myColor)
        moves.emplace_back(pos, dst, MoveType::Capture);
    // if friendly piece: do nothing (can't land there)
}
```
Tries each of the 8 adjacent squares. `continue` skips squares off the board. Only adds a move if the destination is empty (Normal) or has an enemy (Capture).

```cpp
if (!getHasMoved()) {
    int backRank = (myColor == Color::White) ? 0 : 7;
    // Kingside: rook on col 7, empty cols 5 & 6
    if (board.getCastlingRight(myColor, 1)) {
        Piece* rook = board.getPieceAt({backRank, 7});
        if (rook && rook->getType() == PieceType::Rook &&
            rook->getColor() == myColor && !rook->getHasMoved()) {
            if (!board.getPieceAt({backRank, 5}) &&
                !board.getPieceAt({backRank, 6})) {
                moves.emplace_back(pos, Position(backRank, 6), MoveType::CastleKingside);
            }
        }
    }
    // Queenside: similar but checks cols 1, 2, 3
```
Castling is only available if: (1) the king hasn't moved, (2) the castling right is still set, (3) the relevant rook is still on its starting square and hasn't moved, (4) the squares between them are empty. Note: these are *pseudo-legal* checks. Whether the king passes through check is verified in `Board::getLegalMoves`.

---

### Queen.cpp

```cpp
std::vector<Position> dirs = {
    {-1, 0}, {1, 0}, {0, -1}, {0, 1},   // rook directions (rank/file)
    {-1,-1}, {-1, 1}, {1,-1}, {1, 1}    // bishop directions (diagonals)
};
addSlideMoves(moves, pos, board, dirs);
```
The Queen combines Rook and Bishop movement. All 8 directions are handed to `addSlideMoves`. This is a perfect example of **code reuse via inheritance** — no duplication of the sliding logic.

---

### Rook.cpp

```cpp
std::vector<Position> dirs = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };
addSlideMoves(moves, pos, board, dirs);
```
Only the 4 orthogonal (rank/file) directions. That's it — the `addSlideMoves` helper does all the work.

---

### Bishop.cpp

```cpp
std::vector<Position> dirs = { {-1,-1}, {-1, 1}, {1,-1}, {1, 1} };
addSlideMoves(moves, pos, board, dirs);
```
Only the 4 diagonal directions.

---

### Knight.cpp

```cpp
static const int dr[] = {-2,-2,-1,-1, 1, 1, 2, 2};
static const int dc[] = {-1, 1,-2, 2,-2, 2,-1, 1};
```
The 8 L-shaped knight jumps. The knight is the only piece that **cannot** use `addSlideMoves` because it jumps over other pieces rather than sliding. Each (dr, dc) pair is a valid L-shape: always one step of 2 and one step of 1.

Unlike the king, the knight has no "blocked by friendly pieces in between" because it jumps — but it still can't land on a friendly piece. The loop handles that: if `target` is a friendly piece, neither `Normal` nor `Capture` conditions are met, so no move is added.

---

### Pawn.cpp

The pawn is the most complex piece because it has the most special cases.

```cpp
int dir      = (myColor == Color::White) ? 1 : -1;
int promRank = (myColor == Color::White) ? 7 : 0;
```
White pawns move toward increasing row numbers (toward rank 8). Black pawns move toward decreasing row numbers (toward rank 1). `dir` is +1 or -1 accordingly. `promRank` is the destination rank that triggers promotion.

```cpp
auto addMove = [&](Position src, Position dst, MoveType type) {
    if (dst.row == promRank) {
        for (PieceType pt : {PieceType::Queen, PieceType::Rook,
                              PieceType::Bishop, PieceType::Knight}) {
            moves.emplace_back(src, dst, MoveType::Promotion, pt);
        }
    } else {
        moves.emplace_back(src, dst, type);
    }
};
```
Lambda function (anonymous function) that wraps move-adding. When a pawn reaches the promotion rank, one destination square generates **4 separate Move objects** — one for each piece the pawn can become. The `[&]` capture means the lambda reads/writes the surrounding local variables (`moves`, `promRank`) by reference.

```cpp
Position fwd(pos.row + dir, pos.col);
if (fwd.isValid() && !board.getPieceAt(fwd)) {
    addMove(pos, fwd, MoveType::Normal);
    int startRank = (myColor == Color::White) ? 1 : 6;
    if (pos.row == startRank) {
        Position fwd2(pos.row + 2 * dir, pos.col);
        if (fwd2.isValid() && !board.getPieceAt(fwd2))
            moves.emplace_back(pos, fwd2, MoveType::Normal);
    }
}
```
**Single push**: pawn can advance one square if the square in front is empty. **Double push**: only from the starting rank (row 1 for white, row 6 for black), and only if BOTH the intermediate square AND the destination are empty. The double push can never be a promotion, so it uses `emplace_back` directly.

```cpp
for (int dc : {-1, 1}) {
    Position diag(pos.row + dir, pos.col + dc);
    if (!diag.isValid()) continue;
    Piece* target = board.getPieceAt(diag);
    if (target && target->getColor() != myColor) {
        addMove(pos, diag, MoveType::Capture);  // normal diagonal capture
    } else if (diag == board.getEnPassantTarget()) {
        moves.emplace_back(pos, diag, MoveType::EnPassant);  // ghost square capture
    }
}
```
Pawns capture diagonally (not forward). Checks both left and right diagonals. En passant is special: the target square (`board.getEnPassantTarget()`) is the square *behind* the enemy pawn that just double-pushed, not the square the enemy pawn is on. The captured pawn's removal is handled separately in `Board::applyMove`.

---

## 8. src/ — Game Logic Layer

### Exceptions.h

```cpp
class InvalidMoveException : public std::runtime_error {
public:
    explicit InvalidMoveException(const std::string& msg)
        : std::runtime_error("Invalid move: " + msg) {}
};
```
Inherits from `std::runtime_error` which itself inherits from `std::exception`. The `explicit` keyword prevents accidental implicit conversion from string to exception. `"Invalid move: "` is prepended to every message so the context is always clear. Thrown in `HumanPlayer` when a move doesn't pass validation.

```cpp
class BoardInitException : public std::runtime_error { ... };
```
A second custom exception for board initialisation failures (e.g., if a save file is corrupt). Currently not thrown in normal play, but it documents the intended error handling boundary.

---

### Board.h & Board.cpp

Board is the heart of the engine. It owns the 64-square grid, enforces all chess rules, and is the most complex class in the project.

#### The BoardSnapshot struct

```cpp
struct BoardSnapshot {
    Piece*   capturedPiece;
    Position enPassantTarget;
    bool     castlingRights[2][2];
    bool     rookMoved;
    Position rookPos;
    bool     movingHasMoved;
};
```
This struct stores everything needed to **undo a move** exactly. The AI calls `applyMove` / `undoMove` millions of times during search — this struct carries state between those two calls. Without it, undoing a move would be impossible (or would require keeping full board copies, which is much more expensive).

#### Board.cpp — Construction

```cpp
Board::Board() : enPassantTarget(-1, -1), currentTurn(Color::White) {
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
            grid[r][c] = nullptr;
    castlingRights[0][0] = castlingRights[0][1] = true;
    castlingRights[1][0] = castlingRights[1][1] = true;
    initBoard();
}
```
Initialises the 8×8 `grid` array of raw pointers to `nullptr`, sets all castling rights to true (everyone can castle until they move their king or rook), and calls `initBoard()` to place pieces.

```cpp
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
    placeBackRow(0, Color::White);   // row 0 = rank 1
    placeBackRow(7, Color::Black);   // row 7 = rank 8
    for (int c = 0; c < 8; ++c) {
        grid[1][c] = new Pawn(Color::White);  // row 1 = rank 2
        grid[6][c] = new Pawn(Color::Black);  // row 6 = rank 7
    }
}
```
Heap-allocates every piece with `new`. Each `new Piece(color)` calls the subclass constructor which calls `Piece::Piece`. The board owns all these pointers and deletes them in the destructor.

#### Deep Copy (Copy Constructor & Assignment)

```cpp
Piece* Board::clonePiece(Piece* p) const {
    return p ? p->clone() : nullptr;
}
```
Calls the virtual `clone()` on whatever concrete piece type `p` is. Because `clone()` is virtual and returns `new King(*this)` etc., this works for all piece types without any `switch` or `if`. This is **runtime polymorphism** in action.

```cpp
void Board::copyFrom(const Board& other) {
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
            grid[r][c] = clonePiece(other.grid[r][c]);
    enPassantTarget = other.enPassantTarget;
    currentTurn     = other.currentTurn;
    // copy castling rights, captured pieces, move history...
}
```
Every piece is deep-copied (not just the pointer — a new object is created). This is essential for the AI: when it creates `Board workBoard(board)`, it gets a completely independent board. Moves it applies to `workBoard` don't affect the real `board`. Without deep copy, the AI would corrupt the live game state.

#### Board::isInCheck

```cpp
bool Board::isInCheck(Color color) const {
    Position kingPos = findKing(color);
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
```
Finds the king's position, then asks every enemy piece "can you attack this square?" using pseudo-legal moves. If any enemy piece can reach the king's square, the king is in check. This is the most called function in the engine — every candidate move in `getLegalMoves` calls it.

#### Board::getLegalMoves

```cpp
std::vector<Move> Board::getLegalMoves(Color color) {
    std::vector<Move> legal;
    for (const Move& m : getPseudoLegalMoves(color)) {
        // Special castling check: king can't castle through check
        if (m.type == MoveType::CastleKingside || m.type == MoveType::CastleQueenside) {
            if (isInCheck(color)) continue;
            // Temporarily move king to the passing square and check for attacks
            int passCol = (m.type == MoveType::CastleKingside) ? 5 : 3;
            Piece* king = grid[backRank][4];
            grid[backRank][4]       = nullptr;
            grid[backRank][passCol] = king;
            bool throughCheck = isInCheck(color);
            grid[backRank][passCol] = nullptr;
            grid[backRank][4]       = king;
            if (throughCheck) continue;
        }
        BoardSnapshot snap = applyMove(m);       // make the move temporarily
        bool stillInCheck  = isInCheck(color);   // does it leave our king in check?
        undoMove(m, snap);                        // undo it
        if (!stillInCheck) legal.push_back(m);   // only keep it if king is safe
    }
    return legal;
}
```
The key legality filter: every pseudo-legal move is tested by actually applying it, checking if the king is now in check, then undoing it. If the king is safe after the move, it's legal. This "make/check/unmake" approach is standard in chess engines.

#### Board::applyMove

`applyMove` handles six cases in sequence:
1. **Saves snapshot** of pre-move state (en passant, castling rights, which pieces moved)
2. **En passant**: removes the captured pawn from the *side* (not the destination square)
3. **Normal capture**: stores the captured piece in the snapshot (not immediately deleted — needed for undo)
4. **Castling**: slides the rook to its new square alongside the king
5. **Moves the piece**: sets `dst` to the moving piece, clears `src`
6. **Records side effects**: sets `hasMoved = true`, updates en passant target for double pawn pushes, strips castling rights if king or rook moves
7. **Promotion**: deletes the pawn at `dst`, creates the new piece type

```cpp
moveHistory.push(move);
return snap;
```
The move is pushed onto the history stack (for move history display in the sidebar) and the snapshot is returned to the caller so `undoMove` can reverse everything.

#### Board::undoMove

The exact reverse of `applyMove`. Restores en passant target, castling rights, and `hasMoved` flags from the snapshot. For en passant: the captured pawn goes back to the side square (not dst). For castling: the rook slides back. For promotion: the promoted piece is deleted and a new Pawn is recreated.

---

### Player.h — Abstract Player Base

```cpp
class Player {
public:
    explicit Player(Color color) : color(color) {}
    virtual ~Player() = default;
    virtual Move chooseMove(const Board& board) = 0;   // pure virtual
    virtual bool isHuman() const { return false; }     // virtual with default
    Color getColor() const { return color; }
private:
    Color color;
};
```
**Second polymorphic hierarchy** alongside `Piece`. Both `HumanPlayer` and `AIPlayer` inherit from this. `chooseMove` is pure virtual — the base class can't know how to choose a move. `isHuman()` returns false by default; `HumanPlayer` overrides it to return true, allowing `Game` to route mouse events without `dynamic_cast` on every frame.

---

### HumanPlayer.h & HumanPlayer.cpp

Manages all player input. Supports both classic click-to-move and full drag-and-drop.

#### State fields

```cpp
Position          selectedSquare {-1, -1};   // which piece is currently selected
std::vector<Move> legalMovesCache;            // legal destinations of selected piece
bool              pendingMove = false;        // true when a move is ready to execute
Move              storedMove;                 // the move waiting to be picked up by Game

// Drag state
bool     dragging   = false;    // is the player actively dragging right now?
Position dragFromSq {-1, -1};   // which square the drag started on
int      dragPx     = 0;        // current mouse X pixel (updated every MouseMoved event)
int      dragPy     = 0;        // current mouse Y pixel
```

#### selectPiece (private helper)

```cpp
void HumanPlayer::selectPiece(Position sq, const Board& board) {
    selectedSquare = sq;
    Board& mb = const_cast<Board&>(board);
    legalMovesCache = mb.getLegalMoves(getColor());
    std::vector<Move> filtered;
    for (const Move& m : legalMovesCache)
        if (m.src == sq) filtered.push_back(m);
    legalMovesCache = filtered;
}
```
Gets all legal moves for the colour, then keeps only those originating from the clicked square. The cast to non-const is needed because `getLegalMoves` is non-const (it internally applies/undoes moves). The cache is read by Renderer to draw the green legal-move dots.

#### handleMousePress

```cpp
void HumanPlayer::handleMousePress(Position sq, const Board& board) {
    Piece* p = board.getPieceAt(sq);
    if (p && p->getColor() == getColor()) {
        selectPiece(sq, board);    // select the piece
        dragging   = true;         // begin drag
        dragFromSq = sq;
    } else if (selectedSquare.isValid()) {
        if (!tryMove(sq)) {        // second click: try to land here
            selectedSquare = {-1,-1};
            legalMovesCache.clear();
        }
        dragging = false;
    }
}
```
On pressing down on own piece: select it and start drag. On pressing down elsewhere with a piece already selected: treat it as click-to-move destination.

#### handleMouseRelease

```cpp
void HumanPlayer::handleMouseRelease(Position dst, const Board& /*board*/) {
    if (!dragging) return;
    dragging = false;
    if (dst.isValid() && dst != dragFromSq) {
        if (!tryMove(dst)) {
            selectedSquare = {-1,-1};
            legalMovesCache.clear();
        }
    }
    // Same square: drag cancelled, selection stays for click-to-move
}
```
When the mouse button is released on a different square, complete the drag move. If the move is illegal, deselect. If the same square was released on, it was a stationary click — keep the selection so the player can then click the destination.

#### tryMove (private helper)

```cpp
bool HumanPlayer::tryMove(Position dst) {
    for (const Move& m : legalMovesCache) {
        if (m.dst == dst) {
            storedMove  = m;
            pendingMove = true;
            selectedSquare = {-1,-1};
            legalMovesCache.clear();
            return true;
        }
    }
    return false;
}
```
Searches the cached legal moves for one that ends at `dst`. If found, stores it and sets `pendingMove = true` — the game loop will pick it up next frame via `hasPendingMove()` / `getPendingMove()`.

---

### AIPlayer.h & AIPlayer.cpp

The computer opponent. Uses the **Minimax** algorithm with **Alpha-Beta pruning**.

#### pieceValue

```cpp
case PieceType::Pawn:   return 100;
case PieceType::Knight: return 300;
case PieceType::Bishop: return 300;
case PieceType::Rook:   return 500;
case PieceType::Queen:  return 900;
case PieceType::King:   return 20000;
```
Classic chess piece values multiplied by 100 (to allow fractional positional bonuses later if added). The king has an astronomically high value so that checkmate detection dominates any material consideration.

#### evaluate

```cpp
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
```
Sums up piece values from White's perspective. Positive = White is ahead, negative = Black is ahead, zero = balanced. This is a "material balance" evaluation — it ignores piece position, but combined with search depth 3, it plays reasonably well.

#### minimax

```cpp
int AIPlayer::minimax(Board& board, int depth, int alpha, int beta, bool isMaximising) {
```
`isMaximising` is true when it's White's turn (White wants to maximise the score), false for Black. `alpha` is the best score White is guaranteed regardless of Black's response. `beta` is the best score Black is guaranteed.

```cpp
    if (board.isCheckmate(side)) return isMaximising ? -100000 : 100000;
    if (board.isStalemate(side)) return 0;
    if (depth == 0) return evaluate(board);
```
Base cases: checkmate is a terminal loss (−100000 for the side to move), stalemate is a draw (0), depth 0 returns the static material evaluation.

```cpp
    if (isMaximising) {
        int best = std::numeric_limits<int>::min();
        for (const Move& m : moves) {
            auto snap = board.applyMove(m);
            best = std::max(best, minimax(board, depth-1, alpha, beta, false));
            board.undoMove(m, snap);
            alpha = std::max(alpha, best);
            if (beta <= alpha) break;  // Beta cut-off: Black would have chosen differently earlier
        }
        return best;
    }
```
White's maximising node: tries every move, recursively scores the resulting position, keeps the best. If `beta <= alpha`, this node will never be reached in optimal play — the opponent already has a better alternative. `break` prunes the remaining moves (this is the pruning in Alpha-Beta).

#### chooseMove

```cpp
Move AIPlayer::chooseMove(const Board& board) {
    Board workBoard(board);  // deep copy — never touches the real board
    ...
    for (const Move& m : moves) {
        auto snap = workBoard.applyMove(m);
        int score = minimax(workBoard, searchDepth-1, alpha, beta, !isMaximising);
        workBoard.undoMove(m, snap);
        if (isMaximising ? (score > bestScore) : (score < bestScore)) {
            bestScore = score;
            bestMove  = m;
        }
    }
    return bestMove;
}
```
The top-level search. Works on `workBoard` (a deep copy) so the real game is never disturbed. With `searchDepth = 3`, it looks 3 half-moves deep. The first move tried is always returned as the best if all moves score equally (tiebreaker).

---

### LeaderBoard.h & LeaderBoard.cpp

Persists game results between sessions using a simple CSV text file.

```cpp
struct GameRecord {
    std::string player1;   // White's name
    std::string player2;   // Black's name
    std::string winner;    // "White", "Black", or "Draw"
    int moveCount;         // total half-moves played
    int durationSec;       // how many seconds the game lasted
};
```
Plain data structure. `std::string` for names so they can be any length. Integers for statistics.

```cpp
void LeaderBoard::sortRecords() {
    std::sort(records.begin(), records.end(),
        [](const GameRecord& a, const GameRecord& b) {
            if (a.winner == "Draw" && b.winner != "Draw") return false;
            if (a.winner != "Draw" && b.winner == "Draw") return true;
            return a.moveCount < b.moveCount;
        });
}
```
**Lambda comparator** passed to `std::sort`. Draws are ranked lower than decisive results. Among decisive games, fewer moves = more impressive = ranked higher. This is the lambda requirement for the course.

```cpp
void LeaderBoard::saveToFile() {
    std::ofstream file(filePath);
    for (const GameRecord& r : records)
        file << r.player1 << ',' << r.player2 << ',' << r.winner
             << ',' << r.moveCount << ',' << r.durationSec << '\n';
}
```
Writes every record as one comma-separated line. File is completely overwritten each save (no append) — records are always written in sorted order.

```cpp
void LeaderBoard::loadFromFile() {
    std::ifstream file(filePath);
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        GameRecord r;
        std::string mc, dur;
        if (std::getline(ss, r.player1, ',') && ...)
```
Reads line by line. `std::istringstream` lets the comma-separated line be treated like a stream, so `std::getline(ss, field, ',')` splits on commas. The move count and duration are stored as strings (`mc`, `dur`) and converted to int with `std::stoi` because `std::getline` only works with strings.

---

### Game.h & Game.cpp

The game loop — connects all other systems.

#### Key fields

```cpp
std::chrono::steady_clock::time_point startTime;     // when the game began
std::chrono::steady_clock::time_point turnStartTime; // when the current turn began
int whiteElapsedSecs = 0;  // total seconds White has spent thinking
int blackElapsedSecs = 0;  // total seconds Black has spent thinking
```
`std::chrono::steady_clock` is the C++11 high-resolution monotonic clock — it can't go backwards even if the system clock is adjusted. `steady_clock::time_point` stores a moment in time.

#### swapCurrentPlayer (time tracking)

```cpp
void Game::swapCurrentPlayer() {
    auto now = std::chrono::steady_clock::now();
    int elapsed = (int)std::chrono::duration_cast<std::chrono::seconds>(now - turnStartTime).count();
    if (currentPlayer == whitePlayer.get()) whiteElapsedSecs += elapsed;
    else                                    blackElapsedSecs += elapsed;
    turnStartTime = now;
    board.swapTurn();
    currentPlayer = ...;
    hp->resetSelection();  // clear selection on turn change
}
```
Every time a move is made and turns swap, the elapsed time since `turnStartTime` is added to the right player's total. Then `turnStartTime` is reset to now for the next player's turn.

#### Game::run() event loop

```cpp
while (const auto event = window.pollEvent()) {
```
`pollEvent` returns immediately whether or not there's an event — it doesn't block. This keeps the game loop running at 60 FPS regardless of input. Returns `std::optional<sf::Event>` — the `while` loop runs as long as events are queued.

```cpp
    if (const auto* mv = event->getIf<sf::Event::MouseMoved>())
        hp->handleMouseMove(mv->position.x, mv->position.y);
    if (const auto* press = event->getIf<sf::Event::MouseButtonPressed>())
        ...hp->handleMousePress(sq, board);
    if (const auto* rel = event->getIf<sf::Event::MouseButtonReleased>())
        ...hp->handleMouseRelease(dst, board);
```
Three separate event types are routed to `HumanPlayer`. `getIf<EventType>()` returns a pointer only if the event is of that type, or nullptr. This is SFML3's type-safe event API — no more `switch` on `event.type`.

#### Live time computation

```cpp
auto now = std::chrono::steady_clock::now();
int liveSecs = (int)std::chrono::duration_cast<std::chrono::seconds>(now - turnStartTime).count();
bool wTurn = (board.getCurrentTurn() == Color::White);
int wDisp = whiteElapsedSecs + (wTurn  ? liveSecs : 0);
int bDisp = blackElapsedSecs + (!wTurn ? liveSecs : 0);
```
Every frame, adds the current ongoing turn's elapsed seconds to the right player's accumulated total. This makes the active clock count upward in real time. The inactive player's clock is frozen.

---

### Renderer.h & Renderer.cpp

Handles all visual output. Nothing in Renderer touches game logic — it only reads the Board (never writes to it).

#### Layout constants

```cpp
static constexpr int SQUARE_SIZE   = 100;   // pixels per board square
static constexpr int BOARD_PIXELS  = 800;   // 8 × 100
static constexpr int SIDEBAR_WIDTH = 380;   // info panel width
static constexpr int WINDOW_W      = 1180;  // 800 + 380
static constexpr int WINDOW_H      = 800;   // same as board
```
`constexpr` means these are compile-time constants — the compiler replaces every use of `SQUARE_SIZE` with the literal `100`. Changing one number here resizes everything proportionally.

#### loadPieceSprites

```cpp
for (const char* k : keys) {
    std::string path = dir + "/" + k + ".png";
    sf::Texture tex;
    if (tex.loadFromFile(path)) {
        tex.setSmooth(true);   // bilinear filtering when scaling
        textures[k] = std::move(tex);  // move into map, avoids copy
        ++loaded;
    }
}
spritesLoaded = (loaded == 12);
```
Loads all 12 PNGs into `sf::Texture` objects stored in a `std::map<std::string, sf::Texture>`. `setSmooth(true)` enables bilinear filtering so pieces look crisp when scaled. `std::move` transfers ownership of the texture without copying pixel data.

#### drawSquares

```cpp
bool isLight = (r + c) % 2 == 0;
sq.setFillColor(isLight ? LIGHT_SQ : DARK_SQ);
```
The chess checkerboard pattern: if row + col is even, it's a light square. This formula always produces the right alternating pattern regardless of board orientation.

```cpp
sf::Color cc = isLight ? sf::Color(100, 80, 55) : sf::Color(175, 170, 160);
```
Coordinate labels use colours that contrast with their own square — dark brownish text on beige squares, light grey text on charcoal squares.

```cpp
sf::RectangleShape sep({2.f, (float)BOARD_PIXELS});
sep.setPosition({(float)BOARD_PIXELS, 0.f});
sep.setFillColor(sf::Color(180, 145, 40, 180));
window.draw(sep);
```
A 2px gold-tinted vertical line separating the board from the sidebar. `180` in the last colour component is the alpha — slightly transparent.

#### drawPiece (sprite rendering)

```cpp
sf::Sprite sprite(it->second);
sf::Vector2u ts = it->second.getSize();
sprite.setScale({ static_cast<float>(SQUARE_SIZE) / ts.x,
                   static_cast<float>(SQUARE_SIZE) / ts.y });
sprite.setPosition(topLeft);
window.draw(sprite);
```
`sf::Sprite` is SFML's image renderer — it holds a reference to a `sf::Texture` and knows where to draw it. The scale factor converts from the sprite's pixel size (80×80) to the current square size (100×100). If the texture has transparent pixels (RGBA), SFML respects the alpha channel automatically.

#### Drag-and-drop rendering (in render())

```cpp
if (isDragging && dragFrom.isValid()) {
    Piece* dp = board.getPieceAt(dragFrom);
    // Draw shadow (offset sprite with dark tint)
    sf::Sprite shadow(it->second);
    shadow.setColor(sf::Color(0, 0, 0, 80));   // 80/255 opacity black
    shadow.setPosition({(float)dragPx - halfSz + 5.f,
                         (float)dragPy - halfSz + 7.f});
    window.draw(shadow);
    // Draw actual piece at 108% scale centred on cursor
    float scl = drawSz / it->second.getSize().x;
    piece.setScale({scl, scl});
    piece.setPosition({(float)dragPx - halfSz, (float)dragPy - halfSz});
    window.draw(piece);
}
```
The dragged piece is drawn last (on top of everything). It's drawn twice: first a near-black transparent copy offset by (5, 7) pixels to simulate a drop shadow, then the real piece at 108% size. `halfSz` centres the piece on the cursor rather than anchoring the top-left corner.

#### drawSidebar (panel layout)

The sidebar uses three internal lambda helpers:
- `txt(string, x, y, size, color)` — draws a single text block at fixed coordinates
- `ctxt(string, y, size, color)` — draws centred text in the sidebar
- `rule(y, color)` — draws a 1px horizontal separator line

The sidebar sections in order:
1. **Title** "MUGHLIA SHATRANJ" — centred, 18pt gold
2. **Turn indicator** — coloured dot (gold for White, green for Black) + big turn text
3. **Status message** — shown in red when in check, otherwise hidden
4. **CLOCKS** — White and Black rows, right-aligned MM:SS timers; active clock glows bright
5. **LAST MOVE** — algebraic notation (e.g., "e2 -> e4") from `getMoveHistory().top()`
6. **WHITE CAPTURES / BLACK CAPTURES** — piece letter abbreviations

---

### main.cpp — Entry Point

```cpp
sf::RenderWindow window(
    sf::VideoMode({static_cast<unsigned>(WINDOW_W),
                   static_cast<unsigned>(WINDOW_H)}),
    "Mughlia Shatranj",
    sf::Style::Titlebar | sf::Style::Close);
window.setFramerateLimit(60);
```
Creates the OS window. `sf::Style::Titlebar | sf::Style::Close` gives it a title bar and close button but no resize handle (board layout is fixed). `setFramerateLimit(60)` caps at 60 FPS to avoid wasting CPU.

```cpp
if (!renderer.loadPieceSprites("assets"))
    std::cerr << "Note: piece sprites not found, using letter fallback.\n";
```
Tries to load the PNG sprites. If any fail (e.g., missing assets folder), the game still runs using the geometric fallback renderer (`drawPieceFallback`).

```cpp
switch (key->code) {
    case sf::Keyboard::Key::Num1: {
        Game g(window, renderer, leaderboard, "hvh", "White", "Black");
        g.run();
        chosen = true; break;
    }
    case sf::Keyboard::Key::Num2: {
        Game g(window, renderer, leaderboard, "hvai", "You", "AI");
        g.run();
        chosen = true; break;
    }
```
Pressing 1 constructs a Human vs Human game; pressing 2 constructs Human vs AI. The `Game` object is created on the stack — when `g.run()` returns (game over), `g` is destroyed, which in turn destroys the `Board` (deleting all pieces) and frees the players.

```cpp
static void showLeaderboard(sf::RenderWindow& window, Renderer& renderer, LeaderBoard& lb) {
```
`static` on a function inside a `.cpp` file means it has file scope — it can't be called from other `.cpp` files. Used for functions that are logically part of `main.cpp` but don't need to be public.

---

### test_main.cpp — Unit Test Runner

A standalone `main()` that compiles without SFML. Exercises the core chess logic:

1. **Initial board print** — verifies the board sets up correctly
2. **Legal move count** — at the start of the game, White has exactly 20 legal moves (16 pawn + 4 knight)
3. **Apply/undo moves** — plays e2e4, e7e5, d2d4, then undoes d2d4 and verifies the board matches
4. **isInCheck** — verifies no check after opening moves
5. **Board copy constructor** — applies a move to a copy, verifies the original is unchanged (deep copy test)
6. **Piece operator==** — verifies two white pawns are equal, white ≠ black
7. **Move << operator** — prints a promotion move in algebraic notation

Run as: `./build/chess_test.exe` (no window needed).

---

## 9. OOP Pillars Demonstrated

| Pillar | Where |
|--------|-------|
| **Encapsulation** | `Piece` hides `color`, `type`, `hasMoved` behind private fields with public getters/setters. `Board` hides the raw grid and all its logic. `HumanPlayer` hides drag state from external code. |
| **Inheritance** | `King`, `Queen`, `Rook`, `Bishop`, `Knight`, `Pawn` all inherit from `Piece`. `HumanPlayer` and `AIPlayer` both inherit from `Player`. |
| **Polymorphism** | `board.getPieceAt(pos)->getPseudoLegalMoves(...)` works for any piece type via the virtual function. `currentPlayer->chooseMove(board)` works for both Human and AI. `clonePiece(p)` clones the correct subtype. |
| **Abstraction** | `Piece` is an abstract class (pure virtual `getPseudoLegalMoves` and `clone`) — you can never create a `Piece` directly. `Player` is abstract — you can never create a bare `Player`. |

---

## 10. Data Structures Used

| Structure | Where used | Why |
|-----------|-----------|-----|
| `Piece* grid[8][8]` | Board | 2D array — direct O(1) square lookup by index. Fixed size known at compile time. |
| `std::vector<Move>` | getLegalMoves, legalMovesCache | Dynamic array — size varies per position. Resizes automatically. Supports range-for iteration. |
| `std::vector<Piece*>` | capturedPieces | Same — unknown number of captured pieces over a game. |
| `std::stack<Move>` | moveHistory | LIFO — the last move made is always on top, which is what sidebar/undo needs. |
| `std::map<std::string, sf::Texture>` | Renderer textures | Key-value lookup by sprite name ("wK", "bN", etc.). O(log n) lookup, sorted by key. |
| Lambda comparator | LeaderBoard::sortRecords | Anonymous function passed to `std::sort` — avoids writing a separate named comparator function. |
| `BoardSnapshot` struct | applyMove/undoMove | Bundles all pre-move state into one object so undo is clean and atomic. |
| `std::unique_ptr<Player>` | Game | Smart pointer — automatically deletes the Player when Game is destroyed, preventing memory leaks. |
