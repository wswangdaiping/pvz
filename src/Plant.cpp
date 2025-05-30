#include <QtMultimedia>
#include "Plant.h"
#include "ImageManager.h"
#include "GameScene.h"
#include "GameLevelData.h"
#include "MouseEventPixmapItem.h"
#include "Timer.h"
#include "Animate.h"


//Plant 类是所有植物类的基类，它定义了植物的基本属性和方法，例如植物的名称、生命值、尺寸、攻击范围、冷却时间等
// Plant类构造函数，初始化植物基础属性
Plant::Plant()
        : hp(300),              // 植物基础生命值
          pKind(1), bKind(0),    // pKind:植物类型标识，bKind:子弹类型标识
          beAttackedPointL(20), beAttackedPointR(20), // 攻击判定左右边界
          zIndex(0),             // 渲染层级（Z轴顺序）
          canEat(true), canSelect(true), night(false), // 可被吃/可选中/夜间植物标识
          coolTime(7.5), stature(0), sleep(0), scene(nullptr) // 冷却时间/形态/睡眠状态/所属场景
{}

// 获取植物X轴偏移量（用于居中显示）
double Plant::getDX() const
{
    return -0.5 * width; // 返回宽度一半的负值，使植物图片居中
}

// 获取植物Y轴偏移量（根据下方植物类型调整高度）
double Plant::getDY(int x, int y) const
{
    //偏移-15
    return scene->getPlant(x, y).contains(0) ? -21 : -15;
}

// 判断植物是否可在指定位置种植
bool Plant::canGrow(int x, int y) const
{
    // 检查坐标是否越界
    if (x < 1 || x > 9 || y < 1 || y > 5)
        return false;

    if (scene->isCrater(x, y) || scene->isTombstone(x, y))
        return false;

    int groundType = scene->getGameLevelData()->LF[y]; // 获取地形类型（1=陆地，2=水面）
    QMap<int, PlantInstance *> plants = scene->getPlant(x, y); // 获取目标格子植物

    // 陆地地形：不能种植在已有非睡莲植物上
    if (groundType == 1)
        return !plants.contains(1);

    return plants.contains(0) && !plants.contains(1);
}

// 更新植物图片尺寸（根据静态图自动获取）
void Plant::update()
{
    QPixmap pic = gImageCache->load(staticGif); // 加载静态图片
    width = pic.width();  // 更新宽度
    height = pic.height();// 更新高度
}

// PlantInstance构造函数，初始化植物实例
PlantInstance::PlantInstance(const Plant *plant) : plantProtoType(plant)
{
    uuid = QUuid::createUuid(); // 生成唯一UUID
    hp = plantProtoType->hp;    // 继承原型生命值
    canTrigger = true;          // 初始可触发攻击
    picture = new MoviePixmapItem; // 创建动画图片项
}

// 析构函数，延迟释放图片资源
PlantInstance::~PlantInstance()
{
    picture->deleteLater(); // 延迟删除避免渲染冲突
}

// 植物出生逻辑（种植时调用）
void PlantInstance::birth(int c, int r)
{
    Coordinate &coordinate = plantProtoType->scene->getCoordinate(); // 获取坐标系统
    // 计算植物左上角坐标（含偏移量）
    double x = coordinate.getX(c) + plantProtoType->getDX();
    double y = coordinate.getY(r) + plantProtoType->getDY(c, r) - plantProtoType->height;

    col = c, row = r; // 记录行列
    // 计算攻击判定区域
    attackedLX = x + plantProtoType->beAttackedPointL;
    attackedRX = x + plantProtoType->beAttackedPointR;

    // 初始化动画图片
    picture->setMovie(plantProtoType->normalGif);
    picture->setPos(x, y);
    picture->setZValue(plantProtoType->zIndex + 3 * r); // 按行设置渲染层级

    // 添加阴影效果
    shadowPNG = new QGraphicsPixmapItem(gImageCache->load("interface/plantShadow.png"));
    shadowPNG->setPos(plantProtoType->width * 0.5 - 48, plantProtoType->height - 22);
    shadowPNG->setFlag(QGraphicsItem::ItemStacksBehindParent);
    shadowPNG->setParentItem(picture);

    picture->start(); // 播放动画
    plantProtoType->scene->addToGame(picture); // 添加到场景
    initTrigger(); // 初始化触发器
}

// 初始化攻击触发器（检测僵尸进入范围）
void PlantInstance::initTrigger()
{
    // 创建从植物左侧到场景右侧的触发区域
    Trigger *trigger = new Trigger(this, attackedLX, 880, 0, 0);
    triggers.insert(row, QList<Trigger *>{ trigger }); // 按行存储触发器
    plantProtoType->scene->addTrigger(row, trigger); // 注册到场景
}

// 检测点是否在植物范围内（用于交互）
bool PlantInstance::contains(const QPointF &pos)
{
    return picture->contains(pos); // 调用图片项的包含检测
}

// 触发器回调（僵尸进入触发区域时调用）
void PlantInstance::triggerCheck(ZombieInstance *zombieInstance, Trigger *trigger)
{
    if (zombieInstance->altitude > 0) { // 仅处理地面僵尸
        canTrigger = false; // 防止重复触发
        QSharedPointer<std::function<void(QUuid)> > triggerCheck(new std::function<void(QUuid)>);

        // 递归检查逻辑（处理僵尸移动中的持续触发）
        *triggerCheck = [this, triggerCheck] (QUuid zombieUuid) {
            (new Timer(picture, 1400, [this, zombieUuid, triggerCheck] {
                ZombieInstance *zombie = this->plantProtoType->scene->getZombie(zombieUuid);
                if (zombie) {
                    // 遍历当前行触发器，检查僵尸是否仍在范围内
                    for (auto i: triggers[zombie->row]) {
                        if (zombie->hp > 0 && i->from <= zombie->ZX && i->to >= zombie->ZX && zombie->altitude > 0) {
                            normalAttack(zombie); // 执行攻击
                            (*triggerCheck)(zombie->uuid); // 继续递归检查
                            return;
                        }
                    }
                }
                canTrigger = true; // 退出递归时恢复触发状态
            }))->start();
        };

        normalAttack(zombieInstance); // 首次触发攻击
        (*triggerCheck)(zombieInstance->uuid); // 启动递归检查
    }
}

