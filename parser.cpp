#include "parser.h"
#include <iostream>
using namespace std;

AstNode* expression(const std::vector<Token>& tokens, int& index);
AstNode* declaration(const std::vector<Token>& tokens, int& index);
AstNode* variable_declaration(const std::vector<Token>& tokens, int& index);
AstNode* parse_params(const std::vector<Token>& tokens, int& index); 
AstNode* function_declaration(const std::vector<Token>& tokens, int& index);
AstNode* parse_args(const std::vector<Token>& tokens, int& index); 

bool check(TokenType next, const std::vector<Token>& tokens, int& index)
{
	if (tokens[index].type != next)
	{
		cout << "Expected token at line " << tokens[index].line
			<< " is [" << TranslateType(next) << "] but given is [" << TranslateType(tokens[index].type) << "]" << endl;
		return false;
	}
	return true;
}
void consume(TokenType next, const std::vector<Token>& tokens, int& index)
{
	bool result = check(next, tokens, index);
	if (!result) exit(-1);
	index++;
}

bool match(vector<TokenType> types, const std::vector<Token>& tokens, int& index)
{
	if (index >= tokens.size()) return false;
	for (auto& type : types)
	{
		if (type == tokens[index].type)
		{
			index++;
			return true;
		}
	}
	return false;
}

AstNode* primary(const std::vector<Token>& tokens, int& index)
{
	AstNode* node = new AstNode();
	switch (tokens[index].type)
	{
		case TokenType::STRING:
		{
			node->type = AstNodeType::STRING;
			node->data = new string();
			*(string*)node->data = *(string*)tokens[index].data; // copy strings
			index++;
			return node;
		}
		case TokenType::NUMBER:
		{
			node->type = AstNodeType::NUMBER;
			node->data = new double();
			*(double*)node->data = stod(*(string*)tokens[index].data); // cast to double and store
			index++;
			return node;
		}
		case TokenType::IDENTIFIER:
		{
			node->type = AstNodeType::IDENTIFIER;
			node->data = new string();
			*(string*)node->data = *(string*)tokens[index].data; // copy strings
			index++;
			return node;
		}
		case TokenType::TRUE:
		case  TokenType::FALSE:
		{
			node->type = AstNodeType::LOGICAL;
			node->data = new bool();
			*(bool*)node->data = *(string*)tokens[index].data == "true" ? true : false; // convert to logical
			index++;
			return node;
		}
		case TokenType::NIL:
		{
			node->type = AstNodeType::NIL;
			node->data = nullptr; // convert to null;
			index++;
			return node;
		}
		case TokenType::LEFT_PAREN:
		{
			index++;
			delete node;
			node = expression(tokens, index);
			if (!match({ TokenType::RIGHT_PAREN }, tokens, index))
			{
				cout << "Lack of expression closure\n";
				exit(-1);
			}
			return node;
		}
		case TokenType::THIS:
		case TokenType::SUPER:
		{
			cout << "Unsoported language feature" << endl;
			exit(-1);
		}
	}
}

AstNode* call(const std::vector<Token>& tokens, int& index)
{
	AstNode* parent = primary(tokens, index);
	while (match({ TokenType::LEFT_PAREN,}, tokens, index))
	{
		Token previous = tokens[index - 1];
		AstNode* right;
		if (match({ TokenType::RIGHT_PAREN }, tokens, index)) right == nullptr;
		else
		{
			right = parse_args(tokens, index);
			consume(TokenType::RIGHT_PAREN, tokens, index);
		}
		AstNode* new_parent = new AstNode();
		new_parent->children.push_back(parent);
		new_parent->children.push_back(right);
		new_parent->type = AstNodeType::OP_CALL;
		parent = new_parent;
	}
	return parent;
}

AstNode* unary(const std::vector<Token>& tokens, int& index)
{
	if (match({ TokenType::BANG, TokenType::MINUS }, tokens, index))
	{
		Token previous = tokens[index - 1];
		AstNode *node = unary(tokens, index);
		AstNode* parent = new AstNode();
		parent->children.push_back(node);
		parent->data = new string();
		*(string*)parent->data = previous.type == TokenType::MINUS ? "-" : "!";
		parent->type = previous.type == TokenType::MINUS ? 
			AstNodeType::OP_NEGATE : AstNodeType::OP_BANG;

		return parent;
	}
	else return call(tokens, index);
}

