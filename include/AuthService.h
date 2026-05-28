#pragma once
#include <string>

class AuthService {
public:
    static bool login(const std::string& username, const std::string& password);
    static bool registerUser(const std::string& username, const std::string& password);
    static std::string getCurrentUser();
    static std::string getSessionKey();
    static void logout();

private:
    static std::string current_user;
    static std::string session_key;
};
