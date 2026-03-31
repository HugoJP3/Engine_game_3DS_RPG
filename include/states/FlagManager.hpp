// Gestión de la memoria del juego

#pragma once
#include <map>
#include <string>

class FlagManager {
private:
    std::map<std::string, bool> flags; // flags activas-desactivas

public:
    void setFlag(std::string key, bool value) { flags[key] = value; }
    
    bool getFlag(std::string key) {
        if (flags.find(key) == flags.end()) return false;
        return flags[key];
    }

    std::map<std::string, bool>& getCurrentFlags() { return flags; }
};