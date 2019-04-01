/**
    语法分析
**/
#include "lexical.h"
#include "syntax.h"


/**
    递归下降法，按照给出的EBNF描述写程序。用过程表示非终结符，遇到终结符即规约。
    对tokens数组遍历，根据token做语法分析，在匹配终结符时创建抽象语法树节点
**/
ASTTree *synParse()
{
    ASTTree *tree;
    int i, len;

    tree = program();

    return tree;
}

/**
    程序
**/
ASTTree *program()
{
    ASTTree *t;
    ASTNode *root;
    int nodeType = -1;
    char *name = "program";

    //抽象语法树根结点
    t = (ASTTree *)malloc(sizeof(ASTTree));
    t->length = 0;
    t->maxSize = 100000;
    t->root = NULL;
    t->root = makeTreeNode(PROGRAM, -1, -1, name, 0);
    if ((++t->length) > t->maxSize)
    {
        printf("max length!\n");
        exit(0);
    }
    t->root->child[0] = subProgram(t);

    return t;
}




/**
    分程序 ::=  [const<常量定义>{,<常量定义>};]
                [var<变量定义>{,<变量定义>};]
                [{procedure<id>;<分程序>;}]
                <语句>

    抽象语法树
                    subprogram
            child[0] child[1]   child[2] child[3]
            分别对应上面的非终结符
**/
ASTNode *subProgram(ASTTree *tree)
{
    //临时变量
    ASTNode *p, *node, *q;
    char *nodename = "subprogram";

    //创建语法树结点
    node = makeTreeNode(SUBPROGRAM, -1, -1, nodename, 0);
    tree->length++;

    curtoken = tokens[curpos].token;

    if (curtoken == constsym)
    {
        //常量定义
        getNextToken();
        p = node->child[0] = constant(tree);

        //可能定义多个常量，以','分隔
        while(curtoken == commasym)
        {
            getNextToken();
            p->sibling = constant(tree);
            p = p->sibling;
        }

        if (curtoken == semicosym)
        {
            //常量声明结束
            getNextToken();
        }
        else
        {
            synParseError(1, node->line);
        }
    }
    if (curtoken == varsym)
    {
        getNextToken();
        p = node->child[1] = variable(tree);

        //循环判断
        while(curtoken == commasym)
        {
            getNextToken();
            p->sibling = variable(tree);
            p = p->sibling;
        }

        // ';' 号
        if (curtoken == semicosym)
        {
            //变量声明结束
            getNextToken();
        }
        else
        {
            synParseError(111, node->line);
        }
    }
    if (curtoken == proceduresym)
    {
        node->child[2] = procedure(tree);
    }

    //语句
    node->child[3] = stmt(tree);

    return node;
}


/**
    常量定义 ::= <标识符>=<integer>
**/
ASTNode *constant(ASTTree *tree)
{
    ASTNode *node, *p, *q, *r;
    TokenNode tnode;
    node = p = q = NULL;

    if (curtoken == identsym)
    {
        tnode = tokens[curpos];
        //常量结点
        printf("sss: %s \n", tnode.id);
        node = makeTreeNode(CONSTANT, tnode.token, tnode.line, tnode.id, 0);
        tree->length ++;

        getNextToken();
    }
    else
    {
        synParseError(2, node->line);
    }

    if (curtoken == eqsym)
    {
        getNextToken();
    }
    else
    {
        synParseError(3, node->line);
    }

    if (curtoken == numbersym)
    {
        //数字, 修改树结点data的val值, 常数在定义时已赋值
        tnode = tokens[curpos];
        node->isAssign = 1;
        node->data.val = tnode.num;

        getNextToken();
    }
    else
    {
        synParseError(4, node->line);
    }

    return node;
}


