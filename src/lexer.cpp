#include <lexer.hpp>

Lexer::Lexer(std::string source){
	this->source = source;
	this->index = 0;
};

Token Lexer::NextToken(){
	if(this->index >= this->source.length()){
		return Token{.type=TOK_EOF, .value=0};
	} else if(this->IsInstruction(this->source.at(this->index))) {
		return this->Instruction(this->source.at(this->index));
	}

	this->index++;

	return Token{.type=TOK_UNKNOWN, .value=0};
}

Token Lexer::Instruction(char inst){
	Token res{.type=TOK_UNKNOWN, .value=0};
	switch(inst){
		case '+':
			res.type = TOK_PLUS;
			break;
		case '-':
			res.type = TOK_MINUS;
			break;
		case '>':
			res.type = TOK_RIGHT;
			break;
		case '<':
			res.type = TOK_LEFT;
			break;
		case '.':
			res.type = TOK_OUT;
			break;
		case ',':
			res.type = TOK_IN;
			break;
		case '[':
			res.type = TOK_WHILE;
			break;
		case ']':
			res.type = TOK_END_WHILE;
			break;
		default:
			return res;
			break;
	}

	//res.value++;

	for(;;){
		if(this->index >= this->source.length()){
			break;
		}
		char ch = this->source.at(this->index);
		if(ch != inst){
			if(this->IsInstruction(ch)){
				break;
			} else {
				res.value--;
			}
		}
		res.value++;
		this->index++;
	}

	return res;
}

bool Lexer::IsInstruction(char ch){
	return ch == '+' || ch == '-' || ch == '>' || ch == '<' || ch == '.' || ch == ',' || ch == '[' || ch == ']';
}
