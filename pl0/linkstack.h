#include <stdio.h>
#include <stdlib.h>
#include "syntax.h"

typedef ASTNode* ElementType;  //��ElementType����Ϊ�ַ�����

typedef struct
{
	ElementType data;
	struct LinkStack *next;
}LinkStack;

LinkStack* InitStack( );  //��ʽջ�Ƿ�Ҫ��ʼ��һ������?����Ҫ
ElementType Top(LinkStack *s);
int Push(LinkStack *s, ElementType x);
int Pop(LinkStack *s, ElementType *x);
int IsEmpty(LinkStack *s);


//��ʼ��ջ, ����ָ�룬��ͷ����ָ��
LinkStack* InitStack()
{
	LinkStack *s;
	s = (LinkStack *)malloc(sizeof(LinkStack));
	s->next = NULL;

	return s;
}


//��ǰջ��Ԫ��
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


//��ջ
int Push(LinkStack *s, ElementType x)
{
	LinkStack *p;

	p = (LinkStack *)malloc(sizeof(LinkStack));
	if (p == NULL)
	{
		printf("�����ڴ�ռ�ʧ��!\n");
		return 0;
	}

	p->data = x;
	p->next = s->next;
	s->next = p;

	return 1;
}

//��ջ
int Pop(LinkStack *s, ElementType *x)
{
	LinkStack *p;

	p = s->next;
	if (p == NULL)
	{
		printf("ջ�ѿ�!\n");
		return 0;
	}

	*x = p->data;
	s->next = p->next;

	free(p);

	return 1;
}


//ջ�Ƿ�Ϊ��
int IsEmpty(LinkStack *s)
{
	if (s->next != NULL)
	{
		return 0;
	}
	return 1;
}