/**
    变量定义 ::= <id>
**/
ASTNode *variable(ASTTree *tree)
{
    ASTNode *node;
    TokenNode tnode;
    node = NULL;

    if (curtoken == identsym)
    {
        //抽象语法树结点-变量类型
        tnode = tokens[curpos];
        //常量名
        node = makeTreeNode(VAR, tnode.token, tnode.line, tnode.id, 0);
        tree->length ++;

        //变量定义检查匹配
        getNextToken();
    }
    else
    {
        synParseError(5, node->line);
    }

    return node;
}



/**
    过程 := {procedure <id>;<分程序>;}

**/
ASTNode *procedure(ASTTree *tree)
{
    ASTNode *node, *curnode, *prenode;  //prenode指向上一个过程结点
    TokenNode tnode;
    node = curnode = prenode = NULL;

    //多个过程嵌套
    while(curtoken == proceduresym)
    {
        //函数过程类型结点, 开始结点没有过程名，识别到identsym才赋值
        tnode = tokens[curpos];
        curnode = makeTreeNode(FUNCTION, tnode.token, tnode.line, "", 0);
        tree->length ++;

        getNextToken();
        //识别出函数名称, procedure之后必须是函数名
        if (curtoken == identsym)
        {
            //函数名 赋值到抽象语法树结点上
            tnode = tokens[curpos];
            strcpy(curnode->name, tnode.id);

            getNextToken();
        }
        else
        {
            synParseError(6, node->line);
        }

        //分号
        if (curtoken == semicosym)
        {
            getNextToken();
        }
        else
        {
            synParseError(7, node->line);
        }

        //递归调用分程序
        curnode->child[0] = subProgram(tree);

        //分号
        if (curtoken == semicosym)
        {
            getNextToken();
        }
        else
        {
            synParseError(8, node->line);
        }

        //继续获取下一token
        //getNextToken();

        if (node == NULL)
        {
            //处理第一个结点
            node = curnode; //node 指向第一个过程
            prenode = node;
        }
        else
        {
            prenode->sibling = curnode;
            prenode = prenode->sibling;
        }
    }

    return node;
}



