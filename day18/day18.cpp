#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <optional>
#include <string>
#include <vector>

enum Operation { ADDITION = '+', MULTIPLICATION = '*', IDENTITY = 'i' };
class ExpressionNode;
// A strategy to evaluate the value of a node. Takes the left child, the right child, and the current operation at the
// node.
using EvaluationStrategy =
	std::function<double(const std::unique_ptr<ExpressionNode> &, const std::unique_ptr<ExpressionNode> &, Operation)>;

/**
 * Represents an abstract node in an expression tree
 */
class ExpressionNode {
 public:
	ExpressionNode() {
	}

	/**
	 * Get the current node's value
	 * @return long The value of the node
	 */
	virtual long evaluate() const = 0;
	/**
	 * Get references to all of the children to this node
	 * @return std::vector<std::reference_wrapper<const ExpressionNode>>
	 */
	virtual std::vector<std::reference_wrapper<const ExpressionNode>> getChildren() const = 0;
	/**
	 * Get the operation of this node
	 * @return Operation The operation of this node
	 */
	virtual Operation getOperation() const = 0;

	virtual ~ExpressionNode() {
	}
};

/**
 * Represents a node that only holds a value
 */
class ValueNode : public ExpressionNode {
 public:
	ValueNode(long n) : n(n) {
	}

	long evaluate() const override {
		return n;
	}

	std::vector<std::reference_wrapper<const ExpressionNode>> getChildren() const override {
		return std::vector<std::reference_wrapper<const ExpressionNode>>{*this};
	}

	Operation getOperation() const {
		return IDENTITY;
	}

 private:
	long n;
};

/**
 * Represents a node that can hold a more complex operation (which really ends up being a tree)
 */
class ExpressionTree : public ExpressionNode {
 public:
	ExpressionTree(EvaluationStrategy strategy) : strategy(strategy) {
	}

	void setLeft(std::unique_ptr<ExpressionNode> &&left) {
		this->left = std::move(left);
	}

	void setRight(std::unique_ptr<ExpressionNode> &&left) {
		this->right = std::move(left);
	}

	void setOp(Operation op) {
		this->op = op;
	}

	long evaluate() const override {
		if (!this->canFullyEvaluate()) {
			// It USUALLY shouldn't happen but sometimes parentheticals can only have one child
			if (this->left) {
				return this->left->evaluate();
			} else if (this->right) {
				return this->right->evaluate();
			} else {
				throw "Tree must have a component to evaluate";
			}
		}

		return this->strategy(this->left, this->right, *(this->op));
	}

	std::vector<std::reference_wrapper<const ExpressionNode>> getChildren() const override {
		if (!this->left && !this->right) {
			throw "Cannot evaluate empty tree's children";
		}

		std::vector<std::reference_wrapper<const ExpressionNode>> children;
		// It USUALLY shouldn't happen but sometimes parentheticals can only have one child
		if (this->left) {
			children.push_back(*this->left);
		}
		if (this->right) {
			children.push_back(*this->right);
		}

		return children;
	}

	Operation getOperation() const {
		if (!this->op) {
			throw "Cannot get an unset operator";
		}

		return *this->op;
	}

 private:
	std::unique_ptr<ExpressionNode> left;
	std::unique_ptr<ExpressionNode> right;
	std::optional<Operation> op;
	EvaluationStrategy strategy;

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

/**
 * Parse a string component into an operator
 * @param component The string component to check
 * @return std::optional<Operation> The operation, if this component represents one. If not, returns an empty optional.
 */
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

/**
 * Parse a string component into a number
 * @param component The string component to check
 * @return std::optional<Operation> The number, if this component represents one. If not, returns an empty optional.
 */
std::optional<long> parseNumber(const std::string_view component) {
	try {
		// This will only really ever copy some (usually small) number of digits... I don't consider it a very
		// expensive copy
		return std::stol(std::string(component));
	} catch (std::invalid_argument) {
		return std::nullopt;
	}
}

/**
 * Parse a parenthetical component of an expression
 * @param cursor The cursor to start searching backwards from
 * @param input The full input
 * @return std::optional<std::pair<std::string_view, int>> The parenthetical, if one exists (without parens).
 */
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

/**
 * Build a parse tree
 * @param input The input to build from
 * @param strategy The evaluation strategy for each of the nodes to use
 * @return std::unique_ptr<ExpressionNode> The root of the tree
 */
std::unique_ptr<ExpressionNode> buildTree(const std::string_view input, const EvaluationStrategy &strategy) {
	if (std::count_if(input.cbegin(), input.cend(), [](char c) { return c == ' '; }) == 0) {
		std::optional<long> value = parseNumber(input);
		if (!value) {
			throw std::invalid_argument("one-component string is expected to be number");
		}

		return std::make_unique<ValueNode>(*value);
	}

	ExpressionTree tree(strategy);
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
			auto parentheticalTree = buildTree(parentheticalPart->first, strategy);
			// NOTE: Just like concrete values, parentheticals will *ALWAYS* (except for leaves) be in the right
			// subtree. This helps with precedence parsing later
			tree.setRight(std::move(parentheticalTree));
			cursor = parentheticalPart->second - 2;
			continue;
		}

