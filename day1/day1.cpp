#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <set>

constexpr int TARGET_NUM = 2020;

/**
 * Read the input and get it as a set of integers
 * @param filename The filename to read from
 * @return std::set<int> A set of the numbers in the input
 */
std::set<int> read_input(const std::string &filename)
{
	std::set<int> input;
	std::string line;
	std::ifstream file(filename);

	while (std::getline(file, line))
	{
		int num = std::stoi(line);
		input.insert(num);
	}

	return input;
}

int part1(const std::set<int> &inputs)
{
	for (int num : inputs)
	{
		int desired = TARGET_NUM - num;
		if (inputs.find(desired) != inputs.end())
		{
			return desired * num;
		}
	}

	throw "Does not contain solution";
}

int part2(const std::set<int> &inputs)
{
	for (int num : inputs)
	{
		for (int num2 : inputs)
		{
			int desired = TARGET_NUM - (num + num2);
			if (inputs.find(desired) != inputs.end())
			{
				return desired * num2 * num;
			}
		}
	}

	throw std::runtime_error("Does not contain solution");
}

int main(int argc, const char *argv[])
{
	if (argc != 2)
	{
		std::cerr << "./day1 <input_file>" << std::endl;
		return 1;
	}

	std::set<int> inputs = read_input(argv[1]);
	std::cout << part1(inputs) << std::endl;
	std::cout << part2(inputs) << std::endl;
}
