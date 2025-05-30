#ifndef PLANTS_VS_ZOMBIES_ZOMBIE_H
#define PLANTS_VS_ZOMBIES_ZOMBIE_H

#include <QtCore>
#include <QtWidgets>
#include <QtMultimedia>
#include "Plant.h"

class MoviePixmapItem;
class GameScene;
class PlantInstance;

/**
 * @brief 僵尸基类，定义了僵尸的基本属性和行为
 *
 * 包含僵尸的基本属性如生命值、速度、各种动画资源路径等
 * 以及基本行为如判断是否可以通过、更新属性等
 */
class Zombie
{
    Q_DECLARE_TR_FUNCTIONS(Zombie)
public:
    Zombie();
    virtual ~Zombie() {}

    QString eName, cName;             // 僵尸的英文和中文名称

    int width, height;                // 僵尸的宽度和高度

    int hp, level;                    // 生命值和等级
    qreal speed;                      // 移动速度
    int aKind, attack;                // 攻击类型和攻击力
    bool canSelect, canDisplay;       // 是否可选择、是否可显示

    // 各种动画资源路径
    QString cardGif, staticGif, normalGif, attackGif, lostHeadGif,
            lostHeadAttackGif, headGif, dieGif, boomDieGif, standGif, crushedDieGif;

    int beAttackedPointL, beAttackedPointR;  // 左右受击判定点
    int breakPoint, sunNum;                  // 临界点和阳光值
    qreal coolTime;                          // 冷却时间

    /**
     * @brief 判断僵尸是否可以通过指定行
     * @param row 指定的行号
     * @return 如果可以通过返回true，否则返回false
     */
    virtual bool canPass(int row) const;

    /**
     * @brief 更新僵尸属性
     *
     * 通常用于加载图片资源后更新僵尸的宽高等属性
     */
    void update();

    GameScene *scene;  // 所属游戏场景
};

/**
 * @brief 普通僵尸1类，继承自Zombie
 *
 * 最基础的僵尸类型，其他僵尸类型可在此基础上扩展
 */
class Zombie1: public Zombie
{
    Q_DECLARE_TR_FUNCTIONS(Zombie1)
public:
    Zombie1();
};

/**
 * @brief 普通僵尸2类，继承自Zombie1
 *
 * 普通僵尸的变种，可能有不同的外观或轻微属性差异
 */
class Zombie2: public Zombie1
{
    Q_DECLARE_TR_FUNCTIONS(Zombie2)
public:
    Zombie2();
};

/**
 * @brief 普通僵尸3类，继承自Zombie1
 *
 * 普通僵尸的另一种变种
 */
class Zombie3: public Zombie1
{
    Q_DECLARE_TR_FUNCTIONS(Zombie3)
public:
    Zombie3();
};

/**
 * @brief 旗帜僵尸类，继承自Zombie1
 *
 * 通常作为一波僵尸的首领出现，外观带有旗帜
 */
class FlagZombie: public Zombie1
{
    Q_DECLARE_TR_FUNCTIONS(FlagZombie)
public:
    FlagZombie();
};

/**
 * @brief 僵尸实例类，包含僵尸实例的具体行为
 *
 * 处理僵尸的出生、攻击判断、攻击行为、死亡方式等具体逻辑
 */
class ZombieInstance
{
public:
    ZombieInstance(const Zombie *zombie);
    virtual ~ZombieInstance();

    /**
     * @brief 处理僵尸被爆炸攻击的逻辑
     *
     * 例如被辣椒炸弹等范围攻击击中时调用
     */
    virtual void getBoomed();  // 新增爆炸死亡函数

    virtual void birth(int row);                // 僵尸出生逻辑
    virtual void checkActs();                   // 检查行为状态
    virtual void judgeAttack();                 // 判断是否可以攻击
    virtual void normalAttack(PlantInstance *plant); // 普通攻击行为
    virtual void crushDie();                    // 被压碎死亡
    virtual void getPea(int attack, int direction); // 被普通豌豆击中
    virtual void getSnowPea(int attack, int direction); // 被冰冻豌豆击中
    virtual void getFirePea(int attack, int direction); // 被火球击中
    virtual void getHit(int attack);            // 被普通攻击击中
    virtual void autoReduceHp();                // 自动减少生命值
    virtual void normalDie();                   // 普通死亡方式
    virtual void playNormalballAudio();         // 播放普通豌豆击中音效
    virtual void playSlowballAudio();           // 播放冰冻豌豆击中音效
    virtual void playFireballAudio();           // 播放火球击中音效
    virtual QPointF getShadowPos();             // 获取阴影位置
    virtual QPointF getDieingHeadPos();         // 获取死亡时头部位置
    virtual bool getCrushed(PlantInstance *instance); // 判断是否被压碎

    QUuid uuid;                  // 唯一标识
    int hp;                      // 当前生命值
    qreal speed, orignSpeed;     // 当前速度和原始速度
    int attack, orignAttack;     // 当前攻击力和原始攻击力
    int altitude;                // 高度（用于判断是否能被某些攻击击中）
    bool beAttacked, isAttacking, goingDie; // 被攻击状态、攻击状态、死亡状态

    qreal X, ZX;                 // 位置坐标
    qreal attackedLX, attackedRX; // 受击范围
    int row;                     // 所在行
    const Zombie *zombieProtoType; // 僵尸原型

