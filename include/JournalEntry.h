//
//  JournalEntry.h
//  
//
//  Created by Thorsten Kreutz on 04.05.26.
//

#ifndef JournalEntry_h
#define JournalEntry_h


#endif /* JournalEntry_h */

#pragma once
#include <string>
#include <chrono>
#include <vector>

namespace Axiom {

enum class TradeType { Buy, Sell, PlannedBuy, PlannedSell };

struct TradePin {
    double price;
    std::chrono::system_clock::time_point timestamp;
    TradeType type;
    std::string note;
    // Die "Nadel"-Farbe wird später durch den Typ im UI bestimmt
};

class Journal {
public:
    void addEntry(const TradePin& entry) { entries.push_back(entry); }
    const std::vector<TradePin>& getEntries() const { return entries; }

private:
    std::vector<TradePin> entries;
};

} // namespace Axiom

