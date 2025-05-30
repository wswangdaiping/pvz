#include <QtAlgorithms>
#include <QtMultimedia>
#include <stdlib.h>
#include <math.h>
#include "GameScene.h"
#include "MainView.h"
#include "ImageManager.h"
#include "Timer.h"
#include "Plant.h"
#include "Zombie.h"
#include "GameLevelData.h"
#include "MouseEventPixmapItem.h"
#include "PlantCardItem.h"
#include "Animate.h"
#include "SelectorScene.h"

GameScene::GameScene(GameLevelData *gameLevelData)
        : QGraphicsScene(0, 0, 900, 600),  // 场景尺寸：900x600像素
          gameLevelData(gameLevelData),  // 关联关卡数据
          // 背景与资源加载（使用QGraphicsPixmapItem显示图片）
          background(new QGraphicsPixmapItem(gImageCache->load(gameLevelData->backgroundImage))),
          gameGroup(new QGraphicsItemGroup),  // 游戏主体组（植物、僵尸等）
          // 信息显示组件
          infoText(new QGraphicsSimpleTextItem),
          infoTextGroup(new QGraphicsRectItem(0, 0, 900, 50)),
          // 菜单与交互按钮
          menuGroup(new MouseEventPixmapItem(gImageCache->load("interface/Button.png"))),
          // 阳光数值显示
          sunNumText(new QGraphicsSimpleTextItem(QString::number(gameLevelData->sunNum))),
          sunNumGroup(new QGraphicsPixmapItem(gImageCache->load("interface/SunBack.png"))),
          // 植物卡片选择按钮
          selectCardButtonReset(new MouseEventPixmapItem(gImageCache->load("interface/SelectCardButton.png"))),
          selectCardButtonOkay(new MouseEventPixmapItem(gImageCache->load("interface/SelectCardButton.png"))),
          selectCardTextReset(new QGraphicsSimpleTextItem(tr("Reset"))),
          selectCardTextOkay(new QGraphicsSimpleTextItem(tr("Go"))),
          // 选卡面板与卡片组
          selectingPanel(new QGraphicsPixmapItem(gImageCache->load("interface/SeedChooser_Background.png"))),
          cardPanel(new QGraphicsItemGroup),
          // 铲子与移动植物相关组件
          shovel(new QGraphicsPixmapItem(gImageCache->load("interface/Shovel.png"))),
          shovelBackground(new QGraphicsPixmapItem(gImageCache->load("interface/ShovelBack.png"))),
          movePlantAlpha(new QGraphicsPixmapItem),
          movePlant(new QGraphicsPixmapItem),
          // 植物生长动画
          imgGrowSoil(new MoviePixmapItem("interface/GrowSoil.gif")),
          imgGrowSpray(new MoviePixmapItem("interface/GrowSpray.gif")),
          // 波次进度条与胜负图片
          flagMeter(new FlagMeter(gameLevelData)),
          losePicture(new QGraphicsPixmapItem),
          winPicture(new QGraphicsPixmapItem),
          sunGroup(new QGraphicsItemGroup),  // 阳光组（管理所有阳光对象）
          // 音频与游戏状态变量
          backgroundMusic(new QMediaPlayer(this)),
          coordinate(gameLevelData->coord),
          choose(0), sunNum(gameLevelData->sunNum),
          waveTimer(nullptr), monitorTimer(new QTimer(this)), waveNum(0)
{
    // 注册植物原型（通过工厂模式创建实例）
    for (const auto &eName: gameLevelData->pName)
        plantProtoTypes.insert(eName, PlantFactory(this, eName));
    // 注册僵尸原型
    for (const auto &eName: gameLevelData->zName)
        zombieProtoTypes.insert(eName, ZombieFactory(this, eName));
    // z-value -- 0: normal 1: tooltip 2: dialog
    // Background (parent of the zombies displayed on the road)
    // 添加背景到场景
    addItem(background);
    // 若显示滚动条，加载预览僵尸（用于关卡开始前显示）
    if (gameLevelData->showScroll) {
        QList<qreal> yPos;
        QList<Zombie *> zombies;
        // 遍历僵尸配置，生成预览对象
        for (const auto &zombieData: gameLevelData->zombieData) {
            Zombie *item = getZombieProtoType(zombieData.eName);
            if(item->canDisplay) {
                for (int i = 0; i < zombieData.num; ++i) {
                    yPos.push_back(qFloor(100 +  qrand() % 400));  // 随机Y坐标
                    zombies.push_back(item);
                }
            }
        }
        // 排序Y坐标并随机打乱僵尸顺序
        qSort(yPos.begin(), yPos.end());
        std::random_shuffle(zombies.begin(), zombies.end());
        // 创建僵尸动画并添加到背景
        for (int i = 0; i < zombies.size(); ++i) {
            MoviePixmapItem *pixmap = new MoviePixmapItem(zombies[i]->standGif);
            QSizeF size = pixmap->boundingRect().size();
            // 右侧随机位置显示预览
            pixmap->setPos(qFloor(1115 + qrand() % 200) - size.width() * 0.5, yPos[i] - size.width() * 0.5);
            pixmap->setParentItem(background);
        }
    }
    // 游戏组（包含植物、僵尸等动态对象，不处理子事件）
    gameGroup->setHandlesChildEvents(false);
    addItem(gameGroup);
    // 信息文本（显示关卡提示等）
    infoText->setBrush(Qt::white);
    infoText->setFont(QFont("SimHei", 16, QFont::Bold));
    infoText->setParentItem(infoTextGroup);
    infoTextGroup->setPos(0, 500);  // 底部位置
    infoTextGroup->setPen(Qt::NoPen);
    infoTextGroup->setBrush(QColor::fromRgb(0x5b432e));  // 半透明背景
    infoTextGroup->setOpacity(0);  // 初始隐藏
    addItem(infoTextGroup);
    // Menu
    // 菜单按钮（显示"Back"文本）
    QGraphicsSimpleTextItem *menuText = new QGraphicsSimpleTextItem(tr("Back"));
    menuText->setBrush(QColor::fromRgb(0x00cb08));  // 绿色文本
    menuText->setFont(QFont("SimHei", 12, QFont::Bold));
    // 文本居中显示
    menuText->setPos(sizeToPoint(menuGroup->boundingRect().size() - menuText->boundingRect().size()) / 2);
    menuText->setParentItem(menuGroup);
    // 按钮定位到场景右上角
    menuGroup->setPos(sceneRect().topRight() - sizeToPoint(menuGroup->boundingRect().size()));
    menuGroup->setCursor(Qt::PointingHandCursor);  // 鼠标悬停变手型
    addItem(menuGroup);
    // 点击事件：停止计时器、音乐，返回主菜单
    connect(menuGroup, &MouseEventPixmapItem::clicked, [this] {
        monitorTimer->stop();
        delete monitorTimer;
        backgroundMusic->blockSignals(true);
        backgroundMusic->stop();
        backgroundMusic->blockSignals(false);
        gMainView->switchToScene(new SelectorScene);
    });
    // Sun number
    // 阳光数值文本
    sunNumText->setFont(QFont("Verdana", 16, QFont::Bold));
    QSizeF sunNumTextSize = sunNumText->boundingRect().size();
    // 文本在阳光背景框中居中
    sunNumText->setPos(76 - sunNumTextSize.width() / 2,
                       (sunNumGroup->boundingRect().height() - sunNumTextSize.height()) / 2);
    sunNumText->setParentItem(sunNumGroup);
    sunNumGroup->setPos(100, 560);  // 底部阳光显示位置
    sunNumGroup->setVisible(false);  // 初始隐藏
    addItem(sunNumGroup);
    // Select Card
    if (gameLevelData->canSelectCard && gameLevelData->maxSelectedCards > 0) {
        // 选卡面板标题

        QGraphicsSimpleTextItem *selectCardTitle = new QGraphicsSimpleTextItem(tr("Choose your cards"));
        selectCardTitle->setBrush(QColor::fromRgb(0xf0c060));
        selectCardTitle->setFont(QFont("NSimSun", 12, QFont::Bold));
        QSizeF selectCardTitleSize = selectCardTitle->boundingRect().size();
        selectCardTitle->setPos((selectingPanel->boundingRect().width() - selectCardTitleSize.width()) / 2,
                                15 - selectCardTitleSize.height() / 2);
        selectCardTitle->setParentItem(selectingPanel);

        // 重置按钮（Reset）
        selectCardTextReset->setBrush(QColor::fromRgb(0x808080));  // 灰色文本（未激活状态）
        selectCardTextReset->setFont(QFont("SimHei", 12, QFont::Bold));
        selectCardTextReset->setParentItem(selectCardButtonReset);
        selectCardButtonReset->setPos(162, 500);
        selectCardButtonReset->setEnabled(false);  // 初始禁用
        selectCardButtonReset->setParentItem(selectingPanel);

        // 确认按钮（Go）
        selectCardTextOkay->setBrush(QColor::fromRgb(0x808080));
        selectCardTextOkay->setFont(QFont("SimHei", 12, QFont::Bold));
        selectCardTextOkay->setParentItem(selectCardButtonOkay);
        selectCardButtonOkay->setPos(237, 500);
        selectCardButtonOkay->setEnabled(false);
        selectCardButtonOkay->setParentItem(selectingPanel);

        // 遍历可选择的植物卡片
        int cardIndex = 0;
        for (auto item: plantProtoTypes.values()) {
            if (!item->canSelect) continue;  // 跳过不可选植物

            // 创建植物卡片项
            PlantCardItem *plantCardItem = new PlantCardItem(item, true);
            plantCardItem->setPos(15 + cardIndex % 6 * 72, 40 + cardIndex / 6 * 50);  // 网格布局
            plantCardItem->setCursor(Qt::PointingHandCursor);
            plantCardItem->setParentItem(selectingPanel);

            // 卡片提示框（显示植物信息）
            QString tooltipText = "<b>" + item->cName + "</b><br />" +
                    QString(tr("Cool down: %1s")).arg(item->coolTime) + "<br />";
            if (gameLevelData->dKind != 0 && item->night)
                tooltipText += "<span style=\"color:#F00\">" + tr("Nocturnal - sleeps during day") + "</span><br>";
            tooltipText += item->toolTip;
            TooltipItem *tooltipItem = new TooltipItem(tooltipText);
            tooltipItem->setVisible(false);
            tooltipItem->setOpacity(0.9);
            tooltipItem->setZValue(1);  // 提示框层级
            addItem(tooltipItem);

            // 鼠标悬停事件：显示/移动提示框
            QPointF posDelta(5, 15);
            connect(plantCardItem, &PlantCardItem::hoverEntered, [tooltipItem, posDelta](QGraphicsSceneHoverEvent *event) {
                tooltipItem->setPos(event->scenePos() + posDelta);
                tooltipItem->setVisible(true);
            });
            connect(plantCardItem, &PlantCardItem::hoverMoved, [tooltipItem, posDelta](QGraphicsSceneHoverEvent *event) {
                tooltipItem->setPos(event->scenePos() + posDelta);
            });
            connect(plantCardItem, &PlantCardItem::hoverLeft, [tooltipItem, posDelta](QGraphicsSceneHoverEvent *event) {
                tooltipItem->setPos(event->scenePos() + posDelta);
                tooltipItem->setVisible(false);
            });

            // 点击事件：选择卡片并添加到已选组
            connect(plantCardItem, &PlantCardItem::clicked, [this, item, plantCardItem] {
                if (!plantCardItem->isChecked()) return;  // 未选中则跳过
                QSound::play(":/audio/tap.wav");  // 播放点击音效
                int count = selectedPlantArray.size();
                // 检查是否达到最大选卡数量
                if (this->gameLevelData->maxSelectedCards > 0 && count >= this->gameLevelData->maxSelectedCards)
                    return;

                // 取消卡片选中状态，创建已选卡片实例
                plantCardItem->setChecked(false);
                PlantCardItem *selectedPlantCardItem = new PlantCardItem(item, true);
                selectedPlantCardItem->setPos(plantCardItem->scenePos());
                cardPanel->addToGroup(selectedPlantCardItem);
                selectedPlantArray.push_back(item);

                // 激活重置/确认按钮（当有卡片选中时）
                if (count == 0) {
                    selectCardTextReset->setBrush(QColor::fromRgb(0xf0c060));
                    selectCardTextOkay->setBrush(QColor::fromRgb(0xf0c060));
                    selectCardButtonReset->setEnabled(true);
                    selectCardButtonOkay->setEnabled(true);
                    selectCardButtonReset->setCursor(Qt::PointingHandCursor);
                    selectCardButtonOkay->setCursor(Qt::PointingHandCursor);
                }
                // 动画效果：已选卡片移动到选卡栏
                Animate(selectedPlantCardItem, this).move(QPointF(0, 60 * count)).scale(1).speed(1.5).replace().finish();

                // 反选逻辑：点击已选卡片可取消选择
                QSharedPointer<QMetaObject::Connection> deselectConnnection(new QMetaObject::Connection), resetConnnection(new QMetaObject::Connection);
                auto deselectFunctor = [this, item, plantCardItem, selectedPlantCardItem, deselectConnnection, resetConnnection] {
                    disconnect(*deselectConnnection);
                    disconnect(*resetConnnection);
                    cardPanel->removeFromGroup(selectedPlantCardItem);
                    selectedPlantArray.removeOne(item);
                    // 无卡片选中时禁用按钮
                    if (selectedPlantArray.size() == 0) {
                        selectCardTextReset->setBrush(QColor::fromRgb(0x808080));
                        selectCardTextOkay->setBrush(QColor::fromRgb(0x808080));
                        selectCardButtonReset->setEnabled(false);
                        selectCardButtonOkay->setEnabled(false);
                        selectCardButtonReset->setCursor(Qt::ArrowCursor);
                        selectCardButtonOkay->setCursor(Qt::ArrowCursor);
                    }
                    // 动画效果：卡片移回原位置
                    Animate(selectedPlantCardItem, this).move(plantCardItem->scenePos()).scale(0.7).speed(1.5).replace().finish(
                        [plantCardItem, selectedPlantCardItem] {
                            plantCardItem->setChecked(true);
                            delete selectedPlantCardItem;
                    });
                };
                // 连接反选事件与重置按钮事件
                *deselectConnnection = connect(selectedPlantCardItem, &PlantCardItem::clicked, [this, selectedPlantCardItem, deselectFunctor] {
                    QSound::play(":/audio/tap.wav");
                    QList<QGraphicsItem *> selectedCards = cardPanel->childItems();
                    for (int i = qFind(selectedCards, selectedPlantCardItem) - selectedCards.begin() + 1; i != selectedCards.size(); ++i)
                        Animate(selectedCards[i], this).move(QPointF(0, 60 * (i - 1))).speed(1.5).replace().finish();
                    deselectFunctor();
                });
                *resetConnnection = connect(selectCardButtonReset, &MouseEventPixmapItem::clicked, deselectFunctor);
            });
            ++cardIndex;
        }
        // 连接确认/重置按钮的点击音效
        connect(selectCardButtonOkay, &MouseEventPixmapItem::clicked, [this] { QSound::play(":/audio/tap.wav"); });
        connect(selectCardButtonReset, &MouseEventPixmapItem::clicked, [this] { QSound::play(":/audio/tap.wav"); });
        // 选卡面板初始隐藏在场景外（Y=-高度）
        selectingPanel->setPos(100, -selectingPanel->boundingRect().height());
        addItem(selectingPanel);
    }
    // Selected card
    // 已选卡片组（不处理子事件，由父级统一管理交互）
    cardPanel->setHandlesChildEvents(false);
    addItem(cardPanel);

    // 铲子工具（用于铲除植物）
    shovel->setPos(0, -5);  // 相对于背景的位置偏移
    shovel->setCursor(Qt::PointingHandCursor);  // 鼠标悬停变手型
    shovel->setParentItem(shovelBackground);  // 铲子图标作为背景图的子项
    shovelBackground->setPos(235, -100);  // 初始位置在场景外（Y=-100，隐藏）
    shovelBackground->setCursor(Qt::PointingHandCursor);  // 背景图也响应鼠标事件
    shovelBackground->setZValue(1);  // 层级高于背景但低于植物/僵尸
    addToGame(shovelBackground);  // 添加到游戏主体组
    // 移动植物半透明遮罩（选中植物时显示位置预览）
    movePlantAlpha->setOpacity(0.4);  // 40%透明度
    movePlantAlpha->setVisible(false);  // 初始隐藏
    movePlantAlpha->setZValue(30);  // 层级高于普通植物（20-30区间）
    gameGroup->addToGroup(movePlantAlpha);  // 添加到游戏主体组

    // 移动植物实体图片（拖动时显示）
    movePlant->setVisible(false);  // 初始隐藏
    movePlant->setZValue(254);  // 高优先级层级（接近顶层）
    addItem(movePlant);  // 直接添加到场景

    // 植物生长土壤动画
    imgGrowSoil->setVisible(false);  // 初始隐藏
    imgGrowSoil->setZValue(50);  // 中等优先级层级
    gameGroup->addToGroup(imgGrowSoil);  // 添加到游戏主体组

    // 植物生长喷水动画
    imgGrowSpray->setVisible(false);  // 初始隐藏
    imgGrowSpray->setZValue(50);  // 与土壤动画同层级
    gameGroup->addToGroup(imgGrowSpray);  // 添加到游戏主体组
    // Flag progress
    // 波次进度条（显示当前关卡进度）
    flagMeter->setPos(700, 610);  // 底部右侧位置
    addItem(flagMeter);

    // 阳光组（管理所有阳光对象，不处理子事件）
    sunGroup->setHandlesChildEvents(false);
    addItem(sunGroup);

    // 失败图片（僵尸获胜时显示）
    losePicture->setPixmap(gImageCache->load("interface/ZombiesWon.png"));  // 加载失败图片
    // 图片居中显示
    losePicture->setPos(sizeToPoint(sceneRect().size() - losePicture->boundingRect().size()) / 2);
    losePicture->setVisible(false);  // 初始隐藏
    addItem(losePicture);

    // 胜利图片（玩家获胜时显示）
    winPicture->setPixmap(gImageCache->load("interface/trophy.png"));  // 加载胜利图片
    // 图片居中显示
    winPicture->setPos(sizeToPoint(sceneRect().size() - winPicture->boundingRect().size()) / 2);
    winPicture->setVisible(false);  // 初始隐藏
    addItem(winPicture);
    // 连接音乐状态变化事件（循环播放）
    connect(backgroundMusic, &QMediaPlayer::stateChanged, [this](QMediaPlayer::State state) {
        if (state == QMediaPlayer::StoppedState)  // 当音乐停止时
            backgroundMusic->play();  // 重新播放
    });
    // 植物触发区域与僵尸行数据初始化
    for (int i = 0; i <= coordinate.rowCount(); ++i) {
        plantTriggers.push_back(QList<Trigger *>());  // 每行的植物触发区域列表
        zombieRow.push_back(QList<ZombieInstance *>());  // 每行的僵尸实例列表
    }

    loadReady();  // 加载完成，触发场景准备事件
}

