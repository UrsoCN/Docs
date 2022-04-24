/*
Solo和koko是两兄弟，妈妈给了他们一大堆积木，每块积木上都有自己的重量。现在他们想要将这些积木分成两堆。
哥哥Solo负责分配，弟弟koko要求两个人获得的积木总重量"相等”(根据Koko的逻辑)，个数可以不同，不然就会哭，
但koko只会先将两个数转成二进制再进行加法，而且总会忘记进位（每个进位都忘记)。
如当25(11101)加11(1011)时，koko得到的计算结果是18(10010):
11001
+01011
------
10010
Solo想要尽可能使自己得到的积木总重量最大，且不让koko哭。

输入描述:
3
3 5 6
第一行是一个整数N (2≤N≤100)，表示有多少块积木;第二行为空格分开的x个整数ci(1<ci<10^6)，表示第i块积木的重量。
输出描述:
11
让koko不哭,输出solo所能获得积木的最大总重量;否则输出"NO"。
*/

#include <iostream>
#include <vector>

int addXOR(int a, int b)
{
    return a ^ b;
}

int main()
{
    int count;
    int tmp;
    int sumXOR = 0;
    int sum = 0;
    int minWeight = INT32_MAX;
    std::vector<int> weightD;
    std::cin >> count;

    if ((count > 2) && (count < 100))
    {
        for (int i = 0; i < count; i++)
        {
            std::cin >> tmp;
            if ((tmp > 1) && (count < 1e6))
            {
                weightD.push_back(tmp);
                sumXOR ^= tmp;
                sum += tmp;
                minWeight = (tmp < minWeight) ? tmp : minWeight;
            }
        }
        // 要求两个人获得的积木总重量"相等"，也就是异或的"和"相等，再次异或则得0
        if (sumXOR == 0)
        {
            sum -= minWeight;
            std::cout << sum;
        }
        else
        {
            std::cout << "NO";
        }
    }
    return 0;
}