AstNode* factor(const std::vector<Token>& tokens, int& index)
{
	AstNode* parent = unary(tokens, index);
	while (match({ TokenType::SLASH, TokenType::STAR }, tokens, index))
	{
		Token previous = tokens[index - 1];
		AstNode* right = unary(tokens, index);
		AstNode* new_parent = new AstNode();
		new_parent->children.push_back(parent);
		new_parent->children.push_back(right);
		new_parent->data = new string();
		*(string*)new_parent->data = previous.type == TokenType::SLASH ? "/" : "*";
		new_parent->type = previous.type == TokenType::SLASH ?
			AstNodeType::OP_DIVIDE : AstNodeType::OP_MUL;
		parent = new_parent;
	}
	return parent;
}

AstNode* term(const std::vector<Token>& tokens, int& index)
{
	AstNode* parent = factor(tokens, index);
	while (match({ TokenType::MINUS, TokenType::PLUS }, tokens, index))
	{
		Token previous = tokens[index - 1];
		AstNode* right = factor(tokens, index);
		AstNode* new_parent = new AstNode();
		new_parent->children.push_back(parent);
		new_parent->children.push_back(right);
		new_parent->data = new string();
		*(string*)new_parent->data = previous.type == TokenType::PLUS ? "+" : "-";
		new_parent->type = previous.type == TokenType::PLUS ?
			AstNodeType::OP_ADD : AstNodeType::OP_SUBTRACT;
		parent = new_parent;
	}

	return parent;
}

AstNode* comparison(const std::vector<Token>& tokens, int& index)
{
	AstNode* parent = term(tokens, index);
	while (match({ TokenType::GREATER,TokenType::GREATER_EQUAL,
		TokenType::LESS,TokenType::LESS_EQUAL }, tokens, index))
	{
		Token previous = tokens[index - 1];
		AstNode* right = term(tokens, index);
		AstNode* new_parent = new AstNode();
		new_parent->children.push_back(parent);
		new_parent->children.push_back(right);
		new_parent->data = new string();

		switch (previous.type)
		{
		case TokenType::GREATER:
			*(string*)new_parent->data = ">";
			new_parent->type = AstNodeType::OP_GREATER;
			break;
		case TokenType::GREATER_EQUAL:
			*(string*)new_parent->data = ">=";
			new_parent->type = AstNodeType::OP_GREATER_EQUAL;
			break;
		case TokenType::LESS:
			*(string*)new_parent->data = "<";
			new_parent->type = AstNodeType::OP_LESS;
			break;
		case TokenType::LESS_EQUAL:
			*(string*)new_parent->data = "<=";
			new_parent->type = AstNodeType::OP_LESS_EQUAL;
			break;
		}
		parent = new_parent;
	}
	return parent;
}

AstNode* equality(const std::vector<Token>& tokens, int& index)
{
	AstNode* parent = comparison(tokens, index);
	while (match({ TokenType::BANG_EQUAL,TokenType::EQUAL_EQUAL}, tokens, index))
	{
		Token previous = tokens[index - 1];
		AstNode* right = comparison(tokens, index);
		AstNode* new_parent = new AstNode();
		new_parent->children.push_back(parent);
		new_parent->children.push_back(right);
		new_parent->data = new string();

		switch (previous.type)
		{
		case TokenType::BANG_EQUAL:
			*(string*)new_parent->data = "!=";
			new_parent->type = AstNodeType::OP_NOT_EQUAL;
			break;
		case TokenType::EQUAL_EQUAL:
			*(string*)new_parent->data = "==";
			new_parent->type = AstNodeType::OP_EQUAL_EQUAL;
			break;
		}
		parent = new_parent;
	}
	return parent;
}

AstNode* logic_and(const std::vector<Token>& tokens, int& index)
{
	AstNode* parent = equality(tokens, index);
	while (match({ TokenType::AND }, tokens, index))
	{
		Token previous = tokens[index - 1];
		AstNode* right = equality(tokens, index);
		AstNode* new_parent = new AstNode();
		new_parent->children.push_back(parent);
		new_parent->children.push_back(right);
		new_parent->data = new string();

		*(string*)new_parent->data = "and";
		new_parent->type = AstNodeType::OP_AND;
		parent = new_parent;
	}
	return parent;
}