GameScene::~GameScene()
{
    // 释放植物触发区域内存
    for (int i = 0; i < coordinate.rowCount(); ++i) {
        for (auto item: plantTriggers[i])
            delete item;
    }

    // 释放原型对象内存（工厂模式创建的实例）
    for (auto i: plantProtoTypes.values())
        delete i;
    for (auto i: zombieProtoTypes.values())
        delete i;

    // 释放游戏实例内存（场景中的实际对象）
    for (auto i: plantInstances)
        delete i;
    for (auto i: zombieInstances)
        delete i;

    // 释放关卡数据内存
    delete gameLevelData;
}

void GameScene::setInfoText(const QString &text)
{
    if (text.isEmpty()) {
        // 文本为空时，淡出信息框（200ms动画）
        Animate(infoTextGroup, this).fade(0).duration(200).finish();
    } else {
        // 更新文本内容并居中显示
        infoText->setText(text);
        infoText->setPos(sizeToPoint(infoTextGroup->boundingRect().size() - infoText->boundingRect().size()) / 2);
        // 淡入信息框（80%透明度，200ms动画）
        Animate(infoTextGroup, this).fade(0.8).duration(200).finish();
    }
}

void GameScene::loadReady()
{
    // 设置窗口标题（游戏名称 + 关卡名称）
    gMainView->getMainWindow()->setWindowTitle(tr("Plants vs. Zombies") + " - " + gameLevelData->cName);

    // 不显示滚动条时调整背景位置
    if (!gameLevelData->showScroll)
        background->setPos(-115, 0);

    // 通知关卡数据加载场景访问权限（可能涉及资源预加载）
    gameLevelData->loadAccess(this);
}

