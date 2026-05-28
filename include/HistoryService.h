#pragma once
#include <string>
#include <vector>

class HistoryService {
public:
    static void saveSearch(const std::string& username, const std::string& search_phrase);
    static std::vector<std::string> getTopSearches(const std::string& username);
};
