/*
题目描述：
给定一组不等式，判断是否成立并输出不等式的最大差(输出浮点数的整数部分)，
要求:
1)不等式系数为double类型，是一个二维数组;
2)不等式的变量为int类型，是一维数组;
3)不等式的目标值为double类型，是一维数组;
4)不等式约束为字符串数组，只能是:">",">=","<","<=","="，
例如,不等式组:
a11*x1+a12*x2+a13*x3+a14*x4+a15*x5<=b1;
a21*x1+a22*×2+a23*x3+a24*x4+a25*x5<=b2;
a31*x1+a32*x2+a33*x3+a34*x4+a35*x5<=b3;
最大差=max{(a11*x1+a12*x2+a13*x3+a14*x4+a15*x5-b1),
        (a21*x1+a22*x2+a23*x3+a24*x4+a25*x5-b2),
        (a3Lxx1+a32*x2+a33*x3+a34*x4+a35*x5-b3)}，
类型为整数(输出浮点数的整数部分)

输入描述:
1)不等式组系数(double类型):
all,a12,a13,al4,a15
a21,a22,a23,a24,a25
a31,a32,a33,a34,a35
2)不等式变量(int类型):
xl,x2,x3,x4,x5
3)不等式目标值(double类型): b1,b2,b3
4)不等式约束(字符串类型):<=,<=,<=
输入:
al1,a12,a13,a14,a15;a21,a22,a23,a24,a25;a31,a32,a33,a34,a35;x1,x2,x3,x4,x5;b1,b2,b3;<=,<=,<=
输出描述:
true或者false，最大差
*/

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
    
    return 0;
}

// 2.3,3,5.6,7,6;11,3,8.6,25,1;0.3,9,5.3,66,7.8;1,3,2,7,5;340,670,80.6;<=,<=,<=