void GameScene::loadAcessFinished()
{
    // 自动选择植物卡片（当禁用选卡或无滚动条时）
    if (!gameLevelData->showScroll || !gameLevelData->canSelectCard) {
        for (auto item: plantProtoTypes.values()) {
            if (item->canSelect) {
                selectedPlantArray.push_back(item);  // 添加可选择植物到已选列表
                if (gameLevelData->maxSelectedCards > 0 && selectedPlantArray.size() >= gameLevelData->maxSelectedCards)
                    break;  // 达到最大选卡数量时停止
            }
        }
    }

    // 显示滚动条的关卡流程
    if (gameLevelData->showScroll) {
        // 设置并播放背景音乐
        backgroundMusic->setMedia(QUrl("qrc:/audio/Look_up_at_the_Sky.mp3"));
        backgroundMusic->play();

        // 显示欢迎信息（玩家用户名）
        setInfoText(QString(tr("%1\' house")).arg(QSettings().value("Global/Username").toString()));

        // 1秒后执行动画序列
        (new Timer(this, 1000, [this]{
            setInfoText("");  // 隐藏欢迎信息

            // 启动背景中预览僵尸的动画
            for (auto zombie: background->childItems())
                static_cast<MoviePixmapItem *>(zombie)->start();

            // 背景向左移动（模拟场景滚动）
            Animate(background, this).move(QPointF(-500, 0)).speed(0.5).finish([this] {
                // 菜单按钮从顶部滑入
                Animate(menuGroup, this).move(QPointF(sceneRect().topRight() - QPointF(menuGroup->boundingRect().width(), 0))).speed(0.5).finish();

                // 背景回滚并开始游戏的函数
                auto scrollBack = [this] {
                    Animate(background, this).move(QPointF(-115, 0)).speed(0.5).finish([this] {
                        for (auto zombie: background->childItems())
                            delete zombie;  // 删除预览僵尸
                        letsGo();  // 开始游戏主循环
                    });
                };

                // 可选择卡片的关卡
                if (gameLevelData->canSelectCard) {
                    // 选卡面板从顶部滑入
                    Animate(selectingPanel, this).move(QPointF(100, 0)).speed(3).finish([this] { sunNumGroup->setVisible(true); });

                    // 点击"Go"按钮后的操作
                    connect(selectCardButtonOkay, &MouseEventPixmapItem::clicked, [this, scrollBack] {
                        sunNumGroup->setVisible(false);

                        // 禁用已选卡片的交互
                        for (auto card: cardPanel->childItems()) {
                            card->setCursor(Qt::ArrowCursor);
                            card->setEnabled(false);
                        }

                        // 选卡面板滑出，背景回滚，开始游戏
                        Animate(selectingPanel, this).move(QPointF(100, -selectingPanel->boundingRect().height())).speed(3).finish(scrollBack);
                    });
                }
                // 不可选择卡片的关卡
                else {
                    // 延迟1秒后直接回滚背景并开始游戏
                    (new Timer(this, 1000, scrollBack))->start();
                }
            });
        }))->start();
    }
    // 不显示滚动条的关卡流程
    else {
        // 菜单按钮从顶部滑入
        Animate(menuGroup, this).move(QPointF(sceneRect().topRight() - QPointF(menuGroup->boundingRect().width(), 0))).speed(0.5).finish();
        letsGo();  // 直接开始游戏
    }
}


void GameScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsScene::mouseMoveEvent(mouseEvent);  // 调用父类实现
    emit mouseMove(mouseEvent);  // 转发鼠标移动事件到其他组件
}

void GameScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsScene::mousePressEvent(mouseEvent);  // 调用父类实现
    emit mousePress(mouseEvent);  // 转发鼠标按下事件到其他组件
}

GameLevelData *GameScene::getGameLevelData() const
{
    return gameLevelData;  // 返回当前场景关联的关卡数据
}

void GameScene::letsGo()
{
    // 阳光数值显示框从顶部滑入
    sunNumGroup->setPos(105, -sunNumGroup->boundingRect().height());  // 初始位置在场景外
    sunNumGroup->setVisible(true);
    Animate(sunNumGroup, this).move(QPointF(105, 0)).speed(0.5).finish();  // 0.5秒滑入底部

    // 铲子工具显示（如果关卡允许使用）
    if (gameLevelData->hasShovel)
        Animate(shovelBackground, this).move(QPointF(235, 0)).speed(0.5).finish();  // 铲子滑入场景

    // 已选卡片组显示（如果无需选卡流程）
    if (!gameLevelData->showScroll || !gameLevelData->canSelectCard) {
        cardPanel->setPos(-100, 0);  // 初始位置在场景外
        Animate(cardPanel, this).move(QPointF(0, 0)).speed(0.5).finish();  // 卡片组滑入
    }

    // 清空已选卡片组（重新初始化）
    for (auto i: cardPanel->childItems())
        delete i;

    // 生成已选植物卡片
    for (int i = 0; i < selectedPlantArray.size(); ++i) {
        auto &item = selectedPlantArray[i];  // 当前植物原型

        // 创建植物卡片项
        PlantCardItem *plantCardItem = new PlantCardItem(item);
        plantCardItem->setChecked(false);
        cardPanel->addToGroup(plantCardItem);
        plantCardItem->setPos(0, i * 60);  // 垂直排列卡片

        // 创建卡片提示框
        TooltipItem *tooltipItem = new TooltipItem("");
        tooltipItem->setVisible(false);
        tooltipItem->setOpacity(0.9);
        tooltipItem->setZValue(1);  // 提示框层级
        addItem(tooltipItem);

        // 鼠标悬停事件（显示/移动提示框）
        connect(plantCardItem, &PlantCardItem::hoverEntered, [this, tooltipItem](QGraphicsSceneHoverEvent *event) {
            if (choose) return;  // 正在选择植物时不响应
            tooltipItem->setPos(event->scenePos() + QPointF(5, 15));
            tooltipItem->setVisible(true);
        });
        connect(plantCardItem, &PlantCardItem::hoverMoved, [this, tooltipItem](QGraphicsSceneHoverEvent *event) {
            if (choose) return;
            tooltipItem->setPos(event->scenePos() + QPointF(5, 15));
        });
        connect(plantCardItem, &PlantCardItem::hoverLeft, [this, tooltipItem](QGraphicsSceneHoverEvent *event) {
            if (choose) return;
            tooltipItem->setPos(event->scenePos() + QPointF(5, 15));
            tooltipItem->setVisible(false);
        });

        // 存储卡片与提示框引用
        cardGraphics.push_back({ plantCardItem, tooltipItem });
        cardReady.push_back({ false, false });  // 初始化卡片状态（冷却/阳光）
        updateTooltip(i);  // 更新提示框内容
    }

    // 鼠标按下事件处理（选择植物或铲子）
    connect(this, &GameScene::mousePress, [this](QGraphicsSceneMouseEvent *event) {
        if (choose) return;  // 正在选择中时不响应新点击

        int i;
        // 遍历已选卡片，检查鼠标是否点击在卡片上
        for (i = 0; i < selectedPlantArray.size(); ++i) {
            if (cardReady[i].cool && cardReady[i].sun &&  // 卡片冷却完成且阳光足够
                cardGraphics[i].plantCard->contains(event->scenePos() - cardGraphics[i].plantCard->scenePos()))
                break;
        }

        Plant *item = nullptr;  // 选中的植物原型
        QPointF delta;  // 位置偏移量

        // 情况1：点击了植物卡片
        if (i != selectedPlantArray.size()) {
            cardGraphics[i].tooltip->setVisible(false);  // 隐藏提示框
            item = selectedPlantArray[i];
            QPixmap staticGif = gImageCache->load(item->staticGif);  // 加载植物静态图片
            // 计算拖拽时植物图片的偏移（居中显示）
            delta = QPointF(-0.5 * (item->beAttackedPointL + item->beAttackedPointR), 20 - staticGif.height());
            Animate(movePlant, this).finish();  // 清除之前的动画
            // 设置移动植物的图片
            movePlantAlpha->setPixmap(staticGif);
            movePlant->setPixmap(staticGif);
            movePlant->setPos(event->scenePos() + delta);  // 跟随鼠标位置
            movePlant->setVisible(true);  // 显示植物图片
            QSound::play(":/audio/seedlift.wav");  // 播放拾取音效
            choose = 1;  // 设置选择状态为"选择植物"
        }
        // 情况2：点击了铲子工具
        else if (shovel->contains(event->scenePos() - shovel->scenePos()) || shovelBackground->contains(event->scenePos() - shovelBackground->scenePos())) {
            delta = QPointF(-28, -25);  // 铲子图片偏移量
            Animate(shovel, this).finish();  // 清除之前的动画
            shovel->setCursor(Qt::ArrowCursor);  // 鼠标样式改为箭头
            shovelBackground->setCursor(Qt::ArrowCursor);
            shovel->setPos(event->scenePos() - shovelBackground->scenePos() + delta);  // 跟随鼠标位置
            QSound::play(":/audio/shovel.wav");  // 播放铲子音效
            choose = 2;  // 设置选择状态为"选择铲子"
        }
        else return;  // 其他情况不处理

        // 存储连接对象（用于后续断开连接）
        QSharedPointer<QMetaObject::Connection> moveConnection(new QMetaObject::Connection), clickConnection(new QMetaObject::Connection);
        QSharedPointer<QUuid> uuid(new QUuid);  // 记录当前选中的植物UUID

        // 鼠标移动事件处理（拖拽植物或铲子）
        *moveConnection = connect(this, &GameScene::mouseMove, [this, delta, item, uuid](QGraphicsSceneMouseEvent *e) {
            if (choose == 1) {  // 拖拽植物
                movePlant->setPos(e->scenePos() + delta);  // 植物跟随鼠标
                // 计算植物可种植的格子坐标
                auto xPair = coordinate.choosePlantX(e->scenePos().x()), yPair = coordinate.choosePlantY(e->scenePos().y());
                if (item->canGrow(xPair.second, yPair.second)) {  // 检查是否可种植
                    movePlantAlpha->setVisible(true);  // 显示半透明遮罩
                    // 遮罩定位到格子中心
                    movePlantAlpha->setPos(xPair.first + item->getDX(), yPair.first + item->getDY(xPair.second, yPair.second) - item->height);
                } else {
                    movePlantAlpha->setVisible(false);  // 不可种植时隐藏遮罩
                }
            } else {  // 拖拽铲子
                shovel->setPos(e->scenePos() - shovelBackground->scenePos() + delta);  // 铲子跟随鼠标
                PlantInstance *plant = getPlant(e->scenePos());  // 获取鼠标下的植物
                // 处理植物高亮效果
                if (!uuid->isNull() && (!plant || plant->uuid != *uuid)) {
                    PlantInstance *prevPlant = getPlant(*uuid);
                    if (prevPlant) prevPlant->picture->setOpacity(1.0);  // 取消之前植物的高亮
                }
                if (plant && plant->uuid != *uuid) {
                    plant->picture->setOpacity(0.6);  // 高亮当前植物
                }
                if (plant) {
                    *uuid = plant->uuid;  // 记录当前植物UUID
                    shovel->setCursor(Qt::PointingHandCursor);  // 鼠标样式改为手型
                } else {
                    *uuid = QUuid();  // 无植物时清空UUID
                    shovel->setCursor(Qt::ArrowCursor);  // 鼠标样式改为箭头
                }
            }
        });

        // 鼠标释放事件处理（种植植物或铲除植物）
        *clickConnection = connect(this, &GameScene::mousePress, [this, i, moveConnection, clickConnection, item, uuid](QGraphicsSceneMouseEvent *e) {
            disconnect(*moveConnection);  // 断开移动事件连接
            disconnect(*clickConnection);  // 断开点击事件连接

            if (choose == 1) {  // 种植植物
                movePlantAlpha->setVisible(false);  // 隐藏半透明遮罩
                // 计算植物格子坐标
                auto xPair = coordinate.choosePlantX(e->scenePos().x()), yPair = coordinate.choosePlantY(e->scenePos().y());
                if (e->button() == Qt::LeftButton && item->canGrow(xPair.second, yPair.second)) {  // 左键且可种植
                    movePlant->setVisible(false);  // 隐藏移动植物图片

                    // 播放生长动画（土壤或喷水）
                    MoviePixmapItem *growGif;
                    if (gameLevelData->LF[yPair.second] == 1)
                        growGif = imgGrowSoil;
                    else
                        growGif = imgGrowSpray;
                    growGif->setPos(xPair.first - 30, yPair.first - 30);  // 动画定位到格子中心
                    growGif->setVisible(true);
                    growGif->start();  // 播放动画
                    // 动画结束后隐藏并重置
                    QSharedPointer<QMetaObject::Connection> connection(new QMetaObject::Connection);
                    *connection = connect(growGif, &MoviePixmapItem::finished, [growGif, connection]{
                        growGif->setVisible(false);
                        growGif->reset();
                        disconnect(*connection.data());
                    });

                    // 处理植物位置冲突（替换同位置同类型植物）
                    auto key = qMakePair(xPair.second, yPair.second);
                    if (plantPosition.contains(key) && plantPosition[key].contains(item->pKind))
                        plantDie(plantPosition[key][item->pKind]);  // 移除原有植物

                    // 创建植物实例并添加到场景
                    PlantInstance *plantInstance = PlantInstanceFactory(item);
                    plantInstance->birth(xPair.second, yPair.second);  // 初始化植物位置
                    plantInstances.push_back(plantInstance);
                    if (!plantPosition.contains(key))
                        plantPosition.insert(key, QMap<int, PlantInstance *>());
                    plantPosition[key].insert(item->pKind, plantInstance);  // 记录植物位置
                    plantUuid.insert(plantInstance->uuid, plantInstance);  // 记录植物UUID

                    // 重置卡片冷却时间
                    doCoolTime(i);
                    // 扣除阳光并更新显示
                    sunNum -= item->sunNum;
                    updateSunNum();
                    // 播放种植音效
                    if (qrand() % 2)
                        QSound::play(":/audio/plant1.wav");
                    else
                        QSound::play(":/audio/plant2.wav");
                } else {  // 不可种植时返回卡片位置
                    QSound::play(":/audio/tap.wav");
                    Animate(movePlant, this).move(cardGraphics[i].plantCard->scenePos() + QPointF(10, 0)).speed(1.5).finish([this] {
                        movePlant->setVisible(false);
                    });
                }
            } else {  // 铲除植物
                // 铲子回到初始位置
                Animate(shovel, this).move(QPointF(0, -5)).speed(1.5).finish([this] {
                    shovel->setCursor(Qt::PointingHandCursor);
                    shovelBackground->setCursor(Qt::PointingHandCursor);
                });
                // 取消所有植物高亮
                if (!uuid->isNull()) {
                    PlantInstance *prevPlant = getPlant(*uuid);
                    if (prevPlant) prevPlant->picture->setOpacity(1.0);
                }
                // 左键点击且有植物时铲除
                PlantInstance *plant;
                if (e->button() == Qt::LeftButton && (plant = getPlant(e->scenePos()))) {
                    plantDie(plant);  // 调用植物死亡逻辑
                    QSound::play(":/audio/plant2.wav");  // 播放铲除音效
                } else {
                    QSound::play(":/audio/tap.wav");  // 播放点击音效
                }
            }
            choose = 0;  // 重置选择状态
        });
    });

    // 通知关卡数据开始游戏（生成僵尸等）
    gameLevelData->startGame(this);
}

