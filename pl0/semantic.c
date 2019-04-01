#include "linkstack.h"
#include "syntax.h"
#include "symtable.h"
#include "semantic.h"

/**
    语义分析：
    1、使用变量，调用函数前要先声明
    2、表达式类型检查
    5、生成四元式
**/
Icode *semanticanalysis(ASTTree *tree, Symtable *table)
{
    Icode *codeArr;
    char *temp;  //生成中间代码的临时变量， 非叶结点生成
    codeArr = (Icode *)malloc(sizeof(Icode) * 10000);

    typeCheck(tree->root, table, 1, "main");

    printf("\n************************ semantic analysis success! *********************\n\n");

    /**
        类型检查完毕后生成中间代码
        生成if,while等布尔表达式使用回填法生成代码
    **/
    traverseTree(codeArr, tree->root, table, 1, "main");

    return codeArr;
}




/**
    遍历树
    声明不需要检查，只要处理语句、过程的嵌套
    只要沿着program的第3,4子树遍历
**/
void traverseTree(Icode *code, ASTNode *t, Symtable *table, int lev, char *fname)
{
    int i, curlev, curinstr;  // curinstr: 当前指令位置
    HashNode *symNode;  //指向符号表结点
    char *curFname;  //当前层函数名

    curlev = lev;
    curFname = fname;

    while(t != NULL)
    {
        switch(t->type)
        {
            case PROGRAM:
                curinstr = icodeIndex;  // 把当前指令位置存起来后面回填地址用
                genIcode(code, IJMP, "JMP", "", "_", "");  //跳到主函数, 跳转主函数的代码一定是第一条
                traverseTree(code, t->child[0], table, lev, fname);
                //生成主函数出口代码
                genOutCode(code, t, table, lev, fname);
                break;
            case FUNCTION:
                //将结点函数名赋值给fname传给下一层递归
                fname = t->name;
                lev++;   //层级+1
                traverseTree(code, t->child[0], table, lev, fname);
                //生成函数出口代码
                genOutCode(code, t, table, lev, fname);

                //同级funtion, 要退回一个lev
                if (t->sibling != NULL)
                {
                    lev--;
                }
                break;
            case SUBPROGRAM:
                for(i = 2; i < 4; i++)
                {
                    //递归遍历program语法树的第3,4子树
                    if (i == 3)
                    {
                        //生成下一层函数代码之后，恢复fname值，用于生成当前层函数代码
                        fname = curFname;
                    }
                    traverseTree(code, t->child[i], table, lev, fname);
                }
                break;

            /**
                以下是处理各种语句生成代码
            **/
            case BEGIN_STMT:
                //生成函数入口
                genEnterCode(code, t, table, lev, fname);
                traverseTree(code, t->child[0], table, lev, fname);
                break;
            case ASSIGN_STMT:
                genAssignStmtCode(code, t, table);
                break;
            case IF_STMT:
                genIfStmtCode(code, t, table, lev);
                break;
            case WHILE_STMT:
                genWhileStmtCode(code, t, table, lev);
                break;
            case READ_STMT:
                genReadStmtCode(code, t, table);
                break;
            case WRITE_STMT:
                genWriteStmtCode(code, t, table);
                break;
            case CALL_STMT:
                genCallStmtCode(code, t, table, lev, fname);
                break;
            default:
                break;
        }

        t = t->sibling;
    }

}



/**
    返回临时变量
**/
char *newtemp()
{
    static int temporder = 0;
    char *t;
    char str[10] = "t";

    t = (char *)malloc(sizeof(char)*10);
    temporder++;
    sprintf(t, "t%d", temporder);

    return t;
}


/**
    生成函数开始代码
    生成1条指令
    1、函数开始 FUNCSTART NULL functionName lev
**/
void genEnterCode(Icode *code, ASTNode *t, Symtable *table, int lev, char *fname)
{
    int num;
    int optype;
    char arg2[10], mainEnter[10];
    char *op = "FUNSTART", *arg1,  *res = "";

    optype = IFUNCSTART;
    memset(mainEnter, '\0', strlen(mainEnter));
    memset(arg2, '\0', strlen(arg2));

    arg1 = fname;
    itoa(lev, arg2, 10);

    //回写跳转main函数的跳转地址
    if (strcmp(fname, "main") == 0)
    {
        itoa(icodeIndex, mainEnter, 10);
        strcpy(code[0].arg1, mainEnter);
    }

    genIcode(code, optype, op, res, arg1, arg2);
}