// 普通攻击逻辑（子类重写实现具体攻击）
void PlantInstance::normalAttack(ZombieInstance *zombieInstance)
{
    // 打印调试日志（子类可重写）
    qDebug() << plantProtoType->cName << uuid << "Attack" << zombieInstance->zombieProtoType->cName << zombieInstance;
}

// 植物受击逻辑
void PlantInstance::getHurt(ZombieInstance *zombie, int aKind, int attack)
{
    if (aKind == 0) // 普通攻击（非秒杀类）
        hp -= attack;
    // 生命值归0或受到秒杀攻击时触发死亡
    if (hp < 1 || aKind != 0)
        plantProtoType->scene->plantDie(this);
}
//实现仙人掌
// Cactus 植物定义
Cactus::Cactus() {
    eName = "oCactus";
    cName = tr("仙人掌");
    hp = 300;
    beAttackedPointR = 40;
    sunNum = 100;
    coolTime = 7.5;
    cardGif = "Card/Plants/Cactus.png";
    staticGif = "Plants/Cactus/0.gif";
    normalGif = "Plants/Cactus/Cactus.gif";
    attackGif = "Plants/Cactus/Cactus.gif"; // 攻击动画（伸长刺）
    toolTip = tr("发射尖刺，这些尖刺能打爆气球并刺穿护盾");
}

// CactusInstance 实现
CactusInstance::CactusInstance(const Plant* plant) : PlantInstance(plant) {

}


void CactusInstance::normalAttack(ZombieInstance* zombie) {

    // 发射尖刺子弹（类型2，用于区分普通豌豆） //PB20
    (new Bullet(plantProtoType->scene, 2, row, attackedLX+20, attackedLX - 40+20,
                picture->y() + 30, picture->zValue() + 2, 0))->start();

}

// Squash植物实现
Squash::Squash()
{
    eName = "oSquash";
    cName = tr("倭瓜");
    hp = 300;
    beAttackedPointR = 40;
    sunNum = 50;
    coolTime = 10;
    cardGif = "Card/Plants/Squash.png";
    staticGif = "Plants/Squash/0.gif";
    leftGif = "Plants/Squash/SquashL.PNG";
    rightGif = "Plants/Squash/SquashR.png";
    attackGif = "Plants/Squash/SquashAttack.gif";
    normalGif = "Plants/Squash/Squash.gif";
    toolTip = tr("跳起并压死靠近它的僵尸(别靠近我|~~|");
    pKind = 1;
}

SquashInstance::SquashInstance(const Plant *plant)
    : PlantInstance(plant)
{
     initialY = picture->y(); // 初始化Y坐标
}

void SquashInstance::birth(int c, int r)
{
    Coordinate &coordinate = plantProtoType->scene->getCoordinate();
    double x = coordinate.getX(c) + plantProtoType->getDX(),
           y = coordinate.getY(r) + plantProtoType->getDY(c, r) - plantProtoType->height;
    col = c, row = r;
    attackedLX = x + plantProtoType->beAttackedPointL;
    attackedRX = x + plantProtoType->beAttackedPointR;

    picture->setMovie(plantProtoType->normalGif);
    picture->setPos(x, y);
    picture->setZValue(plantProtoType->zIndex + 3 * r);

    shadowPNG = new QGraphicsPixmapItem(gImageCache->load("interface/plantShadow.png"));
    shadowPNG->setPos(plantProtoType->width * 0.5 - 48, plantProtoType->height - 22);
    shadowPNG->setFlag(QGraphicsItem::ItemStacksBehindParent);
    shadowPNG->setParentItem(picture);

    picture->start();
    plantProtoType->scene->addToGame(picture);

    // 初始化触发器
    initTrigger();
}
void SquashInstance::initTrigger()
{
    // 创建三个触发器：左侧、自身位置、右侧
    Trigger *leftTrigger = new Trigger(this, attackedLX - 50, attackedLX - 5, 0, 0);  // 左侧区域
    Trigger *selfTrigger = new Trigger(this, attackedLX - 5, attackedRX + 5, 0, 0);  // 自身周围区域
    Trigger *rightTrigger = new Trigger(this, attackedRX + 5, attackedRX + 50, 0, 0); // 右侧区域

    triggers.insert(row, QList<Trigger *>{ leftTrigger, selfTrigger, rightTrigger });
    plantProtoType->scene->addTrigger(row, leftTrigger);
    plantProtoType->scene->addTrigger(row, selfTrigger);
    plantProtoType->scene->addTrigger(row, rightTrigger);
}
void SquashInstance::triggerCheck(ZombieInstance *zombieInstance, Trigger *trigger)
{
    if (zombieInstance->altitude > 0 && canTrigger) {
        // 获取Squash原型指针
        const Squash* squashProto = static_cast<const Squash*>(plantProtoType);

        // 判断僵尸触发区域
        bool isLeft = (trigger == triggers[row][0]);
        bool isRight = (trigger == triggers[row][2]);

        // 对于跳高僵尸，只有完全跳过植物后才触发左侧检测
        if (zombieInstance->canJump && !isLeft) {
            // 检查僵尸是否已经完全跳过植物
            if (zombieInstance->attackedLX > attackedRX + 20) {
                isLeft = true;
                isRight = false;
            }
        }

        if (isLeft || isRight) {
            canTrigger = false;

            // 显示对应方向的图片
            picture->setMovie(isLeft ? squashProto->leftGif
                                   : squashProto->rightGif);

            // 延迟后跳跃攻击
            (new Timer(picture, 500, [this, isLeft] {
                jumpAndCrush(isLeft);
            }))->start();
        }

        // 在triggerCheck中添加
        if (isLeft) {
            qDebug() << "Left trigger activated by zombie at:" << zombieInstance->attackedLX;
        } else {
            qDebug() << "Right trigger activated by zombie at:" << zombieInstance->attackedLX;
        }
    }

}
void SquashInstance::jumpAndCrush(bool jumpLeft)
{
    const Squash* squashProto = static_cast<const Squash*>(plantProtoType);
    ZombieInstance* targetZombie = nullptr;
    qreal targetX = 0;

    QList<ZombieInstance*> zombies = plantProtoType->scene->getZombieOnRow(row);

    if (jumpLeft) {
        // 向左跳，找左侧已跳过植物的僵尸
        for (ZombieInstance* zombie : zombies) {
            if (zombie->hp > 0 && (zombie->attackedRX <= attackedLX || (zombie->canJump && zombie->attackedLX > attackedRX+20))) {
                if (!targetZombie || zombie->attackedRX > targetZombie->attackedRX) {
                    targetZombie = zombie;
                }
            }
        }
        targetX = targetZombie ?
                 qMin(targetZombie->attackedLX - plantProtoType->width/2,picture->x() - 50) :
                 (picture->x() - 100);
    } else {
        // 向右跳，找右侧的僵尸
        for (ZombieInstance* zombie : zombies) {
            if (zombie->hp > 0 && zombie->attackedLX >= attackedLX) {
                if (!targetZombie || zombie->attackedLX < targetZombie->attackedLX) {
                    targetZombie = zombie;
                }
            }
        }
        targetX = targetZombie ?
                 qMax(targetZombie->attackedLX - plantProtoType->width/2,picture->x()+50) :
                 (picture->x() + 100);
    }

    // 确保方向一致性
       if (jumpLeft && targetX > picture->x()) {
           targetX = picture->x() - 50;  // 强制向左移动
       } else if (!jumpLeft && targetX < picture->x()) {
           targetX = picture->x() + 50;  // 强制向右移动
       }

    // 使用智能指针管理目标僵尸
    QSharedPointer<ZombieInstance> safeTarget(targetZombie, [](ZombieInstance*){});

    // 跳跃动画
    Animate(picture, plantProtoType->scene)
        .move(QPointF(targetX, picture->y() - 50))
        .speed(0.2)
        .shape(QTimeLine::EaseOutCurve)
        .move(QPointF(targetX, picture->y()))
        .speed(0.1)
        .shape(QTimeLine::EaseInCurve)
        .finish([this, squashProto, safeTarget] {
            // 播放攻击动画
            picture->setMovie(squashProto->attackGif);
            picture->start();

            // 对僵尸造成伤害
            if (safeTarget && safeTarget->hp > 0) {
                safeTarget->getCrushed(this);
                safeTarget->crushDie();
            }

            // 攻击完成后移除植物
            (new Timer(picture, 500, [this] {
                plantProtoType->scene->plantDie(this);
            }))->start();
        });


    qDebug() << "Jump direction:" << (jumpLeft ? "Left" : "Right")
             << "Target zombie pos:" << (targetZombie ? targetZombie->attackedLX : -1)
             << "Squash pos:" << attackedLX;
}
//end squash
//wdp Jalapeno bomb

