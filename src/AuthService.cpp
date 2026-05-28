#include "AuthService.h"
#include "CipherPhase2.h"
#include <fstream>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

std::string AuthService::current_user = "";
std::string AuthService::session_key  = "";

bool AuthService::login(const std::string& username, const std::string& password) {
    fs::create_directories("data/users");
    std::ifstream file("data/users/" + username + ".user");
    if (!file.is_open()) return false;

    std::string stored;
    std::getline(file, stored);

    // Fase 3: contraseñas guardadas con cifrado Fase 2
    std::string encrypted_input = CipherPhase2::encrypt(password);
    if (stored != encrypted_input) return false;

    current_user = username;
    // Clave de sesión = cifrado Fase 2 de (usuario + contraseña) → usada como clave XOR en Fase 3
    session_key = CipherPhase2::encrypt(username + password);
    return true;
}

bool AuthService::registerUser(const std::string& username, const std::string& password) {
    fs::create_directories("data/users");
    std::string path = "data/users/" + username + ".user";
    if (fs::exists(path)) return false;

    std::ofstream file(path);
    if (!file.is_open()) return false;

    // Fase 3: guardar contraseña encriptada con cifrado Fase 2
    file << CipherPhase2::encrypt(password) << "\n";
    return true;
}

std::string AuthService::getCurrentUser() { return current_user; }
std::string AuthService::getSessionKey()  { return session_key;  }

void AuthService::logout() {
    current_user = "";
    session_key  = "";
}
