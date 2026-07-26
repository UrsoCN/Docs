// 1、旋转字符游戏：给定两个整数M，N，生成一个M*N的矩阵，
// 矩阵中元素取值为A-Z的26个字母中的一个，A在左上角，
// 其余各数按顺时针方向旋转前进，依次递增放置，当超过Z时，
// 又从A开始填充。例如，当M=5，N=8时。矩阵中的内容如下：
// A B C D E F G H
// V W X Y Z A B I
// U J K L M N C J
// T I H G F E D K
// S R Q P O N M L
// 输入：一行字符串，包含两个M，N，M和N均为大于0，小于10000的整数，M表示行，N表示列。
// 输出：M*N的矩阵，以空格分隔

#include <iostream>
using namespace std;

int main()
{
    char character[26];
    for (size_t i = 0; i < 26; i++)
    {
        character[i] = (char)(65 + i);
        // cout << character[i];
    }

    int M, N = 0;
    cin >> M >> N;
    char **matrix = new char *[M];
    for (int i = 0; i < M; ++i)
    {
        matrix[i] = new char[N];
    }
    matrix[0][0] = 'A';

    int x = 0;
    int y = 0;
    size_t flag = 0;

    // 分别表示上下左右填充满的行数
    int u = 0;
    int d = 0;
    int l = 0;
    int r = 0;

    for (size_t i = 1; i < M * N; i++)
    {
        switch (flag)
        {
        case 0:
            y++;
            matrix[x][y] = character[i % 26];
            if ((y + r) == (N - 1))
            {
                u++;
                flag++;
            }
            break;
        case 1:
            x++;
            matrix[x][y] = character[i % 26];
            if ((x + d) == (M - 1))
            {
                r++;
                flag++;
            }
            break;
        case 2:
            y--;
            matrix[x][y] = character[i % 26];
            if ((y - l) == 0)
            {
                d++;
                flag++;
            }
            break;
        case 3:
            x--;
            matrix[x][y] = character[i % 26];
            if ((x - u) == 0)
            {
                l++;
                flag = 0;
            }
            break;
        }
    }

    for (size_t i = 0; i < M; i++)
    {
        for (size_t j = 0; j < N; j++)
        {
            cout << matrix[i][j] << " ";
        }
        cout << endl;
    }
}