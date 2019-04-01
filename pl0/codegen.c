/**
    代码生成器, 根据四元式结合符号表生成对应的指令
**/
#include <ctype.h>
#include "symtable.h"
#include "semantic.h"
#include "codegen.h"

/**

    根据类型：

        IASSIGN, IPLUS, IMINUS, IMUL, IDIV, IEQ, INEQ, IGREATER, ILESS, IGREATEREQ, ILESSEQ, IREAD,
        IWRITE, ICALL, IHALT, IRETURN, ICMP, IJMP, IJMPC, IFUNCSTART, IFUNCEND

    生成目标代码
**/

Code *genCode(Icode *icode, Symtable *table)
{
    Code *dcode ;
    int i = 0, j = 0, k = 0, n = 0, lev = 0, iJmpTo = -1;
    char calFunName[32];
    JmpMap *mapAddress;

    mapAddress = (JmpMap *)malloc(sizeof(JmpMap) * 500);

    char *op  = "lit";

    dcode = (Code *)malloc(sizeof(Code) * 500);
    strcpy(calFunName, "main");

    /**
        第一次遍历中间代码，找出跳转的指令放到mapAddress
    **/
    for(i = 0; i < icodeIndex; i++)
    {
        if (icode[i].itype == IJMP || icode[i].itype == IJPC)
        {
            if (icode[i].itype == IJMP)
            {
                iJmpTo = atoi(icode[i].arg1);
            }
            else if(icode[i].itype == IJPC)
            {
                iJmpTo = atoi(icode[i].res);
            }
            mapAddress[j].iJmpFrom = i;
            mapAddress[j].iJmpTo = iJmpTo;
            mapAddress[j].dJmpFrom = -1;
            mapAddress[j].dJmpTo = -1;
            j++;
        }
    }


    //第二次遍历中间代码
    for(i = 0; i < icodeIndex; i++)
    {
        //记录映射关系
        for(n = 0; n < j; n++)
        {
            if (mapAddress[n].iJmpFrom == i)
            {
                mapAddress[n].dJmpFrom = codeindex;
            }
            if (mapAddress[n].iJmpTo == i)
            {
                mapAddress[n].dJmpTo = codeindex;
            }
        }

        switch(icode[i].itype)
        {
            case IFUNCSTART:    //函数开始, 申请内存, 设置调用层级
                genFunstartCode(icode, i, dcode, table, &lev, calFunName);
                break;
            case IFUNCEND:      //函数结束
                addCode(dcode, OPR, 0, 0);  //释放函数
                lev = -1;       //重置层级
                break;
            case IASSIGN:       //赋值
                genAssignCode(icode, i, dcode, table, lev, calFunName);
                break;
            case IPLUS:         // +     四则运算
            case IMINUS:        // -
            case IMUL:          // *
            case IDIV:          // /
            case IEQ:           // =    关系运算
            case INEQ:          // #
            case IGREATER:      // >
            case ILESS:         // <
            case IGREATEREQ:    // >=
            case ILESSEQ:       // <=
                genOperationCode(icode[i].itype, icode, i, dcode, table, lev, calFunName);
                break;
            case IREAD:         //读
                genReadCode(icode, i, dcode, table, lev, calFunName);
                break;
            case IWRITE:        //写
                genWriteCode(icode, i, dcode, table, lev, calFunName);
                break;
            case ICALL:         //函数调用
                genCalCode(icode, i, dcode, table, lev, calFunName);
                break;
//            case IHALT:
//                break;
//            case IRETURN:
//                break;
//            case ICMP:
//                break;
            case IJMP:          //跳转, 要记住待回填的指令位置, 即当前的codeindex
                genJmpCode(icode, i, dcode);
                break;
            case IJPC:         //条件跳转
                genJpcCode(icode, i, dcode);
                break;
            default:
                break;
        }

    }


    //将映射地址写到dcode数组
    for(i = 0; i < j; i++)
    {
        dcode[mapAddress[i].dJmpFrom].a = mapAddress[i].dJmpTo;
    }

    return dcode;
}



