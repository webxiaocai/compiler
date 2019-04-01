#include "symtable.h"

/**
    符号表
**/



/**
    初始化
**/
Symtable *initSymtable()
{
    Symtable *table;
    int tablesize = 100;
    int i;

    table = (Symtable *)malloc(sizeof(Symtable));

    table->length = 0;
    table->maxSize = 100000;
    table->tablesize = tablesize;
    table->nodelist = (HashNode **)malloc(sizeof(HashNode*) * tablesize);

    for(i = 0; i < tablesize; i++)
    {
        table->nodelist[i] = (HashNode *)malloc(sizeof(HashNode));
        table->nodelist[i]->next = NULL;
        table->nodelist[i]->parentScope = NULL;
        table->nodelist[i]->addr = -1;
        memset(table->nodelist[i]->type, '\0', strlen(table->nodelist[i]->type));
        memset(table->nodelist[i]->name, '\0', strlen(table->nodelist[i]->name));
        table->nodelist[i]->level = -1;
    }

    return table;
}



//输出符号表
void printSymTable(Symtable *table)
{
    HashNode *p, *q;
    int i;

    printf("symtable symtable size:%d \n", table->tablesize);

    for(i = 0; i < table->tablesize; i++)
    {
        p = table->nodelist[i];

        if (p->next != NULL)
        {
            p = p->next;
            printf("name: %s, pscope:%s, type:%s, lev:%d, val:%d, size:%d, addr:%d\n", p->name, p->parentScope->name, p->type, p->level, p->val, p->arsize, p->addr);
        }

        q = p;
        while (q->next != NULL)
        {
            q = q->next;
            printf("name:%s, pscope:%s,type:%s, level:%d, val:%d, size:%d, addr:%d\n",q->name, q->parentScope->name, q->type, q->level, q->val, q->arsize, q->addr);
        }
    }

}



/**
    计算哈希
**/
int genhash(char *s)
{
    int i = 0;
    int hash = 0;
    while(s[i] != '\0')
    {
        hash = (s[i] + hash + '0') % 100;
        i++;
    }
    return hash;
}



/**
    往符号表添加元素

    返回插入的哈希表数组下标
**/
HashNode *addElement(Symtable *table, char *type, enum DATATYPE datatype, HashNode *parentFun, char *ele, int lev, int val, int size)
{
    int pos;
    HashNode *p, *q, *temp;

    if (table->length == table->maxSize)
    {
        perror("symtable full!");
        exit(EXIT_FAILURE);
    }

    pos = genhash(ele);

    p = table->nodelist[pos];

    //申请结点
    q = (HashNode *)malloc(sizeof(HashNode));
    strcpy(q->type, type);
    q->datatype = datatype;
    strcpy(q->name, ele);
    q->parentScope = parentFun;
    q->addr = -1;
    q->level = lev;
    q->val = val;
    q->next = NULL;
    q->arsize = size;

    //nodelist 数组只存放地址，不存放值
    if (p->next == NULL)
    {
        //没有哈希冲突，直接添加元素
        p->next = q;
    }
    else
    {
        //出现哈希冲突，链接到表最末尾
        temp = p->next;
        p->next = q;
        q->next = temp;
    }

    table->length++;

    return q;
}



/**
    查询符号表
    返回结点信息
    lev 表示引用标识符时的调用层级，只允许调用比调用层级低的标识符，由内向外寻找作用域链
    callFun 表示引用标识符时所在的函数
**/
HashNode *findElement(Symtable *table, char *name, int lev, char *callFun)
{
    int pos;
    HashNode *findNode = NULL, *funNode = NULL, *temp1 = NULL, *temp2 = NULL;;

    pos = genhash(name);
    temp1 = table->nodelist[pos];

    //调用函数在符号表的结点地址
    pos = genhash(callFun);
    temp2 = table->nodelist[pos];
    while(temp2->next != NULL)
    {
        temp2 = temp2->next;
        if ((strcmp(callFun, temp2->name) == 0) && temp2->level == lev-1 && strcmp(temp2->type, "function")==0)
        {
            funNode = temp2;
            break;
        }
    }

    while(temp1->next != NULL)
    {
        temp1 = temp1->next;
        if (temp1->level <= (lev) && (strcmp(name, temp1->name) == 0))
        {
            findNode = temp1;
            //找到的标识符在作用域链上
            if (isInScopeLink(findNode, funNode))
            {
                break;
            }
            else
            {
                findNode = NULL;
            }
        }
    }

    return findNode;
}



