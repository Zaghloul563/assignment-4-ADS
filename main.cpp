#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

constexpr size_t ALPHABET_SIZE = 256;
constexpr long long HASH_BASE = 256;
constexpr long long HASH_PRIME = 1000003;

struct SearchResult {
    string algorithm;
    string pattern;
    vector<size_t> matches;
    long long comparisons = 0;
    double elapsedMs = 0.0;
};

string currentText;
vector<SearchResult> history;

vector<string> splitPatterns(const string& line) {
    vector<string> patterns;
    string item;
    stringstream ss(line);

    while (getline(ss, item, ',')) {
        size_t first = item.find_first_not_of(" \t\r\n");
        size_t last = item.find_last_not_of(" \t\r\n");
        if (first != string::npos) {
            patterns.push_back(item.substr(first, last - first + 1));
        }
    }

    if (patterns.empty() && !line.empty()) {
        patterns.push_back(line);
    }

    return patterns;
}

string readLine(const string& prompt) {
    cout << prompt;
    string line;
    getline(cin, line);
    return line;
}

string highlightedText(const string& text, const vector<size_t>& matches, const string& pattern) {
    if (matches.empty() || pattern.empty()) {
        return text;
    }

    vector<bool> starts(text.size(), false);
    for (size_t index : matches) {
        if (index < text.size()) {
            starts[index] = true;
        }
    }

    string output;
    for (size_t i = 0; i < text.size(); ++i) {
        if (starts[i]) {
            output += '[';
        }

        output += text[i];

        for (size_t index : matches) {
            if (index + pattern.size() == i + 1) {
                output += ']';
            }
        }
    }

    return output;
}

void printStatistics(const SearchResult& result) {
    cout << "\nAlgorithm used: " << result.algorithm << '\n';
    cout << "Pattern: " << result.pattern << '\n';
    cout << "Number of comparisons: " << result.comparisons << '\n';
    cout << "Number of matches: " << result.matches.size() << '\n';
    cout << fixed << setprecision(4);
    cout << "Execution time (ms): " << result.elapsedMs << '\n';

    if (result.matches.empty()) {
        cout << "No match found.\n";
    } else {
        for (size_t index : result.matches) {
            cout << "Match at index: " << index << '\n';
        }
        cout << "Highlighted text:\n" << highlightedText(currentText, result.matches, result.pattern) << '\n';
    }
}

SearchResult boyerMooreSearchForPattern(const string& text, const string& pattern) {
    SearchResult result;
    result.algorithm = "Boyer-Moore";
    result.pattern = pattern;

    auto start = chrono::high_resolution_clock::now();

    if (pattern.empty() || text.size() < pattern.size()) {
        auto end = chrono::high_resolution_clock::now();
        result.elapsedMs = chrono::duration<double, milli>(end - start).count();
        return result;
    }

    vector<int> badCharacter(ALPHABET_SIZE, -1);
    for (size_t i = 0; i < pattern.size(); ++i) {
        badCharacter[static_cast<unsigned char>(pattern[i])] = static_cast<int>(i);
    }

    size_t shift = 0;
    while (shift <= text.size() - pattern.size()) {
        int j = static_cast<int>(pattern.size()) - 1;

        while (j >= 0) {
            ++result.comparisons;
            if (pattern[static_cast<size_t>(j)] != text[shift + static_cast<size_t>(j)]) {
                break;
            }
            --j;
        }

        if (j < 0) {
            result.matches.push_back(shift);
            ++shift; // Allows overlapping matches.
        } else {
            unsigned char mismatched = static_cast<unsigned char>(text[shift + static_cast<size_t>(j)]);
            int lastSeen = badCharacter[mismatched];
            size_t moveBy = max(1, j - lastSeen);
            shift += static_cast<size_t>(moveBy);
        }
    }

    auto end = chrono::high_resolution_clock::now();
    result.elapsedMs = chrono::duration<double, milli>(end - start).count();
    return result;
}

SearchResult rabinKarpSearchForPattern(const string& text, const string& pattern) {
    SearchResult result;
    result.algorithm = "Rabin-Karp";
    result.pattern = pattern;

    auto start = chrono::high_resolution_clock::now();

    if (pattern.empty() || text.size() < pattern.size()) {
        auto end = chrono::high_resolution_clock::now();
        result.elapsedMs = chrono::duration<double, milli>(end - start).count();
        return result;
    }

    long long patternHash = 0;
    long long windowHash = 0;
    long long highestPower = 1;

    for (size_t i = 0; i < pattern.size() - 1; ++i) {
        highestPower = (highestPower * HASH_BASE) % HASH_PRIME;
    }

    for (size_t i = 0; i < pattern.size(); ++i) {
        patternHash = (HASH_BASE * patternHash + static_cast<unsigned char>(pattern[i])) % HASH_PRIME;
        windowHash = (HASH_BASE * windowHash + static_cast<unsigned char>(text[i])) % HASH_PRIME;
    }

    for (size_t i = 0; i <= text.size() - pattern.size(); ++i) {
        if (patternHash == windowHash) {
            bool matched = true;
            for (size_t j = 0; j < pattern.size(); ++j) {
                ++result.comparisons;
                if (text[i + j] != pattern[j]) {
                    matched = false;
                    break;
                }
            }
            if (matched) {
                result.matches.push_back(i);
            }
        } else {
            ++result.comparisons;
        }

        if (i < text.size() - pattern.size()) {
            windowHash = (HASH_BASE * (windowHash - static_cast<unsigned char>(text[i]) * highestPower)
                          + static_cast<unsigned char>(text[i + pattern.size()])) % HASH_PRIME;
            if (windowHash < 0) {
                windowHash += HASH_PRIME;
            }
        }
    }

    auto end = chrono::high_resolution_clock::now();
    result.elapsedMs = chrono::duration<double, milli>(end - start).count();
    return result;
}

