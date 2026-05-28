#include "CipherPhase2.h"
#include <vector>
#include <utility>
#include <cstdio>
#include <stdexcept>

static const std::vector<std::pair<std::string, std::string>> encrypt_map = {
    // Vocales (minúsculas y mayúsculas → mismo código según spec "U representa TODAS las vocales")
    {"a", "U1"}, {"e", "U2"}, {"i", "U3"}, {"o", "U4"}, {"u", "U5"},
    {"A", "U1"}, {"E", "U2"}, {"I", "U3"}, {"O", "U4"}, {"U", "U5"},

    // Consonantes minúsculas
    {"b", "m1"},  {"c", "m2"},  {"d", "m3"},  {"f", "m4"},  {"g", "m5"},
    {"h", "m6"},  {"j", "m7"},  {"k", "m8"},  {"l", "m9"},  {"m", "m10"},
    {"n", "m11"}, {"p", "m13"}, {"q", "m14"}, {"r", "m15"}, {"s", "m16"},
    {"t", "m17"}, {"v", "m18"}, {"w", "m19"}, {"x", "m20"}, {"y", "m21"},
    {"z", "m22"},

    // Consonantes mayúsculas
    {"B", "g1"},  {"C", "g2"},  {"D", "g3"},  {"F", "g4"},  {"G", "g5"},
    {"H", "g6"},  {"J", "g7"},  {"K", "g8"},  {"L", "g9"},  {"M", "g10"},
    {"N", "g11"}, {"P", "g13"}, {"Q", "g14"}, {"R", "g15"}, {"S", "g16"},
    {"T", "g17"}, {"V", "g18"}, {"W", "g19"}, {"X", "g20"}, {"Y", "g21"},
    {"Z", "g22"}
};

std::string CipherPhase2::encrypt(const std::string& input) {
    std::string result;
    for (size_t i = 0; i < input.length(); ) {
        // UTF-8: ñ = 0xC3 0xB1 , Ñ = 0xC3 0x91
        if (i + 1 < input.length()) {
            unsigned char b0 = (unsigned char)input[i];
            unsigned char b1 = (unsigned char)input[i + 1];
            if (b0 == 0xC3 && b1 == 0xB1) { result += "m12"; i += 2; continue; }
            if (b0 == 0xC3 && b1 == 0x91) { result += "g12"; i += 2; continue; }
        }

        std::string one = input.substr(i, 1);
        bool matched = false;
        for (const auto& p : encrypt_map) {
            if (p.first == one) { result += p.second; matched = true; break; }
        }
        if (!matched) result += one;
        i += 1;
    }
    return result;
}

std::string CipherPhase2::decrypt(const std::string& input) {
    std::string result;
    for (size_t i = 0; i < input.length(); ) {
        char ch = input[i];
        if (ch == 'U' || ch == 'm' || ch == 'g') {
            size_t j = i + 1;
            while (j < input.length() && std::isdigit((unsigned char)input[j])) j++;
            if (j > i + 1) {
                std::string token = input.substr(i, j - i);
                bool matched = false;
                for (const auto& p : encrypt_map) {
                    if (p.second == token) { result += p.first; matched = true; break; }
                }
                if (matched) { i = j; continue; }
            }
        }
        result += ch;
        i++;
    }
    return result;
}

// Fase 3: cifrado XOR con clave de usuario; salida en hexadecimal para ser texto-segura
std::string CipherPhase2::encryptWithKey(const std::string& data, const std::string& key) {
    if (key.empty()) return data;
    std::string result;
    result.reserve(data.length() * 2);
    for (size_t i = 0; i < data.length(); i++) {
        unsigned char c = (unsigned char)data[i] ^ (unsigned char)key[i % key.length()];
        char hex[3];
        snprintf(hex, sizeof(hex), "%02x", (unsigned int)c);
        result += hex;
    }
    return result;
}

std::string CipherPhase2::decryptWithKey(const std::string& data, const std::string& key) {
    if (key.empty() || data.length() % 2 != 0) return data;
    std::string result;
    result.reserve(data.length() / 2);
    for (size_t i = 0; i < data.length(); i += 2) {
        try {
            unsigned int byte_val = 0;
            sscanf(data.substr(i, 2).c_str(), "%02x", &byte_val);
            unsigned char c = (unsigned char)byte_val ^ (unsigned char)key[(i / 2) % key.length()];
            result += (char)c;
        } catch (...) {
            return data;
        }
    }
    return result;
}