void GameScene::beginCool()
{
    for (int i = 0; i < selectedPlantArray.size(); ++i) {
        auto &item = selectedPlantArray[i];  // 当前植物原型
        auto &plantCardItem = cardGraphics[i].plantCard;  // 当前卡片UI

        // 处理冷却时间小于7.6秒的植物（直接激活）
        if (item->coolTime < 7.6) {
            plantCardItem->setPercent(1.0);  // 冷却进度100%
            cardReady[i].cool = true;  // 冷却完成
            if (item->sunNum <= sunNum) {  // 阳光足够
                cardReady[i].sun = true;  // 阳光条件满足
                plantCardItem->setChecked(true);  // 卡片激活
            }
            updateTooltip(i);  // 更新提示框
            continue;
        }
        doCoolTime(i);  // 启动冷却计时
    }
}

void GameScene::updateTooltip(int index)
{
    auto &item = selectedPlantArray[index];  // 当前植物原型
    // 构建提示框文本（植物名称、冷却时间、描述）
    QString text = "<b>" + item->cName + "</b><br />" +
                   QString(tr("Cool down: %1s")).arg(item->coolTime) + "<br />";
    text += item->toolTip;  // 植物描述

    // 添加冷却未完成提示（红色文本）
    if (!cardReady[index].cool)
        text += "<br><span style=\"color:#f00\">" + tr("Rechanging...") + "</span>";
    // 添加阳光不足提示（红色文本）
    if (!cardReady[index].sun)
        text += "<br><span style=\"color:#f00\">" + tr("Not enough sun!") + "</span>";

    cardGraphics[index].tooltip->setText(text);  // 更新提示框内容
}

QPair<MoviePixmapItem *, std::function<void(bool)> > GameScene::newSun(int sunNum)
{
    // 创建阳光动画对象
    MoviePixmapItem *sunGif = new MoviePixmapItem("interface/Sun.gif");

    // 根据阳光值调整缩放比例
    if (sunNum == 15)
        sunGif->setScale(46.0 / 79.0);  // 小阳光（15点）
    else if (sunNum != 25)
        sunGif->setScale(100.0 / 79.0);  // 大阳光（非25点，可能是50点）

    // 设置阳光属性
    sunGif->setZValue(2);  // 层级高于背景但低于植物
    sunGif->setOpacity(0.8);  // 80%透明度
    sunGif->setCursor(Qt::PointingHandCursor);  // 鼠标悬停变手型
    sunGroup->addToGroup(sunGif);  // 添加到阳光组

    // 存储定时器与连接对象（用于后续释放）
    QSharedPointer<QTimer *> timer(new QTimer *(nullptr));
    QSharedPointer<QMetaObject::Connection> connection(new QMetaObject::Connection);

    // 点击阳光事件处理
    *connection = connect(sunGif, &MoviePixmapItem::click, [this, sunGif, sunNum, timer] {
        if (choose != 0) return;  // 正在选择时不响应
        if (*timer) delete *timer;  // 清除之前的定时器

        QSound::play(":/audio/points.wav");  // 播放收集音效
        // 阳光移动到阳光数值框并缩放消失
        Animate(sunGif, this).finish().move(QPointF(100, 0)).speed(1).scale(34.0 / 79.0).finish([this, sunGif, sunNum] {
            delete sunGif;  // 销毁阳光对象
            this->sunNum += sunNum;  // 增加阳光数值
            updateSunNum();  // 更新阳光显示
        });
    });

    // 返回阳光对象与完成回调函数
    return qMakePair(sunGif, [this, sunGif, timer, connection](bool finished) {
        if (finished) {
            // 8秒后未收集则自动消失
            (*timer = new Timer(this, 8000, [this, sunGif, connection] {
                disconnect(*connection);  // 断开点击事件连接
                sunGif->setCursor(Qt::ArrowCursor);  // 鼠标样式改为箭头
                // 淡出动画
                Animate(sunGif, this).fade(0).duration(500).finish([sunGif] {
                    delete sunGif;  // 销毁阳光对象
                });
            }))->start();
        }
    });
}


