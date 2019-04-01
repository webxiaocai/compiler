#include <stdio.h>
#include <stdlib.h>
#include "syntax.h"

typedef ASTNode* ElementType;  //将ElementType定义为字符类型

typedef struct
{
	ElementType data;
	struct LinkStack *next;
}LinkStack;

LinkStack* InitStack( );  //链式栈是否要初始化一堆数据?不需要
ElementType Top(LinkStack *s);
int Push(LinkStack *s, ElementType x);
int Pop(LinkStack *s, ElementType *x);
int IsEmpty(LinkStack *s);


//初始化栈, 二级指针，带头结点的指针
LinkStack* InitStack()
{
	LinkStack *s;
	s = (LinkStack *)malloc(sizeof(LinkStack));
	s->next = NULL;

	return s;
}


//当前栈顶元素
ElementType Top(LinkStack *s)
{
    ElementType x;
    LinkStack *p;

    if (s != NULL && s->next != NULL)
    {
        p = s->next;
        x = p->data;
    }

    return x;
}


//进栈
int Push(LinkStack *s, ElementType x)
{
	LinkStack *p;

	p = (LinkStack *)malloc(sizeof(LinkStack));
	if (p == NULL)
	{
		printf("申请内存空间失败!\n");
		return 0;
	}

	p->data = x;
	p->next = s->next;
	s->next = p;

	return 1;
}

//出栈
int Pop(LinkStack *s, ElementType *x)
{
	LinkStack *p;

	p = s->next;
	if (p == NULL)
	{
		printf("栈已空!\n");
		return 0;
	}

	*x = p->data;
	s->next = p->next;

	free(p);

	return 1;
}


//栈是否为空
int IsEmpty(LinkStack *s)
{
	if (s->next != NULL)
	{
		return 0;
	}
	return 1;
}
