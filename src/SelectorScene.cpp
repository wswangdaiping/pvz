// 选择场景类的实现文件，负责显示选择场景的界面和处理相关交互

#include "SelectorScene.h"
#include "MainView.h"
#include "MouseEventPixmapItem.h"
#include "ImageManager.h"
#include "GameLevelData.h"
#include "GameScene.h"
#include "ZombieInfoScene.h"

// 无边界文本项类的绘制函数，去除选中和焦点状态的边框
TextItemWithoutBorder::TextItemWithoutBorder(const QString &text, QGraphicsItem *parent)
        : QGraphicsTextItem(text, parent)
{}

void TextItemWithoutBorder::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyleOptionGraphicsItem myOption(*option);
    myOption.state &= ~(QStyle::State_Selected | QStyle::State_HasFocus);
    QGraphicsTextItem::paint(painter, &myOption, widget);
}

// 选择场景构造函数，初始化场景的背景、按钮、文本和背景音乐等
SelectorScene::SelectorScene()
        : QGraphicsScene(0, 0, 900, 600),
          background      (new QGraphicsPixmapItem    (gImageCache->load("interface/SelectorBackground.png"))),
          adventureShadow (new QGraphicsPixmapItem    (gImageCache->load("interface/SelectorAdventureShadow.png"))),
          adventureButton (new HoverChangedPixmapItem (gImageCache->load("interface/SelectorAdventureButton.png"))),
          survivalShadow  (new QGraphicsPixmapItem    (gImageCache->load("interface/SelectorSurvivalShadow.png"))),
          bookButton  (new HoverChangedPixmapItem (gImageCache->load("interface/bookButton.png"))),
          //challengeShadow (new QGraphicsPixmapItem    (gImageCache->load("interface/SelectorChallengeShadow.png"))),
         // challengeButton (new HoverChangedPixmapItem (gImageCache->load("interface/SelectorChallengeButton.png"))),
          woodSign1       (new QGraphicsPixmapItem    (gImageCache->load("interface/SelectorWoodSign1.png"))),
          woodSign2       (new QGraphicsPixmapItem    (gImageCache->load("interface/SelectorWoodSign2.png"))),
          woodSign3       (new QGraphicsPixmapItem    (gImageCache->load("interface/SelectorWoodSign3.png"))),
          zombieHand      (new MoviePixmapItem        ("interface/SelectorZombieHand.gif")),
          quitButton      (new MouseEventRectItem     (QRectF(0, 0, 79, 53))),
          usernameText    (new TextItemWithoutBorder  (gMainView->getUsername())),
          backgroundMusic(new QMediaPlayer(this))
{
    // 添加背景到场景
    addItem(background);

    // 设置退出按钮无边框
    quitButton      ->setPen(Qt::NoPen);

    // 设置按钮的鼠标指针样式
    adventureButton ->setCursor(Qt::PointingHandCursor);
    bookButton  ->setCursor(Qt::PointingHandCursor);
    //challengeButton ->setCursor(Qt::PointingHandCursor);
    quitButton      ->setCursor(Qt::PointingHandCursor);

    // 设置按钮和标志的位置并添加到场景
    adventureShadow ->setPos(468, 82);  addItem(adventureShadow);
    adventureButton ->setPos(474, 80);  addItem(adventureButton);
    survivalShadow  ->setPos(476, 208); addItem(survivalShadow);
    bookButton  ->setPos(474, 203); addItem(bookButton);
    //challengeShadow ->setPos(480, 307); addItem(challengeShadow);
    //challengeButton ->setPos(478, 303); addItem(challengeButton);
    quitButton      ->setPos(800, 495); addItem(quitButton);
    woodSign1       ->setPos(20, -8);   addItem(woodSign1);
    woodSign2       ->setPos(23, 126);  addItem(woodSign2);
    woodSign3       ->setPos(34, 179);  addItem(woodSign3);
    zombieHand      ->setPos(262, 264); addItem(zombieHand);

    // 设置用户名文本的父项、位置、文本宽度和样式
    usernameText->setParentItem(woodSign1);
    usernameText->setPos(35, 91);
    usernameText->setTextWidth(230);
    usernameText->document()->setDocumentMargin(0);
    usernameText->document()->setDefaultTextOption(QTextOption(Qt::AlignCenter));
    usernameText->setDefaultTextColor(QColor::fromRgb(0xf0c060));
    usernameText->setFont(QFont("Microsoft YaHei", 14, QFont::Bold));

    // 安装事件过滤器，处理用户名文本的输入
    usernameText->installEventFilter(this);
    usernameText->setTextInteractionFlags(Qt::TextEditorInteraction);

    // 设置背景音乐并连接循环播放信号
    backgroundMusic->setMedia(QUrl("qrc:/audio/Faster.mp3"));
    connect(backgroundMusic, &QMediaPlayer::stateChanged, [this](QMediaPlayer::State state) {
        if (state == QMediaPlayer::StoppedState)
            backgroundMusic->play();
    });

    // 连接按钮的悬停信号到播放音效
    connect(adventureButton, &HoverChangedPixmapItem::hoverEntered, [] { QSound::play(":/audio/bleep.wav"); });
    connect(bookButton, &HoverChangedPixmapItem::hoverEntered, [] { QSound::play(":/audio/bleep.wav"); });
    //connect(challengeButton, &HoverChangedPixmapItem::hoverEntered, [] { QSound::play(":/audio/bleep.wav"); });

    // 连接冒险按钮的点击信号到僵尸手动画和场景切换
    connect(adventureButton, &HoverChangedPixmapItem::clicked, zombieHand, [this] {
        adventureButton->setCursor(Qt::ArrowCursor);
        bookButton->setCursor(Qt::ArrowCursor);
        //challengeButton->setCursor(Qt::ArrowCursor);
        woodSign3->setCursor(Qt::ArrowCursor);
        adventureButton->setEnabled(false);
        bookButton->setEnabled(false);
        //challengeButton->setEnabled(false);
        woodSign3->setEnabled(false);

        zombieHand->start();
        backgroundMusic->blockSignals(true);
        backgroundMusic->stop();
        backgroundMusic->blockSignals(false);
        backgroundMusic->setMedia(QUrl("qrc:/audio/losemusic.mp3"));
        backgroundMusic->play();
    });

    // 连接书本按钮的点击信号到僵尸信息场景切换
    connect(bookButton, &HoverChangedPixmapItem::clicked, [this] {
           backgroundMusic->blockSignals(true);
           backgroundMusic->stop();
           backgroundMusic->blockSignals(false);
           gMainView->switchToScene(new ZombieInfoScene);
       });

    // 连接僵尸手动画的结束信号到游戏场景切换
    connect(zombieHand, &MoviePixmapItem::finished, [this] {
        (new Timer(this, 2500, [this](){
            backgroundMusic->blockSignals(true);
            backgroundMusic->stop();
            backgroundMusic->blockSignals(false);
            gMainView->switchToScene(new GameScene(GameLevelDataFactory(QSettings().value("Global/NextLevel", "1").toString())));
        }))->start();
    });

    // 连接退出按钮的点击信号到关闭主窗口
    connect(quitButton, &MouseEventRectItem::clicked, [] {
        gMainView->getMainWindow()->close();
    });

    // 加载完成后的处理
    loadReady();
}

// 事件过滤器，处理用户名文本的回车键输入
bool SelectorScene::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == usernameText) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
                // 保存用户名
                gMainView->setUsername(usernameText->toPlainText());
                setFocusItem(nullptr);
                return true;
            }
            return false;
        }
        return false;
    }
    return QGraphicsScene::eventFilter(watched, event);
}

// 加载完成后的处理，设置主窗口标题并播放背景音乐
void SelectorScene::loadReady()
{
    // Animation is so UGLY.
    //moveItemWithDuration(woodSign1, QPointF(20, -8), 400, [] {}, QTimeLine::EaseOutCurve);
    //moveItemWithDuration(woodSign2, QPointF(23, 126), 500, [] {}, QTimeLine::EaseOutCurve);
    //moveItemWithDuration(woodSign3, QPointF(34, 179), 600, [] {}, QTimeLine::EaseOutCurve);
    gMainView->getMainWindow()->setWindowTitle(tr("Plants vs. Zombies"));
    backgroundMusic->play();
}
