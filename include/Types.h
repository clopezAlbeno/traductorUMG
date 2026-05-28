#pragma once
#include <string>

struct WordRecord {
    std::string spanish;
    std::string italian;
    std::string french;
    std::string german;
    std::string english;

    bool operator<(const WordRecord& other) const { return spanish < other.spanish; }
    bool operator>(const WordRecord& other) const { return spanish > other.spanish; }
    bool operator==(const WordRecord& other) const { return spanish == other.spanish; }
};
