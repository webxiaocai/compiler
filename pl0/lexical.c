#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "lexical.h"

/**
    词法分析后的单词序列是先存在一个变量，后面再解析变量里的数据，
    或者是只采用临时变量保存程序当前读到的字符，直接进入语法分析？
    先解析存到一个变量
**/



/**
    打印token序列
**/
void printToken()
{
    int i, len;
    char *kindstr = "";

    for(i = 0; i < tokensLen; i++)
    {
        switch(tokens[i].type)
        {
            case 0:
                kindstr = "keyword ";
                break;
            case 1:
                kindstr = "ident ";
                break;
            case 2:
                kindstr = "operator ";
                break;
            case 3:
                kindstr = "number ";
                break;
            default:
                kindstr = "jiefu  ";
                break;
        }

        printf("token %d : %d-> %s -> %s  %d  line: %d\n",
               i, tokens[i].token, kindstr, tokens[i].id, tokens[i].num, tokens[i].line);
    }
}


/**
    词法分析源码程序
    读入程序，从左自由，逐个逐行分析

**/
void lexicalParse(char *codePath)
{
    //printf("syn analysis:\n");

    FILE *fp;
    int iToken, iNum, num, startType; //startType:单词的开始字符类型
    int ch, ch2;  //ch 需定义为整型, 后面自动把int的ch 转为ch 做比较
    char c, c2;
    int line, tokenKind, tokenLen;
    long filePointPos; //文件指针位置

    if (!(fp = fopen(codePath, "r")))
    {
        perror("打开源码文件错误!");
        exit(EXIT_FAILURE);
    }

    startType = -1;
    iToken = 0;
    line = 1;
    //循环读取字符
    while((ch = fgetc(fp)) != EOF)
    {
        //初始化设置
        memset(strToken, '\0', strlen(strToken));
        numToken = -32768;
        c = ch;

        filePointPos = ftell(fp);
        if (ch == '\n')
        {
            line++;
        }

        if (ch == ' ' || ch == '\n')
        {
            continue;
        }

        //数字
        if (isdigit(ch))
        {
            startType = number;
            tokens[iToken].type = number;
            tokens[iToken].token = numbersym;
        }

        //以运算符开头
        if ((tokenKind = isOperation(ch)) != -1)
        {
            startType = operation;
            tokens[iToken].type = operation;
            tokens[iToken].token = tokenKind;
        }

        //以界符开头
        if ((tokenKind = isBoundary(ch)) != -1)
        {
            startType = boundary;
            tokens[iToken].type = boundary;
            tokens[iToken].token = tokenKind;
        }

        //字母开头
        if (isalpha(ch))
        {
            startType = identifier; //表示可识别
        }

        //获取token
        filePointPos = getOneToken(fp, filePointPos, startType);

        //如是字母开头还要再判断一次token类型
        if (isalpha(ch))
        {
            //是否保留字
            if ((tokenKind = isKeyword(strToken)) != -1)
            {
                tokens[iToken].type = keyword;
                tokens[iToken].token = tokenKind;
            }
            else
            {
                tokens[iToken].type = identifier;
                tokens[iToken].token = identsym;
            }
        }

        //再判断一次以运算符开头的token单词类型, 回退2个字符判断即可
        if (startType == operation)
        {
            fseek(fp, filePointPos-2, SEEK_SET);
            ch = fgetc(fp);
            c = ch;
            ch2 = fgetc(fp);
            c2 = ch2;
            //表示 :=  >=, <=
            if ((c == ':' || c == '>' || c == '<') && (c2 == '='))
            {
                if (ch == ':')
                {
                    tokens[iToken].token = assignsym;  //赋值
                }
                else if (ch == '>')
                {
                    tokens[iToken].token = greatereqsym;
                }
                else if (ch == '<')
                {
                    tokens[iToken].token = lesseqsym;
                }
            }
        }

        strcpy(tokens[iToken].id, strToken);
        tokens[iToken].num = numToken;
        tokens[iToken].line = line;
        iToken++;
        tokensLen = iToken;
        //设置最新文件指针位置
        fseek(fp, filePointPos, SEEK_SET);
    }

    fclose(fp);

    //输出token
    //printToken();

}


/**
    获取一个字母/数字单词字符
    返回获取单词后最新的文件指针位置
**/
long getOneToken(FILE *fp, long pos, int stype)
{
    int ch;
    char c;
    int i = 0;
    long filePos; //文件指针位置

    if ((pos - 1) < 0)
    {
        perror("指针回退位置出错!");
        exit(EXIT_FAILURE);
    }

    //回退一个字符
    fseek(fp, pos-1, SEEK_SET);

    if (stype == number)
    {
        numToken = 0;

        while(isdigit(ch = fgetc(fp)))
        {
            numToken = 10*numToken + (ch - '0');
        };
    }
    else
    {
        if (stype == identifier)
        {
            while(isalpha(ch = fgetc(fp)))
            {
                strToken[i] = ch;
                i++;
            };
        }

        if (stype == boundary)
        {
            strToken[i] = fgetc(fp);
            i++;
        }

        //操作符需要判断多一个字符
        if (stype == operation)
        {
            strToken[i] = ch = fgetc(fp);
            c = ch;
            i++;
            if (c == ':' || c == '>' || c == '<')
            {
                //表示 :=  >=, <=
                ch = fgetc(fp);
                if (ch == '=')
                {
                    strToken[i] = ch;
                    i++;
                }
                else
                {
                    //如果找不到 = ,要回退一个位置
                    filePos = ftell(fp);
                    filePos--;
                    fseek(fp, filePos, SEEK_SET);
                }
            }
        }

        //设置结束符
        if (i < 100)
        {
            strToken[i] = '\0';
        }
    }

    if (stype == operation || stype == boundary)
    {
       return ftell(fp);
    }
    else
    {
        return ftell(fp)-1;
    }
}


/**
    判断是否是关键字
**/
int isKeyword(char *str)
{
    int i, kind, length;
    kind = -1;
    length = sizeof(keywords) / sizeof(keywords[0]);
    for(i = 0; i < length; i++)
    {
        if (strcmp(keywords[i], str) == 0)
        {
            kind = i;
            //printf("%s is keyword!\n", str);
            break;
        }
    }

    return kind;
}


/**
    判断是否是运算符
    "+", "-", "*", "/", "<", "<=", ">", ">=", "#", "=", ":="
    返回运算符类型, -1 表示不属于运算符类型
**/
int isOperation(char ch)
{
    int kind = -1;

    switch(ch)
    {
        case '+':
            kind = plussym;
            break;
        case '-':
            kind = minussym;
            break;
        case '*':
            kind = mulsym;
            break;
        case '/':
            kind = divsym;
            break;
        case '>':
            kind = greatersym;
            break;
        case '<':
            kind = lesssym;
            break;
        case '=':
            kind = eqsym;
            break;
        case '#':
            kind = neqsym;
            break;
        case ':':
            kind = eqsym;
            break;
        default:
            break;
    }
    return kind;
}


/**
    判断界符
**/
int isBoundary(char ch)
{
    int kind = -1;

    switch(ch)
    {
        case ',':
            kind = commasym;
            break;
        case '.':
            kind = periodsym;
            break;
        case ';':
            kind = semicosym;
            break;
        case '(':
            kind = lbracketsym;
            break;
        case ')':
            kind = rbracketsym;
            break;
        default:
            break;
    }
    return kind;
}



/**
    获取下一token
    curpos++后，改变当前curtoken的值
**/
void getNextToken()
{
    curpos++;
    curtoken = tokens[curpos].token;
}