void GameScene::beginSun(int sunNum)
{
    // 创建阳光对象并获取回调函数
    auto sunGifAndOnFinished = newSun(sunNum);
    MoviePixmapItem *sunGif = sunGifAndOnFinished.first;
    std::function<void(bool)> onFinished = sunGifAndOnFinished.second;

    // 随机生成阳光目标位置（格子内）
    double toX = coordinate.getX(1 + qrand() % coordinate.colCount()),
           toY = coordinate.getY(1 + qrand() % coordinate.rowCount());

    // 阳光从场景顶部生成并下落
    sunGif->setPos(toX, -100);  // 初始位置在场景外（顶部）
    sunGif->start();  // 播放阳光动画
    // 下落动画（速度0.04，完成后调用回调）
    Animate(sunGif, this).move(QPointF(toX, toY - 53)).speed(0.04).finish(onFinished);

    // 定时生成下一个阳光（3-12秒随机间隔）
    (new Timer(this, (qrand() % 9000 + 3000), [this, sunNum] { beginSun(sunNum); }))->start();
}

void GameScene::doCoolTime(int index)
{
    auto &item = selectedPlantArray[index];  // 当前植物原型
    auto &plantCardItem = cardGraphics[index].plantCard;  // 当前卡片UI

    // 初始化卡片状态
    plantCardItem->setPercent(0);  // 冷却进度0%
    plantCardItem->setChecked(false);  // 卡片禁用

    // 标记冷却未完成并更新提示框
    if (cardReady[index].cool) {
        cardReady[index].cool = false;
        updateTooltip(index);
    }

    // 创建冷却进度条动画
    (new TimeLine(this, qRound(item->coolTime * 1000), 20, [this, index](qreal x) {
        cardGraphics[index].plantCard->setPercent(x);  // 更新进度条
    }, [this, index] {
        cardReady[index].cool = true;  // 冷却完成
        if (cardReady[index].sun)  // 阳光足够时激活卡片
            cardGraphics[index].plantCard->setChecked(true);
        updateTooltip(index);  // 更新提示框
    }))->start();
}

void GameScene::updateSunNum()
{
    // 更新阳光数字文本并居中显示
    sunNumText->setText(QString::number(sunNum));
    QSizeF sunNumTextSize = sunNumText->boundingRect().size();
    sunNumText->setPos(76 - sunNumTextSize.width() / 2, (sunNumGroup->boundingRect().height() - sunNumTextSize.height()) / 2);

    // 遍历所有卡片，根据阳光值更新卡片状态
    for (int i = 0; i < selectedPlantArray.size(); ++i) {
        auto &item = selectedPlantArray[i];  // 当前植物原型
        auto &plantCardItem = cardGraphics[i].plantCard;  // 当前卡片UI

        if (item->sunNum <= sunNum) {  // 阳光足够
            if (!cardReady[i].sun) {  // 阳光状态未更新时
                cardReady[i].sun = true;  // 标记阳光充足
                updateTooltip(i);  // 更新提示框
            }
            if (cardReady[i].cool)  // 冷却也完成时
                plantCardItem->setChecked(true);  // 激活卡片
        }
        else {  // 阳光不足
            if (cardReady[i].sun) {  // 阳光状态未更新时
                cardReady[i].sun = false;  // 标记阳光不足
                updateTooltip(i);  // 更新提示框
            }
            plantCardItem->setChecked(false);  // 禁用卡片
        }
    }
}

QPointF GameScene::sizeToPoint(const QSizeF &size)
{
    return QPointF(size.width(), size.height());  // 直接转换宽高为坐标点
}

void GameScene::customSpecial(const QString &name, int col, int row)
{
    // 创建植物实例（通过名称查找原型）
    PlantInstance *plantInstance = PlantInstanceFactory(getPlantProtoType(name));
    plantInstance->birth(col, row);  // 初始化植物位置

    // 存储植物实例到管理容器
    plantInstances.push_back(plantInstance);
    auto key = qMakePair(col, row);  // 位置键值对

    // 更新位置映射表
    if (!plantPosition.contains(key))
        plantPosition.insert(key, QMap<int, PlantInstance *>());
    plantPosition[key].insert(plantInstance->plantProtoType->pKind, plantInstance);

    // 更新UUID映射表
    plantUuid.insert(plantInstance->uuid, plantInstance);
}

void GameScene::addToGame(QGraphicsItem *item)
{
    gameGroup->addToGroup(item);  // 将图形项添加到游戏主组
}

void GameScene::beginZombies()
{
    QSound::play(":/audio/awooga.wav");  // 播放警报声（僵尸来袭）

    // 旗帜进度条下移（显示波次进度）
    Animate(flagMeter, this).move(QPointF(700, 560)).speed(0.5).finish();
    advanceFlag();  // 更新旗帜进度

    // 循环播放僵尸呻吟声（随机选择音效）
    QSharedPointer<std::function<void(void)> > playGroan(new std::function<void(void)>);
    *playGroan = [this, playGroan] {
        switch (qrand() % 6) {
            case 0: QSound::play(":/audio/groan1.wav"); break;
            case 1: QSound::play(":/audio/groan2.wav"); break;
            case 2: QSound::play(":/audio/groan3.wav"); break;
            case 3: QSound::play(":/audio/groan4.wav"); break;
            case 4: QSound::play(":/audio/groan5.wav"); break;
            default: QSound::play(":/audio/groan6.wav"); break;
        }
        // 每20秒播放一次
        (new Timer(this, 20000, *playGroan))->start();
    };
    (new Timer(this, 20000, *playGroan))->start();
}