AstNode* logic_or(const std::vector<Token>& tokens, int& index)
{
	AstNode* parent = logic_and(tokens, index);
	while (match({ TokenType::OR }, tokens, index))
	{
		Token previous = tokens[index - 1];
		AstNode* right = logic_and(tokens, index);
		AstNode* new_parent = new AstNode();
		new_parent->children.push_back(parent);
		new_parent->children.push_back(right);
		new_parent->data = new string("or");
		new_parent->type = AstNodeType::OP_OR;
		parent = new_parent;
	}
	return parent;
}

AstNode* assignment(const std::vector<Token>& tokens, int& index)
{
	AstNode* parent = logic_or(tokens, index);
	if (match({ TokenType::EQUAL }, tokens, index))
	{
		Token previous = tokens[index - 1];
		AstNode* right = assignment(tokens, index);
		AstNode* new_parent = new AstNode();
		new_parent->children.push_back(parent);
		new_parent->children.push_back(right);
		new_parent->data = new string("=");
		new_parent->type = AstNodeType::OP_EQUAL;
		parent = new_parent;
	}
	return parent;
}

AstNode* expression(const std::vector<Token>& tokens, int& index)
{
	return assignment(tokens, index);
}

AstNode* statement(const std::vector<Token>& tokens, int& index)
{
	AstNode* root = new AstRoot;
	//block
	if (match({ TokenType::LEFT_BRACE }, tokens, index))
	{
		root->type = AstNodeType::BLOCK;
		root->data = new string("NEW BLOCK");
		while (!match({ TokenType::RIGHT_BRACE }, tokens, index) )
		{
			if (index >= tokens.size())
			{
				cout << "BLOCK NOT CLOSED " << endl;
				exit(-1);
			}

			root->children.push_back(declaration(tokens, index));
		}
		return root;

	}
	//while
	else if (match({ TokenType::WHILE }, tokens, index))
	{
		root->type = AstNodeType::OP_WHILE;
		root->data = new string("WHILE LOOP");

		consume(TokenType::LEFT_PAREN, tokens, index);
		root->children.push_back(expression(tokens, index) );
		consume(TokenType::RIGHT_PAREN, tokens, index);
		root->children.push_back(statement(tokens, index) );

		return root;
	}
	//if
	else if (match({ TokenType::IF }, tokens, index))
	{
		root->type = AstNodeType::OP_IF;
		root->data = new string("IF");

		consume(TokenType::LEFT_PAREN, tokens, index);
		root->children.push_back(expression(tokens, index) );
		consume(TokenType::RIGHT_PAREN, tokens, index);
		root->children.push_back(statement(tokens, index) );
		if (match({ TokenType::ELSE }, tokens, index))
		{
			root->children.push_back(statement(tokens, index));
		}
		return root;
	}
	//for
	else if (match({ TokenType::FOR }, tokens, index))
	{
		root->type = AstNodeType::OP_FOR;
		root->data = new string("FOR LOOP");
		consume(TokenType::LEFT_PAREN, tokens, index);
		//first part
		if (match({ TokenType::VAR }, tokens, index))
		{
			index--;
			root->children.push_back(variable_declaration(tokens, index));
		}
		else if (match({ TokenType::SEMICOLON }, tokens, index))
		{
			root->children.push_back(nullptr);
		}
		else
		{
			root->children.push_back(expression(tokens, index));
			consume(TokenType::SEMICOLON, tokens, index);
		}
		//second part
		if (!match({ TokenType::SEMICOLON }, tokens, index))
		{
			root->children.push_back(expression(tokens, index));
			consume(TokenType::SEMICOLON, tokens, index);
		}
		else root->children.push_back(nullptr);
		//third part
		if (!match({ TokenType::RIGHT_PAREN }, tokens, index))
		{
			root->children.push_back(expression(tokens, index));
			consume(TokenType::RIGHT_PAREN, tokens, index);
		}
		else root->children.push_back(nullptr);

		root->children.push_back(statement(tokens, index));
		return root;
	}
	//return
	else if (match({ TokenType::RETURN }, tokens, index))
	{
		root->type = AstNodeType::OP_RETURN;
		root->data = new string("return: ");
		if (!match({ TokenType::SEMICOLON }, tokens, index))
		{
			root->children.push_back(expression(tokens, index));
		}
		else
		{
			index--;
			AstNode *NIL_child = new AstNode();
			NIL_child->type = AstNodeType::NIL;
			NIL_child->data = new string("NIL");
			root->children.push_back(NIL_child);
		}
	}
	//print
	else if (match({TokenType::PRINT}, tokens, index))
	{
		root->type = AstNodeType::OP_PRINT;
		root->data = new string("print: ");
		root->children.push_back(expression(tokens, index));
	}
	//expression
	else
	{
		delete root;
		root = expression(tokens, index);
	}
	consume(TokenType::SEMICOLON, tokens, index);

	return root;
}

