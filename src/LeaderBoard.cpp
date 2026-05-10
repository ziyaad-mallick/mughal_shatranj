#include "LeaderBoard.h"
#include <fstream>
#include <algorithm>
#include <sstream>

LeaderBoard::LeaderBoard(const std::string& path) : filePath(path) {
    loadFromFile();
}

void LeaderBoard::addRecord(const GameRecord& record) {
    records.push_back(record);
    sortRecords();
    saveToFile();
}

// Sort by move count ascending (fewer moves = more decisive game = higher rank).
// Lambda comparator — satisfies both the sorting and lambda requirements.
void LeaderBoard::sortRecords() {
    std::sort(records.begin(), records.end(),
        [](const GameRecord& a, const GameRecord& b) {
            if (a.winner == "Draw" && b.winner != "Draw") return false;
            if (a.winner != "Draw" && b.winner == "Draw") return true;
            return a.moveCount < b.moveCount;
        });
}

void LeaderBoard::saveToFile() {
    std::ofstream file(filePath);
    if (!file.is_open()) return;
    for (const GameRecord& r : records) {
        file << r.player1 << ','
             << r.player2 << ','
             << r.winner  << ','
             << r.moveCount << ','
             << r.durationSec << '\n';
    }
}

void LeaderBoard::loadFromFile() {
    records.clear();
    std::ifstream file(filePath);
    if (!file.is_open()) return;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        GameRecord r;
        std::string mc, dur;
        if (std::getline(ss, r.player1, ',') &&
            std::getline(ss, r.player2, ',') &&
            std::getline(ss, r.winner,  ',') &&
            std::getline(ss, mc,  ',') &&
            std::getline(ss, dur, ',')) {
            r.moveCount    = std::stoi(mc);
            r.durationSec  = std::stoi(dur);
            records.push_back(r);
        }
    }
}

const std::vector<GameRecord>& LeaderBoard::getRecords() const {
    return records;
}
