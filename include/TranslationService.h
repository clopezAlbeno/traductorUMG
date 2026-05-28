#pragma once
#include "AVLTree.h"
#include <string>

class TranslationService {
public:
    TranslationService();

    // Fase 1: manejo de archivo local
    static void initDefaultDictionary();
    void loadFromFile(const std::string& filepath);
    void saveToFile(const std::string& filepath);
    void addWord(const WordRecord& word);
    void removeWord(const std::string& spanish_word);
    WordRecord* searchLocal(const std::string& spanish_word);
    void clear();

    // Audio TTS
    void playAudio(const std::string& text, const std::string& lang_code);

    // API MyMemory
    std::string translatePhraseAPI(const std::string& phrase, const std::string& langpair);

private:
    AVLTree dictionary;
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
};
