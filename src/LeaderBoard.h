#pragma once
#include <string>
#include <vector>

struct GameRecord {
    std::string player1;   // White side
    std::string player2;   // Black side
    std::string winner;    // "White", "Black", or "Draw"
    int         moveCount;
    int         durationSec;
};

class LeaderBoard {
public:
    explicit LeaderBoard(const std::string& filePath);

    // Add a finished game record
    void addRecord(const GameRecord& record);

    // Sort by move count (ascending) then save
    void saveToFile();

    // Load records from file
    void loadFromFile();

    // Get sorted records for display
    const std::vector<GameRecord>& getRecords() const;

private:
    std::string           filePath;
    std::vector<GameRecord> records; // dynamic array — DS requirement

    void sortRecords(); // uses std::sort + lambda — sorting + lambda requirement
};
