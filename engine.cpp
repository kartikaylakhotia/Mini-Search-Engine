#include <bits/stdc++.h>
using namespace std;
namespace fs = std::filesystem;
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
using namespace __gnu_pbds;
#define ordered_set tree<int, null_type,less<int>, rb_tree_tag,tree_order_statistics_node_update>
# define endl "\n"
int MOD = 1e9 + 7;
int MOD2 = 998244353;
int INF = 1e18;
const int MAX_N = 2e6 + 5;

// Golden Rules
/*
    Solutions are simple.
    Proofs are simple.
    Implementations are simple.
*/

//Some Observations & Problem Statement
/*
 
*/

//Claims on Algorithm
/*
   
*/

struct DocInfo {
    int id;
    string path;
    int total_words;
};

unordered_set<string> stop_words = {
    "a", "about", "above", "after", "again", "against", "all", "am", "an", "and", "any", "are", "aren't", "as", "at", 
    "be", "because", "been", "before", "being", "below", "between", "both", "but", "by", 
    "can't", "cannot", "could", "couldn't", "did", "didn't", "do", "does", "doesn't", "doing", "don't", "down", "during", 
    "each", "few", "for", "from", "further", "had", "hadn't", "has", "hasn't", "have", "haven't", "having", "he", "he'd", "he'll", "he's", "her", "here", "here's", "hers", "herself", "him", "himself", "his", "how", "how's", 
    "i", "i'd", "i'll", "i'm", "i've", "if", "in", "into", "is", "isn't", "it", "it's", "its", "itself", 
    "let's", "me", "more", "most", "mustn't", "my", "myself", 
    "no", "nor", "not", "of", "off", "on", "once", "only", "or", "other", "ought", "our", "ours", "ourselves", "out", "over", "own", 
    "same", "shan't", "she", "she'd", "she'll", "she's", "should", "shouldn't", "so", "some", "such", 
    "than", "that", "that's", "the", "their", "theirs", "them", "themselves", "then", "there", "there's", "these", "they", "they'd", "they'll", "they're", "they've", "this", "those", "through", "to", "too", 
    "under", "until", "up", "very", 
    "was", "wasn't", "we", "we'd", "we'll", "we're", "we've", "were", "weren't", "what", "what's", "when", "when's", "where", "where's", "which", "while", "who", "who's", "whom", "why", "why's", "with", "won't", "would", "wouldn't", 
    "you", "you'd", "you'll", "you're", "you've", "your", "yours", "yourself", "yourselves"
};

string clean_token(string &token) {
    string cleaned = "";
    for (char c : token) {
        if (isalnum(c)) {
            cleaned += tolower(c);
        }
    }
    return cleaned;
}

vector<string> preprocess(const string &text) {
    vector<string> tokens;
    stringstream ss(text);
    string temp;
    while (ss >> temp) {
        string cleaned = clean_token(temp);
        if (cleaned.length() > 2 && stop_words.find(cleaned) == stop_words.end()) {
            tokens.push_back(cleaned);
        }
    }
    return tokens;
}

unordered_map<string, vector<pair<int, int>>> inverted_index;
unordered_map<int, DocInfo> documents;
vector<string> vocabulary; 
int doc_counter = 0;

void add_document(const fs::path &filepath) {
    ifstream file(filepath);
    if (!file.is_open()) return;

    stringstream buffer;
    buffer << file.rdbuf();
    string content = buffer.str();

    vector<string> tokens = preprocess(content);
    
    if (tokens.empty()) return;

    int doc_id = ++doc_counter;
    documents[doc_id] = {doc_id, filepath.string(), (int)tokens.size()};

    unordered_map<string, int> word_counts;
    for (const string &word : tokens) {
        word_counts[word]++;
    }

    for (auto &pair : word_counts) {
        inverted_index[pair.first].push_back({doc_id, pair.second});
    }
}

void build_index(const string &directory_path) {
    auto start = chrono::high_resolution_clock::now();
    cout << "Indexing documents from: " << directory_path << " ..." << endl;

    if (!fs::exists(directory_path)) {
        cerr << "Error: Dataset directory not found!" << endl;
        exit(1);
    }

    for (const auto &entry : fs::recursive_directory_iterator(directory_path)) {
        if (entry.is_regular_file()) {
            add_document(entry.path());
        }
    }

    for (auto const& [word, val] : inverted_index) {
        vocabulary.push_back(word);
    }
    sort(vocabulary.begin(), vocabulary.end());

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;
    
    cout << "Indexing Complete." << endl;
    cout << "Total Documents: " << doc_counter << endl;
    cout << "Total Unique Terms: " << inverted_index.size() << endl;
    cout << "Time taken to index: " << elapsed.count() << " seconds" << endl;
    cout << "------------------------------------------------" << endl;
}