/**
    语句:
    ::=
      <id>:=<表达式>
    | if <条件> then <语句>
    | while <条件> do <语句>
    | call <id>
    | read '('<id>{,<id>}')'
    | write '('<表达式>{,<表达式>}')'
    | begin <语句>{;<语句>}end
    | ε

    ε 不翻译

**/
ASTNode *stmt(ASTTree *tree)
{
    ASTNode *node, *left, *temp, *curnode;
    TokenNode tnode;
    node = curnode = temp = NULL;

    if (curtoken == identsym)
    {
        //赋值语句左边
        tnode = tokens[curpos];
        left = makeTreeNode(IDENT, tnode.token, tnode.line, tnode.id, 0);
        tree->length ++;

        getNextToken();
        if (curtoken == assignsym)
        {
            //赋值符号
            tnode = tokens[curpos];
            node = makeTreeNode(ASSIGN_STMT, tnode.token, tnode.line, tnode.id, 0);
            tree->length ++;

            getNextToken();
            node->child[1] = expression(tree);
        }

        node->child[0] = left;
    }
    else if (curtoken == ifsym)
    {
        //条件语句
        tnode = tokens[curpos];
        node = makeTreeNode(IF_STMT, tnode.token, tnode.line, tnode.id, 0);
        tree->length ++;

        getNextToken();
        node->child[0] = condition(tree);

        if (curtoken == thensym)
        {
            getNextToken();
            node->child[1] = stmt(tree);
        }
        else
        {
            synParseError(9, node->line);
        }
    }
    else if(curtoken == whilesym)
    {
        //while语句
        tnode = tokens[curpos];
        node = makeTreeNode(WHILE_STMT, tnode.token, tnode.line, tnode.id, 0);
        tree->length ++;

        getNextToken();
        //条件
        node->child[0] = condition(tree);

        if (curtoken == dosym)
        {
            getNextToken();
            node->child[1] = stmt(tree);
        }
        else
        {
            synParseError(10, node->line);
        }
    }
    else if (curtoken == callsym)
    {
        //call语句
        tnode = tokens[curpos];
        node = makeTreeNode(CALL_STMT, tnode.token, tnode.line, tnode.id, 0);
        tree->length ++;

        getNextToken();
        if (curtoken == identsym)
        {
            //call 后需跟 标识符
            tnode = tokens[curpos];
            node->child[0] = makeTreeNode(IDENT, tnode.token, tnode.line, tnode.id, 0);
            tree->length ++;

            getNextToken();
        }
        else
        {
            synParseError(11, node->line);
        }
    }
    else if (curtoken == readsym)
    {
        //read '('<id>{,<id>}')'
        //read语句
        tnode = tokens[curpos];
        node = makeTreeNode(READ_STMT, tnode.token, tnode.line, tnode.id, 0);
        tree->length ++;

        getNextToken();
        if (curtoken == lbracketsym)
        {
            getNextToken();
            if (curtoken == identsym)
            {
                //标识符
                tnode = tokens[curpos];
                node->child[0] = curnode = makeTreeNode(IDENT, tnode.token, tnode.line, tnode.id, 0);
                tree->length ++;

                getNextToken();
                while(curtoken == commasym)
                {
                    getNextToken();
                    if (curtoken == identsym)
                    {
                        //逗号之后是标识符
                        tnode = tokens[curpos];
                        curnode->sibling = makeTreeNode(IDENT, tnode.token, tnode.line, tnode.id, 0);
                        tree->length ++;
                        curnode = curnode->sibling;

                        getNextToken();
                    }
                    else
                    {
                        synParseError(12, node->line);
                    }
                }

                //取下一token继续判断右括号
                if (curtoken == rbracketsym)
                {
                    //右括号通过判断
                    getNextToken();
                }
                else
                {
                    //缺少右括号
                    synParseError(13, node->line);
                }
            }
        }
        else
        {
            synParseError(14, node->line);
        }
    }
    else if (curtoken == writesym)
    {
        //write '('<表达式>{,<表达式>}')'
        //写语句
        tnode = tokens[curpos];
        node = makeTreeNode(WRITE_STMT, tnode.token, tnode.line, tnode.id, 0);
        tree->length ++;

        getNextToken();
        if (curtoken == lbracketsym)
        {
            getNextToken();

            tnode = tokens[curpos];
            node->child[0] = curnode = expression(tree);

            while(curtoken == commasym)
            {
                getNextToken();
                curnode->sibling = expression(tree);
                curnode = curnode->sibling;
            }

            if (curtoken == rbracketsym)
            {
                //右括号通过判断
                getNextToken();
            }
            else
            {
                synParseError(15, node->line);
            }
        }
        else
        {
            synParseError(16, node->line);
        }
    }
    else if (curtoken == beginsym)
    {
        tnode = tokens[curpos];
        node = makeTreeNode(BEGIN_STMT, tnode.token, tnode.line, tnode.id, 0);
        tree->length ++;

        getNextToken();
        //begin <语句>{;<语句>}end   递归调用
        node->child[0] = curnode = stmt(tree);

        //';' 之后还可能有其他语句，一直到end才结束
        while(curtoken == semicosym)
        {
            getNextToken();
            curnode->sibling = stmt(tree);
            curnode = curnode->sibling;
        }

        if (curtoken == endsym)
        {
            //end 通过检测
            getNextToken();
        }
        else
        {
            synParseError(17, node->line);
        }
    }
    else
    {
        tnode = tokens[curpos];
        left = makeTreeNode(STMT, tnode.token, tnode.line, tnode.id, 0);
        tree->length ++;
    }

    return node;
}


