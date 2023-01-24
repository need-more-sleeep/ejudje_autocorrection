#include <iostream>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <vector>
#include <locale>
#include <functional>

struct node {
	std::unordered_map <wchar_t, node*> _children;
	std::wstring _data;
	bool _is_end = false;
	node() = default;
	node(std::wstring str, bool is_end) :_data(str), _is_end(is_end) {}
};


class trie {
	node* root = nullptr;

public:

	~trie()
	{
		destruct(root);
	}

	//добавление строки в дерево
	//n- длина входногго слова
	// O(n)
	// ищем среди ключей у детей первый символ строки, если нашли ,ищем максимальный префикс 
	// уменьшим слово на длину префикса и пойдем дальше рекурсивно
	//поиск среди ключей у детей происходит за константу, тк Unordered_map (ключ-первый символ префикса)
	void add(std::wstring str, std::locale& loc, node* it = nullptr) {
		std::transform(str.begin(), str.end(), str.begin(),
			std::bind(std::tolower<wchar_t>,
				std::placeholders::_1,
				std::cref(loc)));

		if (root == nullptr) {
			root = new node();
		}
		auto curr = root;
		if (it != nullptr) {
			curr = it;
		}
		std::wstring substr;



		//if found
		if (curr->_children.find(str[0]) != curr->_children.end()) {

			substr += str[0];

			//find max-length prefix
			for (int i = 1; curr->_children[str[0]]->_data.find(substr) == 0; ++i) {
				substr += str[i];
			}
			substr.pop_back();
			//если данные узла являются подстрокой нашего слова, то спускаемся ниже рекурсивно
			if (substr.find(curr->_children[str[0]]->_data) == 0) {
				str.erase(0, curr->_children[substr[0]]->_data.length());
				return add(str, loc, curr->_children[substr[0]]);
			}

			//новый узел сверху
			node* temp = new node(substr, false);

			curr->_children[str[0]]->_data.erase(0, substr.length());

			temp->_children.emplace(curr->_children[str[0]]->_data[0], curr->_children[str[0]]);

			curr->_children[str[0]] = temp;

			str.erase(0, substr.length());
			if (str.empty()) {
				temp->_is_end = true;
			}
			return add(str, loc, curr->_children[substr[0]]);

		}

		curr->_children.emplace(str[0], new node(str, true));
	}


	//нужна для того, чтобы создать вектор ответов, вызывает check_error
	std::vector<std::wstring> check_error(std::wstring str, std::locale& loc) {
		std::transform(str.begin(), str.end(), str.begin(),
			std::bind(std::tolower<wchar_t>,
				std::placeholders::_1,
				std::cref(loc)));
		std::vector<std::wstring> vec;
		check_word(str, vec);
		std::sort(vec.begin(), vec.end());
		return vec;
	}



	//проверка строки на принадлежность дереву
	// Поиск префиксов за константу!!
	//O(n) , просто будем проходить по префксам и если префикс такой есть, то спустимся в него и так до конца слова
	//если хотя бы на одном уровне не нашли префис, вернем false
	bool is_normal_string(std::wstring str, std::locale& loc) {
		std::transform(str.begin(), str.end(), str.begin(),
			std::bind(std::tolower<wchar_t>,
				std::placeholders::_1,
				std::cref(loc)));
		auto curr = root;
		wchar_t temp;
		while (!str.empty() && curr->_children.find(str[0]) != curr->_children.end()) {
			if (str.find(curr->_children[str[0]]->_data) == 0) {
				temp = str[0];
				str.erase(0, curr->_children[str[0]]->_data.length());
				curr = curr->_children[temp];
			}
			else {
				break;
			}
		}

		if (str.empty() && curr->_is_end) {
			return true;
		}
		return false;
	}

private:

	//O(n*m*k) n-длина входного слова m-число вершин k-длина наибольшего префикса
	//мы проходимя по префиксам, если расстояние левенштейна между префиксом и частью входной строки > 2, то выходим из ветки 
	//если расстояние левенштейна между префиксом и частью входной строки 0 или 1, то спускаемя вниз
	void check_word(std::wstring str, std::vector<std::wstring>& vec, node* it = nullptr, std::wstring answer = L"") {
		auto curr = root;
		if (it != nullptr) {
			curr = it;
		}
		std::wstring temp_str = str.substr(0, answer.length());
		if (dam_lev_dist(temp_str, answer) > 2) {
			return;
		}
		if (curr->_is_end && dam_lev_dist(str, answer) == 1) {
			if (std::find(vec.begin(), vec.end(), answer) == vec.end()) {
				vec.push_back(answer);
			}
		}
		for (auto temp : curr->_children) {
			check_word(str, vec, temp.second, answer + temp.second->_data);
		}
	}


	//Алгоритм Дамерау-Левенштейна
	//O(n*m)  n,m - длины входных слов
	// Считает расстояние Левенштейна
	int dam_lev_dist(std::wstring A, std::wstring B) {
		size_t n = A.length();
		size_t m = B.length();
		std::vector<std::vector<int>> d(n + 1, std::vector<int>(m + 1));
		
		int l_cost;
		for (int i = 0; i <= n; i++) {
			d[i][0] = i;
		}
		for (int j = 0; j <= m; j++) {
			d[0][j] = j;
		}

		for (int i = 1; i <= n; i++) {
			for (int j = 1; j <= m; j++) {
				if (A[i - 1] == B[j - 1]) {
					l_cost = 0;
				}
				else {
					l_cost = 1;
				}
				d[i][j] = std::min(d[i - 1][j] + 1,std::min(d[i][j - 1] + 1,d[i - 1][j - 1] + l_cost));
				if ((i > 1) && (j > 1) && (A[i - 1] == B[j - 2]) && (A[i - 2] == B[j - 1])) {
					d[i][j] = std::min(d[i][j], d[i - 2][j - 2] + l_cost); 
				}
			}
		}
		return d[n][m];
	}


	void destruct(node* curr) {
		if (curr != nullptr) {
			for (auto temp : curr->_children) {
				destruct(temp.second);
			}
			delete curr;
		}
	}

};

int main() {

	std::ios_base::sync_with_stdio(false);
	std::locale loc{ "" };
	std::wcin.imbue(loc);
	std::wcout.imbue(loc);



	trie t;
	int count;
	std::wstring str;
	std::getline(std::wcin, str);
	count = std::stoi(str);
	for (size_t j = 0; j < count; j++)
	{
		std::getline(std::wcin, str);
		t.add(str, loc);
	}
	while (std::getline(std::wcin, str)) {
		std::wstring copy_with_upper = str;

		std::transform(str.begin(), str.end(), str.begin(),
			std::bind(std::tolower<wchar_t>,
				std::placeholders::_1,
				std::cref(loc)));
		if (str.empty()) {
			continue;
		}
		if (count == 0) {
			std::wcout << copy_with_upper << " -?" << std::endl;
			continue;
		}
		if (t.is_normal_string(str, loc)) {
			std::wcout << copy_with_upper << " - ok" << std::endl;
		}
		else {

			std::vector<std::wstring> vec = t.check_error(str, loc);

			if (vec.empty()) {
				std::wcout << copy_with_upper << " -?" << std::endl;
			}
			else {

				std::wcout << copy_with_upper << " -> ";
				for (size_t i = 0; i < vec.size(); i++)
				{
					if (i == 0) {
						std::wcout << vec[i];
					}
					else {
						std::wcout << ", " << vec[i];
					}
				}
				std::wcout << "\n";
			}
		}
	}
}
