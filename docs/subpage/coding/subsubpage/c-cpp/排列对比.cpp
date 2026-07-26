// 米小游有一个排列P，对于这个排列米小游对其循环左移了K1和K2个元素，
// 得到新的排列P1和P2，现在米小游每次可以对P1或P2循环左移一个元素。
// 米小游想知道将排列P1和P2变得相同最少需要操作多少次。
// 输入描述：
// 第一行为t，表示有t组数据。
// 接下来又3×t行，其中第一行为一个ｎ表示排列大小。
// 第二行有n个整数，表示排列P1的元素。
// 第三行有n个整数，表示排列P2的元素。
// 1 <= t <= 10, \sigma n <= 10^5, \sigma n <= 10^5 。
// 输出描述：
// 输出为t行，每行为一组答案。

#include <iostream>

using namespace std;

int comp(int *a, int *b)
{
    int k = 0;
    while (a[0] != b[k])
    {
        k++;
    }
    return k;
}

int main()
{
    int t = 0;
    int n = 0;
    cin >> t;
    for (int i = 0; i < t; i++)
    {
        int narr1[100000];
        int narr2[100000];

        cin >> n;
        int j = 0;
        do
        {
            cin >> narr1[j];
            j++;
        } while ((j < n) && (cin.get() != '\n'));
        j = 0;
        do
        {
            cin >> narr2[j];
            j++;
        } while ((j < n) && (cin.get() != '\n'));

        int k1 = comp(narr1, narr2);
        int k2 = comp(narr2, narr1);

        cout << ((k1 > k2) ? k2 : k1) << endl;
    }
}