/**
    生成函数开始相关代码, 设置符号表的过程标识符addr地址，以便后面call指令使用跳转地址
    calLev: 函数调用层级
**/
void genFunstartCode(Icode *icode, int index, Code *code, Symtable *table, int *curlev, char callFname[])
{
    int size;  //size:需申请的空间大小
    char *fname = "";
    HashNode *hnode = NULL;

    //符号表查
    fname = icode[index].arg1;
    *curlev = atoi(icode[index].arg2);

    //函数开始指令传的参数是fname, 自己查找自己，查找到结果修改callFname 给后面指令用
    hnode = findElement(table, fname, *curlev, fname);

    if (hnode == NULL)
    {
        printf("error!have no function in symtable!fname:%s lev:%d\n", fname, *curlev);
        exit(EXIT_FAILURE);
    }

    size = hnode->arsize;
    hnode->addr = codeindex;   //设置函数开始地址到符号表
    strcpy(callFname, fname);

    addCode(code, INTE, 0, size);
}




/**
    生成赋值, 分情况讨论生成指令

    1.遇到临时变量t*，说明值是在栈顶，如果t1 = t2，不需要生成指令，临时变量值本身保存在栈
    2.遇到变量, lod level addr -> sto level addr | 放到栈顶
    3.遇到常量, lit 0 num  -> sto level addr

**/
void genAssignCode(Icode *icode, int index, Code *code, Symtable *table, int callLev, char *fname)
{
    char *vname = "", *resname = "";
    HashNode *hnode = NULL;

    vname = icode[index].arg1;
    resname = icode[index].res;

    //处理操作数1
    if (!isAllNumber(vname))
    {
        //查找操作数在符号表信息
        hnode = findElement(table, vname, callLev, fname);
        if (hnode != NULL)
        {
            if (strcmp(hnode->type, "constant") == 0)
            {
                addCode(code, LIT, 0, hnode->val);
            }
            else if(strcmp(hnode->type, "variable") == 0)
            {
                addCode(code, LOD, callLev-hnode->level, hnode->addr);
            }
        }
        //hnode == NULL 说明是临时变量，临时变量存在栈顶，不用管
    }
    else
    {
        addCode(code, LIT, 0, atoi(vname));
    }


    //处理结果
    if (!isAllNumber(resname))
    {
        //查找操作数在符号表信息
        hnode = findElement(table, resname, callLev, fname);

        if (hnode != NULL)
        {
            if (strcmp(hnode->type, "constant") == 0)
            {
                perror("constant can not assign value!");
                exit(EXIT_FAILURE);
            }
            else if(strcmp(hnode->type, "variable") == 0)
            {
                addCode(code, STO, callLev-hnode->level, hnode->val);
            }
        }
        //hnode == NULL 说明是临时变量，临时变量存在栈顶，不用管
    }
    else
    {
        perror("res error!");
        exit(EXIT_FAILURE);
    }
}



