#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>

#include <unordered_set>
#include <unordered_map>
using namespace std;

//The irreducible polynomial used for AES
#define AES_POL 	(1 << 8) + (1 << 4) + (1 << 3) + (1 << 1) + 1

//Example cipher key defined in AES spec
vector<uint8_t> cipher_key =   {0x2b, 0x7e, 0x15, 0x16,
								0x28, 0xae, 0xd2, 0xa6,
								0xab, 0xf7, 0x15, 0x88, 
								0x09, 0xcf, 0x4f, 0x3c};

void transpose(vector<uint8_t>& message);

//AES S-box, the non linear component of the s box based off
//number theory (GF[2^8]). Base should be chosen to be an irreducible of
//order 8. 
class Sbox {
	public:
	uint16_t base;
	unordered_map<uint8_t, uint8_t> m;

	Sbox() {};

	Sbox(uint16_t pol) : base(pol) {
		cout << "S-box:" << endl;
		cout << hex; 
		for(int i = 0; i < 16; i++) {
			for(int j = 0; j < 16; j++) {
				m[i * 16 + j] = mapping(i * 16 + j);
				cout << setfill('0') << setw(2) << (int)m[i * 16 + j] << " ";
			}
			cout << endl;
		}
	}; 

	//Returns the mapping of index i
	uint8_t mapping(uint8_t i) {
		if(i == 0) {
			return 0x63;
		}

		uint8_t inv = inverse(i);
		uint8_t res = inv;
		for(int k = 1; k < 5; k++) {
			res ^= cyclic(inv, k);		
		}
		res ^= 0x63;
		return res;
	}

	//Returns a cyclic shift of a number
	uint8_t cyclic(uint8_t num, int shift) {
		uint8_t temp = 0;
		for(int i = shift; i < 8; i++) {
			temp += (1 << i);
		}
		return ((num << shift) & (temp)) ^ ((num >> (8 - shift)));
	}

	//Displays a bit representation of a number
	void disp(uint8_t num) {
		for(int i = 7; i >= 0; i--) {
			//cout << ((num & (1 << i)) >> i);
			if((num & (1 << i)) >> i) {
				printf("x ^ %d + ", i);
			}
		}
		cout << endl;
	}

	uint8_t r_con(int pow) {
		uint8_t res = 0x01;
		for(int i = 0; i < pow; i++) {
			res = mult(res, 0x02);
		}
		return res;
	}

	//Multiplication modulo base
	uint8_t mult(uint8_t p1, uint8_t p2) {
		uint16_t res = 0;
		for(int i = 0; i < 8; i++) {
			if(p2 & (1 << i))
				res ^= (p1 << i);
		}
		return divide(res, base).second;
	}

	//Returns quotient remainder of polynomial based on divisor
	pair<uint8_t, uint8_t> divide(uint16_t pol, uint16_t div) {
		uint8_t quotient = 0;

		//Finds order of pol and divisor
		int pmax = -1;
		int dmax = -1;

		//16 instead of 8 to account for initial polynomial and multiplied
		//polynomials
		for(int i = 0; i < 16; i++) {
			if(pol & (1 << i))
				pmax = i;
			if(div & (1 << i))
				dmax = i;			
		}

		while(pmax >= dmax) {
			if(pol & (1 << pmax)) {
				quotient += (1 << (pmax - dmax));
				pol ^= (div << (pmax - dmax));
			}
			pmax--;
		}

		return {quotient, pol};
	}

	//Returns the inverse of a polynomial modulo x^8 + x^4 + x^3 + x + 1
	uint8_t inverse(uint8_t pol) {
		if(pol == 1) {
			return 1;
		}

		vector<pair<uint8_t, uint8_t>> res;
		res.push_back(divide(base, pol));
		//Assume we are dealing with an irreducible polynomial
		int idx = 0;
		while(res[idx].second != 1) {
			pair<uint8_t, uint8_t> temp = divide(pol, res[idx].second);
			pol = res[idx].second;
			res.push_back(temp);
			idx++;
		}

		//uint16_t L = res[idx - 2].second;
		uint16_t L = 1;
		uint16_t S = res[idx].first;
		while(idx > 0) {
			uint16_t temp = S;
			S = mult(temp, res[idx - 1].first) ^ L;
			L = temp;	
			idx--;
		}
		return S;
	}

	uint8_t look_up(uint8_t byte) {
		return m[byte];
	}
};

//Linear transforms
class LT {
public:
	Sbox s;
	vector<uint8_t> cmix = {2, 3, 1, 1,
							1, 2, 3, 1,
							1, 1, 2, 3,
							3, 1, 1, 2};

	LT(uint16_t pol) {
		s = Sbox(pol);		
	}

	//Shows the resulting matrix
	void show_matrix(vector<uint8_t> input) {
		cout << hex;
		for(int i = 0; i < 4; i++) {
			for(int j = 0; j < 4; j++) {
				cout << setfill('0') << setw(2) << (int)input[4 * i + j] << " ";
			}
			cout << endl;
		}		
	}

	//Mixes rows as per AES spec
	void mix_rows(vector<uint8_t>& input) {
		//Shift row 2, 3 and 4
		for(int i = 1; i < 4; i++) {
			shift_row(input, i, i);
		}
	}