void runSearch(const string& algorithm) {
    if (currentText.empty()) {
        cout << "No text loaded. Use option 1 or 2 first.\n";
        return;
    }

    string line = readLine("Enter pattern(s), separated by commas for multiple patterns: ");
    vector<string> patterns = splitPatterns(line);

    if (patterns.empty()) {
        cout << "No pattern entered.\n";
        return;
    }

    for (const string& pattern : patterns) {
        SearchResult result = algorithm == "Boyer-Moore"
            ? boyerMooreSearchForPattern(currentText, pattern)
            : rabinKarpSearchForPattern(currentText, pattern);
        history.push_back(result);
        printStatistics(result);
    }
}

void loadFile() {
    string fileName = readLine("Enter file name: ");
    ifstream file(fileName);

    if (!file) {
        cout << "Unable to open file: " << fileName << '\n';
        return;
    }

    stringstream buffer;
    buffer << file.rdbuf();
    currentText = buffer.str();

    if (!currentText.empty() && currentText.back() == '\n') {
        currentText.pop_back();
    }

    cout << "Loaded " << currentText.size() << " characters.\n";
}

void manualInput() {
    currentText = readLine("Enter text: ");
    cout << "Stored " << currentText.size() << " characters.\n";
}

void boyerMooreSearch() {
    runSearch("Boyer-Moore");
}

void rabinKarpSearch() {
    runSearch("Rabin-Karp");
}

void compareAlgorithms() {
    if (currentText.empty()) {
        cout << "No text loaded. Use option 1 or 2 first.\n";
        return;
    }

    string line = readLine("Enter pattern(s), separated by commas for multiple patterns: ");
    vector<string> patterns = splitPatterns(line);

    if (patterns.empty()) {
        cout << "No pattern entered.\n";
        return;
    }

    for (const string& pattern : patterns) {
        SearchResult bm = boyerMooreSearchForPattern(currentText, pattern);
        SearchResult rk = rabinKarpSearchForPattern(currentText, pattern);
        history.push_back(bm);
        history.push_back(rk);

        cout << "\nComparison for pattern: " << pattern << '\n';
        cout << left << setw(15) << "Algorithm"
             << right << setw(15) << "Comparisons"
             << setw(12) << "Matches"
             << setw(16) << "Time (ms)" << '\n';
        cout << string(58, '-') << '\n';
        cout << left << setw(15) << bm.algorithm
             << right << setw(15) << bm.comparisons
             << setw(12) << bm.matches.size()
             << setw(16) << fixed << setprecision(4) << bm.elapsedMs << '\n';
        cout << left << setw(15) << rk.algorithm
             << right << setw(15) << rk.comparisons
             << setw(12) << rk.matches.size()
             << setw(16) << fixed << setprecision(4) << rk.elapsedMs << '\n';

        if (bm.matches == rk.matches) {
            cout << "Both algorithms found the same match indexes.\n";
        } else {
            cout << "Warning: algorithms returned different match indexes.\n";
        }
    }
}

void printStatistics() {
    if (history.empty()) {
        cout << "No searches have been run yet.\n";
        return;
    }

    cout << "\nSearch history:\n";
    for (size_t i = 0; i < history.size(); ++i) {
        cout << "\n#" << i + 1 << '\n';
        printStatistics(history[i]);
    }
}

void showMenu() {
    cout << "\nString Processing Menu\n";
    cout << "1. Load Text File\n";
    cout << "2. Enter Text Manually\n";
    cout << "3. Search Using Boyer-Moore\n";
    cout << "4. Search Using Rabin-Karp\n";
    cout << "5. Compare Algorithms\n";
    cout << "6. Exit\n";
    cout << "Choice: ";
}

int main() {
    while (true) {
        showMenu();

        int choice;
        if (!(cin >> choice)) {
            cout << "Invalid input. Exiting.\n";
            return 0;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {
            case 1:
                loadFile();
                break;
            case 2:
                manualInput();
                break;
            case 3:
                boyerMooreSearch();
                break;
            case 4:
                rabinKarpSearch();
                break;
            case 5:
                compareAlgorithms();
                break;
            case 6:
                cout << "Goodbye.\n";
                return 0;
            default:
                cout << "Invalid choice. Try again.\n";
        }
    }
}