/**
    条件  ::= <表达式>(= | # | < | <= | > | >=)<表达式> | odd<表达式>
**/
ASTNode *condition(ASTTree *tree)
{
    ASTNode *node, *left, *temp, *curnode;
    TokenNode tnode;

    tnode = tokens[curpos];
    node = makeTreeNode(CONDITION, tnode.token, tnode.line, tnode.id, 0);
    tree->length ++;

    if (curtoken == oddsym)
    {
        tnode = tokens[curpos];
        node->child[0] = makeTreeNode(ODD, tnode.token, tnode.line, tnode.id, 0);
        tree->length ++;
        //通过
        getNextToken();
        node->child[1] = expression(tree);
    }
    else
    {
        node->child[0] = expression(tree);

        //关系运算符: RE_OPERATOR
        if (
            curtoken == eqsym || curtoken == neqsym || curtoken == lesssym
            || curtoken == lesseqsym || curtoken == greatersym || curtoken == greatereqsym
            )
        {
            tnode = tokens[curpos];
            node->child[1] = makeTreeNode(RE_OPERATOR, tnode.token, tnode.line, tnode.id, 0);
            tree->length ++;
            getNextToken();
        }
        else
        {
            synParseError(18, node->line);
        }

        node->child[2] = expression(tree);
    }

    return node;
}


/**
    表达式 ::= [ + | - ]<项>{(+ | -)<项>}
**/
ASTNode *expression(ASTTree *tree)
{
    ASTNode *node, *firstChild, *nextChild, *temproot, *opnode, *temp;  // temproot 临时根结点
    TokenNode tnode;

    firstChild = nextChild = temproot = opnode = temp = NULL;
    tnode = tokens[curpos];
    node = makeTreeNode(EXPRESSION, tnode.token, tnode.line, "expression", 0);
    tree->length ++;

    if (curtoken == plussym)
    {
        tnode = tokens[curpos];
        node->child[0] = makeTreeNode(ADD, tnode.token, tnode.line, tnode.id, 0);
        tree->length ++;

        getNextToken();
    }
    else if (curtoken == minussym)
    {
        tnode = tokens[curpos];
        node->child[0] = makeTreeNode(MINUS, tnode.token, tnode.line, tnode.id, 0);
        tree->length ++;

        getNextToken();
    }

    //左边部分
    temp = firstChild = term(tree);

    while(curtoken == plussym || curtoken == minussym)
    {
        if (curtoken == plussym)
        {
            tnode = tokens[curpos];
            opnode = makeTreeNode(ADD, tnode.token, tnode.line, tnode.id, 0);
            tree->length ++;

        }
        else if (curtoken == minussym)
        {
            tnode = tokens[curpos];
            opnode = makeTreeNode(MINUS, tnode.token, tnode.line, tnode.id, 0);
            tree->length ++;
        }

        //记录刚进循环的根结点
        if (temp == firstChild)
        {
            temp = temproot = opnode;
            opnode->child[0] = firstChild;  //child[0]  结点赋值
        }
        else
        {
            opnode->child[0] = nextChild;
            temproot->child[1] = opnode;
            temproot = temproot->child[1];
        }

        getNextToken();

        //构建树结点关键在这里， 说明后面还有 +-的项没有处理完
        nextChild = term(tree);

        //如果当前token不是+ / - , 说明表达式已经分析完毕, 将next赋值到操作符结点的子树child[1]
        if (curtoken != plussym && curtoken != minussym)
        {
            opnode->child[1] = nextChild;
        }
    }

    node->child[1] = temp;

    return node;
}