Jalapeno::Jalapeno()
{
    eName = "oJalapeno";
    cName = tr("辣椒炸弹");
    hp = 300;
    beAttackedPointR = 40;
    sunNum = 150;
    coolTime = 20;
    cardGif = "Card/Plants/Jalapeno.png";
    staticGif = "Plants/Jalapeno/0.gif";
    normalGif = "Plants/Jalapeno/Jalapeno.gif";
    toolTip = tr("在其所在行中炸死并烧死所有僵尸");
    pKind = 1;
}

JalapenoInstance::JalapenoInstance(const Plant *plant)
    :PlantInstance(plant)
{

}

void JalapenoInstance::birth(int c,int r)
{
    Coordinate &coordinate = plantProtoType->scene->getCoordinate();
    double x = coordinate.getX(c)+plantProtoType->getDX(),
           y = coordinate.getY(r)+plantProtoType->getDY(c,r)-plantProtoType->height;
    col = c,row = r;
    attackedLX = x + plantProtoType->beAttackedPointL;
    attackedRX = x + plantProtoType->beAttackedPointR;

    picture->setMovie(plantProtoType->normalGif);
    picture->setPos(x,y);
    picture->setZValue(plantProtoType->zIndex +3 * r);

    shadowPNG = new QGraphicsPixmapItem
            (gImageCache->load("interface/plantShadow"));
    shadowPNG->setPos(plantProtoType->width * 0.5 - 48,plantProtoType->height - 22);
    shadowPNG->setFlag(QGraphicsItem::ItemStacksBehindParent);
    shadowPNG->setParentItem(picture);

    picture->start();
    plantProtoType->scene->addToGame(picture);

    //1.5s后开始移动并爆炸
    (new Timer(picture,1500,[this]{
        moveAndExplode();
    }))->start();

}

void JalapenoInstance::moveAndExplode()
{
    // 设置攻击动画
    picture->setMovie("Plants/Jalapeno/JalapenoAttack.gif");
    picture->start();

    // 0.5秒后开始移动和爆炸
    (new Timer(picture, 300, [this] {
        QSound::play(":/audio/jalapeno.wav");

        // 获取整行僵尸（优化后的方式）
        QList<ZombieInstance*> zombies;
        for (int col = 1; col <= 9; ++col) {
            auto plants = plantProtoType->scene->getPlant(col, row);
            for (auto zombie : plantProtoType->scene->getZombieOnRowRange(row,
                 plantProtoType->scene->getCoordinate().getX(col) - 100,
                 plantProtoType->scene->getCoordinate().getX(col) + 100)) {
                if (zombie->hp > 0 && !zombies.contains(zombie)) {
                    zombies.append(zombie);
                }
            }
        }

        // 直接对所有僵尸造成伤害（不再需要爆炸动画）
        for (ZombieInstance *zombie : zombies) {
            zombie->getBoomed(); // 调用新增的灰烬死亡效果
        }

        // 移动到屏幕右侧（保持视觉效果）
        Animate(picture, plantProtoType->scene)
            .move(QPointF(picture->x() + 100, picture->y()))
            .speed(0.5)
            .shape(QTimeLine::EaseOutCurve)
            .finish([this] {
                // 移除植物
                plantProtoType->scene->plantDie(this);
            });
    }))->start();
}
//
Peashooter::Peashooter()
{
    eName = "oPeashooter";
    cName = tr("Peashooter");
    beAttackedPointR = 51;
    sunNum = 100;
    cardGif = "Card/Plants/Peashooter.png";
    staticGif = "Plants/Peashooter/0.gif";
    normalGif = "Plants/Peashooter/Peashooter.gif";
    toolTip = tr("Shoots peas at zombies");
}


