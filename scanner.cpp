#include "scanner.h"
#include <iostream>
#include <unordered_map>
using namespace std;
unordered_map<string, TokenType> keywords;

bool isDigit(const char& c)
{
	return '0' <= c  && c <= '9';
}

bool isAlpha(const char& c)
{
	return  ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

bool match(const string& source, const char& c, int& index)
{
	if (index >= source.size()) return false;
	if (source[index] != c) return false;
	index++;
	return true;
}
string ExctractData(const Token& token)
{
	TokenType type = token.type;
	switch (type)
	{
	case TokenType::PLUS:
	case TokenType::MINUS:
	case TokenType::STAR:
	case TokenType::SLASH:
	case TokenType::SEMICOLON:
	case TokenType::NUMBER:
	case TokenType::STRING:
		return  *((string*)token.data) ;
	case TokenType::EMPTY:
		return "NONE";
	case TokenType::ERROR:
		return "ERROR";
	default:
		return "UNHANDLED";
	}
}

Token parseNumber(const string& source, const int& line, int& index)
{
	int value_offset = 1;
	while (isDigit(source[index + value_offset])) { value_offset++; }

	if (source[index + value_offset] == '.')
	{
		value_offset++;
		while (isDigit(source[index + value_offset])) { value_offset++; }
	} 

	Token out;
	out.type = TokenType::NUMBER;
	out.line = line;
	out.pos = index;
	out.data = new string(source, index, value_offset);
	index += value_offset;
	return out;
}

Token parseIdentifier(const string& source, const int& line, int& index)
{
	Token out;
	out.line = line;
	out.pos = index;
	out.data = new string();

	string identifier = "";
	while ( isAlpha(source[index]) || isDigit(source[index]) )
	{
		identifier += source[index];
		index++;
	}
	*(string*)out.data = identifier;

	auto iter = keywords.find(identifier);
	if (iter == keywords.end()) out.type = TokenType::IDENTIFIER;
	else out.type = iter->second;

	return out;
}

bool isWhiteSpace(const char& c, int& line)
{
	switch (c)
	{
	case ' ':
	case '\r':
	case '\t':
		return true;
	case '\n':
		line++;
		return true;
	default:
		return false;
	}
}

Token parseComment(const string& source, int& line, int& index)
{
	index++;
	while (source[index] != '\0' && source[index] != '\n')
	{
		index++;
	}
	Token out;
	out.type = TokenType::EMPTY;
	return out;
}

Token parseString(const string& source, int& line, int& index)
{
	string target = "";
	
	while (true)
	{
		if (source[index] == '\n' || index >= source.size()) break;
		if (source[index] == '"' && source[index - 1] != '\\') break;
		
		target += source[index];
		index++;
	}
	bool correct_string = index < source.size() && source[index] != '\n';
	index++;
	Token out;
	out.line = line;
	out.type = correct_string ? TokenType::STRING : TokenType::ERROR;
	out.pos = index;
	out.data = new string();
	*(string*)out.data = correct_string ? target : "INCORRECT STRING";
	return out;
}
Token parseLeftovers(const string& source, int& line, int& index)
{
	char c = source[index];
	Token out;
	out.pos = index;
	out.line = line;
	switch (c)
	{
	case '+':
		index++;
		out.type = TokenType::PLUS;
		out.data = new string("+");
		return out;
	case '/':
		if (source[index + 1] == '/') return parseComment(source, line, index); //one line comment
		index++;
		out.type = TokenType::SLASH;
		out.data = new string("/");
		return out;
	case '*':
		index++;
		out.type = TokenType::STAR;
		out.data = new string("*");
		return out;
	case '-':
		index++;
		out.type = TokenType::MINUS;
		out.data = new string("-");
		return out;
	case '!':
	{
		index++;
		bool nq = match(source, '=', index);
		out.type = nq ? TokenType::BANG_EQUAL : TokenType::BANG;
		out.data = new string();
		*(string*)out.data = nq ? "!=" : "!";
		return out;
	}
	case '=':
	{
		index++;
		bool eqeq = match(source, '=', index);
		out.type = eqeq ? TokenType::EQUAL_EQUAL : TokenType::EQUAL;
		out.data = new string();
		*(string*)out.data = eqeq ? "==" : "=";
		return out;
	}
	case '<':
	{
		index++;
		bool lq = match(source, '=', index);
		out.type = lq ? TokenType::LESS_EQUAL : TokenType::LESS;
		out.data = new string();
		*(string*)out.data = lq ?  "<=" : "<";
		return out;
	}
	case '>':
	{
		index++;
		bool gq = match(source, '=', index);
		out.type = gq ? TokenType::GREATER_EQUAL : TokenType::GREATER;
		out.data = new string();
		*(string*)out.data = gq ? ">=" : ">";
		return out;
	}
	case '"':
		index++;
		return parseString(source, line, index);
	case ';':
		index++;
		out.type = TokenType::SEMICOLON;
		out.data = new string(";");
		return out;
	case '(':
		index++;
		out.type = TokenType::LEFT_PAREN;
		out.data = new string("(");
		return out;
	case ')':
		index++;
		out.type = TokenType::RIGHT_PAREN;
		out.data = new string(")");
		return out;
	case '{':
		index++;
		out.type = TokenType::LEFT_BRACE;
		out.data = new string("{");
		return out;
	case '}':
		index++;
		out.type = TokenType::RIGHT_BRACE;
		out.data = new string("}");
		return out;
	case ',':
		index++;
		out.type = TokenType::COMMA;
		out.data = new string(",");
		return out;
	case '.':
		index++;
		out.type = TokenType::DOT;
		out.data = new string(".");
		return out;
	case '~':
		index++;
		out.type = TokenType::TILDE;
		out.data = new string("~");
		return out;
	case '?':
		index++;
		out.type = TokenType::QUESTION_MARK;
		out.data = new string("?");
		return out;
	case ':':
		index++;
		out.type = TokenType::COLON;
		out.data = new string(":");
		return out;
	case '^':
		index++;
		out.type = TokenType::POWER;
		out.data = new string("^");
		return out;
	default:
		out.type = TokenType::EMPTY;
		return out;
		break;
	}
}
vector<Token> scan(std::string source)
{
	std::vector<Token> tokens;
	int index = 0;
	int line = 0;
	// init keyword table
	keywords.emplace("and", TokenType::AND);
	keywords.emplace("class", TokenType::CLASS);
	keywords.emplace("else", TokenType::ELSE);
	keywords.emplace("false", TokenType::FALSE);
	keywords.emplace("for", TokenType::FOR);
	keywords.emplace("fun", TokenType::FUN);
	keywords.emplace("if", TokenType::IF);
	keywords.emplace("nil", TokenType::NIL);
	keywords.emplace("or", TokenType::OR);
	keywords.emplace("print", TokenType::PRINT);
	keywords.emplace("return", TokenType::RETURN);
	keywords.emplace("super", TokenType::SUPER);
	keywords.emplace("this", TokenType::THIS);
	keywords.emplace("true", TokenType::TRUE);
	keywords.emplace("var", TokenType::VAR);
	keywords.emplace("while", TokenType::WHILE);

	while (index < source.size())
	{
		char current = source[index];
		if (isDigit(current)) tokens.push_back(parseNumber(source, line, index));
		else if (isAlpha(current))tokens.push_back(parseIdentifier(source, line, index));
		else if (isWhiteSpace(current, line)) index++;
		else
		{
			Token token = parseLeftovers(source, line, index);
			if (token.type != TokenType::EMPTY)
					tokens.push_back(token);
		}
	}

	return tokens;
}

void printTokens(const vector<Token>& tokens)
{
	for (const Token& token : tokens)
	{
		cout << "Token type [" << TranslateType(token.type) << "] at line [" 
			<< token.line << "] contains [" << *((string*)token.data) <<"]" << endl;
	}
}

string TranslateType(TokenType type)
{
	switch (type)
		{
		case TokenType::AND:
		case TokenType::CLASS:
		case TokenType::ELSE:
		case TokenType::FALSE:
		case TokenType::FOR:
		case TokenType::FUN:
		case TokenType::IF:
		case TokenType::NIL:
		case TokenType::OR:
		case TokenType::PRINT:
		case TokenType::RETURN:
		case TokenType::SUPER:
		case TokenType::THIS:
		case TokenType::TRUE:
		case TokenType::VAR:
		case TokenType::WHILE:
			return "KEYWORD";
		case TokenType::LEFT_BRACE:
			return "LEFT_BRACE";
		case TokenType::RIGHT_BRACE:
			return "RIGHT_BRACE";
		case TokenType::LEFT_PAREN:
			return "LEFT_PAREN";
		case  TokenType::RIGHT_PAREN:
			return "RIGHT_PAREN";
		case TokenType::IDENTIFIER:
			return "IDENTIFIER";
		case TokenType::NUMBER:
			return "NUMBER";
		case TokenType::STRING:
			return "STRING";
		case TokenType::PLUS:
			return "PLUS";
		case TokenType::MINUS:
			return "MINUS";
		case TokenType::STAR:
			return "STAR";
		case TokenType::SLASH:
			return "SLASH";
		case TokenType::EMPTY:
			return "EMPTY";
		case TokenType::SEMICOLON:
			return "SEMICOLON";
		default:
			return "UNHANDLED";
		}
}
