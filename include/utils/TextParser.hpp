// Formateo de textos (strings)

#pragma once
#include <string>

class TextParser {
    public:
        // Función auxiliar para limpiar strings
        static std::string trim(const std::string& s) {
            // Elimina espacios, tabs y retornos de carro al inicio y al final
            size_t first = s.find_first_not_of(" \t\r\n");
            if (std::string::npos == first) return "";
            size_t last = s.find_last_not_of(" \t\r\n");
            return s.substr(first, (last - first + 1));
        }

        static std::string unescape(std::string s) {
            size_t pos = 0;
            while ((pos = s.find("\\n", pos)) != std::string::npos)
                s.replace(pos, 2, "\n");

            pos = 0;
            while ((pos = s.find("\\t", pos)) != std::string::npos)
                s.replace(pos, 2, "\t");

            return s;
        }
};