PeashooterInstance::PeashooterInstance(const Plant *plant)
        : PlantInstance(plant)
{
}

void PeashooterInstance::normalAttack(ZombieInstance *zombieInstance)
{
    QSound::play(":/audio/firepea.wav");
    (new Bullet(plantProtoType->scene, 0, row, attackedLX, attackedLX - 40, picture->y() + 3, picture->zValue() + 2, 0))->start();
}

// Repeater植物类定义
Repeater::Repeater()
{
    eName = "oRepeater";          // 实体名称标识
    cName = tr("双重射手");       // 显示名称（需要国际化）
    beAttackedPointR = 51;        // 受击判定半径（保持与豌豆相同）
    sunNum = 200;                 // 阳光消耗提升至200
    cardGif = "Card/Plants/Repeater.png";  // 卡片图像路径
    staticGif = "Plants/Repeater/0.gif";   // 静态预览图
    normalGif = "Plants/Repeater/Repeater.gif"; // 动态图
    toolTip = tr("一次发射两颗豌豆"); // 功能描述
}

// Repeater实例类
RepeaterInstance::RepeaterInstance(const Plant *plant)
        : PlantInstance(plant)
{
    // 继承基类构造，无需额外初始化
}

void RepeaterInstance::normalAttack(ZombieInstance *zombieInstance)
{
    QSound::play(":/audio/firepea.wav");  // 播放相同射击音效

    // 创建第一颗豌豆（右侧偏移）
    (new Bullet(plantProtoType->scene,
                0,                      // 子弹类型：普通豌豆
                row,                     // 所在行
                attackedLX,              // 攻击起始X坐标
                attackedLX - 40,         // 攻击目标X坐标
                picture->y() + 3,        // Y坐标微调
                picture->zValue() + 2,    // 显示层级
                0))->start();            // 特殊标记

    // 创建第二颗豌豆（左侧偏移，间隔20像素）
    (new Bullet(plantProtoType->scene,
                0,
                row,
                attackedLX - 20,         // 第二颗起始位置左移
                attackedLX - 60,          // 目标位置相应调整
                picture->y() + 3,
                picture->zValue() + 2,    // 保持相同层级
                0))->start();
}

// Threepeater 构造函数（植物卡牌属性）
Threepeater::Threepeater()
{
    eName = "oThreepeater";
    cName = tr("三线射手");
    beAttackedPointR = 51;       // 受击点半径与豌豆相同
    sunNum = 175;                // 阳光消耗提升至175
    cardGif = "Card/Plants/Threepeater.png";
    staticGif = "Plants/Threepeater/0.gif";
    normalGif = "Plants/Threepeater/Threepeater.gif";
    toolTip = tr("同时向三条线路发射豌豆"); // 提示三线攻击
}

// 三线射手实例类
ThreepeaterInstance::ThreepeaterInstance(const Plant *plant)
        : PlantInstance(plant)
{
}

// 攻击逻辑（同时攻击三行）
void ThreepeaterInstance::normalAttack(ZombieInstance *zombieInstance)
{
    QSound::play(":/audio/firepea.wav");

    // 获取坐标系引用
    Coordinate &coord = plantProtoType->scene->getCoordinate();
    const int TOTAL_ROWS = coord.rowCount(); // 动态获取总行数
    const int currentRow = row;              // 当前所在行（0-based）

    // 动态生成deltaRow组合（根据坐标映射规则）
    QVector<int> deltaRows;
    if (currentRow == 0) {                // 首行
        deltaRows = {0, 1};               // 当前行 + 下一行
    } else if (currentRow == TOTAL_ROWS-1) { // 末行
        deltaRows = {-1, 0};              // 上一行 + 当前行
    } else {                              // 中间行
        deltaRows = {-1, 0, 1};           // 完整三行
    }

    // 获取植物基准坐标（需要根据实际实现调整）
    double baseX = attackedLX; // 假设这是植物的左侧攻击点X坐标
    double baseY = coord.getY(currentRow); // 通过坐标类获取精确Y坐标

    foreach(int deltaRow, deltaRows) {
        int targetRow = currentRow + deltaRow;

        // 边界二次校验（防御性编程）
        if(targetRow < 0 || targetRow >= TOTAL_ROWS) continue;

        // 通过坐标类直接获取目标行Y坐标
        double bulletY = coord.getY(targetRow) + 3; // +3像素微调

        // 创建子弹（参数需保持与游戏引擎一致）
        (new Bullet(plantProtoType->scene,
                   0,                  // 子弹类型
                   targetRow,          // 目标行号
                   baseX,              // 攻击起始X（植物左侧）
                   baseX - 40,         // 攻击终点X
                   bulletY,            // 精确Y坐标
                   picture->zValue() + 2,
                   0))->start();
    }
}
SnowPea::SnowPea()
{
    eName = "oSnowPea";
    cName = tr("Snow Pea");
    bKind = -1;
    beAttackedPointR = 51;
    sunNum = 175;
    cardGif = "Card/Plants/SnowPea.png";
    staticGif = "Plants/SnowPea/0.gif";
    normalGif = "Plants/SnowPea/SnowPea.gif";
    toolTip = tr("Slows down zombies with cold precision");
}


SnowPeaInstance::SnowPeaInstance(const Plant *plant)
        : PlantInstance(plant)
{
}

void SnowPeaInstance::normalAttack(ZombieInstance *zombieInstance)
{
    QSound::play(":/audio/firepea.wav");
    (new Bullet(plantProtoType->scene, -1, row, attackedLX, attackedLX - 40, picture->y() + 3, picture->zValue() + 2, 0))->start();
}

