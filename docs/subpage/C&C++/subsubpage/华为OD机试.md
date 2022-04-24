# 华为OD机试

## 身高体重排序

**题目描述：**

某学校举行运动会，学生们按编号（1，2，3...n）进行标识，现需要按照身高由低到高排序，对身高相同的人，按体重由轻到重排列；对于身高体重都相同的人，维持原有的编号顺序关系。请输出排列后的编号顺序关系。

**输入描述：**

两个序列，每个序列由n个正整数组成（0 < n <= 100）。第一个序列中的数值代表身高，第二个序列中的数值代表体重。

**输出描述：**

排列结果，每个数值都是原始数列中的学生编号，编号从1开始。

### 调用stable_sort()

```C++
#include <iostream>
#include <vector>
#include <algorithm>
#include <tuple>

int main()
{
    int count;
    std::cin >> count;

    std::vector<std::tuple<int, int, int>> datas;

    // 读取heights
    if ((count > 0) && (count <= 100))
    {
        for (int i = 0; i < count; i++)
        {
            int tmpHeight;
            std::cin >> tmpHeight;
            datas.push_back({i + 1, tmpHeight, 0});
        }
        // 读取weights
        for (int i = 0; i < count; i++)
        {
            int tmpWeight;
            std::cin >> tmpWeight;
            std::get<2>(datas[i]) = tmpWeight;
        }

        // 打印datas
        // for (int i = 0; i < count; i++)
        // {
        //     std::cout << std::get<0>(datas[i]) << "\t"
        //               << std::get<1>(datas[i]) << "\t"
        //               << std::get<2>(datas[i]) << "\n";
        // }
        // std::cout << "----------" << std::endl;

        // 根据weights升序排列
        std::stable_sort(datas.begin(), datas.end(),
                         [](std::tuple<int, int, int> a, std::tuple<int, int, int> b)
                         { return std::get<2>(a) < std::get<2>(b); });

        // 根据heights升序排列
        std::stable_sort(datas.begin(), datas.end(),
                         [](std::tuple<int, int, int> a, std::tuple<int, int, int> b)
                         { return std::get<1>(a) < std::get<1>(b); });

        for (auto data : datas)
        {
            std::cout << std::get<0>(data) << " ";
        }
    }
    return 0;
}
```

### 自己写排序算法

``` C++
#include <iostream>
#include <vector>
#include <algorithm>
#include <tuple>

// 给heights排序
void bubble_sort_1(std::vector<std::tuple<int, int, int>> &datas)
{
    int i, j;
    for (i = 0; i < datas.size() - 1; i++)
        for (j = 0; j < datas.size() - 1 - i; j++)
            if (std::get<1>(datas[j]) > std::get<1>(datas[j + 1]))
                std::swap(datas[j], datas[j + 1]);
}

// 给weights排序
void bubble_sort_2(std::vector<std::tuple<int, int, int>> &datas)
{
    int i, j;
    for (i = 0; i < datas.size() - 1; i++)
        for (j = 0; j < datas.size() - 1 - i; j++)
            if (std::get<2>(datas[j]) > std::get<2>(datas[j + 1]))
                std::swap(datas[j], datas[j + 1]);
}

int main()
{
    int count;
    std::vector<std::tuple<int, int, int>> datas;
    std::cin >> count;
    int tmpHeight;
    int tmpWeight;

    if ((count > 0) && (count <= 100))
    {
        // 读取heights
        for (int i = 0; i < count; i++)
        {
            std::cin >> tmpHeight;
            datas.push_back({i + 1, tmpHeight, 0});
        }
        // 读取weights
        for (int i = 0; i < count; i++)
        {
            std::cin >> tmpWeight;
            std::get<2>(datas[i]) = tmpWeight;
        }

        bubble_sort_2(datas);
        bubble_sort_1(datas);

        for (auto data : datas)
        {
            std::cout << std::get<0>(data) << " ";
        }
    }

    return 0;
}
```

## 最大差

**题目描述：**

给定一组不等式，判断是否成立并输出不等式的最大差(输出浮点数的整数部分)，
要求:
1)不等式系数为double类型，是一个二维数组;
2)不等式的变量为int类型，是一维数组;
3)不等式的目标值为double类型，是一维数组;
4)不等式约束为字符串数组，只能是:">",">=","<","<=","="，
例如,不等式组:

```
a11*x1+a12*x2+a13*x3+a14*x4+a15*x5<=b1;
a21*x1+a22*×2+a23*x3+a24*x4+a25*x5<=b2;
a31*x1+a32*x2+a33*x3+a34*x4+a35*x5<=b3;
```

