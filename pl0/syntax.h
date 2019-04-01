/**
    保留字:
    "var", "const", "procedure", "beigin", "end", "if", "then", "odd",
    "call", "while", "do", "read", "write"
**/
#ifndef _SYNTAX_H_
#define _SYNTAX_H_

#include "global.h"

enum NodeType
{
    PROGRAM, SUBPROGRAM, CONSTANT, VAR, FUNCTION, STMT, WHILE_STMT, IF_STMT, CALL_STMT, READ_STMT, WRITE_STMT, BEGIN_STMT,
    CONDITION, ASSIGN_STMT, ADD, MINUS, MUL, DIV, NUMBER, IDENT, ODD, RE_OPERATOR, EXPRESSION, TERM, FACTOR
}ASTNodeType;



/**
    抽象语法树节点定义
**/
typedef struct Node
{
    enum NodeType type; //抽象语法树节点类型
    enum Symbol symtype;  //token类型
    char name[50];     //抽象语法树节点名称
    int line;  //行号
    int isAssign; //标识结点的data是否被赋值，用于常量，变量定义时直接赋值
    union {
        char element[50];
        int val;
    }data;  //语法树节点数据，如果是整数直接存val，如果是字符串，存element;
    struct ASTNode *sibling;  //兄弟节点
    struct ASTNode *child[4]; //孩子节点
} ASTNode;


/**
    抽象语法树
**/
typedef struct AST
{
    struct Node *root;  //注意这里要么定义为struct Node *root, 要么定义为ASTNode *root
    int length;  //节点数
    int maxSize;  //最大节点数
}ASTTree;

/**
    错误码定义：

    0 - 缺少"."错误
    1 -

**/
enum errorType
{
    MISS_ERROR,
};


/**
    语法分析主程序
**/
ASTTree *synParse();

/**
    程序
**/
ASTTree *program();

/**
    分程序
**/
ASTNode *subProgram(ASTTree *t);


/**
    常量定义
**/
ASTNode *constant(ASTTree *t);


/**
    变量定义
**/
ASTNode *variable(ASTTree *t);

//过程定义
ASTNode *procedure(ASTTree *t);

/**
    语句
**/
ASTNode *stmt(ASTTree *t);


/**
    条件
**/
ASTNode *condition(ASTTree *t);


/**
    表达式
**/
ASTNode *expression(ASTTree *t);

/**
    项
**/
ASTNode *term(ASTTree *t);


/**
    因子
**/
ASTNode *factor(ASTTree *t);


/**
    出错提示
**/
void synParseError(int errNum, int line);


/**
    创建节点
**/
ASTNode *makeTreeNode(int kind, int sym, int line, char *name, int flag);



/**
    打印抽象语法树
**/
void printTree(ASTNode *t, int deep, int childorder);


char *getTypeName(int type);


char *getSymtypeName(int type);

#endif // _SYNTAX_H_
