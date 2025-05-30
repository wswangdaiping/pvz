// 僵尸信息场景类的实现文件，负责显示僵尸信息场景的界面和处理相关交互

#include "ZombieInfoScene.h"
#include "MainView.h"
#include "SelectorScene.h"

// 僵尸信息场景构造函数，初始化场景的背景、退出按钮和背景音乐等
ZombieInfoScene::ZombieInfoScene()
    : QGraphicsScene(0, 0, 900, 600),
      background(new QGraphicsPixmapItem(gImageCache->load("interface/Almanac_ZombieBack.jpg"))),//暂时用的背景
      exitButton(new MouseEventPixmapItem(gImageCache->load("interface/Button.png"))),//暂时的退出按钮
      backgroundMusic(new QMediaPlayer(this))
{
    // 添加背景到场景
    addItem(background);

    // 设置退出按钮位置
    exitButton->setPos(375, 555);
    exitButton->setCursor(Qt::PointingHandCursor);
    // 添加退出按钮到场景
    addItem(exitButton);

    // 设置背景音乐并播放
    backgroundMusic->setMedia(QUrl("qrc:/audio/Faster.mp3"));
    backgroundMusic->play();

    // 连接退出按钮的点击信号到返回选择场景的槽函数
    connect(exitButton, &MouseEventPixmapItem::clicked, this, &ZombieInfoScene::backToSelector);
}

// 僵尸信息场景析构函数，释放相关资源
ZombieInfoScene::~ZombieInfoScene()
{
    delete background;
    delete exitButton;
    delete backgroundMusic;
}

// 返回选择场景的处理函数
void ZombieInfoScene::backToSelector()
{
    // 阻止背景音乐的信号，停止播放
    backgroundMusic->blockSignals(true);
    backgroundMusic->stop();
    backgroundMusic->blockSignals(false);

    // 切换到选择场景
    gMainView->switchToScene(new SelectorScene);
}
