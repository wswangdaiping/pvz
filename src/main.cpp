// 主程序入口文件，负责初始化应用程序、设置相关参数并启动主窗口
// 同时处理图像管理器的初始化和销毁操作

#include <QtCore>
#include <QtWidgets>
#include "MainView.h"
#include "SelectorScene.h"
#include "ImageManager.h"

int main(int argc, char * *argv)
{
    // 创建 Qt 应用程序实例
    QApplication app(argc, argv);

    // 设置组织名称、域名和应用程序名称，用于 QSettings
    QCoreApplication::setOrganizationName("Sun Ziping");
    QCoreApplication::setOrganizationDomain("sunziping.com");
    QCoreApplication::setApplicationName("Plants vs Zombies");

    // 加载翻译文件，用于多语言支持
    QTranslator appTranslator;
    // TODO: change translation back after debugging
    // 调试时固定加载中文翻译文件
    appTranslator.load(QString(":/translations/main.%1.qm").arg("zh_CN"));
    app.installTranslator(&appTranslator);

    // 初始化图像管理器
    InitImageManager();

    // 初始化随机数种子
    qsrand((uint) QTime::currentTime().msec());

    // 创建主窗口实例
    MainWindow mainWindow;

    // 切换到选择场景
    gMainView->switchToScene(new SelectorScene);

    // 设置主窗口标题
    mainWindow.setWindowTitle("121植物大战僵尸");

    // 显示主窗口
    mainWindow.show();

    // 进入应用程序事件循环
    int res = app.exec();

    // 销毁图像管理器
    DestoryImageManager();

    return res;
}