```
最大差=max{(a11*x1+a12*x2+a13*x3+a14*x4+a15*x5-b1),
        (a21*x1+a22*x2+a23*x3+a24*x4+a25*x5-b2),
        (a3Lxx1+a32*x2+a33*x3+a34*x4+a35*x5-b3)}，类型为整数(输出浮点数的整数部分)
```

**输入描述:**

1)不等式组系数(double类型):

```
all,a12,a13,al4,a15
a21,a22,a23,a24,a25
a31,a32,a33,a34,a35
```

2)不等式变量(int类型):`xl,x2,x3,x4,x5`

3)不等式目标值(double类型): `b1,b2,b3`

4)不等式约束(字符串类型):`<=,<=,<=`

输入:

`al1,a12,a13,a14,a15;a21,a22,a23,a24,a25;a31,a32,a33,a34,a35;x1,x2,x3,x4,x5;b1,b2,b3;<=,<=,<=`

**输出描述:**

true或者false，最大差

``` C++
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

int main()
{
    std::string string;
    std::string tmp;
    std::vector<std::string> dParameters;
    std::vector<std::vector<double>> a;
    std::vector<int> x;
    std::vector<double> b;
    std::vector<std::string> signs;
    std::vector<double> differences;
    bool flag = true;

    int unequals = 0;

    std::getline(std::cin, string);
    std::stringstream ss(string);

    // 将由;分割开的各类参数分别保存在dParameters里
    while (std::getline(ss, tmp, ';'))
    {
        dParameters.push_back(tmp);
        unequals++;
    }

    unequals -= 3; // 不等式数 = 分号数 - 3

    // 读取参数a
    for (int i = 0; i < unequals; i++)
    {
        a.push_back(std::vector<double>());
        std::stringstream ssd(dParameters[i]);
        while (std::getline(ssd, tmp, ','))
        {
            // TODO: 这里出现段错误了
            a[i].push_back(stod(tmp)); // 这里出现段错误了
        }
    }

    // 读取变量x
    ss = std::stringstream(dParameters[unequals]);
    while (std::getline(ss, tmp, ','))
    {
        x.push_back(stoi(tmp));
    }

    // 读取参数b
    ss = std::stringstream(dParameters[unequals + 1]);
    while (std::getline(ss, tmp, ','))
    {
        b.push_back(stod(tmp));
    }

    // 读取不等号
    ss = std::stringstream(dParameters[unequals + 2]);
    while (std::getline(ss, tmp, ','))
    {
        signs.push_back(tmp);
    }

    // 计算差并判断不等号是否正确
    for (int i = 0; i < unequals; i++)
    {
        differences.push_back(0);
        for (int j = 0; j < x.size(); j++)
        {
            differences[i] += (a[i][j] * x[j]);
        }
        differences[i] -= b[i];
        if (differences[i] == 0)
        {
            if ((signs[i] == std::string(">")) || (signs[i] == std::string("<")))
            {
                flag = false;
            }
        }
        else if (differences[i] > 0)
        {
            if ((signs[i] == std::string("<=")) || (signs[i] == std::string("<")))
            {
                flag = false;
            }
        }
        else // < 0
        {
            if ((signs[i] == std::string(">=")) || (signs[i] == std::string(">")))
            {
                flag = false;
            }
        }
    }

    int maxDifference = INT32_MIN;
    for (int i = 0; i < unequals; i++)
    {
        maxDifference = (int)((differences[i] > maxDifference) ? differences[i] : maxDifference);
    }

    if (flag)
    {
        std::cout << "true";
    }
    else
    {
        std::cout << "false";
    }
    std::cout << "," << maxDifference;
}

// 2.3,3,5.6,7,6;11,3,8.6,25,1;0.3,9,5.3,66,7.8;1,3,2,7,5;340,670,80.6;<=,<=,<=
```

## 分积木

Solo和koko是两兄弟，妈妈给了他们一大堆积木，每块积木上都有自己的重量。现在他们想要将这些积木分成两堆。哥哥Solo负责分配，弟弟koko要求两个人获得的积木总重量"相等”(根据Koko的逻辑)，个数可以不同，不然就会哭，但koko只会先将两个数转成二进制再进行加法，而且总会忘记进位（每个进位都忘记)。

如当25(11101)加11(1011)时，koko得到的计算结果是18(10010):

```
 11001
+01011
------
 10010
```

Solo想要尽可能使自己得到的积木总重量最大，且不让koko哭。

**输入描述:**

```
3
3 5 6
```

第一行是一个整数`N(2≤N≤100)`，表示有多少块积木;第二行为空格分开的`x`个整数`ci(1<ci<10^6)`，表示第i块积木的重量。

**输出描述:**

`11`

让koko不哭,输出solo所能获得积木的最大总重量;否则输出"NO"。

``` C++
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
```
