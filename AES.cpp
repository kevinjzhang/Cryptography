#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#include <unordered_set>
#include <unordered_map>
using namespace std;

#define AES_POL 	(1 << 8) + (1 << 4) + (1 << 3) + (1 << 1) + 1

//AES S-box, the non linear component of the s box based off
//number theory (GF[2^8]). Base should be chosen to be an irreducible of
//order 8. 
class Sbox {
	public:
	uint16_t base;
	unordered_map<uint8_t, uint8_t> m;

	Sbox() {};

	Sbox(uint16_t pol) : base(pol) {
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
	LT(uint16_t pol) {
		s = Sbox(pol);
	}

	vector<uint8_t> mix_rows(vector<uint8_t> input) {
		
	}
};


int main(int argc, char** args) {   
	freopen("AES.in","rt",stdin);
	freopen("AES.out","wt",stdout);

	Sbox LT(AES_POL);
	return 0;
}