/**
    生成函数结束代码
    生成1条指令
    1、函数结束 FUNCEND NULL functionName lev
**/
void genOutCode(Icode *code, ASTNode *t, Symtable *table, int lev, char *fname)
{
    char arg2[3];
    char *op = "FUNCEND", *arg1, *res = "";
    int optype = IFUNCEND;

    arg1 = fname;
    itoa(lev, arg2, 10);

    genIcode(code, optype, op, res, arg1, arg2);
}




/**
    生成赋值语句代码
**/
void genAssignStmtCode(Icode *code, ASTNode *t, Symtable *table)
{
    char *op = "ASSIGN";
    int optype = IASSIGN;
    char arg1[10];
    ASTNode *child, *child1;

    child = t->child[0];

    memset(arg1, '\0', 10);
    genExpressionCode(code, t->child[1], table, arg1);

    genIcode(code, optype, op, child->name, arg1, "");
}



/**
    生成if语句代码

**/
void genIfStmtCode(Icode *code, ASTNode *t, Symtable *table, int lev)
{
    int prePos;
    char temp[10], jmpFalsePos[10];  // jmpFalsePos 条件为假时出口地址

    //生成条件代码
    genConditionCode(code, t->child[0], table);

    //记录待回填指令地址
    prePos = icodeIndex - 1;

    //生成条件真后面的语句
    traverseTree(code, t->child[1], table, lev, "");


    //回填跳转地址
    itoa(icodeIndex, jmpFalsePos, 10);
    strcpy(code[prePos].res, jmpFalsePos);
}


/**
    生成while语句代码
**/
void genWhileStmtCode(Icode *code, ASTNode *t, Symtable *table, int lev)
{
    int prePos, jmpEnterPos;
    char temp[10], jmpFalsePos[10];

    memset(temp, '\0', 10);
    memset(jmpFalsePos, '\0', 10);
    //记录while入口位置
    jmpEnterPos = icodeIndex;

    //生成条件代码
    genConditionCode(code, t->child[0], table);

    //记录待回填指令地址
    prePos = icodeIndex - 1;

    //生成条件真后面的语句
    traverseTree(code, t->child[1], table, lev, "");

    //生成跳回入口地址继续判断代码
    itoa(jmpEnterPos, temp, 10);
    genIcode(code, IJMP, "JMP", "", temp, "");

    //回填跳转地址
    itoa(icodeIndex, jmpFalsePos, 10);
    strcpy(code[prePos].res, jmpFalsePos);
}



/**
    生成read语句代码
**/
void genReadStmtCode(Icode *code, ASTNode *t, Symtable *table)
{
    ASTNode *child;
    char *op = "READ";

    child = t->child[0];

    genIcode(code, IREAD, op, "", child->name, "");
}



/**
    生成write语句代码
**/
void genWriteStmtCode(Icode *code, ASTNode *t, Symtable *table)
{
    char *op = "WRITE";
    char arg1[10];

    memset(arg1, '\0', 10);
    genExpressionCode(code, t->child[0], table, arg1);

    genIcode(code, IWRITE, op, "", arg1, "");
}



/**
    生成call语句代码
    call  过程名  revLev(相对层级)
**/
void genCallStmtCode(Icode *code, ASTNode *t, Symtable *table, int lev, char *cfname)
{
    HashNode *symNode;
    ASTNode *child0;
    char *op = "CALL";
    char arg1[10], arg2[10];
    int reLev;   //过程相对层级 = 当前调用层级 - 符号表定义层级

//    if (strcpy(child0->name, "q") == 0)
//    {
//        printf("qqq                %s ",  child0->name);
//    }
    memset(arg1, '\0', 10);
    memset(arg2, '\0', 10);
    child0 = t->child[0];
    symNode =  findElement(table, child0->name, lev, cfname);
    if (symNode == NULL)
    {
        printf("have no procdure!%s, %d\n", child0->name, t->line);
        exit(EXIT_FAILURE);
    }

    //reLev = lev - symNode->level;

    strcpy(arg1, child0->name);
    itoa(lev, arg2, 10);

    genIcode(code, ICALL, op, "", arg1, arg2);
}



