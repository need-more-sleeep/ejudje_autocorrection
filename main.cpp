#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <locale>
#include <functional>

//сложность обращения к следующей вершине O(1) тк хеш-таблица детей
struct Node {
    std::wstring prefix;
    std::unordered_map<wchar_t, Node*> children;
    bool end_of_word;

    Node() {}

    Node(std::wstring word_, bool end_of_word_) : prefix(word_), end_of_word(end_of_word_) {}

    bool operator==(const Node& rhs) const {
        return prefix == rhs.prefix && children == rhs.children && end_of_word == rhs.end_of_word;
    }
};

class Trie {
    Node* root = new Node(std::wstring(), false);

    int max_prefix(const std::wstring& word1, const std::wstring& word2);

    void split(Node& current_node, std::wstring& word, int index);

    //метод поиска подходящих исправлений по сжатому префиксному дереву
    // сложность O(n^2*k)
    //n- длина слова
    //k-мощность
    void trie_travel(std::wstring word, bool error, std::wstring code,
        int index, Node& node, std::vector<std::wstring>& answer);

    std::wstring substr(const std::wstring& word, int pos, int length = -1);

    void check_error(Node& node, std::wstring word, int index, int offset, std::wstring code,
        bool error, std::vector<std::wstring>& answer, std::wstring check_word);

    void destruct(Node* current_node);

public:
    ~Trie();

    std::vector<std::wstring> correction(std::wstring& word, std::locale& loc);

    //метод добавления слова в дерево
    // сложность O(n)
    //находим место в дереве за одно последовательное считывание слова
    void add(std::wstring& word, std::locale& loc);

    //метод поиска слова
    //сложность O(n)
    //проходим по дереву и ищем наше слово,
    //на каждом уровне поиск префикса за константу тк хеш_таблица
    bool find(std::wstring& word, std::locale& loc);
};

int Trie::max_prefix(const std::wstring& word1, const std::wstring& word2) {
    int min_len = std::min(word1.length(), word2.length());
    for (size_t i = 0; i < min_len; i++) {
        if (word1[i] != word2[i]) {
            return i;
        }
    }
    return min_len;
}

Trie::~Trie() {
    destruct(root);
}
void Trie::destruct(Node* current_node) {
    if (current_node != nullptr) {
        for (auto temp : current_node->children) {
            destruct(temp.second);
        }
        delete current_node;
    }
}

void Trie::split(Node& current_node, std::wstring& word, int index) {

    int split = max_prefix(substr(word, index), current_node.prefix);
    std::wstring equal_part = substr(current_node.prefix, 0, split);
    Node* new_node = new Node(substr(current_node.prefix, split), current_node.end_of_word);
    new_node->children = current_node.children;

    current_node.children.clear();
    current_node.children.emplace(current_node.prefix[split], new_node);

    std::wstring word_split = substr(word, index + split);

    if (!word_split.empty()) {
        current_node.children[word_split[0]] = new Node(word_split, true);
        current_node.end_of_word = false;
    }
    current_node.prefix = equal_part;
}

void Trie::trie_travel(std::wstring word, bool error, std::wstring code,
    int index, Node& node, std::vector<std::wstring>& answer) {
    std::wstring check_word = substr(word, index, node.prefix.length());
    if (node.prefix == check_word) {
        if (node.end_of_word && (word.length() == index + node.prefix.length()
            || (word.length() == index + 1 + node.prefix.length() && !error))) {
            answer.emplace_back(code + node.prefix);
        }

        for (auto& it : node.children) {
            trie_travel(word, error, code + node.prefix, index + node.prefix.length(), *it.second, answer);
        }
        return;
    }
    if (error) {
        return;
    }
    error = true;
    int offset = max_prefix(node.prefix, check_word);
    check_error(node, word, index, offset, code, error, answer, check_word);
}

std::wstring Trie::substr(const std::wstring& word, int pos, int length) {
    if (pos >= word.length() || length == 0) {
        return L"";
    }
    if (length == -1) {
        return word.substr(pos);
    }

    if (pos + length - 1 < word.length()) {
        return word.substr(pos, length);
    }

    return word.substr(pos);

}

