#include <stdio.h>
#include <stdlib.h>
#include <curses.h>  //Linux下图形界面库
#include <pthread.h> //Linux下多线程调用函数库
#include <math.h>
#include <unistd.h>

#define UP 1    //预定义键入上下左右键时，用字母表示数字，
#define DOWN -1 //通过绝对值的方法使蛇在向左运行时不能向右,向下时不能向上
#define RIGHT 2
#define LEFT -2

struct Snack //定义贪吃蛇结点结构体
{
    int hang;
    int lie;
    struct Snack *next;
};

struct Snack Food; //定义食物结构体

struct Snack *head = NULL; //定义一个蛇头指针
struct Snack *tail = NULL; //定义一个蛇尾指针
int key;                   //记录键入的值 ChangeDir()函数中使用到
int Dir;                   //记录方向的值 AddNode()函数中使用到

//函数封装初始化Ncurse界面
void InitNcurse()
{
    noecho();          //大多数的交互式应用程序在初始化时会调用noecho()函数，用于在进行控制操作时不显示输入的控制字符。
    initscr();         //ncurse界面的初始化函数
    keypad(stdscr, 1); //从标准stdscr中接受功能键，1代表是否接收
}

//封装初始化食物函数
void InitFood()
{
    int x = 1 + rand() % 18; //rand()随机产生数
    int y = 1 + rand() % 18;
    Food.hang = x;
    Food.lie = y;
}

//判断是否是蛇身结点函数
//(判断扫描传过来的i,j是否是蛇体结点中的hang，lie。如果是就返回1，GamePic()函数会将其打印 )
int HasSnackNode(int i, int j)
{
    struct Snack *p;
    p = head;
    while (p) //循环扫描蛇的链表
    {
        if (p->hang == i && p->lie == j)
        {
            return 1;
        }
        p = p->next;
    }
    return 0;
}

//判断是否是食物函数
int HasFood(int i, int j)
{
    if (Food.hang == i && Food.lie == j)
        return 1;
    else
        return 0;
}

//图像打印函数(扫描打印蛇和食物)
void GamePic()
{
    int hang;
    int lie;

    move(0, 0);

    for (hang = 0; hang < 20; hang++) //打印行为20，列为21的图
    {
        if (hang == 0)
        {
            for (lie = 0; lie < 20; lie++)
            {
                printw("--");
            }
            printw("\n");
        }
        else if (hang != 0 && hang != 19)
        {

            for (lie = 0; lie <= 20; lie++)
            {
                if (lie == 0 || lie == 20)
                {
                    printw("|");
                }
                else if (HasSnackNode(hang, lie)) //如果行，列正好是蛇身结点中的行列。就将其打印
                {
                    printw("[]");
                }
                else if (HasFood(hang, lie)) //如果行，列正好是食物结点中的行列。就将其打印
                    printw("##");
                else
                    printw("  ");
            }
            printw("\n");
        }
        else if (hang == 19)
        {
            for (lie = 0; lie < 20; lie++)
            {
                printw("--");
            }
            printw("\n");
            printw("By Hupeiyu,%d\n", key);
        }
    }
}

//增加节点函数
void AddNode()
{
    struct Snack *new = (struct Snack *)malloc(sizeof(struct Snack));
    new->next = NULL;
    switch (Dir) //根据键入的值来增加结点 ，修改tail所指结点中hang，lie并将修改后的值赋值给新结点
    {
    case UP:
        new->hang = tail->hang - 1;
        new->lie = tail->lie;
        break;
    case DOWN:
        new->hang = tail->hang + 1;
        new->lie = tail->lie;
        break;
    case RIGHT:
        new->hang = tail->hang;
        new->lie = tail->lie + 1;
        break;
    case LEFT:
        new->hang = tail->hang;
        new->lie = tail->lie - 1;
        break;
    }
    tail->next = new;
    tail = new;
}

//删除结点函数
void DeleteNode()
{
    struct Snack *p; //删除头结点
    p = head;
    head = head->next;
    free(p);
}

//-------------------------------------------------------------------
//初始化一条蛇
void InitSnack()
{
    Dir = RIGHT; //初始方向为RIGHT

    struct Snack *p;
    p = head;
    while (p) //判断蛇是否为空，清理空间
    {
        head = head->next;
        free(p);
        p = head;
    }

    InitFood(); //初始化食物位置

    head = (struct Snack *)malloc(sizeof(struct Snack));
    head->hang = 2;
    head->lie = 2;
    head->next = NULL;
    tail = head; //初始时尾指针指向头

    AddNode(); //为初始蛇添加两个结点
    AddNode();
}

//判断蛇如何死掉重新开始函数
int IfSnackDie()
{
    struct Snack *p;
    p = head;
    if (tail->hang == 0 || tail->lie == 0 || tail->hang == 19 || tail->lie == 20) //当尾指针所指结点中数据到达最大边界return 1
    {
        return 1;
    }
    while (p->next) //循环蛇身结点，当tail所指结点中数据与蛇身一致时return 1
    {
        if (p->hang == tail->hang && p->lie == tail->lie)
        {
            return 1;
        }
        p = p->next;
    }
    return 0;
}

//蛇移动函数 (本质就是增加新结点删除最后结点)
void MoveSnack()
{
    AddNode(); //增加新结点

    if (HasFood(tail->hang, tail->lie)) //判断是否吃到食物(判断新添加的结点中数据是否和Food中数据相同)
    {
        InitFood(); //相同就初始化食物位置 不删除结点
    }
    else //否则就 删除结点
    {
        DeleteNode();
    }

    if (IfSnackDie()) //如果tail遇到边界或撞到自己就初始化蛇
    {
        InitSnack();
    }
}

// 刷新页面函数
void *RefreshPic()
{
    while (1) //不停的刷新页面
    {
        MoveSnack();
        GamePic();
        refresh();      //刷新页面，在页面输出时必须刷新才可以显示
        usleep(100000); //以微秒为单位 ，100毫秒睡眠一次 ，执行挂起不动
    }
}

//通过绝对值判断相反方向不触发
void turn(int dir)
{
    if (abs(Dir) != abs(dir)) //abs()函数取绝对值
        Dir = dir;
}

//键入函数
void *ChangeDir()
{
    while (1)
    {
        key = getch();
        switch (key)
        {
        //case 0402:
        case KEY_DOWN:
            turn(DOWN);
            break;
        //case 0403:
        case KEY_UP:
            turn(UP);
            break;
        //case 0404:
        case KEY_LEFT:
            turn(LEFT);
            break;
        //case 0405:
        case KEY_RIGHT:
            turn(RIGHT);
            break;
        }
    }
}

int main()
{
    pthread_t t1; //多线程定义
    pthread_t t2;

    InitNcurse(); //初始化ncurse

    InitSnack(); //初始化蛇

    GamePic(); //初始化打印界面

    pthread_create(&t1, NULL, RefreshPic, NULL); //线程1
    pthread_create(&t2, NULL, ChangeDir, NULL);  //线程2

    while (1)
        getch(); //等待用户输入，如果没有这句话，程序就推出了，看不到运行的结果，也就看不到printw()中的话
    endwin();    //程序退出，调用函数来恢复shell终端显示，如果没有这句话，shell终端字乱码，坏掉
    return 0;
}
