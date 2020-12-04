#include <folly/String.h>

#include <boost/algorithm/string.hpp>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <streambuf>
#include <vector>

// cid is not required
const std::set<std::string> requiredFields{"byr", "iyr", "eyr", "hgt", "hcl", "ecl", "pid"};
const std::set<std::string> validEyeColors{"amb", "blu", "brn", "gry", "grn", "hzl", "oth"};
const std::string PASSPORT_DELIM = "\n\n";

std::string read_input(const std::string &filename) {
	std::vector<std::string> input;
	std::string line;
	std::ifstream file(filename);
	return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

/**
 * Check if the given passport has all of the required fields
 * @param passport The passport to check
 * @return bool Whether or not he given passport has all the fields
 */
bool hasRequiredFields(const std::map<std::string, std::string> &passport) {
	std::set<std::string> fields;
	std::transform(
		passport.begin(),
		passport.end(),
		std::inserter(fields, fields.end()),
		[](const std::pair<const std::string, std::string> &entry) { return entry.first; });

	std::set<std::string> missingFields;
	std::set_difference(
		requiredFields.begin(),
		requiredFields.end(),
		fields.begin(),
		fields.end(),
		std::inserter(missingFields, missingFields.end()));

	return missingFields.empty();
}

/**
 * Get each individual passport as a vector of strings
 * @param input The passport raw from the file
 * @return std::vector<std::string> All of the passports as strings
 */
std::vector<std::string> getPassportStrings(const std::string &input) {
	std::vector<std::string> passports;
	folly::split(PASSPORT_DELIM, input, passports);
	for (std::string &passport : passports) {
		boost::algorithm::trim(passport);
		std::replace(passport.begin(), passport.end(), '\n', ' ');
	}

	return passports;
}

/**
 * Make a vector of maps of all of the passports
 * @param input The raw input from the file
 * @return std::vector<std::map<std::string, std::string>> All of the passports, turned into a vector of key -> value
 * form
 */
std::vector<std::map<std::string, std::string>> makePassportMaps(const std::string &input) {
	std::vector<std::string> passports = getPassportStrings(input);
	std::vector<std::map<std::string, std::string>> passportMaps;
	for (const std::string &passport : passports) {
		std::vector<std::string> entries;
		folly::split(" ", passport, entries);

		std::map<std::string, std::string> passportMap;
		for (const std::string &entry : entries) {
			std::vector<std::string> parts;
			folly::split(":", entry, parts);
			passportMap.insert(std::pair<std::string, std::string>(parts.at(0), parts.at(1)));
		}

		passportMaps.push_back(passportMap);
	}

	return passportMaps;
}

bool isValidBirthYear(int year) {
	return year >= 1920 && year <= 2002;
}

bool isValidIssueYear(int year) {
	return year >= 2010 && year <= 2020;
}

bool isValidExpirationYear(int year) {
	return year >= 2020 && year <= 2030;
}

bool isValidHeight(const std::string &height) {
	std::string suffix = height.substr(height.length() - 2, 2);
	std::string prefix = height.substr(0, height.length() - 2);
	int value = std::stoi(prefix);
	if (suffix == "cm") {
		return value >= 150 && value <= 193;
	} else if (suffix == "in") {
		return value >= 59 && value <= 76;
	} else {
		return false;
	}
}

bool isValidHairColor(const std::string &color) {
	std::regex expression("#[0-9a-f]{6}");
	return std::regex_match(color, expression);
}

bool isValidEyeColor(const std::string &color) {
	return validEyeColors.find(color) != validEyeColors.end();
}

bool isValidPassportNumber(const std::string &num) {
	std::regex expression("[0-9]{9}");
	return std::regex_match(num, expression);
}

bool isFieldValid(const std::string &fieldName, const std::string value) {
	// TODO: This could maybe be changed to a map of lambdas?
	if (fieldName == "byr") {
		return isValidBirthYear(std::stoi(value));
	} else if (fieldName == "iyr") {
		return isValidIssueYear(std::stoi(value));
	} else if (fieldName == "eyr") {
		return isValidExpirationYear(std::stoi(value));
	} else if (fieldName == "hgt") {
		return isValidHeight(value);
	} else if (fieldName == "hcl") {
		return isValidHairColor(value);
	} else if (fieldName == "ecl") {
		return isValidEyeColor(value);
	} else if (fieldName == "pid") {
		return isValidPassportNumber(value);
	} else if (fieldName == "cid") {
		return true;
	}

	throw new std::invalid_argument("Bad field name");
}

int part1(const std::vector<std::map<std::string, std::string>> &passports) {
	int count = 0;
	for (const std::map<std::string, std::string> &passport : passports) {
		count += hasRequiredFields(passport);
	}

	return count;
}

int part2(const std::vector<std::map<std::string, std::string>> &passports) {
	int count = 0;
	for (const std::map<std::string, std::string> &passport : passports) {
		if (!hasRequiredFields(passport)) {
			continue;
		}

		bool valid = true;
		for (const std::pair<const std::string, std::string> entry : passport) {
			if (!isFieldValid(entry.first, entry.second)) {
				valid = false;
				break;
			}
		}

		count += valid;
	}

	return count;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	std::string input = read_input(argv[1]);
	auto passports = makePassportMaps(input);
	std::cout << part1(passports) << std::endl;
	std::cout << part2(passports) << std::endl;
}