void GameScene::prepareGrowPlants(std::function<void(void)> functor)
{
    // 加载"准备种植植物"三连图并分割为三个图像项
    QPixmap imgPrepareGrowPlants = gImageCache->load("interface/PrepareGrowPlants.png");
    QGraphicsPixmapItem *imgPrepare = new QGraphicsPixmapItem(imgPrepareGrowPlants.copy(0, 0, 255, 108)),
            *imgGrow    = new QGraphicsPixmapItem(imgPrepareGrowPlants.copy(0, 108, 255, 108)),
            *imgPlants  = new QGraphicsPixmapItem(imgPrepareGrowPlants.copy(0, 216, 255, 108));

    // 计算居中位置
    QPointF pos = sizeToPoint(sceneRect().size() - imgPrepare->boundingRect().size()) / 2;
    imgPrepare->setPos(pos);
    imgGrow->setPos(pos);
    imgPlants->setPos(pos);

    // 设置层级和可见性
    imgPrepare->setZValue(1);
    imgGrow->setZValue(1);
    imgPlants->setZValue(1);
    imgPrepare->setVisible(false);
    imgGrow->setVisible(false);
    imgPlants->setVisible(false);

    // 添加到场景
    addItem(imgPrepare);
    addItem(imgGrow);
    addItem(imgPlants);

    // 播放"准备种植"音乐并显示动画序列
    imgPrepare->setVisible(true);
    backgroundMusic->blockSignals(true);
    backgroundMusic->stop();
    backgroundMusic->blockSignals(false);
    backgroundMusic->setMedia(QUrl("qrc:/audio/readysetplant.mp3"));
    backgroundMusic->play();

    // 动画序列：Prepare → Grow → Plants → 执行回调
    (new Timer(this, 600, [this, imgPrepare, imgGrow, imgPlants, functor] {
        delete imgPrepare;
        imgGrow->setVisible(true);
        (new Timer(this, 400, [this, imgGrow, imgPlants, functor] {
            delete imgGrow;
            imgPlants->setVisible(true);
            (new Timer(this, 1200, [this, imgPlants, functor] {
                delete imgPlants;
                functor();  // 执行传入的回调函数（通常是开始游戏）
            }))->start();
        }))->start();
    }))->start();
}
void GameScene::advanceFlag()
{
    ++waveNum;  // 增加当前波次数
    flagMeter->updateFlagZombies(waveNum);  // 更新旗帜进度显示

    // 如果未达到最终波次，继续生成僵尸
    if (waveNum < gameLevelData->flagNum) {
        // 检查当前波次是否有关联的特殊事件
        auto iter = gameLevelData->flagToMonitor.find(waveNum);
        if (iter != gameLevelData->flagToMonitor.end())
            (new Timer(this, 16900, [this, iter] { (*iter)(this); }))->start();  // 延迟触发特殊事件

        // 设置下一波僵尸的生成时间（约20秒后）
        (waveTimer = new Timer(this, 19900, [this] { advanceFlag(); }))->start();
    }

    // 根据当前波次选择僵尸类型
    auto &flagToSumNum = gameLevelData->flagToSumNum;
    selectFlagZombie(flagToSumNum.second[qLowerBound(flagToSumNum.first, waveNum) - flagToSumNum.first.begin()]);
}
void GameScene::plantDie(PlantInstance *plant)
{
    // 从位置映射中移除植物
    plantPosition[qMakePair(plant->col, plant->row)].remove(plant->plantProtoType->pKind);

    // 移除植物触发区域
    for (int i = 0; i < plantTriggers[plant->row].size(); ) {
        if (plantTriggers[plant->row][i]->plant == plant) {
            delete plantTriggers[plant->row][i];
            plantTriggers[plant->row].removeAt(i);
            continue;
        }
        ++i;
    }

    // 从实例列表和UUID映射中移除
    plantInstances.removeAt(plantInstances.indexOf(plant));
    plantUuid.remove(plant->uuid);

    // 释放植物对象内存
    delete plant;
}


void GameScene::zombieDie(ZombieInstance *zombie)
{
    // 从实例列表和行僵尸列表中移除
    int i = zombieInstances.indexOf(zombie);
    zombieInstances.removeAt(i);
    zombieRow[zombie->row].removeOne(zombie);

    // 检查是否所有僵尸已被消灭
    if (zombieInstances.isEmpty()) {
        if (waveNum < gameLevelData->flagNum) {  // 还有剩余波次
            delete waveTimer;  // 清除当前波次计时器
            (new Timer(this, 5000, [this] { advanceFlag(); }))->start();  // 5秒后开始下一波
        }
        else {  // 已完成所有波次
            gameWin();  // 触发游戏胜利
        }
    }

    // 从UUID映射中移除并释放内存
    zombieUuid.remove(zombie->uuid);
    delete zombie;
}

void GameScene::selectFlagZombie(int levelSum)
{
    int timeout = 1500;  // 僵尸生成间隔（毫秒）
    QList<Zombie *> zombies, zombiesCandidate;  // 待生成僵尸列表与候选列表

    // 处理大波次（带有旗帜僵尸）
    if (gameLevelData->largeWaveFlag.contains(waveNum)) {
        QSound::play(":/audio/siren.wav");  // 播放警报声
        Zombie *flagZombie = getZombieProtoType("oFlagZombie");  // 获取旗帜僵尸原型
        levelSum -= flagZombie->level;  // 扣除旗帜僵尸等级
        zombies.push_back(flagZombie);
        timeout = 300;  // 大波次减小生成间隔，加快僵尸出现
    }

    // 收集当前波次必须出现的僵尸
    for (const auto &zombieData: gameLevelData->zombieData) {
        if (zombieData.flagList.contains(levelSum)) {
            Zombie *item = getZombieProtoType(zombieData.eName);
            levelSum -= item->level;
            zombies.push_back(item);
        }

        // 收集候选僵尸（根据波次进度解锁）
        if (zombieData.firstFlag <= waveNum) {
            Zombie *item = getZombieProtoType(zombieData.eName);
            for (int i = 0; i < zombieData.num; ++i)
                zombiesCandidate.push_back(item);
        }
    }

    // 按等级排序候选僵尸，便于后续选择
    qSort(zombiesCandidate.begin(), zombiesCandidate.end(), [](Zombie *a, Zombie *b) { return a->level < b->level; });

    // 随机选择僵尸填充剩余等级
    while (levelSum > 0) {
        while (zombiesCandidate.last()->level > levelSum)
            zombiesCandidate.pop_back();  // 移除等级过高的僵尸
        Zombie *item = zombiesCandidate[qrand() % zombiesCandidate.size()];
        levelSum -= item->level;
        zombies.push_back(item);
    }

    // 按间隔时间依次生成僵尸
    for (int i = 0; i < zombies.size(); ++i) {
        Zombie *zombie = zombies[i];
        (new Timer(this, i * timeout, [this, zombie] {
            int row;
            // 随机选择允许僵尸通过的行
            do {
                row = qrand() % coordinate.rowCount() + 1;
            } while (!zombie->canPass(row));

            // 创建僵尸实例并初始化
            ZombieInstance *zombieInstance = ZombieInstanceFactory(zombie);
            zombieInstance->birth(row);
            zombieInstances.push_back(zombieInstance);
            zombieRow[row].push_back(zombieInstance);

            // 按位置排序该行僵尸（从左到右）
            qSort(zombieRow[row].begin(), zombieRow[row].end(), [](ZombieInstance *a, ZombieInstance *b) {
                return b->attackedLX < a->attackedLX;
            });

            zombieUuid.insert(zombieInstance->uuid, zombieInstance);  // 记录UUID映射
        }))->start();
    }

    // 调试输出当前波次的僵尸配置
    qDebug() << "Wave: " << waveNum;
    for (auto item: zombies)
        qDebug() << "    " << item->eName;
}

// 根据行列坐标获取植物（可能多个）
QMap<int, PlantInstance *> GameScene::getPlant(int col, int row)
{
    auto iter = plantPosition.find(qMakePair(col, row));
    if (iter == plantPosition.end())
        return QMap<int, PlantInstance*>();  // 未找到则返回空映射
    return *iter;
}

// 根据屏幕坐标获取植物
PlantInstance *GameScene::getPlant(const QPointF &pos)
{
    for (auto plant: plantInstances)
        if (plant->contains(pos - plant->picture->scenePos()))
            return plant;  // 找到包含该点的植物
    return nullptr;
}

// 检查指定位置是否有弹坑
bool GameScene::isCrater(int col, int row) const
{
    return qBinaryFind(craters, qMakePair(col, row)) != craters.end();
}

// 检查指定位置是否有墓碑
bool GameScene::isTombstone(int col, int row) const
{
    return qBinaryFind(tombstones, qMakePair(col, row)) != tombstones.end();
}

// 获取坐标系统引用
Coordinate &GameScene::getCoordinate()
{
    return coordinate;
}

// 添加植物触发区域（用于碰撞检测）
void GameScene::addTrigger(int row, Trigger *trigger)
{
    plantTriggers[row].push_back(trigger);

    // 按触发范围排序，优化后续检测效率
    qSort(plantTriggers[row].begin(), plantTriggers[row].end(), [](const Trigger *a, const Trigger *b) {
        return a->to < b->to;
    });
}

