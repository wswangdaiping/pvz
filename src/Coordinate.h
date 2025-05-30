#ifndef PLANTS_VS_ZOMBIES_COORDINATE_H
#define PLANTS_VS_ZOMBIES_COORDINATE_H

#include <QtCore>  // 包含Qt核心模块

/**
 * @brief 数值截断函数
 * @param value 输入值
 * @param low 下限
 * @param high 上限
 * @return 截断在[low,high]区间内的值
 */
int truncBetween(int value, int low, int high);

/**
 * @brief 游戏坐标转换类
 *
 * 处理游戏场景中的坐标系统转换，包括：
 * - 屏幕坐标与网格坐标的相互转换
 * - 植物种植位置的坐标计算
 */
class Coordinate
{
public:
    /**
     * @brief 构造函数
     * @param coord 初始化坐标值，默认为0
     */
    explicit Coordinate(int coord = 0);

    // 坐标转换接口
    int getCol(double x) const;    // X屏幕坐标转列号
    int getRow(double y) const;    // Y屏幕坐标转行号
    double getX(int c) const;      // 列号转X屏幕坐标
    double getY(int r) const;      // 行号转Y屏幕坐标

    // 植物选择位置计算
    QPair<double, int> choosePlantX(double x) const;  // 计算植物X坐标及列号
    QPair<double, int> choosePlantY(double y) const;  // 计算植物Y坐标及行号

    // 场景网格信息
    int rowCount() const;   // 获取总行数
    int colCount() const;   // 获取总列数

private:
    // 私有成员
    int row, col;  // 当前行列坐标

    // 坐标映射系统（使用QPair存储双向映射）
    QPair<QList<double>, QList<int>> x2c;  // X坐标到列号的映射
    QPair<QList<double>, QList<int>> y2r;  // Y坐标到行号的映射
    QPair<QList<int>, QList<double>> c2x;  // 列号到X坐标的映射
    QPair<QList<int>, QList<double>> r2y;  // 行号到Y坐标的映射
};

#endif //PLANTS_VS_ZOMBIES_COORDINATE_H
