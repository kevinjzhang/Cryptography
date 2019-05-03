#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
using namespace std;

vector<float> freqTable = {0.082, 0.015, 0.028, 0.043, 0.127, 0.022, 
		0.020, 0.061, 0.070, 0.002, 0.008, 0.040, 0.024, 0.067, 0.075,
		0.019, 0.001, 0.060, 0.063, 0.091, 0.028, 0.010, 0.023, 0.001, 
		0.020, 0.001};

char encode(char c, int key) {
	//Subtracts key modulo 26
	return (c - 'A' + key) % 26 + 'A';
}

char decode(char c, int key) {
	//Subtracts key modulo 26
	return (c - 'A' - key + 26) % 26 + 'A';
}

//Decodes using viginere cipher and outputs text
void vig_decode(string str, vector<int> key) {
	//chosen key index
	int k = 0; 
	for(int i = 0; i < str.size(); i++) {
		cerr << decode(str[i], key[k]);
		k++;
		k %= key.size();
	}
}

//Gcd of 2 numbers
int gcd(int a, int b) {
	//Switches a and b if
	if(a > b) {
		int temp = a;
		a = b;
		b = temp;
	}
	//a < b does Euler
	while(a > 0) {
		int temp = b % a;
		b = a;
		a = temp;
	}
	return b;
}

//Kasisky's test
void kasisky(string str) {
	//Scans over text and returns all indexes of repeated triples
	map<string, vector<int>> m;
	for(int i = 0; i < str.size() - 2; i++) {
		string temp;
		for(int j = 0; j < 3; j++) {
			temp.push_back(str[i + j]);
		}
		//Adds index to a table of 3 common characters
		m[temp].push_back(i);
	}

	//Choose triple that has appeared the most
	string triple;
	int appr = 0;
	for(auto it = m.begin(); it != m.end(); it++) {
		//Chooses triple that has appeared the most
		if(it->second.size() > appr) {
			triple = it->first;
			appr = it->second.size();
		}
	}

	//Iterates over most found triple
	int mVal = m[triple][1] - m[triple][0];
	for(int i = 2; i < m[triple].size(); i++) {
		mVal = gcd(mVal, m[triple][i] - m[triple][i - 1]);
	} 
	//Outputs m value
	cout << mVal << endl;
}

vector<int> min_var(vector<map<char, int>> table) {
	vector<int> keys(table.size());
	vector<vector<int>> allVars;
	int idx = 0;
	for(auto tab : table) {
		//Variances for each corresponding key(0 indexed)
		vector<float> vars(26);
		//Number of values in table
		int count = 0;
		//Iterates over table
		for(auto it = tab.begin(); it != tab.end(); it++) {
			count += it->second; 
		}
		//Min variance desired for a key set to a large value
		float min = 1;
		for(int i = 0; i < 26; i++) {
			//Calculates variance between freq table and text based on given key
			for(int j = 0; j < 26; j++) {
				vars[i] += (tab[encode(j + 'A', i)] / (float)count - freqTable[j]) * 
						(tab[encode(j + 'A', i)] / (float)count - freqTable[j]);
			} 
			vars[i] /= 26;
			//Chooses best key
			if(vars[i] < min) {
				min = vars[i];
				keys[idx] = i;
			}
		}
		idx++;
	}	
	return keys;
}

//Outputs frequency table, index of coincidence and phi
//Returns the variance of phi
float printTable(vector<map<char, int>> table) {	
	float var = 0;
	for(auto tab : table) {
		//Sum for index of coincidence
		int sum = 0;
		//Number of values in table
		int count = 0;
		cout << "m = " << table.size() << ": ";
		//Iterates over table
		for(auto it = tab.begin(); it != tab.end(); it++) {
			cout << it->second << " ";
			sum += it->second * (it->second - 1);
			count += it->second; 
		}
		float phi = sum / (float) (count * (count - 1));
		var += (0.0667 - phi) * (0.0667 - phi);
		cout << endl << "Sum: " << sum << endl;
		cout << "Phi: " << phi << endl;
	}	
	return var / table.size();
}

int main(int argc, char** argv) {   
	//Change output streams to files
    freopen("vig.in","rt",stdin);
    //We use stdout for debugging
	freopen("vig.out","wt",stdout);
	//We can use stderr stream for our answer output file
	freopen("vig.err","wt",stderr);

	//Number of lines
	int L;
	stringstream ss;
	cin >> L;
	//Constructs complete string
	for(int j = 0; j < L; j++) {
		string temp;
		cin >> temp;
		ss << temp;
	}

	string text = ss.str();

	//Kasisky test
	kasisky(text);
	
	//Choosing an arbitrarily big value as our minimum standard
	float optvar = 1;	
	vector<map<char, int>> best;
	//Iterates over m
	for(int m = 1; m <= 5; m++) {
		//Variance
		float var;
		//Creates a table for given m value
		vector<map<char, int>> table(m);
		for(int idx = 0; idx < text.size(); idx++) {
			table[idx % m][text[idx]] += 1;
		}
		//Outputs table
		var = printTable(table);
		//If better choice of m
		if(var < optvar) {
			best = table;
			optvar = var;
		}
	}
	
	//Choose key that minimises the variance between freq
	vector<int> keys = min_var(best);
	for(auto num : keys) {
		cout << num << endl;
	}
	vig_decode(text, keys);
}