AstNode* declaration(const std::vector<Token>& tokens, int& index)
{
	if (match({ TokenType::FUN }, tokens, index))
	{
		index--;
		return function_declaration(tokens, index);
	}
	if (match({ TokenType::VAR }, tokens, index))
	{
		index--;
		return variable_declaration(tokens, index);
	}
	return statement(tokens, index);
}

AstNode* function_declaration(const std::vector<Token>& tokens, int& index)
{
	AstRoot* root = new AstRoot();
	root->type = AstNodeType::FUNCTION_DECLARATION;
	root->data = new string("FUNCTION");
	consume(TokenType::FUN, tokens, index);
	if (!check(TokenType::IDENTIFIER, tokens, index)) exit(-1);
	AstNode* name = primary(tokens, index);

	consume(TokenType::LEFT_PAREN, tokens, index);
	AstNode* params;
	if (match({ TokenType::RIGHT_PAREN }, tokens, index)) params = nullptr;
	else
	{
		params = parse_params(tokens, index);
		consume(TokenType::RIGHT_PAREN, tokens, index);
	}

	if (!check(TokenType::LEFT_BRACE, tokens, index)) exit(-1);
	AstNode* body = statement(tokens, index);

	root->children.push_back(name);
	root->children.push_back(params);
	root->children.push_back(body);
	return root;
}

AstNode* parse_params(const std::vector<Token>& tokens, int& index)
{
	AstRoot* root = new AstRoot();
	root->type = AstNodeType::PARAMS;
	root->data = new string("PARAMS");
	while (true)
	{
		if (!check(TokenType::IDENTIFIER, tokens, index)) exit(-1);
		root->children.push_back(primary(tokens, index));
		if (tokens[index].type != TokenType::COMMA) break;
		consume(TokenType::COMMA, tokens, index);
	}
	return root;
}

AstNode* parse_args(const std::vector<Token>& tokens, int& index)
{
	AstRoot* root = new AstRoot();
	root->type = AstNodeType::ARGS;
	root->data = new string("ARGS");
	while (true)
	{
		root->children.push_back(expression(tokens, index));
		if (!match({ TokenType::COMMA }, tokens, index)) break;
	}
	return root;
}

AstNode* variable_declaration(const std::vector<Token>& tokens, int& index)
{
	AstRoot* root = new AstNode();
	root->type = AstNodeType::VARIABLE_DECLARATION;
	root->data = new string("(VAR) =");
	consume(TokenType::VAR, tokens, index);
	// assert that next token in identifier if not send error
	if (!check(TokenType::IDENTIFIER, tokens, index))
		exit(-1);

	root->children.push_back( primary(tokens, index) );
	if (match({ TokenType::EQUAL }, tokens, index))
		root->children.push_back(expression(tokens, index));
	else 
		root->children.push_back(nullptr);

	consume(TokenType::SEMICOLON, tokens, index);
	return root;
}

std::vector<AstNode*> parse(const std::vector<Token>& tokens)
{
	int index = 0;
	vector<AstNode*> out;
	while (index < tokens.size())
	{
		AstRoot* root;
		root = declaration(tokens, index);
		out.push_back(root);
	}

	return out;
}

