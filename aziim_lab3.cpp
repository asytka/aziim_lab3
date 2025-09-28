#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include <Windows.h>
#include <gplot++.h>


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
        st.second = static_cast<double>(st.first) / result.size();
    }
    for (auto& [c, st] : codeStats) {
        st.second = static_cast<double>(st.first) / totalCodes;
    }

    return result;
}

void plotStats(Gnuplot& plt, const std::map<int, std::pair<int, double>>& codeStats) {
    std::vector<int> x;               
    std::vector<double> counts;      
    std::vector<int> labels;  

    int idx = 0;
    for (const auto& [ch, stats] : codeStats) {
        x.push_back(idx++);
        counts.push_back(stats.first);
        labels.push_back(ch);
    }

    std::ostringstream xtics;
    xtics << "(";
    for (size_t i = 0; i < labels.size(); ++i) {
        xtics << "\"" << labels[i] << "\" " << i;
        if (i != labels.size() - 1) xtics << ", ";
    }
    xtics << ")";

    plt.sendcommand("set title 'Character Statistics'");
    plt.sendcommand("set xlabel 'Character'");
    plt.sendcommand("set ylabel 'Occurrences'");
    plt.sendcommand("set style fill solid 0.5 border -1");
    plt.sendcommand("set style data histograms");
    plt.sendcommand("set boxwidth 0.9");
    plt.sendcommand("set xtics " + xtics.str());

    plt.plot(x, counts, "Occurrences", Gnuplot::LineStyle::BOXES);

    plt.show();
}

void plotCharStats(Gnuplot& plt, const std::map<std::string, std::pair<int, double>>& charStats) {
    std::vector<int> x;               
    std::vector<double> counts;       
    std::vector<std::string> labels;  

    int idx = 0;
    for (const auto& [ch, stats] : charStats) {
        x.push_back(idx++);
        counts.push_back(stats.first);
        labels.push_back(ch);
    }

    std::ostringstream xtics;
    xtics << "(";
    for (size_t i = 0; i < labels.size(); ++i) {
        xtics << "\"" << labels[i] << "\" " << i;
        if (i != labels.size() - 1) xtics << ", ";
    }
    xtics << ")";

    plt.sendcommand("set title 'Character Statistics'");
    plt.sendcommand("set xlabel 'Character'");
    plt.sendcommand("set ylabel 'Occurrences'");
    plt.sendcommand("set style fill solid 0.5 border -1");
    plt.sendcommand("set style data histograms");
    plt.sendcommand("set boxwidth 0.9");
    plt.sendcommand("set xtics " + xtics.str());

    plt.plot(x, counts, "Occurrences", Gnuplot::LineStyle::BOXES);

    plt.show();
}



int main() {
    std::map<std::string, std::pair<int, double>> charStats; // літера → (кількість, частота)
    std::map<int, std::pair<int, double>> codeStats;

    Gnuplot plt{};
    SetConsoleOutputCP(CP_UTF8);

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
        std::cout << code << " → count=" << stat.first << ", freq=" << stat.second << "\n";
    }

    std::cout << "\nСтатистика по символах:\n";
    for (auto& [ch, stat] : charStats) {
        std::cout << ch << " → count=" << stat.first << ", freq=" << stat.second << "\n";
    }

    //plotting
    //plotStats(plt, codeStats);
    //plotCharStats(plt, charStats);
    return 0;
}
