#ifndef PLANTS_VS_ZOMBIES_PLANTCARD_H
#define PLANTS_VS_ZOMBIES_PLANTCARD_H

#include <QtWidgets>
#include "MouseEventPixmapItem.h"

class Plant;

// 工具提示类，继承自QGraphicsRectItem
class TooltipItem: public QGraphicsRectItem
{
public:
    // 构造函数，传入提示文本
    TooltipItem(const QString &text);

    // 设置提示文本内容
    void setText(const QString &text);

private:
    QGraphicsTextItem *tooltipText;  // 用于显示文本的图形文本项
};

// 植物卡片项类，继承自可处理鼠标事件的像素图项
class PlantCardItem: public MouseEventPixmapItem
{
    Q_OBJECT  // Qt宏，启用信号槽机制

public:
    // 构造函数
    // @param plant 关联的植物对象指针
    // @param smaller 是否使用较小尺寸的卡片(默认为false)
    PlantCardItem(const Plant *plant, bool smaller = false);

    // 设置卡片选中状态
    void setChecked(bool newchecked);

    // 获取卡片当前是否被选中
    bool isChecked() const;

    // 设置进度百分比(用于冷却时间显示等)
    void setPercent(double value);

    // 更新卡片显示图像
    void updatePixmap();

private:
    bool checked;       // 卡片是否被选中
    double percent;     // 进度百分比(0.0~1.0)
    int lowestHeight;   // 卡片最低高度(用于布局)
    int highestHeight;  // 卡片最高高度(用于布局)
    QGraphicsPixmapItem *overlayImage;  // 覆盖图像项(如冷却遮罩)
    QPixmap checkedImage;    // 选中状态图像
    QPixmap uncheckedImage;  // 未选中状态图像
};

#endif //PLANTS_VS_ZOMBIES_PLANTCARD_H