		// Attempt to add an operator to our parse if we have it
		std::optional<Operation> componentOperation = parseOperator(component);
		if (componentOperation) {
			tree.setOp(*componentOperation);
			auto rest = input.substr(0, cursor + 1);
			auto rightTree = buildTree(rest, strategy);
			tree.setLeft(std::move(rightTree));
			// Once we have found an operator and the operand to the left of it, we're done
			break;
		}

		// Attempt to find a value
		std::optional<long> componentValue = parseNumber(component);
		if (!componentValue) {
			throw std::invalid_argument("Expected number as last possible option");
		}
		// NOTE: Concrete values will *ALWAYS* (except at the leaf level) be in the right sub-tree. This was not
		// intentional but will help with precedence later.
		tree.setRight(std::make_unique<ValueNode>(*componentValue));
	}

	return std::make_unique<ExpressionTree>(std::move(tree));
}

/**
 * Evaluate the puzzle input, evaluating each node according to the given strategy.
 * @param input The input
 * @param strategy The strategy to evaluate each node
 * @return long The puzzle result
 */
long run(const std::vector<std::string> &input, const EvaluationStrategy &strategy) {
	return std::accumulate(input.cbegin(), input.cend(), 0L, [&strategy](long total, const std::string &expression) {
		auto tree = buildTree(expression, strategy);
		return total + tree->evaluate();
	});
}

long part1(const std::vector<std::string> &input) {
	EvaluationStrategy strategy =
		[](const std::unique_ptr<ExpressionNode> &left, const std::unique_ptr<ExpressionNode> &right, Operation op) {
			long leftValue = left->evaluate();
			long rightValue = right->evaluate();

			switch (op) {
				case ADDITION:
					return leftValue + rightValue;
				case MULTIPLICATION:
					return leftValue * rightValue;
				default:
					throw std::invalid_argument("Invalid operation");
			}
		};

	return run(input, strategy);
}

long part2(const std::vector<std::string> &input) {
	EvaluationStrategy strategy = [](const std::unique_ptr<ExpressionNode> &left,
									 const std::unique_ptr<ExpressionNode> &right,
									 Operation op) -> long {
		auto leftChildren = left->getChildren();
		if (leftChildren.size() == 1 && op == ADDITION) {
			return leftChildren.at(0).get().evaluate() + right->evaluate();
		} else if (leftChildren.size() == 1 && op == MULTIPLICATION) {
			return leftChildren.at(0).get().evaluate() * right->evaluate();
		} else if (leftChildren.size() != 2) {
			throw std::invalid_argument("Cannot perform unknown binary operation on children");
		}

		if (op == MULTIPLICATION) {
			return right->evaluate() * left->evaluate();
		} else if (op != ADDITION && left->getOperation() != MULTIPLICATION) {
			throw std::invalid_argument("Cannot perform an unknown binary operation on children");
		}

		// We wish to traverse down the righthand children until we hit a multiplication operator
		// By doing this, we prioritize adding up the operands first, and then we can evaluate the multiplicand after
		// the fact
		ExpressionNode const *prevCursor = nullptr;
		ExpressionNode const *cursor = left.get();
		long total = right->evaluate();
		while (cursor->getChildren().size() == 2 && cursor->getOperation() == ADDITION) {
			auto cursorChildren = cursor->getChildren();
			const ExpressionNode &leftCursorChild = cursorChildren.at(0);
			const ExpressionNode &rightCursorChild = cursorChildren.at(1);
			total += rightCursorChild.evaluate();
			prevCursor = cursor;
			cursor = &leftCursorChild;
		}

		if (cursor->getChildren().size() == 2) {
			auto cursorChildren = cursor->getChildren();
			const ExpressionNode &leftCursorChild = cursorChildren.at(0);
			const ExpressionNode &rightCursorChild = cursorChildren.at(1);
			// Add the right child of the multiplicand (to keep with addition priority), and multiply the multiplicand
			// (since we know that we can no longer use addition)
			return (total + rightCursorChild.evaluate()) * leftCursorChild.evaluate();
		} else if (cursor->getChildren().size() != 1) {
			throw std::invalid_argument("Only binary and unary operations are supported");
		}

		// If we only have one child, we must consider what the operation was that lead to it.
		const ExpressionNode &cursorChild = cursor->getChildren().at(0);
		Operation finalOperation = op;
		if (prevCursor) {
			finalOperation = prevCursor->getOperation();
		}
		if (finalOperation == ADDITION) {
			return total + cursorChild.evaluate();
		} else if (finalOperation == MULTIPLICATION) {
			return total * cursorChild.evaluate();
		} else {
			throw std::invalid_argument("Cannot perform an unknown binary operation on total");
		}
	};

	return run(input, strategy);
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);

	std::cout << part1(input) << std::endl;
	std::cout << part2(input) << std::endl;
}
