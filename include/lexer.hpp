#if !defined(LEXER_H)
#define LEXER_H

#include <string>
#include <cstdint>

typedef enum
{
	TOK_UNKNOWN,
	TOK_EOF,
	TOK_PLUS,
	TOK_MINUS,
	TOK_RIGHT,
	TOK_LEFT,
	TOK_OUT,
	TOK_IN,
	TOK_WHILE,
	TOK_END_WHILE,
} TokenType;

typedef struct
{
	TokenType type;
	intptr_t value;
} Token;

class Lexer
{
private:
	int index;
	std::string source;

public:
	Lexer(std::string source);
	Token NextToken();
	Token Instruction(char inst);
	bool IsInstruction(char ch);
};

#endif // LEXER_H
