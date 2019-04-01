#include <stdio.h>
#include <stdlib.h>
#include "lexical.h"
#include "syntax.h"
#include "symtable.h"
#include "semantic.h"
#include "codegen.h"

/**
    PL0语言编译器，以《编译原理》(清华大学版)和课本附录源码做参考。
**/


/**
    初始化
**/
void init()
{
    int i;

    for(i = 0; i < 20; i++)
    {
        display[i] = -1;
    }

//    token = (TokenNode *)malloc(sizeof(TokenNode) * INIT_LEN);
//    if(!token)
//    {
//        perror("分配空间失败");
//        exit(EXIT_FAILURE);
//    }


    //初始化保留关键字数组

}



int main()
{
    char *sourceCodePath = "E01.PL0";
    //抽象语法树
    ASTTree *tree;
    //符号表
    Symtable *table;
    //四元式
    Icode *icodeArr;

    //目标代码
    Code *codeArr;

    curpos = 0;

    //初始化
    init();

    //词法分析
    if (IS_DEBUG)
    {
        printf("\n\n/*********************** print lexical analysis now! ***********************/\n");
    }
    lexicalParse(sourceCodePath);
    if (IS_DEBUG)
    {
        printToken();
    }

    //语法分析
    tree = synParse();

    if (tree != NULL)
    {
        printf("\n\n/*********************** syn analysis success! ***********************/\n\n");
    }
    else
    {
        printf("syn error !\n");
    }

    if (IS_DEBUG)
    {
        printf("\n\n/*********************** print ast tree now! ***********************/\n\n");
        printTree(tree->root, 0, 0);
    }

    //创建符号表
    if (IS_DEBUG)
    {
        printf("\n\n/*********************** print symtable now! ***********************/\n\n");
    }
    table = createSymtable(tree);
    if (IS_DEBUG)
    {
        printSymTable(table);
    }

    //输出语义分析结果: 四元式
    if (IS_DEBUG)
    {
        printf("\n\n/*********************** print semantic analysis now! ***********************/\n\n");
    }
    //语义分析
    icodeArr = semanticanalysis(tree, table);
    if (IS_DEBUG)
    {

        printf("\n************************ intermediate list ! *********************\n\n");

        printIcode(icodeArr);
    }

    //中间代码优化
    //printf("\n\n/*********************** print optimize code res now! ***********************/\n");
    //生成目标代码
    if (IS_DEBUG)
    {
        printf("\n\n/*********************** wait backpatch code list! ***********************/\n\n");
    }
    codeArr = genCode(icodeArr, table);


    //解释执行
    printf("\n\n/*********************** final code list! ***********************/\n\n");
    if (IS_DEBUG)
    {
        printCodelist(codeArr);
    }
    interpret(codeArr);

    free(table);
    free(codeArr);
    return 0;
}