// 启动游戏监控定时器（每100ms检查一次）
void GameScene::beginMonitor()
{
    monitorTimer->setInterval(100);
    connect(monitorTimer, &QTimer::timeout, [this] {
        // 遍历每一行
        for (int row = 1; row <= coordinate.rowCount(); ++row) {
            QList<ZombieInstance *> zombiesCopy = zombieRow[row];  // 复制当前行僵尸列表

            // 遍历该行所有僵尸
            for (ZombieInstance *zombie: zombiesCopy) {
                QUuid zombieUuid = zombie->uuid;

                // 检查僵尸是否存活且在有效范围内
                if (zombie->hp > 0 && zombie->ZX <= 900) {
                    QList<Trigger *> triggerCopy = plantTriggers[row];  // 复制当前行触发区域

                    // 遍历所有触发区域，检测碰撞
                    for (auto trigger: triggerCopy) {
                        if (trigger->plant->canTrigger  // 植物可触发
                            && trigger->from <= zombie->attackedLX  // 僵尸进入触发左边界
                            && trigger->to >= zombie->attackedLX) {  // 僵尸进入触发右边界
                            trigger->plant->triggerCheck(zombie, trigger);  // 触发植物效果
                        }
                    }
                }

                // 更新僵尸行为状态
                ZombieInstance *z = getZombie(zombieUuid);
                if (z)
                    z->checkActs();
            }

            // 重新排序僵尸列表（确保按位置排序）
            qSort(zombieRow[row].begin(), zombieRow[row].end(), [](ZombieInstance *a, ZombieInstance *b) {
                return b->attackedLX < a->attackedLX;
            });
        }
    });
    monitorTimer->start();
}

// 根据UUID查找植物实例
PlantInstance *GameScene::getPlant(const QUuid &uuid)
{
    if (plantUuid.contains(uuid))
        return plantUuid[uuid];
    return nullptr;
}

// 根据UUID查找僵尸实例
ZombieInstance *GameScene::getZombie(const QUuid &uuid)
{
    if (zombieUuid.contains(uuid))
        return zombieUuid[uuid];
    return nullptr;
}

// 获取指定行的所有僵尸
QList<ZombieInstance *> GameScene::getZombieOnRow(int row)
{
    return zombieRow[row];  // 直接返回对应行的僵尸列表
}

// 获取指定行特定范围的僵尸（用于植物攻击检测）
QList<ZombieInstance *> GameScene::getZombieOnRowRange(int row, qreal from, qreal to)
{
    QList<ZombieInstance *> zombies;
    for (auto zombie: zombieRow[row])
        // 检查僵尸存活且部分身体在范围内
        if (zombie->hp > 0 && zombie->attackedLX < to && (zombie->attackedLX > from || zombie->attackedRX > from))
            zombies.push_back(zombie);
    return zombies;
}

// 获取植物原型（使用缓存避免重复创建）
Plant *GameScene::getPlantProtoType(const QString &eName)
{
    if (plantProtoTypes.find(eName) == plantProtoTypes.end())
        plantProtoTypes.insert(eName, PlantFactory(this, eName));  // 工厂创建并缓存
    return plantProtoTypes[eName];
}

// 获取僵尸原型（使用缓存避免重复创建）
Zombie *GameScene::getZombieProtoType(const QString &eName)
{
    if (zombieProtoTypes.find(eName) == zombieProtoTypes.end())
        zombieProtoTypes.insert(eName, ZombieFactory(this, eName));  // 工厂创建并缓存
    return zombieProtoTypes[eName];
}

// 开始播放关卡背景音乐
void GameScene::beginBGM()
{
    backgroundMusic->blockSignals(true);  // 临时阻塞信号避免意外触发
    backgroundMusic->stop();
    backgroundMusic->blockSignals(false);
    backgroundMusic->setMedia(QUrl(gameLevelData->backgroundMusic));  // 设置关卡特定音乐
    backgroundMusic->play();
}

// 游戏失败处理
void GameScene::gameLose()
{
    monitorTimer->stop();  // 停止游戏监控
    delete monitorTimer;

    // 播放失败音乐
    backgroundMusic->blockSignals(true);
    backgroundMusic->stop();
    backgroundMusic->blockSignals(false);
    backgroundMusic->setMedia(QUrl("qrc:/audio/losemusic.mp3"));
    backgroundMusic->play();

    losePicture->setVisible(true);  // 显示失败画面

    // 5秒后返回选关界面
    (new Timer(this, 5000, [this] {
        backgroundMusic->blockSignals(true);
        backgroundMusic->stop();
        backgroundMusic->blockSignals(false);
        gMainView->switchToScene(new SelectorScene);
    }))->start();
}

// 游戏胜利处理
void GameScene::gameWin()
{
    monitorTimer->stop();  // 停止游戏监控
    delete monitorTimer;

    // 播放胜利音乐
    backgroundMusic->blockSignals(true);
    backgroundMusic->stop();
    backgroundMusic->blockSignals(false);
    backgroundMusic->setMedia(QUrl("qrc:/audio/winmusic.mp3"));
    backgroundMusic->play();

    winPicture->setVisible(true);  // 显示胜利画面

    // 5秒后返回选关界面
    (new Timer(this, 5000, [this] {
        backgroundMusic->blockSignals(true);
        backgroundMusic->stop();
        backgroundMusic->blockSignals(false);
        gMainView->switchToScene(new SelectorScene);
    }))->start();
}


// 旗帜进度条构造函数
FlagMeter::FlagMeter(GameLevelData *gameLevelData)
    : flagNum(gameLevelData->flagNum),  // 总波次数
      flagHeadStep(140.0 / (flagNum - 1)),  // 旗帜移动步长
      flagMeterEmpty(gImageCache->load("interface/FlagMeterEmpty.png")),  // 空进度条图片
      flagMeterFull(gImageCache->load("interface/FlagMeterFull.png")),  // 满进度条图片
      flagTitle(new QGraphicsPixmapItem(gImageCache->load("interface/FlagMeterLevelProgress.png"))),  // 标题
      flagHead(new QGraphicsPixmapItem(gImageCache->load("interface/FlagMeterParts1.png")))  // 旗帜图标
{
    setPixmap(flagMeterEmpty);  // 初始显示空进度条

    // 设置标题位置并添加到组件
    flagTitle->setPos(35, 12);
    flagTitle->setParentItem(this);

    // 为大波次添加特殊标记
    for (auto i: gameLevelData->largeWaveFlag) {
        QGraphicsPixmapItem *flag = new QGraphicsPixmapItem(gImageCache->load("interface/FlagMeterParts2.png"));
        flag->setPos(150 - (i - 1) * flagHeadStep, -3);  // 计算位置
        flag->setParentItem(this);
        flags.insert(i, flag);  // 存储大波次标记
    }

    // 设置旗帜初始位置
    flagHead->setPos(139, -4);
    flagHead->setParentItem(this);

    updateFlagZombies(1);  // 初始化显示第一波
}

// 更新旗帜进度
void FlagMeter::updateFlagZombies(int flagZombies)
{
    // 大波次标记动画效果（下移）
    auto iter = flags.find(flagZombies);
    if (iter != flags.end())
        iter.value()->setY(-12);

    // 更新旗帜位置和进度条填充
    if (flagZombies < flagNum) {
        qreal x = 150 - (flagZombies - 1) * flagHeadStep;
        flagHead->setPos(x - 11, -4);  // 更新旗帜位置

        // 绘制进度条（部分填充）
        QPixmap flagMeter(flagMeterFull);
        QPainter p(&flagMeter);
        p.drawPixmap(0, 0, flagMeterEmpty.copy(0, 0, qRound(x), 21));  // 绘制未填充部分
        setPixmap(flagMeter);
    }
    else {
        flagHead->setPos(-1, -3);  // 最后一波时隐藏旗帜
        setPixmap(flagMeterFull);  // 显示满进度条
    }
}

// 触发区域构造函数
Trigger::Trigger(PlantInstance *plant, qreal from, qreal to, int direction, int id)
        : plant(plant), from(from), to(to),  // 关联植物和触发范围
          direction(direction), id(id)  // 攻击方向和ID
{}
