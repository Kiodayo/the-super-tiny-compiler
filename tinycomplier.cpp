#include <iostream>
#include <string>
#include <vector>

struct Token
{
    std::string type;
    std::string value;
};

// tokenizer`s output used to be vector<Token> tokens
// string 会截断 空格
// input : (add 2 (subtract 4 2))
std::vector<Token> Tokenizer(std::string input)
{
    std::vector<Token> tokens;
    int current = 0;
    while (current < input.length())
    {
        // space
        if (input[current] == ' ')
        {
            current++;
            continue;
        }
        // paren
        if (input[current] == '(' || input[current] == ')')
        {
            Token token;
            token.type = "paren";
            token.value = input[current];
            tokens.push_back(token);
            current++;
            continue;
        }
        // char
        if ((input[current] >= 'a' && input[current] <= 'z') || (input[current] >= 'A' && input[current] <= 'Z'))
        {
            std::string value;
            while ((input[current] >= 'a' && input[current] <= 'z') || (input[current] >= 'A' && input[current] <= 'Z'))
            {
                value += input[current];
                current++;
            }
            Token token;
            token.type = "name";
            token.value = value;
            tokens.push_back(token);
            continue;
        }
        // number
        if (input[current] >= '0' && input[current] <= '9')
        {
            std::string value;
            while (input[current] >= '0' && input[current] <= '9')
            {
                value += input[current];
                current++;
            }
            Token token;
            token.type = "number";
            token.value = value;
            tokens.push_back(token);
            continue;
        }
    }
    return tokens;
};

void printTokens(std::vector<Token> tokens)
{
    for (auto token : tokens)
    {
        std::cout << token.type << " " << token.value << std::endl;
    }
};

// Tokens to AST Abstract Syntax Tree

/* {
    *type: 'Program',
        * body : [{
        *type: 'CallExpression',
            * name : 'add',
            * params : [{
            *type: 'NumberLiteral',
                * value : '2'
                *       }, {
                *type: 'CallExpression',
                *name : 'subtract',
                *params : [{
                *type: 'NumberLiteral',
                * value : '4'
                *         }, {
                *type: 'NumberLiteral',
                *value : '2'
                *         }]
                * }]
            *     }]
        *   }

*/

struct ASTNode
{
    std::string name;
    std::string type;
    std::string value;
    std::vector<ASTNode> params;
};

struct AST
{
    std::string type = "Program";
    std::vector<ASTNode> body;
};

ASTNode walk(std::vector<Token> tokens, int &current)
{
    Token token = tokens[current];

    // NumberLiteral
    if (token.type == "number")
    {
        current++;
        ASTNode node;
        node.type = "NumberLiteral";
        node.value = token.value;

        return node;
    }

    // StringLiteral
    if (token.type == "string")
    {
        current++;
        ASTNode node;
        node.type = "StringLiteral";
        node.value = token.value;

        return node;
    }

    // 遍历到括号 (重点)
    if (token.type == "paren" && token.value == "(")
    {
        // 跳到下一个token
        token = tokens[++current];

        // 根据Lisp规则，左括号右边一定时是一个函数表达式
        ASTNode node;
        node.type = "CallExpression";
        node.name = token.value; // add , subtract

        // 跳过name类型的token
        token = tokens[++current];

        // And now we want to loop through each token that will be the `params` of our `CallExpression` until we encounter a closing parenthesis.

        // 一直迭代，直到碰到右括号,或者下一个非括号的类型
        // 比如 (add (substract 2 4) 6)
        // 先遍历到第一个左括号，然后遍历到add，创建节点，再遍历到左括号，创建节点substract
        while (token.type != "paren" || (token.type == "paren" && token.value != ")"))
        {
            // 递归
            node.params.push_back(walk(tokens, current));
            token = tokens[current];
        }

        // 跳过右括号
        current++;

        return node;
    }

    throw std::runtime_error("Unknown token: " + token.value);
};

// Okay, so we define a `parser` function that accepts our array of `tokens`.
AST parser(std::vector<Token> tokens)
{
    AST ast;
    int current = 0;
    while (current < tokens.size())
    {
        ast.body.push_back(walk(tokens, current));
    }
    return ast;
}

// paren (
// name add
// number 2
// paren (
// name subtract
// number 4
// number 2
// paren )
// paren )
/*
先遍历到左括号，不管它，current++
遍历到add，创建一个node，类型为CallExpression，名字为add
现在开始一直向下遍历，直到遇到右括号
遇到2，递归调用walk，然后把返回值加在node上
遇到左括号，递归，创建substract node,
substract node 在递归中，把 4 ，2 都push进自己的params里
遇到右括号，substract node 退出, add node 退出

*/

void printAST(ASTNode root)
{
    std::cout << root.type << " " << root.name << " " << root.value << std::endl;
    for (auto param : root.params)
    {
        printAST(param);
    }
}

int main()
{
    std::string input = "(add 2 (subtract 4 2))";
    std::vector<Token> tokens = Tokenizer(input);
    AST ast = parser(tokens);
    printAST(ast.body[0]);
    // printTokens(tokens);
    return 0;
}