/**
    项 ::= <因子> {(* | /) <因子>}
**/
ASTNode *term(ASTTree *tree)
{
    ASTNode *node, *firstChild, *nextChild, *temproot, *opnode, *temp;
    TokenNode tnode;

//    tnode = tokens[curpos];
//    node = makeTreeNode(TERM, tnode.token, tnode.line, "term", 0);
//    tree->length ++;

    temp = firstChild = factor(tree);

    /**
            树的结构:
                term
                /
               temproot(MUL, ADD)
               /  \
          factor   factor

    **/
    while (curtoken == mulsym || curtoken == divsym)
    {
        if (curtoken == mulsym)
        {
            tnode = tokens[curpos];
            opnode = makeTreeNode(MUL, tnode.token, tnode.line, tnode.id, 0);
            tree->length ++;
        }

        if (curtoken == divsym)
        {
            tnode = tokens[curpos];
            opnode = makeTreeNode(DIV, tnode.token, tnode.line, tnode.id, 0);
            tree->length ++;
        }


        //记录刚进循环的根结点
        if (temp == firstChild)
        {
            temp = temproot = opnode;
            opnode->child[0] = firstChild;  //child[0]  结点赋值
        }
        else
        {
            opnode->child[0] = nextChild;
            temproot->child[1] = opnode;
            temproot = temproot->child[1];
        }

        getNextToken();

        //构建树结点关键在这里， 说明后面还有 +-的项没有处理完
        nextChild = factor(tree);

        //如果当前token不是*|/ , 说明因子已经分析完毕, 将next赋值到操作符结点的子树child[1]
        if (curtoken != mulsym && curtoken != divsym)
        {
            opnode->child[1] = nextChild;
        }

    }

    node = temp;
    return node;
}


/**
    因子 ::= <id> | <integer> | '('<表达式>')'
**/
ASTNode *factor(ASTTree *tree)
{
    ASTNode *node, *left, *temp, *curnode;
    TokenNode tnode;

//    tnode = tokens[curpos];
//    node = makeTreeNode(FACTOR, tnode.token, tnode.line, "factor", 0);
//    tree->length ++;

    if (curtoken == identsym)
    {
        //标识符
        tnode = tokens[curpos];
        node = makeTreeNode(IDENT, tnode.token, tnode.line, tnode.id, 0);
        tree->length ++;

        //通过
        getNextToken();
    }
    else if (curtoken == numbersym)
    {
        //数字
        tnode = tokens[curpos];
        temp = makeTreeNode(NUMBER, tnode.token, tnode.line, tnode.id, 1);
        temp->data.val = tnode.num;
        tree->length ++;
        node = temp;
        //通过
        getNextToken();
    }
    else if(curtoken == lbracketsym)
    {
        //因子是表达式
        getNextToken();
        node = expression(tree);

        if (curtoken == rbracketsym)
        {
            //通过
            getNextToken();
        }
        else
        {
            synParseError(21, node->line);
        }
    }
    else
    {
        synParseError(20, node->line);
    }

    return node;
}



/**
    创建抽象语法树结点
**/
ASTNode *makeTreeNode(int kind, int sym, int line, char *name, int flag)
{
    ASTNode *node;
    int i;
    node = (ASTNode *)malloc(sizeof(ASTNode));

    for(i = 0; i < 4; i++)
    {
        node->child[i] = NULL;
    }
    node->line = line;
    node->type = kind;
    node->symtype = sym;
    node->sibling = NULL;
    node->isAssign = flag;
    strcpy(node->name, name);
    strcpy(node->data.element, "");

    return node;
}



/**
    出错提示
**/
void synParseError(int errNum, int line)
{
    switch(errNum)
    {
        case 1:
            printf("error ! %d ,line:%d \n", errNum, line);
            break;
        default:
            printf("unknown error, errorno: %d,line:%d \n", errNum, line);
            break;
    }

    exit(0);
}



