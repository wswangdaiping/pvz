#ifndef PLANTS_VS_ZOMBIES_ASPECTRATIOLAYOUT_H
#define PLANTS_VS_ZOMBIES_ASPECTRATIOLAYOUT_H

#include <QtWidgets>

// 宽高比布局类（继承自QLayout）
class AspectRatioLayout: public QLayout
{
public:
    explicit AspectRatioLayout(QWidget *parent = nullptr);  // 构造函数，可指定父窗口
    virtual ~AspectRatioLayout();                          // 虚析构函数

    // 重写QLayout的虚函数
    virtual void addItem(QLayoutItem *item) override;       // 添加布局项
    virtual QLayoutItem *itemAt(int index) const override; // 获取指定索引的布局项
    virtual QLayoutItem *takeAt(int index) override;       // 移除并返回指定索引的布局项
    virtual int count() const override;                    // 返回布局项数量

    virtual QSize minimumSize() const override;            // 返回布局最小尺寸
    virtual QSize sizeHint() const override;               // 返回布局建议尺寸
    virtual void setGeometry(const QRect &rect) override;  // 设置布局几何形状（核心功能）
    virtual Qt::Orientations expandingDirections() const override; // 返回布局扩展方向

private:
    QLayoutItem *item;  // 存储管理的单个布局项（本布局只管理一个子项）
};

#endif //PLANTS_VS_ZOMBIES_ASPECTRATIOLAYOUT_H
