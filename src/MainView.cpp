// 主视图类的实现文件，负责管理主视图的初始化、场景切换和尺寸调整等操作

#include "MainView.h"
#include "GameLevelData.h"
#include "SelectorScene.h"
#include "GameScene.h"
#include "AspectRatioLayout.h"

// 全局主视图指针
MainView *gMainView;

// 主视图构造函数，初始化主视图的基本属性和用户名
MainView::MainView(MainWindow *mainWindow)
        : width(900), height(600),
          usernameSettingEntry("Global/Username"),
          mainWindow(mainWindow)
{
    gMainView = this;

    // 启用鼠标跟踪
    setMouseTracking(true);

    // 设置渲染提示，提高渲染质量
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::TextAntialiasing, true);
    setRenderHint(QPainter::SmoothPixmapTransform, true);

    // 设置框架样式，隐藏滚动条
    setFrameStyle(0);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMinimumSize(width, height);

    // 设置用户名
    if (getUsername().isEmpty()) {
        QString username = qgetenv("USER"); // Linux or Mac
        if (username.isEmpty())
            username = qgetenv("USERNAME"); // Windows
        if (username.isEmpty())
            username = tr("Guest");
        setUsername(username);
    }
}

// 主视图析构函数，释放场景资源
MainView::~MainView()
{
    if (scene())
        scene()->deleteLater();
}

// 获取用户名
QString MainView::getUsername() const
{
    return QSettings().value(usernameSettingEntry, "").toString();
}

// 设置用户名
void MainView::setUsername(const QString &username)
{
    return QSettings().setValue(usernameSettingEntry, username);
}

// 获取主窗口指针
MainWindow *MainView::getMainWindow() const
{
    return mainWindow;
}

// 切换到指定场景
void MainView::switchToScene(QGraphicsScene *scene)
{
    QGraphicsScene *oldScene = nullptr;
    if (this->scene())
        oldScene = this->scene();
    setScene(scene);
    if (oldScene)
        oldScene->deleteLater();
}

// 处理主视图的尺寸调整事件
void MainView::resizeEvent(QResizeEvent *event)
{
    // `fitInView` has a bug causing extra margin.
    // see "https://bugreports.qt.io/browse/QTBUG-11945"
    QRectF viewRect = frameRect();
    QTransform trans;
    trans.scale(viewRect.width() / width, viewRect.height() / height);
    setTransform(trans);
}

// 主窗口构造函数，初始化主窗口的布局、全屏操作和背景颜色
MainWindow::MainWindow()
    : fullScreenSettingEntry("UI/FullScreen"),
      mainView(new MainView(this)),
      fullScreenAction(new QAction)
{
    // 布局设置
    QWidget *centralWidget = new QWidget;
    QLayout *layout = new AspectRatioLayout;
    layout->addWidget(mainView);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    // 全屏操作，使用 F11 键触发
    fullScreenAction->setCheckable(true);
    fullScreenAction->setShortcut(Qt::Key_F11);
    addAction(fullScreenAction);
    connect(fullScreenAction, &QAction::toggled, [this] (bool checked) {
        if (checked)
            setWindowState(Qt::WindowFullScreen);
        else
            setWindowState(Qt::WindowNoState);
        QSettings().setValue(fullScreenSettingEntry, checked);
    });

    // Buggy on windows
    //fullScreenAction->setChecked(QSettings().value(fullScreenSettingEntry, false).toBool());

    // 设置背景颜色为黑色
    setPalette(Qt::black);
    setAutoFillBackground(true);

    // 调整窗口大小
    adjustSize();
}

// 获取全屏操作的动作指针
QAction *MainWindow::getFullScreenAction() const
{
    return fullScreenAction;
}