// SunFlower类构造函数 - 向日葵原型定义
SunFlower::SunFlower()
{
    eName = "oSunflower";              // 英文名称标识
    cName = tr("Sunflower");            // 中文名称（支持多语言翻译）
    beAttackedPointR = 53;              // 右侧攻击判定点（用于碰撞检测）
    sunNum = 50;                        // 种植所需阳光值
    cardGif = "Card/Plants/SunFlower.png"; // 卡片展示图片路径
    staticGif = "Plants/SunFlower/0.gif";   // 静态站立动画路径
    normalGif = "Plants/SunFlower/SunFlower1.gif"; // 正常生长动画路径
    toolTip = tr("Makes extra Sun for placing plants"); // 工具提示文本
}

// SunFlowerInstance类构造函数 - 向日葵实例初始化
SunFlowerInstance::SunFlowerInstance(const Plant *plant)
        : PlantInstance(plant),
          lightedGif("Plants/SunFlower/SunFlower2.gif") // 发光状态动画路径
{
    // 继承基类初始化，并记录发光动画资源
}

// 初始化触发器与阳光生成逻辑
void SunFlowerInstance::initTrigger()
{
    // 创建定时器：每5秒触发一次阳光生成逻辑
    (new Timer(picture, 5000, [this] {
        // 定义阳光生成的递归函数（使用智能指针避免内存泄漏）
        QSharedPointer<std::function<void(void)> > generateSun(new std::function<void(void)>);
        *generateSun = [this, generateSun] {
            // 切换向日葵到发光状态（生成阳光前的动画）
            picture->setMovieOnNewLoop(lightedGif, [this, generateSun] {
                // 1秒后执行阳光生成
                (new Timer(picture, 1000, [this, generateSun] {
                    // 调用场景方法创建阳光（返回阳光动画与结束回调）
                    auto sunGifAndOnFinished = plantProtoType->scene->newSun(25);
                    MoviePixmapItem *sunGif = sunGifAndOnFinished.first;         // 阳光动画对象
                    std::function<void(bool)> onFinished = sunGifAndOnFinished.second; // 动画结束回调

                    // 获取场景坐标系统
                    Coordinate &coordinate = plantProtoType->scene->getCoordinate();
                    // 计算阳光生成的起始与目标位置
                    double fromX = coordinate.getX(col) - sunGif->boundingRect().width() / 2 + 15,
                           toX = coordinate.getX(col) - qrand() % 80,           // 随机水平偏移
                           toY = coordinate.getY(row) - sunGif->boundingRect().height();

                    // 初始化阳光动画：
                    sunGif->setScale(0.6);              // 初始缩放比例
                    sunGif->setPos(fromX, toY - 25);     // 初始位置（向日葵上方）
                    sunGif->start();                    // 启动动画播放

                    // 定义阳光动画轨迹（使用Animate类实现平滑移动）：
                    Animate(sunGif, plantProtoType->scene)
                        .move(QPointF((fromX + toX) / 2, toY - 50))  // 第一段移动（向上弧线路径）
                        .scale(0.9)                                 // 缩放变化
                        .speed(0.2)                                 // 移动速度
                        .shape(QTimeLine::EaseOutCurve)              // 缓出曲线（开始快，结束慢）
                        .finish()                                   // 第一段结束回调（空）
                        .move(QPointF(toX, toY))                    // 第二段移动（落至目标位置）
                        .scale(1.0)                                 // 恢复原始大小
                        .speed(0.2)                                 // 移动速度
                        .shape(QTimeLine::EaseInCurve)               // 缓入曲线（开始慢，结束快）
                        .finish(onFinished);                        // 整体结束回调（由场景定义）

                    // 阳光生成后，向日葵恢复正常状态并设置下次生成定时器
                    picture->setMovieOnNewLoop(plantProtoType->normalGif, [this, generateSun] {
                        (new Timer(picture, 24000, [this, generateSun] {
                            (*generateSun)(); // 24秒后再次生成阳光（递归调用）
                        }))->start();
                    });
                }))->start();
            });
        };
        (*generateSun)(); // 立即执行第一次阳光生成
    }))->start();
}
// WallNut类 - 坚果墙原型定义
WallNut::WallNut()
{
    eName = "oWallNut";              // 英文名
    cName = tr("Wall-nut");          // 中文名（支持翻译）
    hp = 4000;                       // 高生命值，用于阻挡僵尸
    beAttackedPointR = 45;           // 右侧攻击判定点
    sunNum = 50;                     // 种植所需阳光
    coolTime = 30;                   // 冷却时间（秒）
    cardGif = "Card/Plants/WallNut.png";  // 卡片图片
    staticGif = "Plants/WallNut/0.gif";   // 静态图片
    normalGif = "Plants/WallNut/WallNut.gif";  // 正常动画
    toolTip = tr("Stops zombies with its chewy shell");  // 提示文本
}

// 判断是否可以在指定位置种植坚果墙
bool WallNut::canGrow(int x, int y) const
{
    // 检查坐标是否在有效范围内
    if (x < 1 || x > 9 || y < 1 || y > 5)
        return false;

    if (scene->isCrater(x, y) || scene->isTombstone(x, y))
        return false;
    int groundType = scene->getGameLevelData()->LF[y];

    // 获取目标位置的植物列表
    QMap<int, PlantInstance *> plants = scene->getPlant(x, y);

    // 普通土地：不能种植在其他非坚果墙上
    if (groundType == 1)
        return !plants.contains(1) || plants[1]->plantProtoType->eName == "oWallNut";

    return plants.contains(0) && (!plants.contains(1) || plants[1]->plantProtoType->eName == "oWallNut");
}

// 普通草坪割草机原型定义
LawnCleaner::LawnCleaner()
{
    eName = "oLawnCleaner";          // 英文名
    cName = tr("Lawn Cleaner");      // 中文名
    beAttackedPointL = 0;            // 左侧攻击判定点
    beAttackedPointR = 71;           // 右侧攻击判定点
    sunNum = 0;                      // 无需阳光（游戏开始时自动放置）
    staticGif = normalGif = "interface/LawnCleaner.png";  // 静态和正常状态图片
    canEat = 0;                      // 不可被吃掉
    stature = 1;                      // 高度
    toolTip = tr("Normal lawn cleaner");  // 提示文本
}

