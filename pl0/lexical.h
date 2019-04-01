#include "global.h"
#include <stdio.h>


/**
    存储单词序列
**/
typedef struct Token
{
    enum Types type;  //  大的分类，只有5种，每种type包含若干个token
    enum Symbol token;  //对应Symbol里的token
    int line;  //  所在单词行数
    int startLine;  //开始列
    int endLine;    //单词结束列
    int num;    // 存数字
    char id[100];       //字符串
    int curLength;  //当前token 长度
} TokenNode;

struct Token tokens[MAX_WORDS_LEN];

//初始化操作
void init();

//词法分析,解析源码为token序列,存allToken数组
void lexicalParse(char *codePath);

//获取一个token
long getOneToken(FILE *fp, long pos, int stype);

//打印字符序列
void printToken();

//保留字关键字判断
int isKeyword(char *strToken);

//运算符判断
int isOperation(char ch);

//界符判断
int isBoundary(char ch);

//或者下一个token
void getNextToken();
