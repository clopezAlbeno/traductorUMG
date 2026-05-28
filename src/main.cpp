#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <filesystem>
#include "AuthService.h"
#include "TranslationService.h"
#include "HistoryService.h"
#include "CipherPhase2.h"

using namespace std;
namespace fs = std::filesystem;

// ──────────────────────────────────────────────
// Rutas
// ──────────────────────────────────────────────
static const string GLOBAL_DICT = "data/dictionary.csv";

static string userDictPath(const string& username) {
    return "data/users/" + username + ".traddb";
}

// ──────────────────────────────────────────────
// Helpers
// ──────────────────────────────────────────────
static void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// ──────────────────────────────────────────────
// Fase 3: cargar diccionario del usuario
// Formato .traddb:
//   Línea 1  → magic "TRAD2" (fase 3) o "TRAD" (fase 1/2 legacy)
//   Resto    → cada línea: cifrado XOR (hex) de texto cifrado con Fase 2
// ──────────────────────────────────────────────
static void loadUserDictionary(TranslationService& t, const string& path) {
    if (!fs::exists(path)) return;

    ifstream in(path, ios::binary);
    if (!in.is_open()) return;

    string magic;
    if (!getline(in, magic)) return;

    string temp = path + ".tmp";
    ofstream out(temp);
    string line;

    if (magic == "TRAD2") {
        // Fase 3: descifrar XOR → descifrar Fase 2
        while (getline(in, line)) {
            if (line.empty()) continue;
            string phase2 = CipherPhase2::decryptWithKey(line, AuthService::getSessionKey());
            out << CipherPhase2::decrypt(phase2) << "\n";
        }
    } else if (magic == "TRAD") {
        // Legado fase 1/2: solo Fase 2
        while (getline(in, line)) {
            if (line.empty()) continue;
            out << CipherPhase2::decrypt(line) << "\n";
        }
    }

    out.close();
    in.close();
    t.loadFromFile(temp);
    fs::remove(temp);
}

// ──────────────────────────────────────────────
// Fase 3: guardar diccionario del usuario
// ──────────────────────────────────────────────
static void saveUserDictionary(TranslationService& t, const string& path) {
    string temp = path + ".tmp";
    t.saveToFile(temp);

    ifstream in(temp);
    ofstream out(path, ios::binary);

    if (!in.is_open() || !out.is_open()) { fs::remove(temp); return; }

    out << "TRAD2\n"; // magic number del tipo de archivo propio (Fase 3)
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        // Fase 2 encrypt → Fase 3 XOR encrypt
        string phase2  = CipherPhase2::encrypt(line);
        string phase3  = CipherPhase2::encryptWithKey(phase2, AuthService::getSessionKey());
        out << phase3 << "\n";
    }

    in.close();
    out.close();
    fs::remove(temp);
}

// ──────────────────────────────────────────────
// Menú principal
// ──────────────────────────────────────────────
static void displayMenu() {
    cout << "\n============================================\n";
    cout << "        TRADUCTOR DEFINITIVO - FASES 1/2/3\n";
    cout << "============================================\n";
    cout << "  1. Buscar y traducir palabra (local)\n";
    cout << "  2. Agregar nueva palabra\n";
    cout << "  3. Eliminar palabra\n";
    cout << "  4. Traducir frase (API MyMemory)\n";
    cout << "  5. Ver top 5 busquedas\n";
    cout << "  6. Escuchar audio de traduccion\n";
    cout << "  7. Guardar y cerrar sesion\n";
    cout << "============================================\n";
    cout << "Opcion: ";
}