/**
    生成运算指令

    1.先算操作数
    2.处理结果存放
    3.根据不同运算生成opr指令 -> opr 指令针对栈顶和次栈顶内容作运算，结果存放到栈顶
        +:
            opr 0 2
        -:
            opr 0 3
        *:
            opr 0 4
        /:
            opr 0 5
        =:
            opr 0 8
        #:
            opr 0 9
        >:
            opr 0 10
        >=:
            opr 0 11
        <:
            opr 0 12
        <=:
            opr 0 13

**/
void genOperationCode(enum icodeType type, Icode *icode, int index, Code *code, Symtable *table, int callLev, char *fname)
{
    char *arg1 = "", *arg2 = "", *res = "";

    HashNode *hnode = NULL;

    arg1 = icode[index].arg1;
    arg2 = icode[index].arg2;
    res = icode[index].res;

    if (isAllNumber(arg1))
    {
        addCode(code, LIT, 0, atoi(arg1));
    }
    else
    {
        hnode = findElement(table, arg1, callLev, fname);
        if (hnode != NULL)
        {
            if (strcmp(hnode->type, "constant") == 0)
            {
                addCode(code, LIT, 0, hnode->val);
            }
            else if(strcmp(hnode->type, "variable") == 0)
            {
                addCode(code, LOD, callLev-hnode->level, hnode->val);
            }
        }
        //hnode == NULL 说明是临时变量，临时变量存在栈顶，不用管
    }

    if (isAllNumber(arg2))
    {
        addCode(code, LIT, 0, atoi(arg2));
    }
    else
    {
        hnode = NULL;
        hnode = findElement(table, arg2, callLev, fname);
        if (hnode != NULL)
        {
            if (strcmp(hnode->type, "constant") == 0)
            {
                addCode(code, LIT, 0, hnode->val);
            }
            else if(strcmp(hnode->type, "variable") == 0)
            {
                addCode(code, LOD, callLev-hnode->level, hnode->val);
            }
        }
        //hnode == NULL 说明是临时变量，临时变量存在栈顶，不用管
    }

    switch(type)
    {
        case IPLUS:         //+     四则运算
            addCode(code, OPR, 0, 2);
            break;
        case IMINUS:        //-
            addCode(code, OPR, 0, 3);
            break;
        case IMUL:          //*
            addCode(code, OPR, 0, 4);
            break;
        case IDIV:          ///
            addCode(code, OPR, 0, 5);
            break;
        case IEQ:           // =    关系运算
            addCode(code, OPR, 0, 8);
            break;
        case INEQ:          // #
            addCode(code, OPR, 0, 9);
            break;
        case IGREATER:      // >
            addCode(code, OPR, 0, 10);
            break;
        case IGREATEREQ:    // >=
            addCode(code, OPR, 0, 11);
            break;
        case ILESS:         // <
            addCode(code, OPR, 0, 12);
            break;
        case ILESSEQ:       // <=
            addCode(code, OPR, 0, 13);
            break;
        default:
            break;
    }

    //处理结果
    if (!isAllNumber(res))
    {
        hnode = NULL;
        hnode = findElement(table, res, callLev, fname);
        if (hnode != NULL)
        {
            if (strcmp(hnode->type, "constant") == 0)
            {
                perror("res error! constant can not be change! ");
                exit(EXIT_FAILURE);
            }
            else if(strcmp(hnode->type, "variable") == 0)
            {
                addCode(code, STO, callLev-hnode->level, hnode->val);
            }
        }
        //hnode == NULL 说明是临时变量，临时变量存在栈顶，不用管
    }
    else
    {
        perror("res error! number can not be res! ");
        exit(EXIT_FAILURE);
    }
}



/**
    生成读代码, 对应下面两条指令

    opr 0 16    //从命令行读入输入到栈顶
    sto level addr   //将栈顶数据保存到相应变量里

**/
void genReadCode(Icode *icode, int index, Code *code, Symtable *table, int callLev, char *callFname)
{
    int level;  //size:需申请的空间大小
    char *vname = "";
    HashNode *hnode = NULL;

    //符号表查
    vname = icode[index].arg1;

    hnode = findElement(table, vname, callLev, callFname);

    if (hnode == NULL)
    {
        printf("error!have no vname in symtable!fname:%s lev:%d\n", vname, callLev);
        exit(EXIT_FAILURE);
    }

    //声明变量时作用域级
    level = hnode->level;

    addCode(code, OPR, 0, 16);

    addCode(code, STO, callLev-hnode->level, hnode->val);  //  diff = callLev - hnode->level 表示从调用层向外寻找diff层的过程
}


