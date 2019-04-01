#ifndef _SYMTABLE_H_
#define _SYMTABLE_H_

#include "syntax.h"

/**
    符号表，存

    常量

    变量名

    过程

    符号表的实现方式为：哈希表
**/

/**
    数据类型，目前只支持整型, 字符型
**/
enum DATATYPE
{
    INTEGER, STR,
};


/**
    哈希结点
**/
typedef struct node
{
    char type[32];              //数据类型: constant, variable, function
    enum DATATYPE datatype;     //数据类型子类型
    char name[32];              //名称
    int val;                    //值, 如果是常量，表示常量值，如果是变量，表示变量所在的过程活动记录的偏移地址
    int level;                  //用于表示调用过程层级关系
    int arsize;                 //存过程所需活动记录大小
    int addr;                   //偏移地址
    struct node *parentScope;       //名字父过程作用域, 存放标识符被相应过程包含的符号表地址，用来构建作用域链
    struct node *next;
}HashNode;



/**
    符号表
**/
typedef struct Table
{
    int maxSize; //最大元素个数
    int length;  //哈希表已有元素个数
    int tablesize;  //哈希表数组大小
    struct node **nodelist;  //
}Symtable;



/**
    初始化
**/
Symtable *initSymtable();



/**
    往符号表添加元素
**/
HashNode *addElement(Symtable *table, char *type, enum DATATYPE datatype, HashNode *p, char *ele, int lev, int val, int size);



/**
    删除符号表元素
**/
int remElement();



/**
    查询符号表
    返回结点信息
**/
HashNode *findElement(Symtable *table, char *name, int lev, char *fname);


/**
    计算哈希值
    对每一位字符处理
**/
int genhash(char *str);


//输出符号表
void printSymTable(Symtable *table);


/**
    遍历抽象语法树创建符号表
    返回符号表地址
**/
Symtable *createSymtable(ASTTree *tree);


/**
    遍历抽象语法树
**/
void traverseAstTree(Symtable *table, ASTNode *node, int level, HashNode *parentFun);

#endif