/**
    查到的标识符是否在作用域链上
**/
int isInScopeLink(HashNode *node, HashNode *start)
{
    HashNode *p;

    do
    {
        if (node->parentScope == start)
        {
            return 1;
        }
        start = start->parentScope;

        if ((node->parentScope != start) && (strcmp(start->name, "main") == 0))
        {
            break;
        }
    }
    while(1);

    return 0;
}



/**
    创建符号表
**/
Symtable *createSymtable(ASTTree *tree)
{
    Symtable *stable;

    //初始化符号表
    stable = initSymtable();

    //遍历抽象语法树
    traverseAstTree(stable, tree->root, 1, NULL);

    return stable;
}



/**
    遍历抽象语法树
    抽象语法树的设计上，目前只有变量定义，常量，过程名写入到符号表，所以只需处理这几种情况
**/
void traverseAstTree(Symtable *table, ASTNode *node, int deep, HashNode *parent)
{
    int i, curdeep, vnum = 0, dx = 3;
    ASTNode *childnode;
    HashNode *res, *preParent = NULL;

    //最多嵌套15层
    if (deep > 15)
    {
        perror("reach max nest level!");
        exit(EXIT_FAILURE);
    }

    curdeep = deep;
    while(node != NULL)
    {
        if (node->type == CONSTANT)
        {
            res = addElement(table, "constant", INTEGER, parent, node->name, deep, node->data.val, 0);
            parent->arsize++;
            if (IS_DEBUG)
            {
                printf("add constant:%s \n", node->name);
            }
        }
        else if (node->type == VAR)
        {
            vnum++;
            //Symtable *table, char *type, char *ele, int lev, int val
            res = addElement(table, "variable", INTEGER, parent, node->name, deep, dx+vnum, 0);
            parent->arsize++;
            if (IS_DEBUG)
            {
                printf("add var:%s parent:%s \n", node->name, parent->name);
            }
        }
        else if (node->type == FUNCTION || node->type == SUBPROGRAM)
        {
            //将main函数默认加入符号表, main 函数的层级是0
            if (deep == 1 && (node->type == SUBPROGRAM))
            {
                strcpy(node->name, "main");
                parent = addElement(table, "function", STR, NULL, node->name, 0, 0, 4);
                parent->parentScope = parent;  // main 函数自己指向自己
                if (IS_DEBUG)
                {
                    printf("add function:%s parent:%s \n", node->name, parent->name);
                }
            }

            //deep 表示函数嵌套层次, 将变量生成完毕再将函数信息写到符号表
            if (node->type == FUNCTION)
            {
                preParent = parent;
                deep++;
                parent = addElement(table, "function", STR, preParent, node->name, curdeep, 0, 4);
                if (IS_DEBUG)
                {
                    printf("add function:%s, parent:%s \n", node->name, preParent->name);
                }
            }

            for(i = 0; i < 4; i++)
            {
                //递归遍历
                traverseAstTree(table, node->child[i], deep, parent);
            }

            //有平级的其他过程, 要将第一次父级的过程赋值给当前parent
            if (node->sibling != NULL)
            {
                parent = preParent;
            }
        }
        else
        {
            //递归遍历
            traverseAstTree(table, node->child[0], deep, parent);
        }

        node = node->sibling;
    }
}