void Trie::check_error(Node& node, std::wstring word, int index, int offset, std::wstring code,
    bool error, std::vector<std::wstring>& answer, std::wstring check_word) {

    //изменили 1 символ
    bool replace_error = substr(node.prefix, offset + 1) == substr(check_word, offset + 1);

    //чтобы не брать подстроку несколько раз
    std::wstring buf = substr(node.prefix, offset + 1);

    //пропустили 1 символ
    bool delete_error = buf == substr(check_word, offset, buf.length());

    buf = substr(node.prefix, offset);

    //добавили 1 символ
    bool insert_error = buf == substr(word, index + offset + 1, buf.length());

    //транспозиция двух соседних символов
    bool transpose_error = false;


    if (word.length() > index + offset + 1 && offset == node.prefix.length() - 1
        && word[index + offset + 1] == node.prefix[offset]) {
        if (node.children.find(check_word[offset]) != node.children.end()) {
            trie_travel(std::wstring(1, word[index + offset + 1]) + std::wstring(1, check_word[offset]) +
                substr(word, index + offset + 2), error, code + node.prefix, 1,
                *node.children[check_word[offset]], answer);
        }
    }
    else if (offset + 1 < check_word.length() && std::wstring(1, check_word[offset + 1]) +
        std::wstring(1, check_word[offset]) +
        substr(check_word, offset + 2) == substr(node.prefix, offset)) {
        transpose_error = true;
    }

    if (delete_error) {
        if ((index - 1 + node.prefix.length() == word.length()) && node.end_of_word) {
            answer.push_back(code + node.prefix);
        }

        for (auto& it : node.children) {
            trie_travel(word, error, code + node.prefix, index + node.prefix.length() - 1, *it.second, answer);
        }
    }

    if (insert_error) {
        if (index + node.prefix.length() + 1 == word.length() && node.end_of_word) {
            answer.push_back(code + node.prefix);
        }
        for (auto& it : node.children) {
            trie_travel(word, error, code + node.prefix, index + node.prefix.length() + 1, *it.second, answer);
        }
    }

    if (transpose_error || replace_error) {
        if (index + node.prefix.length() == word.length() && node.end_of_word) {
            answer.push_back(code + node.prefix);
        }

        for (auto& it : node.children) {
            trie_travel(word, error, code + node.prefix, index + node.prefix.length(), *it.second, answer);
        }
    }
}

std::vector<std::wstring> Trie::correction(std::wstring& word, std::locale& loc) {
    std::vector<std::wstring> compare;
    std::transform(word.begin(), word.end(), word.begin(),
        std::bind(std::tolower<wchar_t>,
            std::placeholders::_1,
            std::cref(loc)));
    for (auto& it : root->children) {
        trie_travel(word, false, L"", 0, *it.second, compare);
    }
    return compare;
}


void Trie::add(std::wstring& word, std::locale& loc) {
    if (word.empty()) {
        return;
    }
    std::transform(word.begin(), word.end(), word.begin(),
        std::bind(std::tolower<wchar_t>,
            std::placeholders::_1,
            std::cref(loc)));

    int index = 0;
    Node* current_node = root;
    bool compare = false;
    while (index < word.length() && current_node->children.find(word[index])
        != current_node->children.end()) {
        compare = false;
        current_node = current_node->children[word[index]];
        if (current_node->prefix == substr(word, index, current_node->prefix.length())) {
            compare = true;
            index += current_node->prefix.length();
            continue;
        }
        break;
    }
    if (index == word.length()) {
        current_node->end_of_word = true;
        return;
    }
    if (current_node->children.find(word[index]) == current_node->children.end()
        && (compare || current_node == root)) {
        current_node->children[word[index]] = new Node(substr(word, index), true);
        return;
    }
    split(*current_node, word, index);
}

bool Trie::find(std::wstring& word, std::locale& loc) {
    std::transform(word.begin(), word.end(), word.begin(),
        std::bind(std::tolower<wchar_t>,
            std::placeholders::_1,
            std::cref(loc)));
    if (root->children.empty()) {
        return false;
    }
    int index = 0;
    Node* current_node = root;

    while (index < word.length() && current_node->children.find(word[index]) != current_node->children.end()) {
        current_node = current_node->children[word[index]];
        int len_temp = current_node->prefix.length();
        if (current_node->prefix == substr(word, index, len_temp)) {
            index += len_temp;
            continue;
        }
        break;
    }
    return index == word.length() && current_node->end_of_word;
}


int main() {

    std::ios_base::sync_with_stdio(false);
    std::locale loc{ "" };
    std::wcin.imbue(loc);
    std::wcout.imbue(loc);

    Trie b;
    int words = 0;
    std::wstring line;
    std::getline(std::wcin, line);
    int count = std::stoi(line);
    while (std::getline(std::wcin, line)) {
        std::wstring copy_with_upper = line;

        std::transform(line.begin(), line.end(), line.begin(),
            std::bind(std::tolower<wchar_t>,
                std::placeholders::_1,
                std::cref(loc)));

        if (line.empty()) {
            continue;
        }
        if (words < count) {
            b.add(line, loc);
            words++;
        }
        else {
            std::vector<std::wstring> answer = b.correction(line, loc);
            if (b.find(line, loc)) {
                std::wcout << copy_with_upper << " - ok\n";
                continue;
            }
            if (answer.empty()) {
                std::wcout << copy_with_upper << " -?\n";
                continue;
            }
            if (answer.size() == 1) {
                std::wcout << copy_with_upper << " -> " << answer[0] << "\n";
                continue;
            }
            std::sort(answer.begin(), answer.end());
            std::wcout << copy_with_upper << " -> " << answer[0];
            for (size_t i = 1; i < answer.size(); i++) {
                std::wcout << ", " << answer[i];
            }
            std::wcout << std::endl;
        }
    }
}