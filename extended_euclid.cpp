#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

//Assumes coprime and returns inverse
int inv(int a, int mod) {
	int base = mod;
	vector<int> arr;
	vector<int> nums;
	//Performs euclidean algorithm recording intermediate results
	while(a > 1) { //Terminates a = 1
		nums.push_back(mod);
		arr.push_back(mod / a);
		int temp = mod % a;
		mod = a;
		a = temp;
	}
	nums.push_back(mod);
	nums.push_back(1);

	//Prints Working for euclidean algorithm
	for(int i = 0; i < nums.size() - 2; i++) {
		printf("%d = %d·%d + %d\n", nums[i], arr[i], nums[i + 1], nums[i + 2]);
	}

	int ans = -arr[arr.size() - 1];
	int ind = 1;
	//Prints working for extended euclidean
	printf("1 = %d + %d·%d\n", nums[nums.size() - 3], ans, nums[nums.size() - 2]);
	for(int i = arr.size() - 2; i >= 0; i--) {
		//Substitution and simplifying
		int temp = ans;
		ans = ind - ans * arr[i];
		ind = temp;
		//Prints working for extended euclidean
		printf("= %d·%d + %d·%d\n", ind, nums[i], ans, nums[i + 1]);
	}

	//Ensure answer is positive so modulo can be taken
	while(ans < 0) {
		ans += base;
	}

	return ans % base;
}

int main(int argc, char** args) {   
	//Number of cases
	int T;
	cin >> T;

	for(int i = 1; i <= T; i++) {
		int a, b, mod;
		cin >> a >> mod;
		b = inv(a, mod);
		//Outputs answer
		cout << "Case #" << i << ": " << b << endl;
	}
}