/**
    生成条件代码
**/
void genConditionCode(Icode *code, ASTNode *t, Symtable *table)
{
    char op[10], arg1[10] = "", arg2[10] = "", *res, *res2;
    ASTNode *child1;
    int symtype, optype;

    memset(op, '\0', 10);

    child1 = t->child[1];

    genExpressionCode(code, t->child[0], table, arg1);
    genExpressionCode(code, t->child[2], table, arg2);

    symtype = child1->symtype;

    switch(symtype)
    {
        case eqsym:
            strcpy(op, "EQ");
            optype = IEQ;
            break;
        case neqsym:
            strcpy(op, "NEQ");
            optype = INEQ;
            break;
        case greatersym:
            strcpy(op, "GREATER");
            optype = IGREATER;
            break;
        case greatereqsym:
            strcpy(op, "GREATEREQ");
            optype = IGREATEREQ;
            break;
        case lesssym:
            strcpy(op, "LESS");
            optype = ILESS;
            break;
        case lesseqsym:
            strcpy(op, "LESSEQ");
            optype = ILESSEQ;
            break;
        default:
            break;
    }

    res = newtemp();
    genIcode(code, optype, op, res, arg1, arg2);
    //条件跳转指令, 跳转假出口，待后面回填
    strcpy(op, "JPC");
    optype = IJPC;
    genIcode(code, optype, op, "_", res, "");
}



/**
    生成表达式代码, 采用非递归后序遍历expression树
**/
void genExpressionCode(Icode *code, ASTNode *t, Symtable *table, char *res)
{
    int i = 0;
	LinkStack *s;
	ASTNode *cur, *pre, *left, *right, *child0, *child1;
	ElementType item;
	char *caltemp[100], op[10], arg1[10], arg2[10];  //存储临时计算结果
    char *temp;  //临时变量， temp = newtemp()
    int optype;

    memset(caltemp, '\0', 100);
    memset(op, '\0', 10);
    memset(arg1, '\0', 10);
    memset(arg2, '\0', 10);
	s = InitStack();

    //根结点入栈
    Push(s, t->child[1]);  // expression

     right = t->child[1];

    while(!IsEmpty(s))
    {
        cur = Top(s);

        /**
            cur结点是叶子结点，说明树只有一个结点，就不用继续下面的计算
        **/
        if (isLeaf(cur))
        {
            Pop(s, &item);
            if (item->type == NUMBER)
            {
                itoa(item->data.val, arg1, 10);  //转化为字符串
                strcpy(res, arg1);
            }
            else
            {
                temp = item->name;
            }

            break;
        }

        /**
            中间嵌套有expression结点, 单独处理
        **/
        if (cur->type == EXPRESSION)
        {
            if (isLeaf(cur->child[1]) || (pre != NULL && (cur->child[0] == pre || cur->child[1] == pre)))
            {
                Pop(s, &item);
                pre = cur;

                strcpy(op, "ASSIGN");

                if (isLeaf(cur->child[1]))
                {
                    //叶子结点
                    temp = newtemp();
                    if (child1->type == NUMBER)
                    {
                        itoa(child1->data.val, arg1, 10);  //转化为字符串
                    }
                    else
                    {
                        strcpy(arg1, child1->name);
                    }

                    genIcode(code, IASSIGN, op, temp, arg1, "");
                    //将计算结果存放到临时数组
                    caltemp[i] = temp;
                    i++;
                }
                else
                {
                    if (i >= 0)
                    {
                        i--;
                        strcpy(arg1, caltemp[i]);

                        temp = newtemp();
                        genIcode(code, IASSIGN, op, temp, arg1, "");
                        caltemp[i] = temp;
                        i++;
                    }
                }
            }
            else
            {
                Push(s, cur->child[1]);
            }
            continue;
        }



        /**
            遇到操作符非叶子结点，判断其孩子结点情况
        **/
        if ((isLeaf(cur->child[0]) && isLeaf(cur->child[1])) || (pre != NULL && (cur->child[0] == pre || cur->child[1] == pre)))
        {
            Pop(s, &item);
            pre = cur;

            /**
                执行生成代码， 遇到左右孩结点都是叶结点，计算，把结果存临时数组
            **/
            strcpy(op, "");
            getOpstr(item->type, op, &optype);
            child0 = item->child[0];
            child1 = item->child[1];

            //左右孩子结点均为叶子结点
            if (isLeaf(child0) && isLeaf(child1))
            {
                temp = newtemp();
                if (child0->type == NUMBER)
                {
                    itoa(child0->data.val, arg1, 10);  //转化为字符串
                }
                else
                {
                    strcpy(arg1, child0->name);
                }

                if (child1->type == NUMBER)
                {
                    itoa(child1->data.val, arg2, 10);  //转化为字符串
                }
                else
                {
                    strcpy(arg2, child1->name);
                }

                genIcode(code, optype, op, temp, arg1, arg2);

                //将计算结果存放到临时数组
                caltemp[i] = temp;
                i++;
            }
            else if (!isLeaf(child0) || !isLeaf(child1))
            {
                //左孩或者右孩结点有一个是非叶子结点
                //临时计算结果已用完
                if (i == 0)
                {
                    break;
                }

                //左孩是叶结点
                if (isLeaf(child0))
                {
                    if (child0->type == NUMBER)
                    {
                        itoa(child0->data.val, arg1, 10);  //转化为字符串
                    }
                    else
                    {
                        strcpy(arg1, child0->name);
                    }
                }
                else
                {
                    i--;
                    strcpy(arg1, caltemp[i]);
                }

                //右孩是叶结点
                if (isLeaf(child1))
                {
                    if (child1->type == NUMBER)
                    {
                        itoa(child1->data.val, arg2, 10);  //转化为字符串
                    }
                    else
                    {
                        strcpy(arg2, child1->name);
                    }
                }
                else
                {
                    i--;
                    strcpy(arg2, caltemp[i]);
                }

                temp = newtemp();
                genIcode(code, optype, op, temp, arg1, arg2);

                //如果是expression结点，执行前面的代码
//                if (!isLeaf(item) && item != t)
//                {
                    caltemp[i] = temp;
                    i++;
//                }
            }
            else
            {
                //左右孩子结点均为非叶结点，计算结果从临时数组取
                if (i >= 0)
                {
                    i--;
                    strcpy(arg1, caltemp[i]);
                }
                if (i >= 0)
                {
                    i--;
                    strcpy(arg2, caltemp[i]);
                }

                temp = newtemp();
                genIcode(code, optype, op, temp, arg1, arg2);
            }
        }
        else
        {
            /**
                注意：入栈的都是非叶结点，也就是入栈操作符结点
                先将右孩非叶结点入栈，再入栈左孩结点
            **/
            if (cur->child[1] && !isLeaf(cur->child[1]))
            {
                Push(s, cur->child[1]);
            }
            if (cur->child[0] && !isLeaf(cur->child[0]))
            {
                Push(s, cur->child[0]);
            }
        }
    }

    //express 结点 等于temp
    if (strcmp(res, "") == 0)
    {
        strcpy(res, temp);
    }
}



