#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include <Windows.h>

using json = nlohmann::ordered_json;

std::unordered_map<std::string, std::string> buildReverseTable(const json& j) {
    std::unordered_map<std::string, std::string> reverseTable;
    for (auto& [letter, codes] : j.items()) {
        for (auto& code : codes) {
            reverseTable[code.get<std::string>()] = letter;
        }
    }
    return reverseTable;
}

std::string decrypt(
    const std::string& cipher,
    const std::unordered_map<std::string, std::string>& reverseTable,
    std::map<std::string, std::pair<int, double>>& charStats,
    std::map<int, std::pair<int, double>>& codeStats
) {
    std::string result;

    int totalCodes = 0;

    for (size_t i = 0; i + 3 <= cipher.size(); i += 3) {
        std::string code = cipher.substr(i, 3);
        totalCodes++;

        auto it = reverseTable.find(code);
        if (it != reverseTable.end()) {
            result += it->second;

            charStats[it->second].first++;
        }
        else {
            result += "?";
        }

        int numeric = std::stoi(code);
        codeStats[numeric].first++;
    }

    for (auto& [c, st] : charStats) {
        st.second = static_cast<double>(st.first) / totalCodes;
    }
    for (auto& [c, st] : codeStats) {
        st.second = static_cast<double>(st.first) / totalCodes;
    }
    std::cout << result.size();
    return result;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);

    std::map<std::string, std::pair<int, double>> charStats; 
    std::map<int, std::pair<int, double>> codeStats;         

    std::ifstream f("table.json");
    if (!f) {
        std::cerr << "Не вдалось відкрити table.json\n";
        return 1;
    }
    json j;
    f >> j;
    f.close();

    auto reverseTable = buildReverseTable(j);

    std::ifstream fin("cipher_text.txt", std::ios::binary);
    if (!fin) {
        std::cerr << "Не вдалось відкрити cipher_text.txt\n";
        return 1;
    }
    std::string cipher((std::istreambuf_iterator<char>(fin)), {});
    fin.close();

    std::string plain = decrypt(cipher, reverseTable, charStats, codeStats);

    std::ofstream fout("decrypted_text.txt", std::ios::binary);
    fout << plain;
    fout.close();

    std::cout << "Дешифрування завершено. Результат записано у decrypted_text.txt\n\n";

    std::cout << "Статистика по кодах:\n";
    for (auto& [code, stat] : codeStats) {
        std::cout << code << " → count=" << stat.first << ", freq=" << stat.second * 100 << "%" << "\n";
    }

    std::cout << "\nСтатистика по символах:\n";
    for (auto& [ch, stat] : charStats) {
        std::cout << ch << " → count=" << stat.first << ", freq=" << stat.second * 100 << "%" <<  "\n";
    }

    return 0;
}
