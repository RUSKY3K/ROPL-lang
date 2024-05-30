#include <iostream>
#include <string>
#include <unordered_map>
#include <cctype>
#include <stdexcept>
#include <vector>
#include <sstream>

enum class TokenType {
    NUMBER,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    LPAREN,
    RPAREN,
    ASSIGN,
    VARIABLE,
    IF,
    ELSE,
    WHILE,
    FUNCTION,
    CALL,
    END
};

struct Token {
    TokenType type;
    std::string value;
};

class Lexer {
private:
    std::string input;
    size_t position;

public:
    Lexer(std::string input) : input(input), position(0) {}

    Token getNextToken() {
        while (position < input.size()) {
            char currentChar = input[position];

            if (std::isdigit(currentChar)) {
                std::string number;
                while (std::isdigit(currentChar)) {
                    number += currentChar;
                    position++;
                    currentChar = input[position];
                }
                return {TokenType::NUMBER, number};
            }

            if (std::isalpha(currentChar)) {
                std::string identifier;
                while (std::isalnum(currentChar) || currentChar == '_') {
                    identifier += currentChar;
                    position++;
                    currentChar = input[position];
                }
                if (identifier == "if")
                    return {TokenType::IF, "if"};
                else if (identifier == "else")
                    return {TokenType::ELSE, "else"};
                else if (identifier == "while")
                    return {TokenType::WHILE, "while"};
                else if (identifier == "function")
                    return {TokenType::FUNCTION, "function"};
                else
                    return {TokenType::VARIABLE, identifier};
            }

            switch (currentChar) {
                case '+':
                    position++;
                    return {TokenType::PLUS, "+"};
                case '-':
                    position++;
                    return {TokenType::MINUS, "-"};
                case '*':
                    position++;
                    return {TokenType::MULTIPLY, "*"};
                case '/':
                    position++;
                    return {TokenType::DIVIDE, "/"};
                case '(':
                    position++;
                    return {TokenType::LPAREN, "("};
                case ')':
                    position++;
                    return {TokenType::RPAREN, ")"};
                case '=':
                    position++;
                    return {TokenType::ASSIGN, "="};
                case ',':
                    position++;
                    return {TokenType::CALL, ","};
                default:
                    throw std::runtime_error("Invalid character");
            }
        }
        return {TokenType::END, ""};
    }
};

class Parser {
private:
    Lexer lexer;
    Token currentToken;

    void eat(TokenType expectedType) {
        if (currentToken.type == expectedType)
            currentToken = lexer.getNextToken();
        else
            throw std::runtime_error("Unexpected token");
    }

    int factor(std::unordered_map<std::string, int>& variables) {
        if (currentToken.type == TokenType::NUMBER) {
            int value = std::stoi(currentToken.value);
            eat(TokenType::NUMBER);
            return value;
        } else if (currentToken.type == TokenType::VARIABLE) {
            std::string varName = currentToken.value;
            eat(TokenType::VARIABLE);
            if (variables.find(varName) != variables.end()) {
                return variables[varName];
            } else {
                throw std::runtime_error("Undefined variable");
            }
        } else if (currentToken.type == TokenType::LPAREN) {
            eat(TokenType::LPAREN);
            int result = expr(variables);
            eat(TokenType::RPAREN);
            return result;
        } else {
            throw std::runtime_error("Unexpected token");
        }
    }

    int term(std::unordered_map<std::string, int>& variables) {
        int result = factor(variables);
        while (currentToken.type == TokenType::MULTIPLY || currentToken.type == TokenType::DIVIDE) {
            if (currentToken.type == TokenType::MULTIPLY) {
                eat(TokenType::MULTIPLY);
                result *= factor(variables);
            } else {
                eat(TokenType::DIVIDE);
                result /= factor(variables);
            }
        }
        return result;
    }

    int expr(std::unordered_map<std::string, int>& variables) {
        int result = term(variables);
        while (currentToken.type == TokenType::PLUS || currentToken.type == TokenType::MINUS) {
            if (currentToken.type == TokenType::PLUS) {
                eat(TokenType::PLUS);
                result += term(variables);
            } else {
                eat(TokenType::MINUS);
                result -= term(variables);
            }
        }
        return result;
    }

    void statement(std::unordered_map<std::string, int>& variables) {
        if (currentToken.type == TokenType::IF) {
            eat(TokenType::IF);
            eat(TokenType::LPAREN);
            int condition = expr(variables);
            eat(TokenType::RPAREN);
            if (condition != 0) {
                while (currentToken.type != TokenType::ELSE && currentToken.type != TokenType::END)
                    eat(currentToken.type);
            } else {
                while (currentToken.type != TokenType::ELSE && currentToken.type != TokenType::END)
                    eat(currentToken.type);
                if (currentToken.type == TokenType::ELSE) {
                    eat(TokenType::ELSE);
                    while (currentToken.type != TokenType::END)
                        eat(currentToken.type);
                }
            }
        } else if (currentToken.type == TokenType::WHILE) {
            eat(TokenType::WHILE);
            eat(TokenType::LPAREN);
            int condition = expr(variables);
            eat(TokenType::RPAREN);
            while (condition != 0) {
                while (currentToken.type != TokenType::END)
                    eat(currentToken.type);
                eat(TokenType::WHILE);
                eat(TokenType::LPAREN);
                condition = expr(variables);
                eat(TokenType::RPAREN);
            }
        } else if (currentToken.type == TokenType::FUNCTION) {
            eat(TokenType::FUNCTION);
            std::string functionName = currentToken.value;
            eat(TokenType::VARIABLE);
            eat(TokenType::LPAREN);
            std::vector<std::string> params;
            while (currentToken.type != TokenType::RPAREN) {
                params.push_back(currentToken.value);
                eat(TokenType::VARIABLE);
                if (currentToken.type == TokenType::RPAREN)
                    break;
                eat(TokenType::CALL);
            }
            eat(TokenType::RPAREN);
            int result = expr(variables);
            // For simplicity, just store function result as a variable
            variables[functionName] = result;
        } else if (currentToken.type == TokenType::VARIABLE) {
            std::string varName = currentToken.value;
            eat(TokenType::VARIABLE);
            eat(TokenType::ASSIGN);
            int value = expr(variables);
            variables[varName] = value;
        } else {
            expr(variables);
        }
    }

public:
    Parser(Lexer lexer) : lexer(lexer), currentToken(lexer.getNextToken()) {}

    void parse(std::unordered_map<std::string, int>& variables) {
        while (currentToken.type != TokenType::END) {
            statement(variables);
        }
    }
};

int main
() {
    std::string input;
    std::unordered_map<std::string, int> variables;
    std::cout << "MiniLang Interpreter\n";
    std::cout << "Enter code: \n";
    std::stringstream code;
    std::string line;
    while (std::getline(std::cin, line)) {
        code << line << std::endl;
    }

    Lexer lexer(code.str());
    Parser parser(lexer);

    try {
        parser.parse(variables);
        std::cout << "Variables: " << std::endl;
        for (const auto& pair : variables) {
            std::cout << pair.first << " = " << pair.second << std::endl;
        }
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
