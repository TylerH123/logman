// IDENTIFIER: 01BD41C3BF016AD7E8B6F837DF18926EC3E83350

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <deque>

using namespace std;

class logMan {
private:
	struct logEntry {
		size_t entryID;
		string ts; 
		string category; 
		string msg;
		int64_t tsInt; 
	};

	class entryComp {
	public:
		bool operator() (const logEntry &lhs, const logEntry &rhs) const {
			if (lhs.tsInt < rhs.tsInt) 
				return true;
			else if (rhs.tsInt < lhs.tsInt) 
				return false;
			else {
				int t = strcasecmp(lhs.category.c_str(), rhs.category.c_str());
				if (t < 0) 
					return true; 
				else if (t > 0)
					return false;
				else 
					return lhs.entryID < rhs.entryID; 
			}
		}	
	};
	
	class timeComp {
	public:
		bool operator() (const logEntry &lhs, const int64_t &rhs) const {
			return lhs.tsInt < rhs;
		}
		bool operator() (const int64_t &lhs, const logEntry &rhs) const {
			return lhs < rhs.tsInt;	
		}
	};
	
	ifstream file; 
	vector<logEntry> entries; 
	vector<size_t> oldPos; 
	deque<size_t> excerpt; 
	vector<size_t> prev; 
	unordered_map<string, vector<size_t>> c; 
	unordered_map<string, vector<size_t>> k; 
	bool prevSearch = false; 
	
	int64_t convertToInt(const string &s) {
		char front[9] = {s[0], s[1], s[3], s[4], s[6], s[7], s[9], s[10]};
		char back[3] = {s[12], s[13]}; 
		int64_t t = static_cast<int64_t>(atoi(front))*100+static_cast<int64_t>(atoi(back)); 
		return t; 
	}
	
	void lower(string &s) {
		for (auto &c : s) {
			c = (char) tolower(c);
		}
	}
	
	void parseMsg(string &s, size_t idx) {
		size_t start,i,
			   size = s.size(); 
		bool found = false; 
		for (i = 0; i < size; ++i) {
			if (isalnum(s[i])) {
				s[i] = (char) tolower(s[i]);
				if (!found) {
					start = i;
					found = true;
				}
				if (i == size - 1) {
					auto &t = k[s.substr(start, size-start)]; 
					if (t.empty() || t.back() != idx) 
						t.push_back(idx);
				}
			}
			else if (found) {
				auto &t = k[s.substr(start, i-start)]; 
				if (t.empty() || t.back() != idx) 
					t.push_back(idx);
				found = false; 
			}
		}
	}
	
	void readEntries(string f) {
		file.open(f); 
		string ts, cat, msg; 
		size_t count = 0;
		while(getline(file, ts, '|')) {
			getline(file, cat, '|');
			getline(file, msg);
			entries.push_back({count++,ts,cat,msg,convertToInt(ts)});
		}
		sort(entries.begin(),entries.end(), entryComp()); 
		oldPos.resize(entries.size());
		for (size_t i = 0; i < entries.size(); ++i) {
			logEntry &l = entries[i];
			string cat = l.category;
			string msg = l.msg; 
			oldPos[l.entryID] = i; 
			lower(cat);
			c[cat].push_back(i);
			parseMsg(cat,i);
			parseMsg(msg,i); 
		}
		cout << count << " entries read" << '\n'; 
		file.close(); 
	}
	
	void append() {
		size_t pos;
		cin >> pos; 
		if (pos >= entries.size()) 
			cerr << "Invalid index" << '\n';
		else {
			excerpt.push_back(oldPos[pos]);
			cout << "log entry " << pos << " appended" << '\n';
		}
	}
	
	void print() {
		for (size_t i = 0; i < excerpt.size(); ++i) {
			logEntry &l = entries[excerpt[i]];
			cout << i << "|" << l.entryID << "|" << l.ts 
				 << "|" << l.category << "|" << l.msg << '\n';
		}
	}
	
	void matchSearch() {
		string ts; 
		cin >> ts; 
		if (ts.size() != 14) 
			cerr << "Invalid timestamp" << '\n'; 
		else {
			prevSearch = true; 
			prev.clear();
			int64_t t = convertToInt(ts); 
			auto start = lower_bound(entries.begin(), entries.end(), t, timeComp()); 
			if (start != entries.end()) {
				for (; (*start).tsInt == t; ++start) 
					prev.push_back(start-entries.begin()); 
			}
			cout << "Timestamp search: " << prev.size() << " entries found" << '\n';
		}
	}
	
	void catSearch() {
		string cat; 
		getline(cin,cat); 
		cat = cat.substr(1);
		lower(cat);
		prevSearch = true; 
		prev.clear();
		auto it = c.find(cat); 
		if (it != c.end()) {
			for (auto i : (*it).second)
				prev.push_back(i); 
		}
		cout << "Category search: " << prev.size() << " entries found" << '\n';
	}
	
	void appendResults() {
		if (prevSearch) {
			excerpt.insert(excerpt.end(), prev.begin(), prev.end());
			cout << prev.size() << " log entries appended" << '\n'; 
		}
		else 
			cerr << "Need a previous search" << '\n';
	}
	
	void tsSearch() {
		string ts; 
		cin >> ts; 
		if (ts.size() == 29 && ts[14] == '|') { 
			prevSearch = true; 
			prev.clear();
			int64_t start = convertToInt(ts.substr(0,14)); 
			int64_t end = convertToInt(ts.substr(15));
			auto s = lower_bound(entries.begin(), entries.end(), start, timeComp()); 
			if (s != entries.end()) {
				auto e = upper_bound(entries.begin(), entries.end(), end, timeComp()); 
				for (; s != e; ++s)
					prev.push_back(s-entries.begin());
			}
			cout << "Timestamps search: " << prev.size() << " entries found" << '\n';
		}
		else
			cerr << "Invalid timestamp" << '\n';
	}
	
