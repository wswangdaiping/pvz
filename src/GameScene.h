#ifndef PLANTS_VS_ZOMBIES_GAMESCENE_H
#define PLANTS_VS_ZOMBIES_GAMESCENE_H

#include <QtWidgets>        // Qt窗口部件模块
#include <QtMultimedia>    // Qt多媒体模块
#include "Coordinate.h"    // 坐标系统头文件
#include "Plant.h"         // 植物头文件
#include "Zombie.h"       // 僵尸头文件

class Plant;
class PlantInstance;
class Zombie;
class GameLevelData;
class MouseEventPixmapItem;
class MoviePixmapItem;
class PlantCardItem;
class TooltipItem;
class Zombie;
class ZombieInstance;

// 触发器结构体（用于植物攻击等行为的触发）
struct Trigger {
    Trigger(PlantInstance *plant, qreal from, qreal to, int direction, int id);

    PlantInstance *plant;  // 关联的植物实例
    qreal from, to;       // 触发范围（起始和结束位置）
    int direction;        // 触发方向
    int id;               // 触发器ID
};

// 旗帜进度条类（显示僵尸波次进度）
class FlagMeter: public QGraphicsPixmapItem
{
public:
    FlagMeter(GameLevelData *gameLevelData);

    // 更新旗帜僵尸数量显示
    void updateFlagZombies(int flagZombies);
private:
    int flagNum;                  // 旗帜僵尸数量
    qreal flagHeadStep;           // 旗帜头移动步长
    QPixmap flagMeterEmpty;       // 空进度条图像
    QPixmap flagMeterFull;        // 满进度条图像
    QGraphicsPixmapItem *flagTitle; // 旗帜标题图形项
    QGraphicsPixmapItem *flagHead;  // 旗帜头部图形项
    QMap<int, QGraphicsPixmapItem *> flags; // 旗帜位置映射
};

// 游戏主场景类（继承自QGraphicsScene）
class GameScene: public QGraphicsScene
{
    Q_OBJECT  // Qt元对象系统宏

public:
    GameScene(GameLevelData *gameLevel);  // 构造函数
    ~GameScene();                         // 析构函数

    // 设置信息文本
    void setInfoText(const QString &text);
    // 获取游戏关卡数据
    GameLevelData *getGameLevelData() const;

    // 添加元素到游戏场景
    void addToGame(QGraphicsItem *item);
    // 执行特殊操作
    void customSpecial(const QString &name, int col, int row);
    // 准备种植植物
    void prepareGrowPlants(std::function<void(void)> functor);

    // 游戏加载相关方法
    void loadReady();
    void loadAcessFinished();
    void beginBGM();       // 开始背景音乐
    void beginCool();      // 开始冷却
    void beginSun(int sunNum); // 开始阳光生产
    void beginZombies();   // 开始生成僵尸
    void beginMonitor();   // 开始游戏监控
    void gameLose();       // 游戏失败处理
    void gameWin();        // 游戏胜利处理

    // 植物/僵尸死亡处理
    void plantDie(PlantInstance *plant);
    void zombieDie(ZombieInstance *zombie);

    // 获取原型对象
    Plant *getPlantProtoType(const QString &eName);
    Zombie *getZombieProtoType(const QString &eName);

    // 获取场景中的植物/僵尸
    QMap<int, PlantInstance *> getPlant(int col, int row);
    PlantInstance *getPlant(const QPointF &pos);
    PlantInstance *getPlant(const QUuid &uuid);
    ZombieInstance *getZombie(const QUuid &uuid);
    QList<ZombieInstance *> getZombieOnRow(int row);
    QList<ZombieInstance *> getZombieOnRowRange(int row, qreal from, qreal to);

    // 阳光相关
    QPair<MoviePixmapItem *, std::function<void(bool)> > newSun(int sunNum);
    // 地形检查
    bool isCrater(int col, int row) const;
    bool isTombstone(int col, int row) const;
    // 获取坐标系统
    Coordinate &getCoordinate();

