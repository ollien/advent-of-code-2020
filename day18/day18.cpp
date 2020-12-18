#include <folly/String.h>

#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <regex>
#include <set>
#include <string>
#include <vector>

enum Operation { ADDITION = '+', MULTIPLICATION = '*' };

class ExpressionNode {
 public:
	ExpressionNode() {
	}

	virtual long evaluate() const = 0;

	virtual ~ExpressionNode() {
	}
};

class ValueNode : public ExpressionNode {
 public:
	ValueNode(long n) : n(n) {
	}

	long evaluate() const {
		return n;
	}

 private:
	long n;
};

class ExpressionTree : public ExpressionNode {
 public:
	ExpressionTree() {
	}
	ExpressionTree(std::unique_ptr<ExpressionNode> &&left, std::unique_ptr<ExpressionNode> &&right, Operation &&op)
		: left(std::move(left)), right(std::move(right)), op(std::move(op)) {
	}

	void setLeft(std::unique_ptr<ExpressionNode> &&left) {
		this->left = std::move(left);
	}

	void setRight(std::unique_ptr<ExpressionNode> &&left) {
		this->right = std::move(left);
	}

	void putNextNode(std::unique_ptr<ExpressionNode> &&node) {
		if (!this->left) {
			this->setLeft(std::move(node));
		} else if (!this->right) {
			this->setRight(std::move(node));
		} else {
			std::invalid_argument("Node is already full");
		}
	}

	void setOp(Operation op) {
		this->op = op;
	}

	long evaluate() const {
		if (!this->canFullyEvaluate()) {
			if (this->left) {
				return this->left->evaluate();
			} else if (this->right) {
				return this->right->evaluate();
			} else {
				throw "Tree must have a component to evaluate";
			}
		}

		long leftValue = this->left->evaluate();
		long rightValue = this->right->evaluate();

		switch (*op) {
			case ADDITION:
				return leftValue + rightValue;
			case MULTIPLICATION:
				return leftValue * rightValue;
			default:
				throw std::invalid_argument("Invalid operation");
		}
	}

 private:
	std::unique_ptr<ExpressionNode> left;
	std::unique_ptr<ExpressionNode> right;
	std::optional<Operation> op;
	bool canFullyEvaluate() const {
		return this->left && this->right && this->op;
	}
};

std::vector<std::string> readInput(const std::string &filename) {
	std::vector<std::string> input;
	std::string line;
	std::ifstream file(filename);
	while (std::getline(file, line)) {
		input.push_back(line);
	}

	return input;
}

std::optional<Operation> parseOperator(const std::string_view component) {
	if (component.size() != 1) {
		return std::nullopt;
	}

	switch (component.at(0)) {
		case ADDITION:
			return ADDITION;
		case MULTIPLICATION:
			return MULTIPLICATION;
		default:
			return std::nullopt;
	}
}

std::optional<long> parseNumber(const std::string_view component) {
	try {
		// This will only really ever copy some (usually small) number of digits... I don't consider it a very expensive
		// copy
		return std::stol(std::string(component));
	} catch (std::invalid_argument) {
		return std::nullopt;
	}
}

std::optional<std::pair<std::string_view, int>> parseParenthetical(int cursor, const std::string_view input) {
	if (input.size() == 0 || input.at(cursor) != ')') {
		return std::nullopt;
	}

	int i = cursor;
	int rightCount = 0;
	for (auto it = input.crbegin() + (input.size() - cursor); it != input.crend(); (++it, i--)) {
		if (*it == '(' && rightCount == 0) {
			break;
		}

		if (*it == ')') {
			rightCount++;
		} else if (*it == '(') {
			rightCount--;
		}
	}

	int startPos = i - 1;

	return std::pair<std::string_view, int>(input.substr(startPos + 1, cursor - startPos - 1), startPos);
}

std::unique_ptr<ExpressionNode> buildTree(const std::string_view input, int depth = 0) {
	if (std::count_if(input.cbegin(), input.cend(), [](char c) { return c == ' '; }) == 0) {
		std::cout << std::string(depth, ' ') << "RIGHT: " << input << std::endl;
		std::optional<long> value = parseNumber(input);
		if (!value) {
			throw std::invalid_argument("one-component string is expected to be number");
		}

		return std::make_unique<ValueNode>(*value);
	}

	ExpressionTree tree;
	// This should technically be size_type but I need to be able to go before zero
	int cursor = input.size() - 1;
	while (cursor > 0) {
		// Traverse the string right to left. Evaluating an expression tree will always evaluate right to left.
		std::string_view::size_type previousSpace = cursor + 1;
		std::string_view::size_type nextSpace = input.rfind(" ", cursor - 1);
		std::string_view component = input.substr(nextSpace + 1, cursor - nextSpace);
		cursor = nextSpace - 1;

		// Attempt to find a parenthetical, and get its parse value if we can
		std::optional<std::pair<std::string_view, int>> parentheticalPart =
			parseParenthetical(previousSpace - 1, input);
		if (parentheticalPart) {
			std::cout << std::string(depth, ' ') << "RIGHT (paren): " << parentheticalPart->first << std::endl;
			auto parentheticalTree = buildTree(parentheticalPart->first, depth + 1);
			tree.setRight(std::move(parentheticalTree));
			cursor = parentheticalPart->second - 2;
			continue;
		}

		// Attempt to add an operator to our parse if we have it
		std::optional<Operation> componentOperation = parseOperator(component);
		if (componentOperation) {
			tree.setOp(*componentOperation);
			auto rest = input.substr(0, cursor + 1);
			std::cout << std::string(depth, ' ') << "LEFT: " << component << std::endl;
			auto rightTree = buildTree(rest, depth + 1);
			tree.setLeft(std::move(rightTree));
			// Once we have found an operator and the operand to the left of it, we're done
			break;
		}

		// Attempt to find a value
		std::optional<long> componentValue = parseNumber(component);
		if (!componentValue) {
			throw std::invalid_argument("Expected number as last possible option");
		}
		std::cout << std::string(depth, ' ') << "RIGHT (value): " << component << std::endl;
		tree.setRight(std::make_unique<ValueNode>(*componentValue));
	}

	return std::make_unique<ExpressionTree>(std::move(tree));
}

long part1(const std::vector<std::string> &input) {
	return std::accumulate(input.cbegin(), input.cend(), 0L, [](long total, const std::string &expression) {
		auto tree = buildTree(expression);
		return total + tree->evaluate();
	});
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);

	std::cout << part1(input) << std::endl;
}
