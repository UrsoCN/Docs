/*
题目描述：
某学校举行运动会，学生们按编号（1，2，3...n）进行标识，现需要按照身高由低到高排序，对身高相同的人，
按体重由轻到重排列；对于身高体重都相同的人，维持原有的编号顺序关系。请输出排列后的编号顺序关系。

输入描述：
两个序列，每个序列由n个正整数组成（0 < n <= 100）。第一个序列中的数值代表身高，第二个序列中的数值代表体重。

输出描述：
排列结果，每个数值都是原始数列中的学生编号，编号从1开始。
*/

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

/*  调用stable_sort()
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
*/
