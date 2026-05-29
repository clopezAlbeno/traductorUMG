#include "HistoryService.h"
#include "CipherPhase2.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <map>
#include <algorithm>

namespace fs = std::filesystem;

// La clave del cifrado UMG — el archivo del usuario debe contener exactamente esta cadena
static const std::string REQUIRED_KEY = "Umg";

static std::string keyPath(const std::string& u)  { return "data/users/" + u + "_history_key.txt"; }
static std::string encPath(const std::string& u)  { return "data/users/" + u + "_history_enc.txt"; }
static std::string origPath(const std::string& u) { return "data/users/" + u + "_history_orig.txt"; }

// Retorna "Umg" si el archivo de clave del usuario es válido, "" si no lo es
static std::string validateKey(const std::string& username) {
    std::ifstream kf(keyPath(username));
    if (!kf.is_open()) return "";
    std::string key;
    std::getline(kf, key);
    // El cifrado UMG solo funciona si la clave es exactamente "Umg"
    return (key == REQUIRED_KEY) ? key : "";
}

void HistoryService::saveSearch(const std::string& username, const std::string& search_phrase) {
    fs::create_directories("data/users");

    // Archivo 1 — clave: se crea una sola vez con "Umg"; si ya fue modificado, saveSearch falla
    if (!fs::exists(keyPath(username))) {
        std::ofstream kf(keyPath(username));
        if (kf.is_open()) kf << REQUIRED_KEY << "\n";
    }

    // Validar que la clave del archivo siga siendo "Umg"
    if (validateKey(username).empty()) {
        std::cerr << "Error: la clave de historial del usuario '" << username
                  << "' no es valida. El historial no puede guardarse.\n";
        return;
    }

    // Archivo 2 — conversión resultante: cifrado UMG (sustitución de vocales/consonantes)
    std::string encrypted = CipherPhase2::encrypt(search_phrase);
    std::ofstream enc(encPath(username), std::ios::app);
    if (enc.is_open()) enc << encrypted << "\n";

    // Archivo 3 — información original: texto sin cifrar
    std::ofstream orig(origPath(username), std::ios::app);
    if (orig.is_open()) orig << search_phrase << "\n";
}

std::vector<std::string> HistoryService::getTopSearches(const std::string& username) {
    // Sin clave válida "Umg" no se puede descifrar el historial
    if (validateKey(username).empty()) {
        std::cout << "  (historial no disponible: clave invalida o ausente)\n";
        return {};
    }

    if (!fs::exists(encPath(username))) return {};

    std::map<std::string, int> counts;
    std::ifstream enc_file(encPath(username));
    std::string line;
    while (std::getline(enc_file, line)) {
        if (line.empty()) continue;
        // Descifrar usando la tabla UMG inversa
        std::string decrypted = CipherPhase2::decrypt(line);
        if (!decrypted.empty()) counts[decrypted]++;
    }

    std::vector<std::pair<std::string, int>> vec(counts.begin(), counts.end());
    std::sort(vec.begin(), vec.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    std::vector<std::string> top;
    for (size_t i = 0; i < vec.size() && i < 5; ++i)
        top.push_back(vec[i].first + " (" + std::to_string(vec[i].second) + " veces)");
    return top;
}
