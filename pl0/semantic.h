#ifndef _SEMANTIC_H
#define _SEMANTIC_H


/**
    语义分析，检查：
    1、变量要在使用前先声明
    2、表达式两端类型一致
**/
#include "syntax.h"
#include "symtable.h"

/**
    四元式类型
**/
enum icodeType {
    IASSIGN, IPLUS, IMINUS, IMUL, IDIV, IEQ, INEQ, IGREATER, ILESS, IGREATEREQ, ILESSEQ, IREAD,
    IWRITE, ICALL, IHALT, IRETURN, ICMP, IJMP, IJPC, IFUNCSTART, IFUNCEND
};

/**
    中间代码，四元式
**/
typedef struct code
{
    enum icodeType itype;
    char op[20];
    char res[20];
    char arg1[20];
    char arg2[20];  //操作数2
}Icode;

int icodeIndex;  //标识代码位置



/**
    语义分析
**/
Icode *semanticanalysis(ASTTree *tree, Symtable *table);


/**
    遍历抽象语法树
**/
void traverseTree(Icode *code, ASTNode *t, Symtable *table, int lev, char *fname);


/**
    生成函数开始代码
**/
void genEnterCode(Icode *code, ASTNode *t, Symtable *table, int lev, char *fname);


/**
    生成函数结束代码
**/
void genOutCode(Icode *code, ASTNode *t, Symtable *table, int lev, char *fname);



/**
    生成赋值语句代码
**/
void genAssignStmtCode(Icode *code, ASTNode *t, Symtable *table);


/**
    生成if语句代码
**/
void genIfStmtCode(Icode *code, ASTNode *t, Symtable *table, int lev);


/**
    生成while语句代码
**/
void genWhileStmtCode(Icode *code, ASTNode *t, Symtable *table, int lev);


/**
    生成read语句代码
**/
void genReadStmtCode(Icode *code, ASTNode *t, Symtable *table);


/**
    生成write语句代码
**/
void genWriteStmtCode(Icode *code, ASTNode *t, Symtable *table);


/**
    生成call语句代码
**/
void genCallStmtCode(Icode *code, ASTNode *t, Symtable *table, int lev, char *fname);



/**
    生成表达式代码, 采用非递归后序遍历expression树
**/
void genExpressionCode(Icode *code, ASTNode *t, Symtable *table, char *res);

/**
    判断叶结点
**/
int isLeaf(ASTNode *t);


/**
    查找操作符
**/
void getOpstr(int type, char findstr[], int *optype);


/**
    类型检查
**/
int typeCheck(ASTNode *t, Symtable *table, int lev, char *f);


/**
    生成中间代码所需临时变量
**/
char *newtemp();


/**
    生成中间代码
**/
int genIcode(Icode *codeArr, enum icodeType type, char *op, char *res, char *arg1, char *arg2);



/**
    输出生成的中间代码
**/
void printIcode(Icode *codeArr);


#endif // _SEMANTIC_H
