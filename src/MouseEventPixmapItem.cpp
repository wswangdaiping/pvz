// 鼠标事件图形项类的实现文件，负责处理鼠标事件和图形项的交互

#include "MouseEventPixmapItem.h"

// 鼠标事件矩形项构造函数，启用悬停事件
MouseEventRectItem::MouseEventRectItem()
{
    setAcceptHoverEvents(true);
}

// 鼠标事件矩形项构造函数，根据矩形区域创建并启用悬停事件
MouseEventRectItem::MouseEventRectItem(const QRectF &rect) : QGraphicsRectItem(rect)
{
    setAcceptHoverEvents(true);
}

// 鼠标按下事件处理，发出点击信号
void MouseEventRectItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit clicked(event);
}

// 鼠标悬停进入事件处理，发出悬停进入信号
void MouseEventRectItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    emit hoverEntered(event);
}

// 鼠标悬停离开事件处理，发出悬停离开信号
void MouseEventRectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    emit hoverLeft(event);
}

// 鼠标悬停移动事件处理，发出悬停移动信号
void MouseEventRectItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    emit hoverMoved(event);
}

// 鼠标事件像素图项构造函数，启用悬停事件
MouseEventPixmapItem::MouseEventPixmapItem()
{
    setAcceptHoverEvents(true);
}

// 鼠标事件像素图项构造函数，根据图像创建并启用悬停事件
MouseEventPixmapItem::MouseEventPixmapItem(const QPixmap &image) : QGraphicsPixmapItem(image)
{
    setAcceptHoverEvents(true);
}

// 鼠标按下事件处理，发出点击信号
void MouseEventPixmapItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit clicked(event);
}

// 鼠标悬停进入事件处理，发出悬停进入信号
void MouseEventPixmapItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    emit hoverEntered(event);
}

// 鼠标悬停离开事件处理，发出悬停离开信号
void MouseEventPixmapItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    emit hoverLeft(event);
}

// 鼠标悬停移动事件处理，发出悬停移动信号
void MouseEventPixmapItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    emit hoverMoved(event);
}

// 悬停变化像素图项构造函数，初始化原始图像和悬停图像，并连接悬停信号到图像切换
HoverChangedPixmapItem::HoverChangedPixmapItem(const QPixmap &image) : origImage(image.copy(0, 0, image.width(), image.height() / 2)),
                                                                                 hoverImage(image.copy(0, image.height() / 2, image.width(), image.height() / 2))
{
    setPixmap(origImage);
    setAcceptHoverEvents(true);
    connect(this, &HoverChangedPixmapItem::hoverEntered, [this] { setPixmap(hoverImage); });
    connect(this, &HoverChangedPixmapItem::hoverLeft, [this] { setPixmap(origImage); });
}

// 电影像素图项构造函数，根据文件名初始化电影
MoviePixmapItem::MoviePixmapItem(const QString &filename)
        : movie(nullptr)
{
    setMovie(filename);
}

// 电影像素图项构造函数，默认构造
MoviePixmapItem::MoviePixmapItem()
        : movie(nullptr)
{}

// 电影像素图项析构函数，释放电影资源
MoviePixmapItem::~MoviePixmapItem()
{
    if (movie) {
        if (movie->state() == QMovie::Running)
            movie->stop();
        delete movie;
    }
}

// 设置电影，根据文件名加载电影并连接帧变化和结束信号
void MoviePixmapItem::setMovie(const QString &filename)
{
    if (movie) {
        movie->stop();
        delete movie;
    }
    movie = new QMovie(":/images/" + filename);
    movie->jumpToFrame(0);
    setPixmap(movie->currentPixmap());
    connect(movie, &QMovie::frameChanged, [this](int i){
        setPixmap(movie->currentPixmap());
        if (i == 0)
            emit loopStarted();
    });
    connect(movie, &QMovie::finished, [this]{ emit finished(); });
}

// 开始播放电影
void MoviePixmapItem::start()
{
    movie->start();
}

// 停止播放电影
void MoviePixmapItem::stop()
{
    movie->stop();
}

// 重置电影到第一帧
void MoviePixmapItem::reset()
{
    movie->jumpToFrame(0);
}

// 鼠标按下事件处理，发出点击信号
void MoviePixmapItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit click(event);
}

// 设置电影在新循环时的处理，包括切换电影和执行指定函数
void MoviePixmapItem::setMovieOnNewLoop(const QString &filename, std::function<void(void)> functor)
{
    QSharedPointer<QMetaObject::Connection> connection(new QMetaObject::Connection);
    *connection = QObject::connect(this, &MoviePixmapItem::loopStarted, [this, connection, filename, functor] {
        setMovie(filename);
        QObject::disconnect(*connection);
        start();
        functor();
    });
}