	//Cycles rows to the left
	void shift_row(vector<uint8_t>& input, int row, int shifts) {
		for(int i = 0; i < shifts; i++) {
			uint8_t temp = input[4 * row];
			for(int j = 1; j < 4; j++) {
				input[4 * row + j - 1] = input[4 * row + j];
			}
			input[4 * row + 3] = temp;
		}
	}

	vector<uint8_t> mix_columns(vector<uint8_t> input) {
		vector<uint8_t> ans(16);
		//Matrix multiplication
		for(int i = 0; i < 4; i++) {
			for(int j = 0; j < 4; j++) {
				for(int k = 0; k < 4; k++) {
					ans[4 * i + j] ^= s.mult(cmix[i * 4 + k], input[k * 4 + j]);
				}
			}
		}
		return ans;
	}
};

//Adds a key to a message
void add_key(vector<uint8_t>& message, vector<uint8_t> subkey) {
	transpose(subkey);
	for(int i = 0; i < 16; i++) {
		message[i] ^= subkey[i];
	}
}

//Using Nb = 4, Nk = 6 (128 bit security)
vector<vector<uint8_t>> key_exapansion(vector<uint8_t> key, LT lt) {
	//Number of rounds
	int Nr = 8 + key.size() / 8;
	//Number of 4 byte words in key
	int Nk = key.size() / 4;
	//Number of columns
	int Nb = 4;
	//11 keys for 10 round process etc.
	vector<vector<uint8_t>> subkeys(Nr + 1, vector<uint8_t>(16));

	vector<uint8_t> w;
	int i = 0;
	while(i < Nk) {
		for(int j = 0; j < 4; j++) {
			subkeys[0][4 * i + j] = key[4 * i + j];
		}
		i++;
	}

	i = Nk;
	while(i < (Nb) * (Nr + 1)) {
		int temp1 = i - 1;
		int temp2 = i - Nk;
		vector<uint8_t> temp(4); 
		for(int j = 0; j < 4; j++) {
			temp[j] = subkeys[temp1 / 4][4 * (temp1 % 4) + j];
		}
		
		if(i % Nk == 0) {
			uint8_t rcon = lt.s.r_con(i / Nk - 1);
			lt.shift_row(temp, 0, 1);

			for(int j = 0; j < 4; j++) {
				temp[j] = lt.s.look_up(temp[j]);
			}
			temp[0] ^= rcon;		
		}

		for(int j = 0; j < 4; j++) {
			subkeys[i / 4][4 * (i % 4) + j] = temp[j] ^ subkeys[temp2 / 4][4 * (temp2 % 4) + j];
		}
		i++;
	}

	cout << "Keys:" << endl;
	for(int i = 0; i < 11; i++) {
		for(int j = 0; j < 16; j++) {
			cout << hex << setfill('0') << setw(2) << (int)subkeys[i][j];
		}
		cout << endl;
	}
	return subkeys;
}

//Shows hexadecimal message
void show_message(vector<uint8_t> message) {
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			cout << hex << setfill('0') << setw(2) << (int)message[i * 4 + j] << " ";
		}
		cout << endl;
	}
	cout << endl;
}

//Swaps rows and columns around
void transpose(vector<uint8_t>& message) {
	for(int i = 0; i < 4; i++) {
		for(int j = i + 1; j < 4; j++) {
			uint8_t temp = message[4 * i + j];
			message[4 * i + j] = message[4 * j + i];
			message[4 * j + i] = temp;
		}
	}
}

//Given a block message and master key, encrpts the message
vector<uint8_t> AES_encrypt(vector<uint8_t> message, vector<uint8_t> key) {
	LT lt(AES_POL);
	//Number of rounds
	int rounds = 8 + key.size() / 8;
	//Genetate subkeys
	vector<vector<uint8_t>> subkeys = key_exapansion(key, lt);
	transpose(message);	

	cout << "Process:" << endl;
	//Initial message
	show_message(message);
	//Begin encryption algorithm
	add_key(message, subkeys[0]);
	show_message(message);
	//Intermediate rounds
	for(int i = 1; i < rounds; i++) {
		//Substitution
		for(int j = 0; j < 16; j++) {
			message[j] = lt.s.look_up(message[j]);
		}
		show_message(message);
		//Shift rows
		lt.mix_rows(message);
		show_message(message);
		message = lt.mix_columns(message);
		show_message(message);
		add_key(message, subkeys[i]);
		show_message(message);
	}

	//Final round
	for(int j = 0; j < 16; j++) {
		message[j] = lt.s.look_up(message[j]);
	}
	show_message(message);
	lt.mix_rows(message);
	show_message(message);
	add_key(message, subkeys[rounds]);
	show_message(message);
	transpose(message);
	return message;
}


int main(int argc, char** args) {   
	freopen("AES.in","rt",stdin);
	freopen("AES.out","wt",stdout);

	vector<uint8_t> message; 
	for(int i = 0; i < 16; i++) {
		string s;
		uint8_t c;
		cin >> s;
		sscanf(s.c_str(), "%x", &c);
		message.push_back(c);
	}
	vector<uint8_t> encrypted = AES_encrypt(message, cipher_key);
	cout << "Encrypted:" << endl;
	for(int i = 0; i < 16; i++) {
		cout << setfill('0') << setw(2) << (int)encrypted[i];
	}

	return 0;
}