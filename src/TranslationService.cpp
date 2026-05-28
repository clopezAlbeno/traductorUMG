#include "TranslationService.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <filesystem>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

// Diccionario por defecto con 30 palabras español → italiano, francés, alemán, inglés
static const char* DEFAULT_WORDS[] = {
    "hola,ciao,bonjour,hallo,hello",
    "gracias,grazie,merci,danke,thank you",
    "casa,casa,maison,haus,house",
    "agua,acqua,eau,wasser,water",
    "libro,libro,livre,buch,book",
    "perro,cane,chien,hund,dog",
    "gato,gatto,chat,katze,cat",
    "sol,sole,soleil,sonne,sun",
    "luna,luna,lune,mond,moon",
    "amor,amore,amour,liebe,love",
    "tiempo,tempo,temps,zeit,time",
    "ciudad,città,ville,stadt,city",
    "amigo,amico,ami,freund,friend",
    "madre,madre,mère,mutter,mother",
    "padre,padre,père,vater,father",
    "escuela,scuola,école,schule,school",
    "trabajo,lavoro,travail,arbeit,work",
    "comida,cibo,nourriture,essen,food",
    "dia,giorno,jour,tag,day",
    "noche,notte,nuit,nacht,night",
    "verde,verde,vert,grün,green",
    "rojo,rosso,rouge,rot,red",
    "azul,blu,bleu,blau,blue",
    "blanco,bianco,blanc,weiß,white",
    "negro,nero,noir,schwarz,black",
    "grande,grande,grand,groß,big",
    "pequeño,piccolo,petit,klein,small",
    "bueno,buono,bon,gut,good",
    "feliz,felice,heureux,glücklich,happy",
    "nuevo,nuovo,nouveau,neu,new",
    nullptr
};

TranslationService::TranslationService() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

void TranslationService::initDefaultDictionary() {
    fs::create_directories("data");
    if (fs::exists("data/dictionary.csv")) return;

    std::ofstream file("data/dictionary.csv");
    if (!file.is_open()) return;
    for (int i = 0; DEFAULT_WORDS[i] != nullptr; i++) {
        file << DEFAULT_WORDS[i] << "\n";
    }
}

void TranslationService::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string sp, it, fr, de, en;
        if (std::getline(ss, sp, ',') &&
            std::getline(ss, it, ',') &&
            std::getline(ss, fr, ',') &&
            std::getline(ss, de, ',') &&
            std::getline(ss, en)) {
            dictionary.insert({sp, it, fr, de, en});
        }
    }
}

void TranslationService::saveToFile(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) return;
    for (const auto& w : dictionary.getAll()) {
        file << w.spanish << "," << w.italian << "," << w.french << "," << w.german << "," << w.english << "\n";
    }
}

void TranslationService::addWord(const WordRecord& word)              { dictionary.insert(word); }
void TranslationService::removeWord(const std::string& spanish_word) { dictionary.remove(spanish_word); }
WordRecord* TranslationService::searchLocal(const std::string& w)    { return dictionary.search(w); }
void TranslationService::clear()                                      { dictionary.clear(); }

void TranslationService::playAudio(const std::string& text, const std::string& lang_code) {
    // Voces macOS: IT=Alice FR=Thomas DE=Anna EN=Daniel ES=Jorge
    std::string voice = "Jorge";
    if      (lang_code == "it") voice = "Alice";
    else if (lang_code == "fr") voice = "Thomas";
    else if (lang_code == "de") voice = "Anna";
    else if (lang_code == "en") voice = "Daniel";

    // Sanitizar texto para evitar inyección de comandos
    std::string safe_text;
    for (char c : text) {
        if (c == '"' || c == '\\' || c == '`' || c == '$' || c == '!' || c == ';') {
            safe_text += ' ';
        } else {
            safe_text += c;
        }
    }

    std::string cmd = "say -v " + voice + " \"" + safe_text + "\" &";
    system(cmd.c_str());
    std::cout << "Reproduciendo audio en idioma '" << lang_code << "'...\n";
}

size_t TranslationService::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string TranslationService::translatePhraseAPI(const std::string& phrase, const std::string& langpair) {
    CURL* curl = curl_easy_init();
    if (!curl) return "Error: no se pudo inicializar CURL";

    std::string readBuffer;
    char* encoded = curl_easy_escape(curl, phrase.c_str(), (int)phrase.length());
    std::string url = "https://api.mymemory.translated.net/get?q=" + std::string(encoded) + "&langpair=" + langpair;
    curl_free(encoded);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) return std::string("Error CURL: ") + curl_easy_strerror(res);

    try {
        auto j = json::parse(readBuffer);
        if (j.contains("responseData") && j["responseData"].contains("translatedText")) {
            return j["responseData"]["translatedText"].get<std::string>();
        }
        if (j.contains("responseDetails")) {
            return "Sin traduccion: " + j["responseDetails"].get<std::string>();
        }
    } catch (...) {
        return "Error al parsear respuesta de MyMemory API";
    }
    return "Sin resultado de MyMemory API";
}