/**
    判断是否是叶子结点
**/
int isLeaf(ASTNode *t)
{
    if (t->child[0] == NULL && t->child[1] == NULL)
    {
        return 1;
    }

    return 0;
}



/**
    类型检查

**/
void typeError(int errorno)
{
    printf("type error! error number:%d\n", errorno);
    exit(0);
}



/**
    类型检查
    返回当前结点根据子结点计算得到的类型
**/
int typeCheck(ASTNode *t, Symtable *table, int lev, char *funName)
{
    int i, curlev;
    enum DATATYPE checktype;
    ASTNode *child1;
    HashNode *symNode;

    curlev = lev;
    while(t != NULL)
    {
        switch(t->type)
        {
            case NUMBER:
                checktype = INTEGER;
                break;
            case IDENT:

                //检查是否在符号表声明
                symNode =  findElement(table, t->name, lev, funName);
                if (symNode == NULL)
                {
                    printf("Ident: %s exception, error line:%d,Declaration must before use!", t->name, t->line);
                    typeError(11);
                }

                //返回标识符在符号表里的数据类型
                checktype = symNode->datatype;
                break;
            case ASSIGN_STMT:
                if ((t->child[1] != NULL))
                {
                    if (typeCheck(t->child[0], table, curlev, funName) != INTEGER || typeCheck(t->child[1], table, curlev, funName) != INTEGER)
                    {
                        typeError(1);  //目前只支持整型运算
                    }

                    checktype = INTEGER;  //检查通过，说明ASSIGN_STMT结点计算结果是整型
                }
                break;
            case IF_STMT:
                checktype = typeCheck(t->child[0], table, curlev, funName);
                if (checktype != INTEGER)
                {
                    typeError(2);  //由于没有布尔型，用整型判断条件
                }
                break;
                /**
                    considtion树
                            condition
                           /     |    \
                   expression  re_op  expression

                **/
            case CONDITION:

                if (typeCheck(t->child[0], table, curlev, funName) != INTEGER || typeCheck(t->child[2], table, curlev, funName) != INTEGER)
                {
                    typeError(3);  //由于没有布尔型，用整型判断条件
                }

                //运算符必须是STR
                if (typeCheck(t->child[1], table, curlev, funName) != STR)
                {
                    typeError(4);
                }
                checktype = INTEGER;
                break;
            case READ_STMT:
                checktype = typeCheck(t->child[0], table, curlev, funName);
                if (checktype != INTEGER)
                {
                    typeError(5);  //读入整型
                }
                break;
            case WRITE_STMT:
                checktype =  typeCheck(t->child[0], table, curlev, funName);
                if (checktype != INTEGER)
                {
                    typeError(6);  //读入整型
                }
                break;
            case CALL_STMT:
                child1 = t->child[0];
                checktype = typeCheck(t->child[0], table, curlev, funName);
                if (checktype  != STR)
                {
                    typeError(7);  //调用参数必须是字符
                }
                break;
            case RE_OPERATOR:   //关系运算符
                checktype = STR;
                break;
            case ADD:
            case MINUS:
            case MUL:
            case DIV:  //四则运算
                //暂时注释
                if (typeCheck(t->child[0], table, lev, funName) != INTEGER || typeCheck(t->child[1], table, lev, funName) != INTEGER)
                {
                    typeError(8);
                }

                if (t->type == DIV)
                {
                    //除数不能为0, 这里有一个bug, 暂时不知道为什么，有的地方没有检测到除数是否为0
                    child1 = t->child[1];
                    if (child1->data.val == 0)
                    {
                        printf("error:div zero! %s %d \n", child1->name, child1->data.val);
                        typeError(10);
                    }
                }
                checktype = INTEGER;
                break;
            case EXPRESSION:
                checktype = typeCheck(t->child[1], table, lev, funName);
                if (checktype != INTEGER)
                {

                    typeError(9);
                }
                break;
            case FUNCTION:
                //lev 表示函数嵌套层次
                lev++;
                funName = t->name ;
                typeCheck(t->child[0], table, lev, funName);
                checktype = STR;

                //同级funtion, 要退回一个lev
                if (t->sibling != NULL)
                {
                    lev--;
                }
                break;
            case SUBPROGRAM:

                for(i = 2; i < 4; i++)
                {
                    //递归遍历
                    typeCheck(t->child[i], table, lev, funName);
                }

                checktype = STR;
                break;
            default:
                //递归遍历
                checktype = typeCheck(t->child[0], table, lev, funName);
                break;
        }

        t = t->sibling;
    }

    return checktype;
}




