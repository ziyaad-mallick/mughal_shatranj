#pragma once
#include <stdexcept>
#include <string>

// Thrown when a human player attempts an illegal move
class InvalidMoveException : public std::runtime_error {
public:
    explicit InvalidMoveException(const std::string& msg)
        : std::runtime_error("Invalid move: " + msg) {}
};

// Thrown when the board cannot be initialised (e.g. corrupt save file)
class BoardInitException : public std::runtime_error {
public:
    explicit BoardInitException(const std::string& msg)
        : std::runtime_error("Board init error: " + msg) {}
};