    // 各种动画资源路径
    QString normalGif, attackGif, lostHeadGif, lostHeadAttackGif, crushedDieGif;

    // 新增属性
    bool canJump;  // 是否能跳跃

    QTimer *frozenTimer;          // 冰冻计时器
    QGraphicsPixmapItem *shadowPNG; // 阴影图片
    MoviePixmapItem *picture;     // 主图片
};

/**
 * @brief 装饰性僵尸基类，继承自Zombie1
 *
 * 带有装饰物的僵尸，装饰物有额外生命值，被破坏前提供保护
 */
class OrnZombie1: public Zombie1
{
    Q_DECLARE_TR_FUNCTIONS(OrnZombie1)
public:
    int ornHp;                    // 装饰物生命值
    QString ornLostNormalGif, ornLostAttackGif; // 装饰物丢失后的动画
};

/**
 * @brief 装饰性僵尸实例基类，继承自ZombieInstance
 *
 * 处理带有装饰物的僵尸实例的特殊逻辑
 */
class OrnZombieInstance1: public ZombieInstance
{
public:
    OrnZombieInstance1(const Zombie *zombie);
    const OrnZombie1 *getZombieProtoType();
    virtual void getHit(int attack); // 重写受击逻辑，处理装饰物

    int ornHp;                     // 装饰物当前生命值
    bool hasOrnaments;             // 是否还有装饰物
};

/**
 * @brief 路障僵尸类，继承自OrnZombie1
 *
 * 头部带有路障作为装饰物，提供额外保护
 */
class ConeheadZombie: public OrnZombie1
{
    Q_DECLARE_TR_FUNCTIONS(ConeheadZombie)
public:
    ConeheadZombie();
};

/**
 * @brief 路障僵尸实例类，继承自OrnZombieInstance1
 *
 * 处理路障僵尸实例的特殊逻辑
 */
class ConeheadZombieInstance: public OrnZombieInstance1
{
public:
    ConeheadZombieInstance(const Zombie *zombie);
    virtual void playNormalballAudio(); // 重写音效播放
};

/**
 * @brief 铁桶僵尸类，继承自ConeheadZombie
 *
 * 头部带有铁桶作为装饰物，提供比路障更强的保护
 */
class BucketheadZombie: public ConeheadZombie
{
    Q_DECLARE_TR_FUNCTIONS(BucketheadZombie)
public:
    BucketheadZombie();
};

/**
 * @brief 铁桶僵尸实例类，继承自OrnZombieInstance1
 *
 * 处理铁桶僵尸实例的特殊逻辑
 */
class BucketheadZombieInstance: public OrnZombieInstance1
{
public:
    BucketheadZombieInstance(const Zombie *zombie);
    virtual void playNormalballAudio(); // 重写音效播放
};

/**
 * @brief 铁门僵尸类，继承自ConeheadZombie
 *
 * 手持铁门作为装饰物，提供保护
 */
class ScreenDoorZombie: public ConeheadZombie
{
    Q_DECLARE_TR_FUNCTIONS(ScreenDoorZombie)
public:
    ScreenDoorZombie();
};

/**
 * @brief 铁门僵尸实例类，继承自OrnZombieInstance1
 *
 * 处理铁门僵尸实例的特殊逻辑
 */
class ScreenDoorZombieInstance: public OrnZombieInstance1
{
public:
    ScreenDoorZombieInstance(const Zombie *zombie);
    virtual void playNormalballAudio(); // 重写音效播放
};

/**
 * @brief 撑杆跳僵尸类，继承自Zombie1
 *
 * 可以跳过某些障碍物的特殊僵尸
 */
class PoleVaultingZombie: public Zombie1
{
    Q_DECLARE_TR_FUNCTIONS(PoleVaultingZombie)
public:
    PoleVaultingZombie();

    // 特殊动画资源路径
    QString walkGif, lostHeadWalkGif, jumpGif1, jumpGif2;
};

/**
 * @brief 撑杆跳僵尸实例类，继承自ZombieInstance
 *
 * 处理撑杆跳僵尸实例的特殊逻辑，如跳跃行为
 */
class PoleVaultingZombieInstance: public ZombieInstance
{
    Q_DECLARE_TR_FUNCTIONS(PoleVaultingZombieInstance)
public:
    PoleVaultingZombieInstance(const Zombie *zombie);
    virtual QPointF getShadowPos();           // 重写阴影位置
    virtual QPointF getDieingHeadPos();       // 重写死亡头部位置
    virtual void judgeAttack();               // 重写攻击判断
    virtual void normalAttack(PlantInstance *plantInstance); // 重写攻击行为
    const PoleVaultingZombie *getZombieProtoType(); // 获取原型

    // 特殊状态变量
    bool judgeAttackOrig, lostPole, beginCrushed;
    qreal posX; // 用于传递位置信息到攻击函数
};

// 僵尸工厂函数，用于创建僵尸原型
Zombie *ZombieFactory(GameScene *scene, const QString &ename);

// 僵尸实例工厂函数，用于创建僵尸实例
ZombieInstance *ZombieInstanceFactory(const Zombie *zombie);

#endif //PLANTS_VS_ZOMBIES_ZOMBIE_H
