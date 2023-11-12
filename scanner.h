#ifndef  SCANNER_H
#define SCANNER_H
#include <vector>
#include <string>
enum class TokenType
{
	EMPTY,
	NUMBER,
	STRING,
	ERROR,
	SLASH,
	PLUS,
	MINUS,
	STAR,
	IDENTIFIER,
	SEMICOLON,
	BANG,
	AND,
	CLASS,
	ELSE,
	FALSE,
	FOR,
	FUN,
	IF,
	NIL,
	OR,
	PRINT,
	RETURN,
	SUPER,
	THIS,
	TRUE,
	VAR,
	WHILE,
	LEFT_BRACE,
	RIGHT_BRACE,
	LEFT_PAREN,
	RIGHT_PAREN,
	COMMA,
	TILDE,
	DOT,
	QUESTION_MARK,
	POWER,
	COLON,
	BANG_EQUAL,
	EQUAL_EQUAL,
	EQUAL,
	GREATER,
	GREATER_EQUAL,
	LESS,
	LESS_EQUAL
};

struct Token
{
	TokenType type;
	int line;
	int pos;
	void* data;
};

std::vector<Token> scan(std::string source);
void printTokens(const std::vector<Token>& tokens);
std::string TranslateType(TokenType type);
#endif // ! SCANNER_H