vector<pair<double, int>> search_query(const string &query) {
    vector<string> query_tokens = preprocess(query);
    unordered_map<int, double> doc_scores;
    
    if (query_tokens.empty()) return {};

    for (const string &term : query_tokens) {
        if (inverted_index.find(term) == inverted_index.end()) continue;

        const vector<pair<int, int>> &postings = inverted_index[term];
        int doc_freq = postings.size();
        double idf = log10((double)doc_counter / (double)(1 + doc_freq)); 

        for (const auto &p : postings) {
            int doc_id = p.first;
            int term_freq = p.second;
            int total_terms = documents[doc_id].total_words;
            
            double tf = (double)term_freq / (double)total_terms;
            doc_scores[doc_id] += tf * idf;
        }
    }

    vector<pair<double, int>> ranked_results;
    for (auto &p : doc_scores) {
        ranked_results.push_back({p.second, p.first});
    }

    sort(ranked_results.rbegin(), ranked_results.rend());
    return ranked_results;
}

vector<string> autocomplete(string prefix) {
    vector<string> suggestions;
    prefix = clean_token(prefix); 
    
    if (prefix.empty()) return suggestions;

    auto it = lower_bound(vocabulary.begin(), vocabulary.end(), prefix);

    int count = 0;
    while (it != vocabulary.end() && count < 5) {
        if (it->find(prefix) == 0) { 
            suggestions.push_back(*it);
            count++;
        } else {
            break; 
        }
        it++;
    }
    return suggestions;
}

int levenshtein_distance(const string &s1, const string &s2) {
    int m = s1.length();
    int n = s2.length();
    
    if (abs(m - n) > 2) return 100; 

    vector<vector<int>> dp(m + 1, vector<int>(n + 1));

    for (int i = 0; i <= m; i++) dp[i][0] = i;
    for (int j = 0; j <= n; j++) dp[0][j] = j;

    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            if (s1[i - 1] == s2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                dp[i][j] = 1 + std::min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]});
            }
        }
    }
    return dp[m][n];
}

string get_suggestion(string word) {
    word = clean_token(word);
    if (word.empty() || inverted_index.count(word)) return ""; 

    string best_match = "";
    int min_dist = 3; 

    for (const string &term : vocabulary) {
        
        if (abs((int)term.length() - (int)word.length()) > 2) continue;
        if (term[0] != word[0]) continue; 

        int dist = levenshtein_distance(word, term);
        if (dist < min_dist) {
            min_dist = dist;
            best_match = term;
        }
    }
    return best_match;
}

signed main() {
    string dataset_path = "20_newsgroups"; 
    
    build_index(dataset_path);

    string input;
    while (true) {
        cout << "\nEnter search query (or type 'exit' to quit): ";
        getline(cin, input);

        if (input == "exit") break;

        
        stringstream ss(input);
        string last_word;
        while(ss >> last_word) {}
        vector<string> auto_sug = autocomplete(last_word);
        if (!auto_sug.empty()) {
            cout << "Autocomplete: ";
            for (const string& s : auto_sug) cout << s << " ";
            cout << endl;
        }

        
        stringstream ss2(input);
        string temp_word;
        string corrected_query = "";
        bool typo_found = false;
        
        while(ss2 >> temp_word) {
            string suggestion = get_suggestion(temp_word);
            if (!suggestion.empty()) {
                cout << "Did you mean '" << suggestion << "' instead of '" << temp_word << "'?" << endl;
                corrected_query += suggestion + " ";
                typo_found = true;
            } else {
                corrected_query += temp_word + " ";
            }
        }

        auto start_search = chrono::high_resolution_clock::now();
        
        vector<pair<double, int>> results = search_query(input);
        
        auto end_search = chrono::high_resolution_clock::now();
        chrono::duration<double> search_time = end_search - start_search;

        cout << "\nResults found: " << results.size() << endl;
        cout << "Search time: " << search_time.count() << " seconds" << endl;
        
        int limit = std::min<int>(results.size(), 5);
        if (limit > 0) {
            cout << "Top " << limit << " results:" << endl;
            for (int i = 0; i < limit; i++) {
                int doc_id = results[i].second;
                double score = results[i].first;
                cout << "[" << i + 1 << "] Score: " << score << " | File: " << documents[doc_id].path << endl;
            }
        } else {
            cout << "No matching documents found." << endl;
        }
    }

    return 0;
}
