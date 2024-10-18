#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <iomanip>

using namespace std;

struct WordCount {
    string word;
    int count;
    double prior;

    bool operator<(const WordCount& other) const {
        return prior > other.prior; // Sort in descending order of prior
    }
};

// Function to convert correctly guessed letters to string
string convert_to_string(const vector<char>& guessed) {
    string result;
    for (char c : guessed) {
        if (c != '\0') {
            result += c; // Only add guessed characters
        } else {
            result += '_'; // Use '_' for unguessed characters for better visualization
        }
    }
    return result;
}

// Function to check if a word is valid based on the guessed letters
bool is_valid_word(const string& word, const vector<char>& correctly_guessed, const set<char>& incorrectly_guessed) {
    for (size_t i = 0; i < correctly_guessed.size(); ++i) {
        if (correctly_guessed[i] != '\0' && word[i] != correctly_guessed[i]) {
            return false;
        }
        // Check for letters that should not be in the word
        if (correctly_guessed[i] == '\0' && incorrectly_guessed.count(word[i])) {
            return false;
        }
    }
    for (char letter : incorrectly_guessed) {
        if (word.find(letter) != string::npos) {
            return false;
        }
    }
    return true;
}

// Function to compute posterior probabilities
vector<double> find_posteriors(vector<WordCount>& word_counts, const vector<char>& correctly_guessed, const set<char>& incorrectly_guessed) {
    vector<double> posteriors(word_counts.size(), 0.0);
    double total_posterior = 0.0;

    for (size_t i = 0; i < word_counts.size(); i++) {
        bool valid = is_valid_word(word_counts[i].word, correctly_guessed, incorrectly_guessed);
        posteriors[i] = valid ? word_counts[i].prior : 0.0;
        total_posterior += posteriors[i];
    }

    for (double& p : posteriors) {
        p /= total_posterior; // Normalize the posterior probabilities
    }
    return posteriors;
}

// Function to calculate predictive probability for a letter
double predictive_probability(char letter, vector<WordCount>& word_counts, const vector<double>& posteriors, const vector<char>& correctly_guessed) {
    double probability = 0.0;

    for (size_t i = 0; i < word_counts.size(); ++i) {
        if (word_counts[i].word.find(letter) != string::npos) {
            bool is_guessed = false;
            for (char j : correctly_guessed) {
                if (j == letter) {
                    is_guessed = true;
                    break;
                }
            }
            if (!is_guessed) {
                probability += posteriors[i];
            }
        }
    }
    return probability;
}

int main() {
    ifstream infile;
    infile.open("hw1_word_counts_05-1.txt");
    if (!infile.is_open()) {
        cerr << "Error: Could not open the file!" << endl;
        return 1; // Exit the program if the file can't be opened
    }
    vector<WordCount> word_counts;
    string line;

    // Read word counts from file
    while (getline(infile, line)) {
        istringstream iss(line);
        string word;
        int count;
        iss >> word >> count;
        word_counts.push_back({word, count, 0.0});
    }
    infile.close();

    // Calculate total count and prior probabilities
    int total_count = 0;
    for (const auto& wc : word_counts) {
        total_count += wc.count;
    }
    for (auto& wc : word_counts) {
        wc.prior = static_cast<double>(wc.count) / total_count;
    }

    // Sort words by prior probability
    sort(word_counts.begin(), word_counts.end());

    // Print most and least probable words
    size_t count_to_print = min(static_cast<size_t>(14), word_counts.size());
    cout << "MOST:\tLEAST:\n";
    for (size_t i = 0; i < count_to_print; ++i) {
        cout << word_counts[i].word << "\t";
        cout << word_counts[word_counts.size() - 1 - i].word << "\n";
    }

    // Printing the 15th word
    if (word_counts.size() > 14) {
        cout << word_counts[14].word << "\n"; // Print the middle word for reference
    }

    // Define correctly guessed letters
    vector<vector<char>> correctly_guessed_list = {
        {'\0', '\0', '\0', '\0', '\0'},
        {'\0', '\0', '\0', '\0', '\0'},
        {'A', '\0', '\0', '\0', 'S'},
        {'A', '\0', '\0', '\0', 'S'},
        {'\0', '\0', 'O', '\0', '\0'},
        {'\0', '\0', '\0', '\0', '\0'},
        {'D', '\0', '\0', 'I', '\0'},
        {'D', '\0', '\0', 'I', '\0'},
        {'\0', 'U', '\0', '\0', '\0'}
    };

    // Define incorrectly guessed letters
    vector<set<char>> incorrectly_guessed_list = {
        {},
        {'E', 'A'},
        {},
        {'I'},
        {'A', 'E', 'M', 'N', 'T'},
        {'E', 'O'},
        {},
        {'A'},
        {'A', 'E', 'I', 'O', 'S'}
    };

    // Prepare to find the best letter to guess
    string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    vector<char> best_letters;
    vector<double> best_probs;

    for (size_t i = 0; i < correctly_guessed_list.size(); ++i) {
        const auto& correctly_guessed = correctly_guessed_list[i];
        const auto& incorrectly_guessed = incorrectly_guessed_list[i];

        vector<double> posteriors = find_posteriors(word_counts, correctly_guessed, incorrectly_guessed);
        char best_letter = '\0';
        double best_probability = 0.0;

        for (char letter : alphabet) {
            if (incorrectly_guessed.count(letter) == 0 && find(correctly_guessed.begin(), correctly_guessed.end(), letter) == correctly_guessed.end()) {
                double probability = predictive_probability(letter, word_counts, posteriors, correctly_guessed);
                if (probability > best_probability) {
                    best_probability = probability;
                    best_letter = letter;
                }
            }
        }
        best_letters.push_back(best_letter);
        best_probs.push_back(best_probability);
    }

    // Print results
    for (size_t i = 0; i < best_letters.size(); ++i) {
        cout << convert_to_string(correctly_guessed_list[i]) << " "
             << (incorrectly_guessed_list[i].empty() ? "" : string(incorrectly_guessed_list[i].begin(), incorrectly_guessed_list[i].end())) << " "
             << best_letters[i] << " "
             << fixed << setprecision(4) << best_probs[i] << "\n";
    }

    return 0;
}