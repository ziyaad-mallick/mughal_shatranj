cmake_minimum_required(VERSION 3.10)
project(ChessEngine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── SFML from MSYS2/MinGW ─────────────────────────────────────────────────────
set(CMAKE_PREFIX_PATH "C:/msys64/mingw64" ${CMAKE_PREFIX_PATH})
find_package(SFML 3 COMPONENTS Graphics Window System Audio REQUIRED)

# ── Source files ──────────────────────────────────────────────────────────────
set(SOURCES
    src/Board.cpp
    src/pieces/Piece.cpp
    src/pieces/King.cpp
    src/pieces/Queen.cpp
    src/pieces/Rook.cpp
    src/pieces/Bishop.cpp
    src/pieces/Knight.cpp
    src/pieces/Pawn.cpp
    src/HumanPlayer.cpp
    src/AIPlayer.cpp
    src/Renderer.cpp
    src/Game.cpp
    src/LeaderBoard.cpp
    src/main.cpp
)

# ── Main executable ───────────────────────────────────────────────────────────
add_executable(chess ${SOURCES})

target_include_directories(chess PRIVATE src)

target_link_libraries(chess
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
)

target_compile_options(chess PRIVATE -Wall -Wextra -Wpedantic)

# ── Copy DLLs next to the executable (required on Windows) ───────────────────
if(WIN32)
    add_custom_command(TARGET chess POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "C:/msys64/mingw64/bin/libsfml-graphics-3.dll"
            "C:/msys64/mingw64/bin/libsfml-window-3.dll"
            "C:/msys64/mingw64/bin/libsfml-system-3.dll"
            "C:/msys64/mingw64/bin/libsfml-audio-3.dll"
            "C:/msys64/mingw64/bin/libgcc_s_seh-1.dll"
            "C:/msys64/mingw64/bin/libstdc++-6.dll"
            "C:/msys64/mingw64/bin/libwinpthread-1.dll"
            "C:/msys64/mingw64/bin/libopenal-1.dll"
            "C:/msys64/mingw64/bin/libFLAC.dll"
            "C:/msys64/mingw64/bin/libvorbis-0.dll"
            "C:/msys64/mingw64/bin/libvorbisenc-2.dll"
            "C:/msys64/mingw64/bin/libvorbisfile-3.dll"
            "C:/msys64/mingw64/bin/libogg-0.dll"
            $<TARGET_FILE_DIR:chess>
        COMMENT "Copying SFML DLLs..."
    )

    # Copy font for text rendering
    add_custom_command(TARGET chess POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:chess>/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "C:/Windows/Fonts/arial.ttf"
            "$<TARGET_FILE_DIR:chess>/assets/arial.ttf"
        COMMENT "Copying font..."
    )

    # Create leaderboard file if it doesn't exist
    add_custom_command(TARGET chess POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E touch
            "$<TARGET_FILE_DIR:chess>/assets/leaderboard.txt"
        COMMENT "Creating leaderboard file..."
    )

    # Copy piece PNG sprites
    add_custom_command(TARGET chess POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:chess>/assets"
        COMMENT "Copying piece sprites..."
    )
endif()

add_executable(chess_test ${TEST_SOURCES})
target_include_directories(chess_test PRIVATE src)
target_compile_options(chess_test PRIVATE -Wall -Wextra)