// ──────────────────────────────────────────────
// main
// ──────────────────────────────────────────────
int main() {
    fs::create_directories("data/users");

    // Fase 1: crear diccionario global por defecto si no existe
    TranslationService::initDefaultDictionary();

    TranslationService translator;

    while (true) {
        cout << "\n========== INICIO DE SESION ==========\n";
        cout << "  1. Iniciar sesion\n";
        cout << "  2. Registrarse\n";
        cout << "  3. Salir\n";
        cout << "Opcion: ";

        int opt;
        if (!(cin >> opt)) { clearInput(); continue; }

        if (opt == 3) break;

        string user, pass;
        cout << "Usuario: "; cin >> user;
        cout << "Password: "; cin >> pass;
        clearInput();

        if (opt == 1) {
            if (!AuthService::login(user, pass)) {
                cout << "Error: usuario o contraseña incorrectos.\n";
                continue;
            }
        } else if (opt == 2) {
            if (!AuthService::registerUser(user, pass)) {
                cout << "Error: el usuario '" << user << "' ya existe.\n";
                continue;
            }
            AuthService::login(user, pass);
        } else {
            cout << "Opcion invalida.\n";
            continue;
        }

        cout << "Bienvenido, " << AuthService::getCurrentUser() << "!\n";

        // Limpiar árbol de sesión anterior y cargar datos de este usuario
        translator.clear();
        translator.loadFromFile(GLOBAL_DICT);                         // Fase 1: diccionario global
        loadUserDictionary(translator, userDictPath(user));           // Fase 3: .traddb personal

        bool logged_in = true;
        while (logged_in) {
            displayMenu();
            int choice;
            if (!(cin >> choice)) { clearInput(); continue; }
            clearInput();

            switch (choice) {

            // ── Fase 1: Buscar localmente ──────────────────
            case 1: {
                cout << "Palabra en espanol: ";
                string word;
                getline(cin, word);

                WordRecord* rec = translator.searchLocal(word);
                if (rec) {
                    cout << "  Italiano : " << rec->italian  << "\n";
                    cout << "  Frances  : " << rec->french   << "\n";
                    cout << "  Aleman   : " << rec->german   << "\n";
                    cout << "  Ingles   : " << rec->english  << "\n";
                    // Fase 2: guardar historial cifrado
                    HistoryService::saveSearch(AuthService::getCurrentUser(), word);
                } else {
                    cout << "Palabra no encontrada en el diccionario local.\n";
                }
                break;
            }

            // ── Fase 1: Agregar palabra ────────────────────
            case 2: {
                WordRecord w;
                cout << "Espanol : "; getline(cin, w.spanish);
                cout << "Italiano: "; getline(cin, w.italian);
                cout << "Frances : "; getline(cin, w.french);
                cout << "Aleman  : "; getline(cin, w.german);
                cout << "Ingles  : "; getline(cin, w.english);
                translator.addWord(w);
                cout << "Palabra agregada.\n";
                break;
            }

            // ── Fase 1: Eliminar palabra ───────────────────
            case 3: {
                cout << "Palabra en espanol a eliminar: ";
                string word;
                getline(cin, word);
                translator.removeWord(word);
                cout << "Palabra eliminada (si existia).\n";
                break;
            }

            // ── Fase 1+2: Traducir frase con API MyMemory ─
            case 4: {
                cout << "Frase a traducir: ";
                string phrase;
                getline(cin, phrase);
                cout << "Idioma destino (it/fr/de/en): ";
                string lang;
                getline(cin, lang);

                cout << "Consultando MyMemory API...\n";
                string result = translator.translatePhraseAPI(phrase, "es|" + lang);
                cout << "Traduccion: " << result << "\n";
                // Fase 2: guardar historial cifrado
                HistoryService::saveSearch(AuthService::getCurrentUser(), phrase);
                break;
            }

            // ── Fase 2: Top 5 búsquedas (AVL historial) ───
            case 5: {
                auto top = HistoryService::getTopSearches(AuthService::getCurrentUser());
                cout << "--- Top 5 busquedas ---\n";
                if (top.empty()) {
                    cout << "  (sin busquedas aun)\n";
                } else {
                    for (size_t i = 0; i < top.size(); ++i)
                        cout << "  " << i + 1 << ". " << top[i] << "\n";
                }
                break;
            }

            // ── Fase 1: Audio TTS ──────────────────────────
            case 6: {
                cout << "Texto a pronunciar: ";
                string txt;
                getline(cin, txt);
                cout << "Idioma (es/it/fr/de/en): ";
                string lang;
                getline(cin, lang);
                translator.playAudio(txt, lang);
                break;
            }

            // ── Fase 3: Guardar y cerrar sesión ───────────
            case 7: {
                saveUserDictionary(translator, userDictPath(AuthService::getCurrentUser()));
                cout << "Diccionario guardado (cifrado Fase 2 + Fase 3).\n";
                AuthService::logout();
                logged_in = false;
                break;
            }

            default:
                cout << "Opcion invalida.\n";
            }
        }
    }

    cout << "Hasta luego.\n";
    return 0;
}