    // 添加触发器
    void addTrigger(int row, Trigger *trigger);

protected:
    // 游戏流程控制
    void letsGo();
    void doCoolTime(int index);
    // 更新工具提示
    void updateTooltip(int index);
    // 更新阳光数量显示
    void updateSunNum();
    // 更新旗帜进度
    void advanceFlag();
    // 选择旗帜僵尸
    void selectFlagZombie(int levelSum);

    // 坐标转换
    static QPointF sizeToPoint(const QSizeF &size);

    // 鼠标事件处理
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);

signals:
    // 鼠标事件信号
    void mouseMove(QGraphicsSceneMouseEvent *mouseEvent);
    void mousePress(QGraphicsSceneMouseEvent *mouseEvent);

private:
    // 游戏元素容器
    QVector<QPair<QGraphicsItem *, qreal> > itemsOnScreen;

    // 游戏数据
    GameLevelData *gameLevelData;

    // 场景图形元素
    QGraphicsPixmapItem *background;       // 背景图
    QGraphicsItemGroup *gameGroup;         // 游戏元素组
    QGraphicsSimpleTextItem *infoText;     // 信息文本
    QGraphicsRectItem *infoTextGroup;      // 信息文本背景
    MouseEventPixmapItem *menuGroup;       // 菜单组
    QGraphicsSimpleTextItem *sunNumText;   // 阳光数量文本
    QGraphicsPixmapItem *sunNumGroup;      // 阳光数量背景
    // 卡片选择按钮
    MouseEventPixmapItem *selectCardButtonReset, *selectCardButtonOkay;
    QGraphicsSimpleTextItem *selectCardTextReset, *selectCardTextOkay;
    QGraphicsPixmapItem *selectingPanel;   // 选择面板
    QGraphicsItemGroup *cardPanel;         // 卡片面板
    QGraphicsPixmapItem *shovel;           // 铲子
    QGraphicsPixmapItem *shovelBackground; // 铲子背景
    QGraphicsPixmapItem *movePlantAlpha, *movePlant; // 移动植物效果
    MoviePixmapItem *imgGrowSoil, *imgGrowSpray; // 种植动画
    FlagMeter *flagMeter;                  // 旗帜进度条
    QGraphicsPixmapItem *losePicture, *winPicture; // 输赢画面
    QGraphicsItemGroup *sunGroup;          // 阳光组

    // 多媒体
    QMediaPlayer *backgroundMusic;         // 背景音乐播放器

    // 原型容器
    QMap<QString, Plant *> plantProtoTypes;  // 植物原型
    QMap<QString, Zombie *> zombieProtoTypes; // 僵尸原型

    Coordinate coordinate;  // 坐标系统

    // 游戏卡片相关
    QList<Plant *> selectedPlantArray;  // 已选植物数组
    struct CardGraphicsItem {
        PlantCardItem *plantCard;  // 植物卡片
        TooltipItem *tooltip;      // 工具提示
    };
    QList<CardGraphicsItem > cardGraphics;  // 卡片图形项
    struct CardReadyItem {
        bool cool;  // 是否冷却
        bool sun;   // 是否有足够阳光
    };
    QList<CardReadyItem> cardReady;  // 卡片准备状态

    // 游戏实例容器
    QList<PlantInstance *> plantInstances;  // 植物实例
    QList<ZombieInstance *> zombieInstances; // 僵尸实例
    QMap<QPair<int, int>, QMap<int, PlantInstance *> > plantPosition; // 植物位置
    QList<QPair<int, int> > craters, tombstones; // 弹坑和墓碑位置
    QList<QList<Trigger *> > plantTriggers; // 植物触发器
    QList<QList<ZombieInstance *> > zombieRow; // 僵尸行数组
    QMap<QUuid, PlantInstance *> plantUuid;    // 植物UUID映射
    QMap<QUuid, ZombieInstance *> zombieUuid;  // 僵尸UUID映射

    // 游戏状态变量
    int choose;      // 当前选择
    int sunNum;      // 阳光数量
    QTimer *waveTimer, *monitorTimer; // 波次计时器和监控计时器
    int waveNum;     // 当前波次数
};

#endif //PLANTS_VS_ZOMBIES_GAMESCENE_H
