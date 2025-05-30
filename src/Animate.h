#ifndef PLANTS_VS_ZOMBIES_BASESCENE_H
#define PLANTS_VS_ZOMBIES_BASESCENE_H

#include <QtWidgets>

// 动画控制类，用于创建和管理图形项的动画效果
class Animate
{
public:
    // 构造函数，传入要进行动画的图形项和所属场景
    Animate(QGraphicsItem *item, QGraphicsScene *scene);

    // 以下方法返回Animate&以支持链式调用

    // 设置移动动画到指定位置
    Animate &move(QPointF toPos);
    // 设置缩放动画到指定比例
    Animate &scale(qreal toScale);
    // 设置淡入淡出动画到指定透明度
    Animate &fade(qreal toOpacity);

    // 设置替换动画(新老交替效果)
    Animate &replace();
    // 设置动画曲线形状
    Animate &shape(QTimeLine::CurveShape shape);
    // 设置动画速度
    Animate &speed(qreal speed);
    // 设置动画持续时间(毫秒)
    Animate &duration(int duration);

    // 设置动画完成时的回调函数(无参数版本)
    Animate &finish(std::function<void(void)> functor);
    // 设置动画完成时的回调函数(带bool参数版本，默认为空函数)
    Animate &finish(std::function<void(bool)> functor = [](bool) {});

protected:
    // 动画关键帧类型枚举(使用位掩码)
    enum KeyFrameType {
        MOVE = 0x01,    // 移动动画
        SCALE = 0x02,   // 缩放动画
        FADE = 0x04,    // 透明度动画
        REPLACE = 0x08  // 替换动画
    };

    // 关键帧结构体，保存单次动画的参数
    struct KeyFrame {
        int type;               // 动画类型(KeyFrameType的组合)
        int duration;           // 持续时间(毫秒)
        qreal speed;            // 动画速度(当duration=0时使用)
        QPointF toPos;          // 目标位置
        qreal toScale;          // 目标缩放比例
        qreal toOpacity;        // 目标透明度
        std::function<void(bool)> finished; // 完成回调
        QTimeLine::CurveShape shape; // 动画曲线形状
    };

    // 动画结构体，保存一个完整的动画序列
    struct Animation {
        QTimeLine *anim;        // Qt动画时间线对象
        QGraphicsScene *scene;   // 所属场景
        QList<KeyFrame> frames;  // 关键帧列表
    };

    // 静态方法，获取图形项关联的动画对象
    static Animation *getAnimation(QGraphicsItem *item);
    // 静态方法，设置图形项的动画对象
    static void setAnimation(QGraphicsItem *item, Animation *animation);
    // 静态方法，生成并执行动画
    static void generateAnimation(QGraphicsItem *item);

private:
    static const int AnimationKey; // 用于在图形项中存储动画的键值

    // 当前动画参数
    int type;                   // 动画类型
    int m_duration;             // 持续时间
    qreal m_speed;             // 动画速度
    QPointF toPos;              // 目标位置
    qreal toScale;             // 目标缩放
    qreal toOpacity;           // 目标透明度
    QTimeLine::CurveShape m_shape; // 动画曲线形状

    // 关联的图形项和场景
    QGraphicsItem *item;       // 要进行动画的图形项
    QGraphicsScene *scene;     // 图形项所属场景
};

#endif //PLANTS_VS_ZOMBIES_BASESCENE_H
