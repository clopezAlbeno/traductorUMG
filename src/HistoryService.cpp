#include "HistoryService.h"
#include "CipherPhase2.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <map>
#include <algorithm>

namespace fs = std::filesystem;

void HistoryService::saveSearch(const std::string& username, const std::string& search_phrase) {
    fs::create_directories("data/" + username + "/history");
    
    // Generar un nombre de archivo único para la búsqueda
    static int counter = 0;
    std::string base_path = "data/" + username + "/history/search_" + std::to_string(time(nullptr)) + "_" + std::to_string(counter++);

    std::string encrypted_phrase = CipherPhase2::encrypt(search_phrase);

    // 1 un archivo con la llave este contendrá la palabra Umg
    std::ofstream key_file(base_path + "_key.txt");
    if (key_file.is_open()) {
        key_file << "Umg\n";
        key_file.close();
    }

    // 2 un archivo con la conversión resultante
    std::ofstream enc_file(base_path + "_enc.txt");
    if (enc_file.is_open()) {
        enc_file << encrypted_phrase << "\n";
        enc_file.close();
    }

    // 3 un archivo con la información original
    std::ofstream orig_file(base_path + "_orig.txt");
    if (orig_file.is_open()) {
        orig_file << search_phrase << "\n";
        orig_file.close();
    }
}

std::vector<std::string> HistoryService::getTopSearches(const std::string& username) {
    std::string path = "data/" + username + "/history";
    if (!fs::exists(path)) return {};

    std::map<std::string, int> counts;

    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.path().string().find("_orig.txt") != std::string::npos) {
            std::ifstream file(entry.path());
            std::string line;
            if (std::getline(file, line)) {
                counts[line]++;
            }
        }
    }

    std::vector<std::pair<std::string, int>> vec(counts.begin(), counts.end());
    std::sort(vec.begin(), vec.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    std::vector<std::string> top;
    for (size_t i = 0; i < vec.size() && i < 5; ++i) {
        top.push_back(vec[i].first);
    }
    return top;
}