// 泳池割草机原型定义
PoolCleaner::PoolCleaner()
{
    eName = "oPoolCleaner";          // 英文名
    cName = tr("Pool Cleaner");      // 中文名
    beAttackedPointR = 47;           // 右侧攻击判定点
    staticGif = normalGif = "interface/PoolCleaner.png";  // 静态和正常状态图片
    toolTip = tr("Pool Cleaner");    // 提示文本
    update();                        // 更新图片尺寸信息
}



// 坚果墙实例类 - 禁用触发器（坚果墙不需要触发器）
void WallNutInstance::initTrigger()
{}

// 坚果墙实例构造函数
WallNutInstance::WallNutInstance(const Plant *plant)
    : PlantInstance(plant)
{
    hurtStatus = 0;                  // 伤害状态（0=正常，1=轻度损坏，2=严重损坏）
    crackedGif1 = "Plants/WallNut/Wallnut_cracked1.gif";  // 轻度损坏动画
    crackedGif2 = "Plants/WallNut/Wallnut_cracked2.gif";  // 严重损坏动画
}

// 处理坚果墙受到伤害
void WallNutInstance::getHurt(ZombieInstance *zombie, int aKind, int attack)
{
    // 调用基类处理伤害逻辑
    PlantInstance::getHurt(zombie, aKind, attack);

    if (hp > 0) {
        // 根据生命值切换不同的损坏状态
        if (hp < 1334) {
            if (hurtStatus < 2) {
                hurtStatus = 2;      // 严重损坏
                picture->setMovie(crackedGif2);
                picture->start();
            }
        }
        else if (hp < 2667) {
            if (hurtStatus < 1) {
                hurtStatus = 1;      // 轻度损坏
                picture->setMovie(crackedGif1);
                picture->start();
            }
        }
    }
}
// 割草机实例类 - 实现自动清除僵尸的功能
LawnCleanerInstance::LawnCleanerInstance(const Plant *plant)
    : PlantInstance(plant)
{}

// 初始化触发器 - 设置碰撞检测区域
void LawnCleanerInstance::initTrigger()
{
    // 创建一个触发器，用于检测僵尸是否进入激活范围
    Trigger *trigger = new Trigger(this, attackedLX, attackedRX, 0, 0);
    // 将触发器添加到当前行的触发器列表中
    triggers.insert(row, QList<Trigger *>{ trigger } );
    // 将触发器注册到场景中，开始监听碰撞事件
    plantProtoType->scene->addTrigger(row, trigger);
}

// 触发器检测回调 - 当僵尸进入触发区域时调用
void LawnCleanerInstance::triggerCheck(ZombieInstance *zombieInstance, Trigger *trigger)
{
    // 检查僵尸是否可被攻击且高度匹配
    if (zombieInstance->beAttacked && zombieInstance->altitude > 0) {
        canTrigger = 0;  // 禁用触发器，防止重复触发
        normalAttack(nullptr);  // 启动割草机攻击模式
    }
}

// 割草机攻击逻辑 - 清除整行僵尸
void LawnCleanerInstance::normalAttack(ZombieInstance *zombieInstance)
{
    // 播放割草机启动音效
    QSound::play(":/audio/lawnmower.wav");

    // 创建一个递归函数，用于持续清除僵尸并向右移动
    QSharedPointer<std::function<void(void)> > crush(new std::function<void(void)>);
    *crush = [this, crush] {
        // 获取当前行指定范围内的所有僵尸
        for (auto zombie: plantProtoType->scene->getZombieOnRowRange(row, attackedLX, attackedRX)) {
            // 尝试压碎僵尸（检查是否可以被压碎）
            if (zombie->getCrushed(this))
                zombie->crushDie();  // 压碎僵尸并播放死亡动画
        }

        // 如果割草机已移动到屏幕右侧，移除割草机
        if (attackedLX > 900)
            plantProtoType->scene->plantDie(this);
        else {
            // 否则继续向右移动
            attackedLX += 10;
            attackedRX += 10;
            picture->setPos(picture->pos() + QPointF(10, 0));  // 更新图片位置
            // 定时继续执行清除逻辑，形成持续移动效果
            (new Timer(picture, 10, *crush))->start();
        }
    };

    // 立即执行第一次清除
    (*crush)();
}
// 子弹类 - 表示植物发射的各种类型子弹
Bullet::Bullet(GameScene *scene, int type, int row, qreal from, qreal x, qreal y, qreal zvalue, int direction)
        : scene(scene), type(type), row(row), direction(direction), from(from)
{
    count = 0;  // 移动计数器，用于控制何时添加到场景
    // 根据子弹类型和方向加载对应的图片资源
    picture = new QGraphicsPixmapItem(gImageCache->load(QString("Plants/PB%1%2.gif").arg(type).arg(direction)));
    picture->setPos(x, y);           // 设置初始位置
    picture->setZValue(zvalue);      // 设置渲染层级
}

Bullet::~Bullet()
{
    delete picture;  // 清理图片资源
}

// 启动子弹移动
void Bullet::start()
{
    // 创建一个定时器，每20毫秒调用一次move()函数
    (new Timer(scene, 20, [this] {
        move();
    }))->start();
}