	void printSummary() {
		logEntry &front = entries[excerpt.front()]; 
		logEntry &back = entries[excerpt.back()];
		cout << 0 << "|" << front.entryID << "|" << front.ts 
			 << "|" << front.category << "|" << front.msg << '\n'
			 << "...\n" << excerpt.size()-1 << "|" << back.entryID << "|" 
			 << back.ts << "|" << back.category << "|" << back.msg << '\n';
	}
	
	void clear() {
		cout << "excerpt list cleared" << '\n';
		if (!excerpt.empty()) {
			cout << "previous contents:" << '\n';
			printSummary();
		}
		else 
			cout << "(previously empty)" << '\n';	
		excerpt.clear();
	}
	
	void printResults() {
		if (prevSearch) {
			for (auto &i : prev) {
				logEntry &l = entries[i];
				cout << l.entryID << "|" << l.ts << "|" 
					 << l.category << "|" << l.msg << '\n';
			}
		}
		else 
			cerr << "Need a previous search" << '\n';
	}
	
	void sortExcerpt() {
		cout << "excerpt list sorted" << '\n';
		if (!excerpt.empty()) {
			cout << "previous ordering:" << '\n';
			printSummary();
			sort(excerpt.begin(), excerpt.end());
			cout << "new ordering:" << '\n';
			printSummary();
		}
		else 
			cout << "(previously empty)" << '\n';\
	}
	
	void deleteEntry() {
		size_t pos; 
		cin >> pos; 
		if (pos >= excerpt.size()) 
			cerr << "Invalid index" << '\n';
		else {
			excerpt.erase(excerpt.begin()+pos);
			cout << "Deleted excerpt list entry " << pos << '\n';
		}
	}
	
	void moveFront() {
		size_t pos; 
		cin >> pos; 
		if (pos >= excerpt.size()) 
			cerr << "Invalid index" << '\n';
		else {
			auto l = excerpt[pos];
			excerpt.erase(excerpt.begin()+pos);
			excerpt.push_front(l); 
			cout << "Moved excerpt list entry " << pos << '\n';
		}
	}
	
	void moveBack() {
		size_t pos; 
		cin >> pos; 
		if (pos >= excerpt.size()) 
			cerr << "Invalid index" << '\n';
		else {
			auto l = excerpt[pos]; 
			excerpt.erase(excerpt.begin()+pos);
			excerpt.push_back(l); 
			cout << "Moved excerpt list entry " << pos << '\n';
		}
	}
	
	void keywordSearch() {
		prevSearch = true; 
		prev.clear();
		string s; 
		getline(cin, s); 
		size_t start,
			   size = s.size();
		bool found = false; 
		vector<size_t> output; 
		for (size_t i = 0; i < size; ++i) {
			if (isalnum(s[i])) {
				s[i] = (char) tolower(s[i]);
				if (!found) {
					start = i;
					found = true;
				}
				if (i == size - 1) {
					auto it = k.find(s.substr(start, size-start));
					if (it != k.end()) {
						if (output.empty())
							output = (*it).second;
						else {
							vector<size_t> temp; 
							swap(output, temp);
							set_intersection((*it).second.begin(), (*it).second.end(), temp.begin(), temp.end(), back_inserter(output));
						}
					}
					else 
						output.clear();
				}
			}
			else if (found) {
				auto it = k.find(s.substr(start, i-start));
				if (it != k.end()) {
					if (output.empty())
						output = (*it).second;
					else {
						vector<size_t> temp; 
						swap(output, temp);
						set_intersection((*it).second.begin(), (*it).second.end(), temp.begin(), temp.end(), back_inserter(output));
						if (output.empty())
							break;
					}
				}
				else {
					output.clear();
					break; 
				}
				found = false; 
			}
		}
		for (auto &i : output) 
			prev.push_back(i); 
		cout << "Keyword search: " << prev.size() << " entries found" << '\n';
	}
	
public:
	logMan(int argc, char * argv[]) {
		parseArgs(argc, argv);
	}

	void parseArgs(int argc, char * argv[]) {
		if (argc < 2) {
			cerr << "Invalid arguments" << '\n';
			exit(1);
		}
		string s = argv[1];
		if (s == "-h" || s == "--help") {
			cout << "THIS IS LOGMAN" << '\n';
			exit(1); 
		}
		else {
			readEntries(s); 
			char cmd; 
			do {
				cout << "% "; 
				cin >> cmd; 
				switch (cmd) {
					case 't':
						tsSearch();
						break; 
					case 'm':
						matchSearch();
						break;
					case 'c':
						catSearch();
						break;
					case 'k':
						keywordSearch();
						break;
					case 'a':
						append(); 
						break;
					case 'r':
						appendResults();
						break;
					case 'd':
						deleteEntry();
						break;
					case 'b':
						moveFront();
						break;
					case 'e':
						moveBack();
						break;
					case 's':
						sortExcerpt();
						break;
					case 'l':
						clear(); 
						break;
					case 'g':
						printResults();
						break;
					case 'p':
						print();
						break;
					case '#': {
						string junk;
						getline(cin, junk);
						break;
					}
					case 'q':
						break; 
					default:
                		cerr << "Unknown command: " << cmd << '\n';
				}
			} while(cmd != 'q');
		}
	}
};

int main(int argc, char * argv[]) {
	ios_base::sync_with_stdio(false);
	logMan l(argc, argv);
	return 0; 
}