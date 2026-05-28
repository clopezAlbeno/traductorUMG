#pragma once
#include <string>

class CipherPhase2 {
public:
    // Fase 2: cifrado por sustitución UMG
    static std::string encrypt(const std::string& input);
    static std::string decrypt(const std::string& input);

    // Fase 3: segunda capa XOR con clave de usuario
    static std::string encryptWithKey(const std::string& data, const std::string& key);
    static std::string decryptWithKey(const std::string& data, const std::string& key);
};