// 子弹移动和碰撞检测逻辑
void Bullet::move()
{
    // 延迟5帧后将子弹添加到场景，实现子弹发射动画效果
    if (count++ == 5)
        scene->addItem(picture);

    // 获取子弹当前所在的列
    int col = scene->getCoordinate().getCol(from);
    // 获取该位置的所有植物
    QMap<int, PlantInstance *> plants = scene->getPlant(col, row);

    // 处理子弹穿过火炬树桩的逻辑：普通豌豆(0)经过火炬树桩会变成火球(1)
    if (type < 1 && plants.contains(1) && plants[1]->plantProtoType->eName == "oTorchwood" && uuid != plants[1]->uuid) {
        ++type;  // 升级子弹类型（0->1）
        uuid = plants[1]->uuid;  // 记录穿过的火炬树桩ID，避免重复转换
        // 更新子弹图片为火球
        picture->setPixmap(gImageCache->load(QString("Plants/PB%1%2.gif").arg(type).arg(direction)));
    }

    // 检测前方是否有僵尸（向右发射的子弹）
    ZombieInstance *zombie = nullptr;
    if (direction == 0) {  // 0表示向右发射
        // 获取当前行的所有僵尸
        QList<ZombieInstance *> zombies = scene->getZombieOnRow(row);
        // 从后向前遍历僵尸列表（离植物最远的僵尸优先）
        for (auto iter = zombies.end(); iter-- != zombies.begin() && (*iter)->attackedLX <= from;) {
            // 找到第一个生命值大于0且在子弹攻击范围内的僵尸
            if ((*iter)->hp > 0 && (*iter)->attackedRX >= from) {
                zombie = *iter;
                break;
            }
        }
    }
    // TODO: 处理向左发射的子弹（direction != 0）

    // 如果检测到僵尸且僵尸高度与子弹匹配
    if (zombie && zombie->altitude == 1) {
        // 根据子弹类型造成不同伤害
        if (type == 0)
            zombie->getPea(20, direction);        // 普通豌豆伤害
        else if(type == -1)
            zombie->getSnowPea(20, direction);    // 冰冻豌豆伤害
        if (type == 1)
            zombie->getFirePea(40, direction);    // 火球伤害（双倍）
        if(type == 2 )zombie->getPea(30,direction); // 尖刺伤害
        // 显示子弹击中效果
        picture->setPos(picture->pos() + QPointF(28, 0));  // 调整位置
        picture->setPixmap(gImageCache->load("Plants/PeaBulletHit.gif"));  // 击中动画
        // 延迟后销毁子弹
        (new Timer(scene, 100, [this] {
            delete this;
        }))->start();
    }
    else {
        // 没有击中僵尸，继续移动
        from += direction ? -10 : 10;  // 更新子弹位置（根据方向决定左右移动）

        // 检查子弹是否在有效范围内
        if (from < 900 && from > 100) {
            picture->setPos(picture->pos() + QPointF(direction ? -10 : 10, 0));  // 更新显示位置
            // 继续移动
            (new Timer(scene, 20, [this] {
                move();
            }))->start();
        }
        else
            delete this;  // 超出范围，销毁子弹
    }
}
// PumpkinHead类 - 南瓜头原型定义
PumpkinHead::PumpkinHead()
{
    eName = "oPumpkinHead";              // 英文名
    cName = tr("Pumpkin Head");           // 中文名（支持翻译）
    beAttackedPointL = 15;                // 左侧攻击判定点
    beAttackedPointR = 82;                // 右侧攻击判定点
    sunNum = 125;                         // 种植需要的阳光
    pKind = 2;                            // 植物类型ID
    hp = 4000;                            // 高生命值，提供保护
    coolTime = 30;                        // 冷却时间（秒）
    zIndex = 1;                           // 渲染层级
    toolTip = tr("Protects the plant inside it"); // 提示文本
    cardGif = "Card/Plants/PumpkinHead.png";     // 卡片图片
    staticGif = "Plants/PumpkinHead/0.gif";      // 静态图片
    normalGif = "Plants/PumpkinHead/PumpkinHead1.gif"; // 正常动画
}

// 判断是否可以在指定位置种植南瓜头
bool PumpkinHead::canGrow(int x, int y) const
{
    // 检查目标位置是否已有南瓜头
    QMap<int, PlantInstance *> plants = scene->getPlant(x, y);
    if (plants.contains(pKind))
        return true;

    // 检查坐标是否在有效范围内
    if (x < 1 || x > 9 || y < 1 || y > 5)
        return false;


    if (scene->isCrater(x, y) || scene->isTombstone(x, y))
        return false;
    int groundType = scene->getGameLevelData()->LF[y];
    if (groundType == 2)
        return plants.contains(0);


    return true; // 普通土地可以直接种植
}

// 获取Y轴偏移量（根据下方是否有睡莲调整位置）
double PumpkinHead::getDY(int x, int y) const
{
    //-5
    return scene->getPlant(x, y).contains(0) ? -12 : -5;
}

// PumpkinHeadInstance类 - 南瓜头实例
PumpkinHeadInstance::PumpkinHeadInstance(const Plant *plant)
    : PlantInstance(plant), picture2(new MoviePixmapItem)
{
    hurtStatus = 0; // 伤害状态（0=正常，1=轻度损坏，2=严重损坏）
}

// 南瓜头出生（种植）时的初始化
void PumpkinHeadInstance::birth(int c, int r)
{
    Coordinate &coordinate = plantProtoType->scene->getCoordinate();
    double x = coordinate.getX(c) + plantProtoType->getDX();
    double y = coordinate.getY(r) + plantProtoType->getDY(c, r) - plantProtoType->height;
    col = c, row = r;

    // 设置攻击判定区域
    attackedLX = x + plantProtoType->beAttackedPointL;
    attackedRX = x + plantProtoType->beAttackedPointR;

    // 初始化主图片（南瓜头主体）
    picture->setMovie(plantProtoType->normalGif);
    picture->setPos(x, y);
    picture->setZValue(plantProtoType->zIndex + 3 * r);

    // 添加阴影效果
    shadowPNG = new QGraphicsPixmapItem(gImageCache->load("interface/plantShadow.png"));
    shadowPNG->setPos(plantProtoType->width * 0.5 - 48, plantProtoType->height - 22);
    shadowPNG->setFlag(QGraphicsItem::ItemStacksBehindParent);
    shadowPNG->setParentItem(picture);

    // 将主图片添加到场景
    picture->start();
    plantProtoType->scene->addToGame(picture);

    // 初始化第二层图片（南瓜头内部）
    picture2->setMovie("Plants/PumpkinHead/PumpkinHead2.gif");
    picture2->setPos(picture->pos());
    picture2->setZValue(picture->zValue() - 2);
    plantProtoType->scene->addToGame(picture2);
    picture2->start();
}

