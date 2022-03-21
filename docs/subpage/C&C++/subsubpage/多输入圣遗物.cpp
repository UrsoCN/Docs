// 米哈游游戏测试开发A卷 2022
// 输入描述：
// 第一行输入一个正整数q，代表事件的数量。
// 接下来的q行，每行输入一行操作。
// 每行的第一个正整数p代表该操作的类型：
// 若p = 1，则后面紧跟两个正整数x和y，代表米小游掉落了一件暴击为x，暴伤为y的圣遗物
// 若p = 2，代表一次询问，请你输出米小游该次失去的圣遗物属性。
// 数据范围：
// 1 <= q <= 200000
// 1 <= p <= 2，且至少存在一个p = 2
// 1 <= x, y <= 10^9
// 输出描述：
// 对于每次p = 2的输入，输出一行结果：
// 若所有的圣遗物已被拾取，或还未掉落圣遗物，输出一行字符串"no reliquaries."
// 否则输出两个正整数x和y，代表米小游拾取到的圣遗物的暴击和暴伤.

#include <iostream>

using namespace std;

int main()
{
    int p = 0;
    int q = 0;
    int i = 0;
    int j = 0;

    cin >> q;

    for (int iter = 0; iter < q; iter++)
    {
        int x = 0, y = 0;
        cin >> p;
        if (cin.get() != '\n')
        {
            cin >> x >> y;
        }
        if (p == 1)
        {
            i = x;
            j = y;
        }
        if (p == 2)
        {
            if ((i == 0) && (j == 0))
            {
                cout << "no reliquaries." << endl;
            }
            else
            {
                cout << i << " " << j << endl;
                i = j = 0; // 这个语句看情况要不要，允许多次询问就注释掉这句
            }
        }
    }
}