string createStringTree(AstRoot* node, string propagate = "")
{
	if (node == nullptr) return propagate + "NULL";
	string space = "   ";
	vector<string> children_strings;
	for (int i = 0; i < node->children.size(); i++)
	{
		string child_string = createStringTree(node->children[i]);
		children_strings.push_back(child_string);
	}
	switch (node->type)
	{
	case AstNodeType::BLOCK:
	{
		string out = *(string*)node->data + "\n";
		for (AstRoot* root : node->children)
		{
			out += propagate + "  " + createStringTree(root, propagate + space) + "\n";
		}
		out +=  "END BLOCK";
		return out;
	}
	case AstNodeType::OP_PRINT:
		return *(string*)node->data + children_strings[0];
	case AstNodeType::NUMBER:
		return to_string(*(double*)node->data);
	case AstNodeType::NIL:
		return "NIL";
	case AstNodeType::IDENTIFIER:
	case AstNodeType::STRING:
		return *(string*)node->data;
	case AstNodeType::LOGICAL:
		return *(bool*)node->data ? "true" : "false";
	case AstNodeType::VARIABLE_DECLARATION:
	case AstNodeType::OP_OR:
	case AstNodeType::OP_AND:
	case AstNodeType::OP_EQUAL_EQUAL:
	case AstNodeType::OP_EQUAL:
	case AstNodeType::OP_NOT_EQUAL:
	case AstNodeType::OP_LESS:
	case AstNodeType::OP_LESS_EQUAL:
	case AstNodeType::OP_GREATER:
	case AstNodeType::OP_GREATER_EQUAL:
	case AstNodeType::OP_SUBTRACT:
	case AstNodeType::OP_DIVIDE:
	case AstNodeType::OP_MUL:
	case AstNodeType::OP_ADD:
	{
		string out = "";
		out = "( " + children_strings[0] + " " + *(string*)node->data + " " + children_strings[1] + " )";
		return out;
	}
	case AstNodeType::OP_NEGATE:
	case AstNodeType::OP_BANG:
	{
		string out = "( ";
		char c = *(char*)node->data;
		out +=  c + children_strings[0] + " )";
		return out;
	}
	case AstNodeType::OP_RETURN:
		return *(string*)node->data + children_strings[0];
	case AstNodeType::OP_WHILE:
	{
		string out;
		out += *(string*)node->data 
			+  children_strings[0] + '\n' + 
			propagate + children_strings[1];

		return out;
	}
	case AstNodeType::OP_IF:
	{
		string out = "";
		string Else = (node->children.size() == 3) ? "ELSE \n" : "";
		out += *(string*)node->data + children_strings[0];
		int i = 1;
		out += '\n' + children_strings[1];
		out += (node->children.size() == 3) ?  '\n' + Else+ children_strings[2] : "";

		return out;
	}
	case AstNodeType::OP_FOR:
	{
		string out = "";
		out += *(string*)node->data + '\n' +
			children_strings[0] + '\n' +
			children_strings[1] + '\n' +
			children_strings[2] + '\n' + 
			children_strings[3];
		return out;
	}
	case AstNodeType::ARGS:
	case AstNodeType::PARAMS:
	{
		string out = *(string*)node->data + ": ";
		for (string& param : children_strings)
		{
			out += param + ',';
		}
		return out;
	}
	case AstNodeType::FUNCTION_DECLARATION:
	{
		string out = "";
		out += *(string*)node->data + ' ' +
			children_strings[0] + '\n' +
			children_strings[1] + '\n' +
			children_strings[2] + '\n';
		return out;
	}
	case AstNodeType::OP_CALL:
	{
		string out = "";
		out += "CALL FUNCTION: "  +
			children_strings[0] + '\n' +
			children_strings[1];
		return out;
	}
	default:
		return "UKNOWN";
	}
}
void printAST(AstRoot* root)
{
	cout << createStringTree(root);
}

void printSequence(std::vector<AstNode*>& AstSequence)
{
	cout <<endl << "SEQUENCE START" << endl << endl;
	for (AstRoot* root : AstSequence)
	{
		printAST(root);
		cout << endl;
	}
	cout << endl << "SEQUENCE END" << endl << endl;
}
