#include <folly/String.h>

#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <numeric>
#include <regex>
#include <string>
#include <vector>

const std::string POLICY_PATTERN = "([0-9]+)-([0-9]+) (.)";
const std::string DELIM = ": ";

/**
 * Policy represents a password policy
 */
class Policy {
 public:
	Policy(int min, int max, char letter) : min(min), max(max), letter(letter) {
	}

	/**
	 * Parse a policy from an input string
	 * @param input The input to parse
	 * @return Policy The policy from the input
	 */
	static Policy parse(const std::string &input) {
		std::regex expression(POLICY_PATTERN);
		std::smatch matches;

		if (!std::regex_search(input, matches, expression)) {
			throw new std::invalid_argument("Invalid string");
		}

		return Policy(std::stoi(matches[1]), std::stoi(matches[2]), matches.str(3).at(0));
	}

	int getMin() const {
		return this->min;
	}

	int getMax() const {
		return this->max;
	}

	char getLetter() const {
		return this->letter;
	}

 private:
	int min;
	int max;
	char letter;
};

std::vector<std::string> read_input(const std::string &filename) {
	std::vector<std::string> input;
	std::string line;
	std::ifstream file(filename);
	while (std::getline(file, line)) {
		input.push_back(line);
	}

	return input;
}

class Entry {
 public:
	Entry(Policy policy, std::string password) : policy(policy), password(password) {
	}

	/**
	 * Parse an input entry from an input string
	 * @param input The input to parse
	 * @return Entry the entry from the input
	 */
	static Entry parse(const std::string &input) {
		int delimIndex = input.find(DELIM);
		std::vector<std::string> components;
		folly::split(DELIM, input, components);
		std::string rawPolicy = components.at(0);
		std::string password = components.at(1);
		Policy policy = Policy::parse(rawPolicy);

		return Entry(policy, password);
	}

	const Policy &getPolicy() const {
		return this->policy;
	}

	const std::string &getPassword() const {
		return this->password;
	}

 private:
	Policy policy;
	std::string password;
};

/**
 * Get the number of valid passwords in the input
 * @param input The input split into lines
 * @param matches A function that checks whether or not the password matches the policy
 * @return int The number ofm atching password
 */
int getNumValidPasswords(const std::vector<std::string> &input, const std::function<bool(const Entry &)> &valid) {
	return std::accumulate(input.begin(), input.end(), 0, [&](int total, std::string rawEntry) {
		Entry entry = Entry::parse(rawEntry);
		return total + valid(entry);
	});
}

int part1(const std::vector<std::string> &input) {
	return getNumValidPasswords(input, [](const Entry &entry) {
		const Policy &policy = entry.getPolicy();
		const std::string &password = entry.getPassword();
		int count = std::count_if(password.begin(), password.end(), [&](char c) { return c == policy.getLetter(); });

		return (count >= policy.getMin() && count <= policy.getMax());
	});
}

int part2(const std::vector<std::string> &input) {
	return getNumValidPasswords(input, [](const Entry &entry) {
		const Policy &policy = entry.getPolicy();
		const std::string &password = entry.getPassword();
		char letter = entry.getPolicy().getLetter();

		return ((password.at(policy.getMin() - 1) == letter) ^ (password.at(policy.getMax() - 1) == letter));
	});
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << "./day2 <input_file>" << std::endl;
		return 1;
	}

	auto input = read_input(argv[1]);
	std::cout << part1(input) << std::endl;
	std::cout << part2(input) << std::endl;
}
