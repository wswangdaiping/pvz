#ifndef PLANTS_VS_ZOMBIES_MAINVIEW_H
#define PLANTS_VS_ZOMBIES_MAINVIEW_H

#include <QtWidgets>  // 包含Qt Widgets模块
#include "Timer.h"    // 包含自定义计时器头文件

// 前向声明相关类
class SelectorScene;
class GameScene;
class MainWindow;

/**
 * @brief 主视图类（继承自QGraphicsView）
 * 负责管理游戏场景的显示和切换
 */
class MainView : public QGraphicsView
{
    Q_OBJECT  // Qt元对象系统宏

public:
    MainView(MainWindow *mainWindow);  // 构造函数，需传入主窗口指针
    ~MainView();  // 析构函数

    // 用户管理接口
    QString getUsername() const;       // 获取当前用户名
    void setUsername(const QString &username);  // 设置用户名
    MainWindow *getMainWindow() const; // 获取关联的主窗口指针

    // 场景管理
    void switchToScene(QGraphicsScene *scene);  // 切换当前显示的场景

protected:
    // 重写父类事件处理
    virtual void resizeEvent(QResizeEvent *event) override;  // 窗口大小调整事件处理

private:
    const int width, height;           // 视图固定尺寸
    const QString usernameSettingEntry; // 用户名配置项键名

    MainWindow *mainWindow;  // 指向主窗口的指针
};

/**
 * @brief 主窗口类（继承自QMainWindow）
 * 包含游戏主界面和全屏控制功能
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();  // 构造函数

    QAction *getFullScreenAction() const;  // 获取全屏控制Action

private:
    const QString fullScreenSettingEntry;  // 全屏配置项键名

    QGraphicsView *mainView;    // 主视图指针
    QAction *fullScreenAction;  // 全屏操作Action
};

// 全局主视图指针（供其他模块访问）
extern MainView *gMainView;

#endif //PLANTS_VS_ZOMBIES_MAINVIEW_H