// 处理南瓜头受到伤害
void PumpkinHeadInstance::getHurt(ZombieInstance *zombie, int aKind, int attack)
{
    // 调用基类处理伤害逻辑
    PlantInstance::getHurt(zombie, aKind, attack);

    if (hp > 0) {
        // 根据生命值切换不同的损坏状态
        if (hp < 1334) {
            if (hurtStatus < 2) {
                hurtStatus = 2; // 严重损坏
                picture->setMovie("Plants/PumpkinHead/pumpkin_damage2.gif");
                picture->start();
            }
        }
        else if (hp < 2667) {
            if (hurtStatus < 1) {
                hurtStatus = 1; // 轻度损坏
                picture->setMovie("Plants/PumpkinHead/pumpkin_damage1.gif");
                picture->start();
                picture2->setMovie("Plants/PumpkinHead/Pumpkin_back.gif");
                picture2->start();
            }
        }
    }
}

// 析构函数 - 清理资源
PumpkinHeadInstance::~PumpkinHeadInstance()
{
    picture2->deleteLater();
}

Torchwood::Torchwood()
{
    eName = "oTorchwood";
    cName = tr("Torchwood");
    beAttackedPointR = 53;
    sunNum = 175;
    toolTip = tr("Turns peas that pass through them into fireballs");
    cardGif = "Card/Plants/Torchwood.png";
    staticGif = "Plants/Torchwood/0.gif";
    normalGif = "Plants/Torchwood/Torchwood.gif";
}

TorchwoodInstance::TorchwoodInstance(const Plant *plant)
    : PlantInstance(plant)
{}

void TorchwoodInstance::initTrigger()
{}

TallNut::TallNut()
{
    eName = "oTallNut";
    cName = tr("Tall Nut");
    beAttackedPointR = 63;
    sunNum = 125;
    hp = 8000;
    toolTip = tr("Heavy-duty wall plants that can't be vaulted or jumped over");
    cardGif = "Card/Plants/TallNut.png";
    staticGif = "Plants/TallNut/0.gif";
    normalGif = "Plants/TallNut/TallNut.gif";
    stature = 1;
}

bool TallNut::canGrow(int x, int y) const
{
    if (x < 1 || x > 9 || y < 1 || y > 5)
        return false;
    if (scene->isCrater(x, y) || scene->isTombstone(x, y))
        return false;
    int groundType = scene->getGameLevelData()->LF[y];
    QMap<int, PlantInstance *> plants = scene->getPlant(x, y);
    if (groundType == 1)
        return !plants.contains(1) || plants[1]->plantProtoType->eName == "oTallNut";
    return plants.contains(0) && (!plants.contains(1) || plants[1]->plantProtoType->eName == "oTallNut");
}

void TallNutInstance::getHurt(ZombieInstance *zombie, int aKind, int attack)
{
    PlantInstance::getHurt(zombie, aKind, attack);
    if (hp > 0) {
        if (hp < 1334) {
            if (hurtStatus < 2) {
                hurtStatus = 2;
                picture->setMovie("Plants/TallNut/TallnutCracked2.gif");
                picture->start();
            }
        }
        else if (hp < 2667) {
            if (hurtStatus < 1) {
                hurtStatus = 1;
                picture->setMovie("Plants/TallNut/TallnutCracked1.gif");
                picture->start();
            }
        }
    }
}

TallNutInstance::TallNutInstance(const Plant *plant)
    : WallNutInstance(plant)
{
    hurtStatus = 0;
}


Plant *PlantFactory(GameScene *scene, const QString &eName)
{
    Plant *plant = nullptr;
    if (eName == "oPeashooter")
        plant = new Peashooter;
    else if (eName == "oSnowPea")
        plant = new SnowPea;
    else if (eName == "oSunflower")
        plant = new SunFlower;
    else if (eName == "oWallNut")
        plant = new WallNut;
    else if (eName == "oLawnCleaner")
        plant = new LawnCleaner;
    else if (eName == "oPoolCleaner")
        plant = new PoolCleaner;
    else if (eName == "oPumpkinHead")
        plant = new PumpkinHead;
    else if (eName == "oTorchwood")
        plant = new Torchwood;
    else if (eName == "oTallNut")
        plant = new TallNut;
    else if (eName == "oThreepeater")
        plant = new Threepeater;
    else if (eName == "oRepeater")
        plant = new Repeater;
    //增加Jalapeno
    else if(eName == "oJalapeno")
        plant = new Jalapeno;
    //增加squash
    else if(eName == "oSquash")
        plant = new Squash;
    else if(eName =="oCactus")
        plant = new Cactus;
    if (plant) {
        plant->scene = scene;
        plant->update();
    }
    return plant;
}

PlantInstance *PlantInstanceFactory(const Plant *plant)
{
    if (plant->eName == "oPeashooter")
        return new PeashooterInstance(plant);
    if (plant->eName == "oSnowPea")
        return new SnowPeaInstance(plant);
    if (plant->eName == "oSunflower")
        return new SunFlowerInstance(plant);
    if (plant->eName == "oWallNut")
        return new WallNutInstance(plant);
    if (plant->eName == "oLawnCleaner")
        return new LawnCleanerInstance(plant);
    if (plant->eName == "oPumpkinHead")
        return new PumpkinHeadInstance(plant);
    if (plant->eName == "oTorchwood")
        return new TorchwoodInstance(plant);
    if (plant->eName == "oTallNut")
        return new TallNutInstance(plant);
    if (plant->eName == "oThreepeater")
        return new ThreepeaterInstance(plant);
    if (plant->eName == "oRepeater")
        return new RepeaterInstance(plant);
    //增加Jalapeno
    if(plant->eName == "oJalapeno")
        return new JalapenoInstance(plant);
    //增加Squash
    if(plant->eName == "oSquash")
        return new SquashInstance(plant);

    // 添加了Cactus
    if(plant->eName == "oCactus")
        return new CactusInstance(plant);

    return new PlantInstance(plant);
}