/**
    生成写指令代码, 对应两条指令

    1.变量：
    lod level addr   //将数据从变量加载到栈顶
    opr 0 14    //栈顶数据打印输出
    opr 0 15    //换行

    2.常量：
    lit 0 num
    opr 0 14
    opr 0 15    //换行

    3.临时变量, 直接将栈顶内容输出
    opr 0 14
    opr 0 15    //换行

**/
void genWriteCode(Icode *icode, int index, Code *code, Symtable *table, int callLev, char *callFname)
{
    char *vname = "";
    int num, optype;
    HashNode *hnode = NULL;

    //参数全是数字，说明是常数
    if(isAllNumber(icode[index].arg1))
    {
        num = atoi(icode[index].arg1);
        optype = LIT;
    }
    else
    {
        vname = icode[index].arg1;
        hnode = findElement(table, vname, callLev, callFname);

        //如果在符号表里找不到，说明是临时变量t
        if (hnode != NULL)
        {
           if (strcmp(hnode->type, "variable") == 0)
            {
                optype = LOD;
            }
            else if (strcmp(hnode->type, "constant") == 0)
            {
                num = hnode->val;
                optype = LIT;
            }
        }
    }


    if (optype == LIT)
    {
        addCode(code, LIT, 0, num);
        addCode(code, OPR, 0, 14);
    }
    else if (optype == LOD)
    {
        addCode(code, LOD, callLev-hnode->level, hnode->val);
        addCode(code, OPR, 0, 14);
    }
    else
    {
        addCode(code, OPR, 0, 14);
    }

    addCode(code, OPR, 0, 15);
}



/**
    生成跳转指令

    jmp 0 a
**/
void genJmpCode(Icode *icode, int index, Code *code)
{
    int jmpPlace;

    //跳转位置
    jmpPlace = atoi(icode[index].arg1);

    addCode(code, JMP, 0, -1);
}



/**
    生成条件跳转指令
    先判断栈顶的结果是真还是假，如果是假，跳到false出口, false出口值存在res
    还要再回填一遍跳转地址
    jmp 0 a

**/
void genJpcCode(Icode *icode, int index, Code *code)
{
    int jmpPlace;

    //跳转位置
    jmpPlace = atoi(icode[index].res);

    addCode(code, JPC, 0, -1);
}



/**
    生成过程调用指令

    cal level addr(函数入口地址)

**/
void genCalCode(Icode *icode, int index, Code *code, Symtable *table, int callLev, char *callFname)
{
    int a, enter;
    char *fname = "";
    HashNode *hnode = NULL;

    //过程名
    fname = icode[index].arg1;
    //相对层级
    //level = atoi(icode[index].arg2);

    hnode = findElement(table, fname, callLev, callFname);

    if (hnode == NULL)
    {
        printf("error2!have no fname in symtable!fname:%s lev:%d\n", fname, callLev);
        exit(EXIT_FAILURE);
    }

    enter = hnode->addr;

    addCode(code, CAL, hnode->level, enter);
}



/**
    判断字符串是否全是数字
**/
int isAllNumber(char str[])
{
    int i = 0;
    int temp;
    while(str[i] != '\0')
    {
        if (!isdigit(str[i]))
        {
            return 0;
        }
        i++;
    }
    return 1;
}



/**
    添加目标指令代码到指令序列
**/
int addCode(Code *code, enum fct f, int l, int a)
{
    char *op  = "";

    switch(f)
    {
        case LIT:
            op = "lit";
            break;
        case LOD:
            op = "lod";
            break;
        case STO:
            op = "sto";
            break;
        case CAL:
            op = "cal";
            break;
        case INTE:
            op = "int";
            break;
        case JMP:
            op = "jmp";
            break;
        case JPC:
            op = "jpc";
            break;
        case OPR:
            op = "opr";
            break;
        default:
            break;
    }

    if (IS_DEBUG)
    {
        printf("add %d: %s %d %d \n", codeindex, op, l, a);
    }


    code[codeindex].f = f;
    code[codeindex].l = l;
    code[codeindex].a = a;

    codeindex++;
    return codeindex;
}



/**
    打印目标代码
**/
void printCodelist(Code *dcode)
{
    int i;
    char *op = "";
    for(i = 0; i < codeindex; i++)
    {
        switch(dcode[i].f)
        {
            case LIT:
                op = "lit";
                break;
            case LOD:
                op = "lod";
                break;
            case STO:
                op = "sto";
                break;
            case CAL:
                op = "cal";
                break;
            case INTE:
                op = "int";
                break;
            case JMP:
                op = "jmp";
                break;
            case JPC:
                op = "jpc";
                break;
            case OPR:
                op = "opr";
                break;
            default:
                break;
        }

        printf("%d: %s %d %d \n", i, op, dcode[i].l, dcode[i].a);
    }
}
