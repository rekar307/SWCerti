#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>

#define MAX_VERTEX 50

int row, col;
int rear, front;
int map[MAX_VERTEX + 1][MAX_VERTEX + 1];
int queue[MAX_VERTEX + 1][3];

int enqueue(int x, int y, int c)
{
    queue[rear][0] = x;
    queue[rear][1] = y;
    queue[rear][2] = c;
    rear++;

    return 1;
}

int dequeue(int *x, int *y, int *c)
{
    if (rear == front) return 0;
    *x = queue[front][0];
    *y = queue[front][1];
    *c = queue[front][2];
    front++;

    return 1;
}

int bfs()
{
    int x = 0, y = 0, c = 0;

    enqueue(0, 0, 0);
    map[0][0] = 0;

    while (front < rear)
    {
        dequeue(&x, &y, &c);

        if (x == (row - 1) && y == (col - 1)) 
            return c;

        if (x + 1 <= col && map[x + 1][y] == 1) // down
        {
            enqueue(x + 1, y, ++c);
            map[x + 1][y] = 0;
        }

        if (y + 1 <= row && map[x][y + 1] == 1) // right
        {
            enqueue(x, y + 1, ++c);
            map[x][y + 1] = 0;
        }

        if (x - 1 >= 0 && map[x - 1][y] == 1) // up
        {
            enqueue(x - 1, y, ++c);
            map[x - 1][y] = 0;
        }

        if (y - 1 >= 0 && map[x][y - 1] == 1) // left
        {
            enqueue(x, y - 1, ++c);
            map[x][y - 1] = 0;
        }
    }

    return -1;
}

void reset()
{
    // reset    
    for (int i = 1; i <= MAX_VERTEX; ++i)
    {
        for (int j = 1; j <= MAX_VERTEX; ++j)
        {
            map[i][j] = 0;
        }

        for (int j = 0; j < 3; ++j)
        {
            queue[i][j] = 0;
        }
    }

    front = rear = 0;
}

int main(void)
{
    int tc = 0, T = 0;
    int i = 0, j = 0;
    int v1 = 0, v2 = 0;

    FILE *fp = fopen("input.txt", "r");

    fscanf(fp, "%d ", &T);

    for (int tc = 1; tc <= T; ++tc)
    {
        reset();

        fscanf(fp, "%d %d", &row, &col);

        for (int i = 0; i < row; ++i)
        {
            for (int j = 0; j < col; ++j)
            {
                fscanf(fp, "%d", &map[i][j]);
            }
        }

        // bfs
        printf("#%d bfs: %d\n", tc, bfs());

        // dfs
        //reset();
        //dfs(start);
        //printf("\n");
    }

    fclose(fp);

    return 0;
}