/**
    打印抽象语法树

**/
void printTree(ASTNode *t, int deep, int childorder)
{
    ASTNode *p, *childnode, *q;
    int i, j = 0;
    char *tname, *symname;

    if (t != NULL)
    {
//        tname = getTypeName(t->type);
//
//        printf("deep: %d, child-order: %d\n", deep, childorder);
//        printf("node: type:%d,tname:%s -> element: %s -> line:%d \n", t->type, tname, t->data.element, t->line);
//        free(tname);


//        for(i = 0; i < 4; i++)
//        {
//            childnode = t->child[i];
//            if (childnode != NULL)
//            {
//                //打印孩子结点
//                tname = getTypeName(childnode->type);
//                printf("child node: type:%d,tname:%s ->  {element: %s | val: %d} -> line:%d \n", childnode->type, tname, childnode->data.element, childnode->data.val, childnode->line);
//                free(tname);
//
//                //打印兄弟节点
//                p = childnode->sibling;
//                while (p != NULL)
//                {
//                    tname = getTypeName(p->type);
//                    printf("child sibling node: type:%d,tname:%s -> {element: %s | val: %d} -> line:%d \n", p->type, tname, p->data.element, p->data.val, p->line);
//                    free(tname);
//                    p = p->sibling;
//                }
//            }
//        }


        //打印兄弟节点
        p = t;
        while (p != NULL)
        {
            tname = getTypeName(p->type);
            symname = getSymtypeName(p->symtype);
            printf("deep:%d, childorder:%d, node:type:%d,tname:%s,symname:%s,name:%s -> {element: %s | val:%d} -> line:%d \n", deep, childorder, p->type, tname, symname, p->name, p->data.element, p->data.val, p->line);
            free(tname);
            p = p->sibling;
        }

        p = t;
        ++deep;
        while (p != NULL)
        {
            for(j = 0; j < 4; j++)
            {
                printTree(p->child[j], deep, j);
            }

            p = p->sibling;
        }
    }

}



/**
    语法树结点类型:
    PROGRAM, SUBPROGRAM, CONSTANT, VAR, FUNCTION, STMT, WHILE_STMT, IF_STMT, CALL_STMT, READ_STMT, WRITE_STMT, BEGIN_STMT,
    CONDITION, ASSIGN_STMT, ADD, MINUS, MUL, DIV, NUMBER, IDENT, ODD, RE_OPERATOR, EXPRESSION, TERM, FACTOR
**/
char *getTypeName(int type)
{
    char *kindstr;
    kindstr = (char *)malloc(20);

    char kinds[25][20] = {
        "PROGRAM", "SUBPROGRAM", "CONSTANT", "VAR", "FUNCTION", "STMT", "WHILE_STMT", "IF_STMT", "CALL_STMT",
        "READ_STMT", "WRITE_STMT", "BEGIN_STMT", "CONDITION", "ASSIGN_STMT", "ADD", "MINUS","MUL", "DIV",
        "NUMBER", "IDENT", "ODD", "RE_OPERATOR", "EXPRESSION", "TERM", "FACTOR"
    };


    strcpy(kindstr, kinds[type]);

    return kindstr;
}



/**
    语法树结点token类型:
    constsym, varsym, proceduresym, beginsym, endsym, oddsym, ifsym, thensym, callsym, whilesym, dosym,
    readsym, writesym,
    identsym, numbersym,
    plussym, minussym, mulsym, divsym, lesssym, lesseqsym, greatersym, greatereqsym, eqsym, neqsym, assignsym,
    commasym, periodsym, semicosym, lbracketsym, rbracketsym
**/
char *getSymtypeName(int type)
{
    char *kindstr = NULL;
    kindstr = (char *)malloc(20);

    char kinds[31][20] = {
        "constsym", "varsym", "proceduresym", "beginsym", "endsym", "oddsym", "ifsym", "thensym", "callsym",
        "whilesym", "dosym", "readsym", "writesym",
        "identsym", "numbersym",
        "plussym", "minussym", "mulsym", "divsym", "lesssym", "lesseqsym", "greatersym", "greatereqsym", "eqsym",
        "neqsym", "assignsym",
        "commasym", "periodsym", "semicosym", "lbracketsym", "rbracketsym"
    };

    if (type == -1)
    {
        kindstr = NULL;
    }
    else
    {
        strcpy(kindstr, kinds[type]);
    }
    return kindstr;
}
