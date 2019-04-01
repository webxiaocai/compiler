/**
    全局定义
**/
#ifndef _GLOBAL_H
#define _GLOBAL_H

#include <stdio.h>
#include <stdlib.h>

#define PRINT_SOURCE_CODE 1  //输出源码
#define PRINT_TOKEN 1   //输出Token
#define PRINT_AST_GRAMMAR_TREE 1//输出抽象语法树
#define PRINT_P_CODE 1//输出目标代码p-code

#define INIT_LEN 1000
#define MAX_WORDS_LEN 10000  //编译源码单词最大数

/**
    自定义bool
**/
#define bool int
#define false 0
#define true 1
#define IS_DEBUG 0  //打开调试1，关闭0

/**
    单词分类:
**/
enum Types{keyword, identifier, operation, number, boundary};

/**
    语言支持的单词, 分为5类：
    1、保留字(关键字13个):
    "const", "var", "procedure", "beigin", "end", "odd", "if", "then", "call", "while", "do", "read",
    "write"
    2、标识符: 以字母开头的字母数字序列
    3、运算符: "+", "-", "*", "/", "<", "<=", ">", ">=", "#", "=", ":="   // #表示不等于,
    4、无符号整数: 1, 10, 100 等整数
    5、界符: ",", ".", ";", "(", ")"
**/
enum Symbol{
    constsym, varsym, proceduresym, beginsym, endsym, oddsym, ifsym, thensym, callsym, whilesym, dosym,
    readsym, writesym,
    identsym, numbersym,
    plussym, minussym, mulsym, divsym, lesssym, lesseqsym, greatersym, greatereqsym, eqsym, neqsym, assignsym,
    commasym, periodsym, semicosym, lbracketsym, rbracketsym
};


/**
    13个保留关键字
**/
const static char *keywords[] = {
    "const", "var", "procedure", "begin", "end", "odd", "if", "then", "call",
    "while", "do", "read", "write"
};


//一个完整的字符串token, 100表示一个表达式或者字符命名长度不能超过100位
char strToken[100];

//数字token
int numToken;

//tokens数组长度
int tokensLen;

//全局变量，存放当前所扫描单词符号的单词种别
int lookahead;

//当前token下标位置，++用于获取下一token
int curpos;

//表示当前token
enum Symbol curtoken;


//全局display表, 初始化-1， 最大嵌套层
int display[20];


#endif
