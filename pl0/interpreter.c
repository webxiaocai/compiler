#include "interpreter.h"

/**
    解释器,执行代码

    使用display表实现非局部变量访问

**/
void interpret(Code *code)
{
    Code i;  //存储当前执行指令地址
    /**

        nestedLev:当前函数嵌套层级
        top: 栈顶指针
        stack: 运行时栈
        base: 基址寄存器，基址指针
        ip: 指令指针寄存器，指向下一条指令地址

    **/
    int count = 0, preLev = 0, nestedLev = 0, top = 0, stack[500], base = 0, b = 0, ip = 0, temp;

    memset(stack, -1, sizeof(stack));
    printf("\n\n/*********************** execute: ***********************\n\n");

    //display[0] 始终指向main
    display[0] = 0;

    while(ip < codeindex)
    {
        i = code[ip];

        preLev = nestedLev;  //保存上一个函数嵌套层级
        switch(i.f)
        {
            //即将进入下一个函数的调用，提前做准备保存现场信息
            case CAL:
                nestedLev = i.l;  // 嵌套层级
                if (display[nestedLev] != -1)
                {
                    //保存先前存在的嵌套地址，后面退出函数要恢复先前的层级
                    stack[top] = display[nestedLev];
                }

                //将被调用函数的基址保存到到display表
                display[nestedLev] = top;
                stack[top+1] = preLev;   //保存当前函数嵌套层级，用于函数退出时现场恢复
                stack[top+2] = base;
                stack[top+3] = ip+1;      //下一条指令，中断返回使用
                base = top;

                ip = i.a;     //跳转到要执行的函数地址

                break;

            case INTE:
                //设置
                top = top + i.a;
                ip++;
                break;

            case LIT:
                stack[top] = i.a;
                top++;
                ip++;
                break;

            case LOD:       //使用display表查找变量位置
                b = display[nestedLev-i.l];  // lod和sto指令 的 l 意思都是在当前函数嵌套层级往前找l个层级
                stack[top] = stack[b+i.a];   //基址 + 偏移地址 = 变量绝对地址。将变量内容放到栈顶
                top++;
                ip++;
                break;

            case STO:
                top--;
                b = display[nestedLev-i.l];  // lod和sto指令 的 l 意思都是在当前函数嵌套层级往前找l个层级
                stack[b+i.a] = stack[top];   //基址 + 偏移地址 = 变量绝对地址。将变量内容放到栈顶
                ip++;
                break;

            case JMP:
                ip = i.a;
                break;

            case JPC:   //取栈顶值判断为false, 跳转到目标地址, 否则顺序执行
                top--;
                if (stack[top] == 0)
                {
                    ip = i.a;
                }
                else
                {
                    ip++;
                }
                break;

            case OPR:
                if (i.a == 0)
                {
                    //退出函数，中断返回
                    top = base;            //top 执向当前函数基址，也就是top指向上一个过程的最边上，相当于把当前函数活动记录都释放掉
                    if (stack[top] > 0)
                    {
                        display[nestedLev] = stack[top];  //将之前保存在基址的嵌套关系还原
                    }
                    nestedLev = stack[top+1];   //将当前嵌套层级恢复为先前保存的层级
                    base = stack[top+2];       //将当前基址改为上一层函数的基址
                    ip = stack[top+3];      //中断返回，即调用函数之后要执行的下一条指令

                    //如果已经退出了主函数，结束程序
                    if (ip == -1 || base < 0)
                    {
                        ip = codeindex + 1;
                    }
                }
                else if (i.a == 1)   // 取反结果
                {
                    top--;
                    stack[top] = !stack[top];
                    ip++;
                }
                else if (i.a == 2)   // +
                {
                    top--;
                    stack[top-1] = stack[top-1] + stack[top];
                    ip++;
                }
                else if (i.a == 3)   // -
                {
                    top--;
                    stack[top-1] = stack[top-1] - stack[top];
                    ip++;
                }
                else if (i.a == 4)   // *
                {
                    top--;
                    stack[top-1] = stack[top-1] * stack[top];
                    ip++;
                }
                else if (i.a == 5)   // /
                {
                    top--;
                    stack[top-1] = stack[top-1] / stack[top];
                    ip++;
                }
                else if (i.a == 8)   // =
                {
                    top--;
                    stack[top-1] = stack[top-1] == stack[top];
                    ip++;
                }
                else if (i.a == 9)   // #
                {
                    top--;
                    stack[top-1] = stack[top-1] != stack[top];
                    ip++;
                }
                else if (i.a == 10)   // >
                {
                    top--;
                    stack[top-1] = stack[top-1] > stack[top];
                    ip++;
                }
                else if (i.a == 11)   // >=
                {
                    top--;
                    stack[top-1] = stack[top-1] >= stack[top];
                    ip++;
                }
                else if (i.a == 12)   // <
                {
                    top--;
                    stack[top-1] = stack[top-1] < stack[top];
                    ip++;
                }
                else if (i.a == 13)   // <=
                {
                    top--;
                    stack[top-1] = stack[top-1] <= stack[top];
                    ip++;
                }
                else if (i.a == 14)    // 输出
                {
                    top--;
                    printf("%d", stack[top]);
                    ip++;
                }
                else if (i.a == 15)     // 换行
                {
                    printf("\n");
                    ip++;
                }
                else if (i.a == 16)     // input
                {
                    printf("input:");
                    scanf("%d", &temp);
                    stack[top] = temp;
                    top++;
                    ip++;
                }
                break;

            default:
                break;
        }
    }


}
