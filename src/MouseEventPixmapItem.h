#ifndef PLANTS_VS_ZOMBIES_MOUSEEVENTPIXMAPITEM_H
#define PLANTS_VS_ZOMBIES_MOUSEEVENTPIXMAPITEM_H

#include <QtWidgets>

/**
 * @brief 支持鼠标事件的矩形图元类
 * 继承QObject和QGraphicsRectItem，提供完整的鼠标交互功能
 */
class MouseEventRectItem: public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    MouseEventRectItem();
    MouseEventRectItem(const QRectF &rect);

protected:
    // 鼠标事件处理
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

signals:
    // 交互信号
    void clicked(QGraphicsSceneMouseEvent *event);       // 点击信号
    void hoverEntered(QGraphicsSceneHoverEvent *event); // 悬停进入
    void hoverMoved(QGraphicsSceneHoverEvent *event);   // 悬停移动
    void hoverLeft(QGraphicsSceneHoverEvent *event);    // 悬停离开
};

/**
 * @brief 支持鼠标事件的位图图元类
 * 继承QObject和QGraphicsPixmapItem，提供完整的鼠标交互功能
 */
class MouseEventPixmapItem: public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    MouseEventPixmapItem();
    MouseEventPixmapItem(const QPixmap &image);

protected:
    // 鼠标事件处理（与MouseEventRectItem相同接口）
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

signals:
    // 交互信号（与MouseEventRectItem相同）
    void clicked(QGraphicsSceneMouseEvent *event);
    void hoverEntered(QGraphicsSceneHoverEvent *event);
    void hoverMoved(QGraphicsSceneHoverEvent *event);
    void hoverLeft(QGraphicsSceneHoverEvent *event);
};

/**
 * @brief 悬停状态切换的位图图元
 * 继承自MouseEventPixmapItem，增加悬停状态图像切换功能
 */
class HoverChangedPixmapItem: public MouseEventPixmapItem
{
    Q_OBJECT
public:
    explicit HoverChangedPixmapItem(const QPixmap &image);

private:
    QPixmap origImage;  // 原始图像
    QPixmap hoverImage; // 悬停状态图像
};

/**
 * @brief 支持动画播放的位图图元
 * 集成QMovie实现动画播放功能
 */
class MoviePixmapItem: public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    MoviePixmapItem();
    explicit MoviePixmapItem(const QString &filename);
    ~MoviePixmapItem() override;

    // 动画控制接口
    void setMovie(const QString &filename);  // 设置动画文件
    void setMovieOnNewLoop(const QString &filename,
                          std::function<void(void)> functor = [] {}); // 带回调的动画设置

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

signals:
    void click(QGraphicsSceneMouseEvent *event); // 点击信号
    void loopStarted();  // 动画循环开始
    void finished();     // 动画播放结束

public slots:
    void start();  // 开始播放
    void stop();   // 停止播放
    void reset();  // 重置动画

private:
    QMovie *movie;  // Qt动画控制器
};

#endif // PLANTS_VS_ZOMBIES_MOUSEEVENTPIXMAPITEM_H
