/**
    生成目标代码
**/

#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include "semantic.h"



enum fct{
    LIT, LOD, STO, CAL, INTE, JMP, JPC, OPR
};

/**
    目标代码结构定义遵循pl0例子的定义
**/
typedef struct
{
    enum fct f;
    int l;
    int a;

} Code;


/**
    中间代码跳转地址和目标代码跳转地址映射结构
**/
typedef struct
{
    int iJmpFrom;     //中间代码跳转起始位置
    int iJmpTo;       //中间代码跳转目标地址
    int dJmpFrom;      // 目标代码跳转起始位置
    int dJmpTo;       // 目标代码跳转目标地址
} JmpMap;

int codeindex;  //目标代码下标

Code *genCode(Icode *iCode, Symtable *table);

void genFunstartCode(Icode *icode, int index, Code *code, Symtable *table, int *curlev, char callFname[]);

void genAssignCode(Icode *icode, int index, Code *code, Symtable *table, int callLev, char *fname);

void genOperationCode(enum icodeType type, Icode *icode, int index, Code *code, Symtable *table, int callLev, char *fname);

void genReadCode(Icode *icode, int index, Code *code, Symtable *table, int callLev, char *fname);

void genWriteCode(Icode *icode, int index, Code *code, Symtable *table, int callLev, char *callFname);

void genJmpCode(Icode *icode, int index, Code *code);

void genJpcCode(Icode *icode, int index, Code *code);

void genCalCode(Icode *icode, int index, Code *code, Symtable *table, int callLev, char *callFname);

int isAllNumber(char str[]);

int addCode(Code *code, enum fct f, int l, int a);

void printCodelist(Code *dcode);



#endif // _CODEGEN_H_