/**
    生成中间代码
**/
int genIcode(Icode *codeArr, enum icodeType type, char *op, char *res, char *arg1, char *arg2)
{
    if (IS_DEBUG)
    {
        printf("icode add:%d, res:%s, op:%s, arg1:%s, arg2:%s \n", icodeIndex, res, op, arg1, arg2);
    }

    strcpy(codeArr[icodeIndex].op, op);
    strcpy(codeArr[icodeIndex].res, res);
    strcpy(codeArr[icodeIndex].arg1, arg1);
    strcpy(codeArr[icodeIndex].arg2, arg2);
    codeArr[icodeIndex].itype = type;
    icodeIndex++;

    return icodeIndex;
}



/**
    输出生成的中间代码
**/
void printIcode(Icode *codeArr)
{
    int i;

    for(i = 0; i < icodeIndex; i++)
    {
        printf("code:%d, res:%s, op:%s, optype:%d, arg1:%s, arg2:%s \n",
               i, codeArr[i].res, codeArr[i].op, codeArr[i].itype, codeArr[i].arg1, codeArr[i].arg2);
    }
}



/**
    查找操作符
    "ASSIGN", "PLUS", "MINUS", "MUL", "DIV", "EQ", "NEQ", "GREATER", "LESS", "LESS", "GREATEREQ", "LESSEQ", "READ",
    "WRITE", "CALL", "HALT", "RETURN", "CMP", "JMP", "JMPC"
**/
void getOpstr(int type, char findstr[], int *optype)
{
    int i = 0, pos = -1;

    switch(type)
    {
        case ADD:
            strcpy(findstr, "PLUS");
            *optype = IPLUS;
            break;
        case MINUS:
            strcpy(findstr, "MINUS");
            *optype = IMINUS;
            break;
        case MUL:
            strcpy(findstr, "MUL");
            *optype = IMUL;
            break;
        case DIV:
            strcpy(findstr, "DIV");
            *optype = IDIV;
            break;
        default:
            break;
    }

//    for(i = 0; i < strlen(instructions); i++)
//    {
//        if (strcmp(findstr, instructions[i]) == 0)
//        {
//            break;
//        }
//    }

    if (strcmp(findstr, "") == 0)
    {
        perror("operator error!");
        exit(EXIT_FAILURE);
    }

}
