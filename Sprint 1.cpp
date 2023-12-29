#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <utility>

using namespace std;
const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string str;
    getline(cin, str);
    return str;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitInToWords(const string& text) {
    vector<string> words;
    string word;
    for (const char ch : text) {
        if (ch == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += ch;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    return words;
}


struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitInToWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {

        vector<string> split = SplitInToWordsNoStop(document);
        double term_frequency = 1. / split.size();
        for (const string& word : split) {
            word_to_document_freqs_[word][document_id] += term_frequency;
        }
        ++document_count_;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        vector<Document> top_documents;
        Query query = ParseQuery(raw_query);

        top_documents = FindAllDocuments(query);

        sort(top_documents.begin(), top_documents.end(),
            [](const Document& lhs, const Document& rhs) {return lhs.relevance > rhs.relevance; });

        if (top_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            top_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return top_documents;
    }

private:

    struct Query {
        set<string> minus_words;
        set<string> plus_words;
    };

    map<string, map<int, double>> word_to_document_freqs_;
    set<string> stop_words_;
    int document_count_ = 0;

    vector<string> SplitInToWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitInToWords(text)) {
            if (stop_words_.count(word) == 0) {
                words.push_back(word);
            }
        }
        return words;
    }

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitInToWordsNoStop(text)) {

            if (word[0] == '-') {
                query.minus_words.insert(word.substr(1));
            }
            else {
                query.plus_words.insert(word);
            }
        }
        return query;
    }

    double ComputInverseDocumentFrequency(const string& word) const {
        double idf = log((static_cast<double>(document_count_)) / 
            word_to_document_freqs_.at(word).size());
        return idf;
    }

    vector<Document> FindAllDocuments(Query query) const {
        vector<Document> relevant_documents;
        map<int, double> document_to_relevance;

        for (const string& plus_word : query.plus_words) {
            if (word_to_document_freqs_.count(plus_word)) {
                double inverse_document_frequency = ComputInverseDocumentFrequency(plus_word);
                for (const auto& [word, id_term_frequency] : word_to_document_freqs_.at(plus_word)) {
                    document_to_relevance[word] += id_term_frequency * inverse_document_frequency;
                }
            }
        }
        for (const string& minus_word : query.minus_words) {
            if (word_to_document_freqs_.count(minus_word)) {
                for (const auto& [word, id_term_frequency] : word_to_document_freqs_.at(minus_word)) {
                    document_to_relevance.erase(word);
                }
            }
        }
        for (const auto& [document_id, relevance] : document_to_relevance) {
            relevant_documents.push_back({ document_id,  relevance });
        }
        return relevant_documents;
    }
};

static SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());
    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }
    return search_server;
}

int main() {
    SearchServer search_server = CreateSearchServer();
    const auto& top_documents = search_server.FindTopDocuments(ReadLine());

    for (auto& [document_id, relevance] : top_documents) {
        cout << "{ document_id = "s << document_id << ", relevance = "s << relevance << " }"s
            << endl;
    }
}

/*
is are was a an in the with near at
3
a colorful parrot with green wings and red tail is lost
a grey hound with black ears is found at the railway station
a white cat with long furry tail is found near the red square
white cat long tail
*/
/*
is are was a an in the with near at
3
a colorful parrot with green wings and red tail is lost
a grey hound with black ears is found at the railway station
a white cat with long furry tail is found near the red square
white -cat long tali
*/