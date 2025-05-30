// 坐标类的实现文件，负责处理游戏中的坐标转换和行列计算等操作

#include "Coordinate.h"

// 坐标类构造函数，初始化坐标映射表
Coordinate::Coordinate(int coord) : col(9)
{
    x2c = QPair<QList<double>, QList<int> >(
            { -50, 100, 140, 220, 295, 379, 460, 540, 625, 695, 775, 855, 935 },
            {  -2,  -1,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11 });
    c2x = QPair<QList<int>, QList<double> >(
            {  -2,  -1,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11 },
            { -50, 100, 140, 187, 267, 347, 427, 507, 587, 667, 747, 827, 865, 950 });
    if (coord == 0) {
        row = 5;
        y2r = QPair<QList<double>, QList<int> >(
                {  86, 181, 281, 386, 476 },
                {   0,   1,   2,   3,   4,   5 });
        r2y = QPair<QList<int>, QList<double> >(
                {   0,   1,   2,   3,   4,   5 },
                {  75, 175, 270, 380, 470, 575 });
    }
    else { // Pool
        row = 6;
        y2r = QPair<QList<double>, QList<int> >(
                {  86, 171, 264, 368, 440, 532 },
                {   0,   1,   2,   3,   4,   5,  6 });
        r2y = QPair<QList<int>, QList<double> >(
                {   0,   1,   2,   3,   4,   5,   6 },
                {  75, 165, 253, 355, 430, 552, 587 });
    }
}

// 根据 X 坐标获取列号
int Coordinate::getCol(double x) const
{
    return x2c.second[qLowerBound(x2c.first, x) - x2c.first.begin()];
}

// 根据 Y 坐标获取行号
int Coordinate::getRow(double y) const
{
    return y2r.second[qLowerBound(y2r.first, y) - y2r.first.begin()];
}

// 根据列号获取 X 坐标
double Coordinate::getX(int c) const
{
    return c2x.second[qBinaryFind(c2x.first, c) - c2x.first.begin()];
}

// 根据行号获取 Y 坐标
double Coordinate::getY(int r) const
{
    return r2y.second[qBinaryFind(r2y.first, r) - r2y.first.begin()];
}

// 选择植物的 X 坐标和列号
QPair<double, int> Coordinate::choosePlantX(double x) const
{
    int c = getCol(x);
    return QPair<double, int>(getX(c), c);
}

// 选择植物的 Y 坐标和行号
QPair<double, int> Coordinate::choosePlantY(double y) const
{
    int r = getRow(y);
    return QPair<double, int>(getY(r), r);
}

// 将值截断在指定范围内
int truncBetween(int value, int low, int high)
{
    if (value < low)
        return low;
    if (value > high)
        return  high;
    return value;
}

// 获取行数
int Coordinate::rowCount() const
{
    return row;
}

// 获取列数
int Coordinate::colCount() const
{
    return col;
}
