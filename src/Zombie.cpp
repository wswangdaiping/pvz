#include "Zombie.h"
#include "GameScene.h"
#include "GameLevelData.h"
#include "ImageManager.h"
#include "MouseEventPixmapItem.h"
#include "Plant.h"
#include "Timer.h"


//Zombie 类是所有僵尸类的基类，它定义了僵尸的基本属性和方法，例如僵尸的名称、生命值、速度、攻击方式等
// Zombie类构造函数，初始化僵尸基础属性
Zombie::Zombie()
    : hp(270), level(1), speed(0.5),    // 生命值/等级/移动速度（可调节难度）
      aKind(0), attack(100),            // 攻击类型/攻击力
      canSelect(true), canDisplay(true), // 可选中/可显示标识
      beAttackedPointL(82), beAttackedPointR(156), // 攻击判定左右边界
      breakPoint(90), sunNum(0), coolTime(0) // 护甲破碎阈值/产生阳光数/冷却时间
{}

// 判断僵尸是否可通过指定行（陆地/水面判定）
bool Zombie::canPass(int row) const
{
    // LF[row]=1表示陆地，僵尸可通过
    return scene->getGameLevelData()->LF[row] == 1;
}

// 更新僵尸图片尺寸（根据静态图自动获取）
void Zombie::update()
{
    QPixmap pic = gImageCache->load(staticGif); // 加载静态图片
    width = pic.width();  // 更新宽度
    height = pic.height();// 更新高度
}

// Zombie1类构造函数（普通僵尸）
Zombie1::Zombie1()
{
    eName = "oZombie";                // 英文名称
    cName = tr("Zombie");            // 中文名称（支持翻译）
    cardGif = "Card/Zombies/Zombie.png"; // 卡片图片
    QString path = "Zombies/Zombie/";  // 动画资源基础路径

    // 各状态动画路径
    staticGif = path + "0.gif";       // 静态站立
    normalGif = path + "Zombie.gif";  // 正常行走
    attackGif = path + "ZombieAttack.gif"; // 攻击
    lostHeadGif = path + "ZombieLostHead.gif"; // 失头行走
    lostHeadAttackGif = path + "ZombieLostHeadAttack.gif"; // 失头攻击
    headGif = path + "ZombieHead.gif"; // 头部飞溅
    dieGif = path + "ZombieDie.gif";  // 死亡
    boomDieGif = path + "BoomDie.gif";// 爆炸死亡
    standGif = path + "1.gif";       // 站立

    // wdp新增：被压碎死亡动画
    crushedDieGif = path+"crushedDie.gif";
}

// Zombie2类构造函数（僵尸变种2）
Zombie2::Zombie2()
{
    eName = "oZombie2";               // 英文名称
    normalGif = "Zombies/Zombie/Zombie2.gif"; // 行走动画
    standGif = "Zombies/Zombie/2.gif"; // 站立动画
}

// Zombie3类构造函数（僵尸变种3）
Zombie3::Zombie3()
{
    eName = "oZombie3";               // 英文名称
    normalGif = "Zombies/Zombie/Zombie3.gif"; // 行走动画
    standGif = "Zombies/Zombie/3.gif"; // 站立动画
}

// FlagZombie类构造函数（旗帜僵尸，更快的普通僵尸）
FlagZombie::FlagZombie()
{
    eName = "oFlagZombie";            // 英文名称
    cName = tr("Flag Zombie");        // 中文名称
    speed = 2.2;                      // 更高的移动速度
    beAttackedPointR = 101;            // 攻击判定右边界
    QString path = "Zombies/FlagZombie/"; // 动画资源路径

    cardGif = "Card/Zombies/FlagZombie.png"; // 卡片图片
    staticGif = path + "0.gif";       // 静态站立
    normalGif = path + "FlagZombie.gif";  // 行走动画（附带音效）
    QSound::play(":/audio/splat1.wav");     // 出场音效
    attackGif = path + "FlagZombieAttack.gif"; // 攻击
    lostHeadGif = path + "FlagZombieLostHead.gif"; // 失头行走
    lostHeadAttackGif = path + "FlagZombieLostHeadAttack.gif"; // 失头攻击
    standGif = path + "1.gif";       // 站立
}

// ZombieInstance构造函数（僵尸实例）
ZombieInstance::ZombieInstance(const Zombie *zombie)
    : zombieProtoType(zombie), frozenTimer(nullptr), picture(new MoviePixmapItem)
{
    uuid = QUuid::createUuid(); // 生成唯一标识
    hp = zombieProtoType->hp;  // 继承原型生命值
    orignSpeed = speed = zombie->speed; // 原始速度/当前速度
    orignAttack = attack = zombie->attack; // 原始攻击/当前攻击
    altitude = 1;            // 高度（1=地面）
    beAttacked = true;       // 可被攻击标识
    isAttacking = false;     // 攻击状态
    goingDie = false;        // 死亡状态
    normalGif = zombie->normalGif; // 行走动画
    attackGif = zombie->attackGif; // 攻击动画
    lostHeadGif = zombie->lostHeadGif; // 失头动画
    lostHeadAttackGif = zombie->lostHeadAttackGif; // 失头攻击动画
    canJump = false;         // 默认不能跳跃（子类可重写）
    crushedDieGif = zombie->crushedDieGif; // 被压碎死亡动画
}
//wdp 新增实现getBoomed()函数
void ZombieInstance::getBoomed() {
    if (goingDie) return; // 防止重复触发
    // 1. 立即标记死亡状态
    goingDie = true;
    beAttacked = false;
    hp = 0;

    // 2. 播放灰烬动画（替换原有死亡动画）
    picture->setMovie(zombieProtoType->boomDieGif);
    printf("boomdie.gif");
    picture->start();

    // 5. 延迟后从场景移除
    (new Timer(picture, 1500, [this] {  // 动画持续1.5秒
        zombieProtoType->scene->zombieDie(this);
    }))->start();
}
//
// 僵尸实例出生初始化函数，设置初始位置和动画
void ZombieInstance::birth(int row)
{
    // 设置僵尸的初始位置，通过场景的坐标系统获取第11列对应的X坐标
    ZX = attackedLX = zombieProtoType->scene->getCoordinate().getX(11);
    // 计算僵尸的实际X坐标，减去僵尸原型的左攻击点偏移量
    X = attackedLX - zombieProtoType->beAttackedPointL;
    // 计算僵尸的右攻击边界，通过实际X坐标加上僵尸原型的右攻击点偏移量
    attackedRX = X + zombieProtoType->beAttackedPointR;
    // 设置僵尸所在的行
    this->row = row;

    // 获取场景的坐标系统引用，方便后续使用
    Coordinate &coordinate = zombieProtoType->scene->getCoordinate();
    // 设置僵尸的动画为正常移动动画
    picture->setMovie(normalGif);
    // 设置僵尸动画的位置，Y坐标根据所在行和僵尸原型的高度进行计算
    picture->setPos(X, coordinate.getY(row) - zombieProtoType->height - 10);
    // 设置僵尸动画的Z值，用于确定其在场景中的绘制顺序
    picture->setZValue(3 * row + 1);

    // 创建一个阴影图形项，加载植物阴影图片
    shadowPNG = new QGraphicsPixmapItem(gImageCache->load("interface/plantShadow.png"));
    // 设置阴影的位置，通过调用getShadowPos()函数计算
    shadowPNG->setPos(getShadowPos());
    // 设置阴影图形项总是位于其父项（即僵尸动画）的后面
    shadowPNG->setFlag(QGraphicsItem::ItemStacksBehindParent);
    // 将阴影图形项设置为僵尸动画的子项
    shadowPNG->setParentItem(picture);

    // 启动僵尸的动画
    picture->start();
    // 将僵尸动画添加到游戏场景中
    zombieProtoType->scene->addToGame(picture);
}
// 僵尸行为检查函数（每帧调用）
void ZombieInstance::checkActs() //判定输赢
{
    if (hp < 1) return;                                              // 生命值为0时不执行任何操作

    // 检查是否可攻击且未在攻击状态
    if (beAttacked && !isAttacking) {
        judgeAttack();                                               // 判断是否攻击植物
    }

    // 未在攻击时执行移动逻辑
    if (!isAttacking) {
        attackedRX -= speed;                                          // 右边界左移（向左移动）
        ZX = attackedLX -= speed;                                     // 更新攻击判定左边界和ZX坐标
        X -= speed;                                                  // 僵尸图片位置左移
        picture->setX(X);                                             // 更新渲染位置

        // 超出屏幕范围时销毁僵尸
        if (attackedRX < -50) {
            zombieProtoType->scene->zombieDie(this);
        }
        // 到达植物区域时判定游戏失败
        else if (attackedRX < 100) {         //小于100 输
            zombieProtoType->scene->gameLose();
        }
    }
}

// 攻击判定函数（检查是否发现可攻击的植物）
void ZombieInstance::judgeAttack()
{
    bool tempIsAttacking = false;                                     // 临时攻击状态
    PlantInstance *plant = nullptr;                                   // 目标植物指针

    // 获取当前ZX坐标对应的列号
    int col = zombieProtoType->scene->getCoordinate().getCol(ZX);
    if (col >= 1 && col <= 9) {                                       // 检查列号有效性
        auto plants = zombieProtoType->scene->getPlant(col, row);      // 获取该位置所有植物
        QList<int> keys = plants.keys();                              // 获取植物类型键列表

        // 按类型降序排序（优先检查高优先级植物）
        qSort(keys.begin(), keys.end(), [](int a, int b) { return b < a; });

        // 遍历植物列表寻找可攻击目标
        for (auto key: keys) {
            plant = plants[key];
            // 检查植物是否可被吃且僵尸在攻击范围内
            if (plant->plantProtoType->canEat && plant->attackedRX >= ZX && plant->attackedLX <= ZX) {
                tempIsAttacking = true;
                break;
            }
        }
    }

    // 状态变化时更新动画
    if (tempIsAttacking != isAttacking) {
        isAttacking = tempIsAttacking;
        if (isAttacking) {
            picture->setMovie(attackGif);                             // 切换为攻击动画
        } else {
            picture->setMovie(normalGif);                             // 切换为行走动画
        }
        picture->start();                                            // 重新启动动画
    }

    // 处于攻击状态时执行攻击逻辑
    if (isAttacking)
        normalAttack(plant);
}

// 普通攻击函数（对植物造成伤害）
void ZombieInstance::normalAttack(PlantInstance *plantInstance)
{
    // 随机播放两种啃食音效
    if (qrand() % 2)
        QSound::play(":/audio/chomp.wav");
    else
        QSound::play(":/audio/chompsoft.wav");

    // 0.5秒后再次播放音效（模拟持续啃食）
    (new Timer(this->picture, 500, [this] {
        if (qrand() % 2)
            QSound::play(":/audio/chomp.wav");
        else
            QSound::play(":/audio/chompsoft.wav");
    }))->start();

    // 记录目标植物UUID
    QUuid plantUuid = plantInstance->uuid;

    // 1秒后执行伤害逻辑
    (new Timer(this->picture, 1000, [this, plantUuid] {
        if (beAttacked) {                                            // 僵尸可被攻击时才执行
            PlantInstance *plant = zombieProtoType->scene->getPlant(plantUuid);
            if (plant)
                plant->getHurt(this, zombieProtoType->aKind, attack);  // 对植物造成伤害
            judgeAttack();                                           // 重新判断攻击状态
        }
    }))->start();
}

// 析构函数（释放资源）
ZombieInstance::~ZombieInstance()
{
    picture->deleteLater();                                          // 延迟删除图片项，避免渲染冲突
}



// 僵尸被压碎死亡的处理函数
void ZombieInstance::crushDie()
{
    // 若已处于死亡状态则直接返回
    if (goingDie)
        return;
    // 标记为死亡状态并设置不可被攻击
    goingDie = true;
    beAttacked = false;
    hp = 0;

    // 调试输出被压碎动画路径（用于排查资源加载问题）
    qDebug() << "crushedDieGif path:" << zombieProtoType->crushedDieGif;
    // 创建被压碎死亡动画对象
    MoviePixmapItem *crushedDieItem = new MoviePixmapItem(zombieProtoType->crushedDieGif);
    // 资源加载异常处理
    if (crushedDieItem == nullptr ) {
        qDebug() << "Failed to load crushedDieGif";
    }
    // 设置动画位置（与僵尸当前位置一致）
    crushedDieItem->setPos(X, picture->y());
    // 添加到游戏场景并播放动画
    zombieProtoType->scene->addToGame(crushedDieItem);
    crushedDieItem->start();

    // 清除阴影和当前僵尸图片
    shadowPNG->setPixmap(QPixmap());
    picture->stop();
    picture->setPixmap(QPixmap());

    // 2秒后清理资源并通知场景移除僵尸
    (new Timer(picture, 2000, [this, crushedDieItem] {
        crushedDieItem->deleteLater();
        zombieProtoType->scene->zombieDie(this);
    }))->start();
}

// 僵尸被普通豌豆击中的处理函数
void ZombieInstance::getPea(int attack, int direction)
{
    // 播放豌豆击中音效
    playNormalballAudio();
    // 执行伤害计算
    getHit(attack);
}

// 僵尸受到伤害的核心处理函数
void ZombieInstance::getHit(int attack)
{
    // 若不可被攻击或已死亡则跳过
    if (!beAttacked || goingDie)
        return;
    // 减少生命值
    hp -= attack;
    // 生命值低于护甲破碎阈值时触发头部脱落
    if (hp < zombieProtoType->breakPoint) {
        // 更新动画为失头状态（根据是否在攻击选择不同动画）
        if (isAttacking)
            picture->setMovie(lostHeadAttackGif);
        else
            picture->setMovie(lostHeadGif);
        picture->start();
        // 创建头部飞溅动画
        MoviePixmapItem *goingDieHead = new MoviePixmapItem(zombieProtoType->headGif);
        goingDieHead->setPos(getDieingHeadPos());       // 设置头部位置
        goingDieHead->setZValue(picture->zValue());    // 保持与僵尸相同层级
        zombieProtoType->scene->addToGame(goingDieHead);
        goingDieHead->start();
        // 2秒后清理头部动画
        (new Timer(zombieProtoType->scene, 2000, [goingDieHead] {
            goingDieHead->deleteLater();
        }))->start();
        // 标记为不可被攻击并启动持续掉血
        beAttacked = 0;
        autoReduceHp();
    }
    // 未触发护甲破碎时显示受击闪烁效果
    else {
        picture->setOpacity(0.5);
        (new Timer(picture, 100, [this] {
            picture->setOpacity(1);
        }))->start();
    }
}

// 僵尸失头后持续掉血的处理函数
void ZombieInstance::autoReduceHp()
{
    // 每秒减少60点生命值
    (new Timer(picture, 1000, [this] {
        hp-= 60;
        // 生命值归0时执行正常死亡逻辑
        if (hp < 1)
            normalDie();
        else
            autoReduceHp();  // 继续掉血
    }))->start();
}

// 僵尸正常死亡的处理函数
void ZombieInstance::normalDie()
{
    // 若已处于死亡状态则直接返回
    if (goingDie)
        return;
    goingDie =  true;
    hp = 0;
    // 播放死亡动画
    picture->setMovie(zombieProtoType->dieGif);
    picture->start();
    // 2.5秒后通知场景移除僵尸
    (new Timer(picture, 2500, [this] {
        zombieProtoType->scene->zombieDie(this);
    }))->start();
}

// 播放普通豌豆击中音效的函数（随机选择三种音效之一）
void ZombieInstance::playNormalballAudio()
{
    switch (qrand() % 3) {
        case 0: QSound::play(":/audio/splat1.wav"); break;
        case 1: QSound::play(":/audio/splat2.wav"); break;
        default: QSound::play(":/audio/splat3.wav"); break;
    }
}

// 获取僵尸阴影位置的函数
QPointF ZombieInstance::getShadowPos()
{
    // 根据僵尸攻击判定点和高度计算阴影偏移
    return QPointF(zombieProtoType->beAttackedPointL - 10, zombieProtoType->height - 22);
}

// 获取僵尸死亡时头部位置的函数
QPointF ZombieInstance::getDieingHeadPos()
{
    // 头部位置在攻击判定左边界上方20像素
    return QPointF(attackedLX, picture->y() - 20);
}

// 判断僵尸是否可被压碎的函数（默认返回true）
bool ZombieInstance::getCrushed(PlantInstance *instance)
{
    return true;
}

// 僵尸被冰冻豌豆击中的处理函数
void ZombieInstance::getSnowPea(int attack, int direction)
{
    // 清除已有的冰冻效果
    if (frozenTimer) {
        frozenTimer->stop();
        frozenTimer->deleteLater();
    }
    // 降低移动速度和攻击力（持续10秒）
    speed = orignSpeed / 2;
    this->attack = 50;
    // 10秒后恢复正常状态
    (frozenTimer = new Timer(picture, 10000, [this] {
        frozenTimer = nullptr;
        speed = orignSpeed;
        this->attack = orignAttack;
    }))->start();
    // 播放冰冻音效
    playSlowballAudio();
    // 执行伤害计算
    getHit(attack);
}

// 播放冰冻音效的函数
void ZombieInstance::playSlowballAudio()
{
    QSound::play(":/audio/frozen.wav");
}

// 僵尸被火球击中的处理函数（移除冰冻效果并造成伤害）
void ZombieInstance::getFirePea(int attack, int direction)
{
    // 若有冰冻效果则清除
    if (frozenTimer) {
        frozenTimer->stop();
        frozenTimer->deleteLater();
        frozenTimer = nullptr;
        speed = orignSpeed;
        this->attack = orignAttack;
    }
    // 播放火焰音效
    playFireballAudio();
    // 执行伤害计算
    getHit(attack);
}

// 播放火焰音效的函数（随机选择两种音效之一）
void ZombieInstance::playFireballAudio()
{
    if (qrand() % 2)
        QSound::play(":/audio/ignite.wav");
    else
        QSound::play(":/audio/ignite2.wav");
}


// 装饰型僵尸实例基类构造函数（装饰性僵尸）
OrnZombieInstance1::OrnZombieInstance1(const Zombie *zombie)
    : ZombieInstance(zombie)
{
    ornHp = getZombieProtoType()->ornHp;    // 初始化护甲生命值
    hasOrnaments = true;                    // 标记初始状态有护甲
}

// 获取装饰型僵尸原型（向下转型）
const OrnZombie1 *OrnZombieInstance1::getZombieProtoType()
{
    return static_cast<const OrnZombie1 *>(zombieProtoType);
}

// 重写受击函数，处理护甲逻辑
void OrnZombieInstance1::getHit(int attack)
{
    if (hasOrnaments) {                     // 护甲存在时
        ornHp -= attack;                    // 先扣除护甲生命值
        if (ornHp < 1) {                    // 护甲被击破
            hp += ornHp;                    // 将剩余护甲值加到本体生命值（可能为负值）
            hasOrnaments = false;           // 标记护甲已丢失
            // 更新动画为无护甲状态
            normalGif = getZombieProtoType()->ornLostNormalGif;
            attackGif = getZombieProtoType()->ornLostAttackGif;
            picture->setMovie(isAttacking ? attackGif : normalGif);
            picture->start();
        }
        // 受击闪烁效果
        picture->setOpacity(0.5);
        (new Timer(picture, 100, [this] {
            picture->setOpacity(1);
        }))->start();
    }
    else
        ZombieInstance::getHit(attack);     // 无护甲时调用基类受击逻辑
}

// 铁桶僵尸原型类构造函数
ConeheadZombie::ConeheadZombie()
{
    eName = "oConeheadZombie";              // 英文名称
    cName = tr("Conehead Zombie");          // 中文名称（支持翻译）
    ornHp = 370;                            // 铁桶护甲生命值
    level = 2;                              // 僵尸等级（难度）


    QString path = "Zombies/ConeheadZombie/"; // 动画资源路径
    cardGif = "Card/Zombies/ConeheadZombie.png"; // 卡片图片
    staticGif = path + "0.gif";             // 静态站立动画
    normalGif = path + "ConeheadZombie.gif"; // 带铁桶行走动画
    attackGif = path + "ConeheadZombieAttack.gif"; // 带铁桶攻击动画
    ornLostNormalGif =  "Zombies/Zombie/Zombie.gif"; // 铁桶丢失后行走动画（复用普通僵尸）
    ornLostAttackGif = "Zombies/Zombie/ZombieAttack.gif"; // 铁桶丢失后攻击动画
    standGif = path + "1.gif";              // 带铁桶站立动画
}

// 铁桶僵尸实例类构造函数
ConeheadZombieInstance::ConeheadZombieInstance(const Zombie *zombie)
    : OrnZombieInstance1(zombie)
{}

// 重写铁桶僵尸受击音效（区分有无铁桶状态）
void ConeheadZombieInstance::playNormalballAudio()
{
    if (hasOrnaments)
        QSound::play(":/audio/plastichit.wav"); // 铁桶被击中音效
    else
        OrnZombieInstance1::playNormalballAudio(); // 无铁桶时使用基类音效
}

// 铁桶僵尸实例类构造函数
BucketheadZombieInstance::BucketheadZombieInstance(const Zombie *zombie)
    : OrnZombieInstance1(zombie)
{}

// 重写铁桶僵尸受击音效（区分有无铁桶状态）
void BucketheadZombieInstance::playNormalballAudio()
{
    if (hasOrnaments) {
        if (qrand() % 2)
            QSound::play(":/audio/shieldhit.wav"); // 铁桶被击中音效1
        else
            QSound::play(":/audio/shieldhit2.wav"); // 铁桶被击中音效2
    }
    else
        OrnZombieInstance1::playNormalballAudio(); // 无铁桶时使用基类音效
}


// 铁桶僵尸原型类构造函数
BucketheadZombie::BucketheadZombie()
{
    eName = "oBucketheadZombie";            // 英文名称
    cName = tr("Buckethead Zombie");        // 中文名称
    ornHp = 1100;                           // 铁桶护甲生命值（更高）
    level = 3;                              // 僵尸等级（更高难度）

    QString path = "Zombies/BucketheadZombie/"; // 动画资源路径
    cardGif = "Card/Zombies/BucketheadZombie.png"; // 卡片图片
    staticGif = path + "0.gif";             // 静态站立动画
    normalGif = path + "BucketheadZombie.gif"; // 带铁桶行走动画
    attackGif = path + "BucketheadZombieAttack.gif"; // 带铁桶攻击动画
    ornLostNormalGif =  "Zombies/Zombie/Zombie2.gif"; // 铁桶丢失后行走动画
    standGif = path + "1.gif";              // 带铁桶站立动画
}
// 在BucketheadZombie实现之后添加
ScreenDoorZombie::ScreenDoorZombie() {
    eName = "oScreenDoorZombie";
    cName = tr("Screen Door Zombie");
    hp = 270;                   // 本体生命值
    ornHp = 1100;               // 护甲生命值
    speed = 0.4;                // 带护甲时移动速度
    level = 3;
    sunNum = 125;
    QString path = "Zombies/ScreenDoorZombie/";
    QString path1 = "Zombies/Zombie/";
    cardGif = "Card/Zombies/ScreenDoorZombie.png";
    staticGif = path + "0.gif";
    normalGif = path + "ScreenDoorZombie.gif";
    attackGif = path + "ScreenDoorZombieAttack.gif";
    ornLostNormalGif = path + "HeadWalk1.gif";       // 护甲破损后行走动画
    ornLostAttackGif = path + "HeadAttack1.gif";     // 护甲破损后攻击动画
    lostHeadGif = path + "LostHeadWalk1.gif";        // 无头行走动画
    lostHeadAttackGif = path + "LostHeadAttack1.gif";// 无头攻击动画
    headGif = path1 + "ZombieHead.gif";               // 头部飞溅动画
    dieGif = path1 + "ZombieDie.gif";
    standGif = path + "1.gif";
    beAttackedPointL = 100;     // 根据实际碰撞点调整
    beAttackedPointR = 140;
    breakPoint = 200;           // 触发护甲破碎的血量阈值
}

// ScreenDoorZombie实例类实现
ScreenDoorZombieInstance::ScreenDoorZombieInstance(const Zombie *zombie)
    : OrnZombieInstance1(zombie) {
    orignSpeed = 0.7; // 护甲破损后的移动速度
}

void ScreenDoorZombieInstance::playNormalballAudio() {
    if (hasOrnaments) {
        // 护甲被击中的金属声
        QSound::play(":/audio/shieldhit2.wav");
    } else {
        // 本体被击中的默认音效
        ZombieInstance::playNormalballAudio();
    }
}
// 撑杆僵尸原型类构造函数
PoleVaultingZombie::PoleVaultingZombie()
{
    eName = "oPoleVaultingZombie";            // 英文名称
    cName = tr("Pole Vaulting Zombie");        // 中文名称
    hp = 500;                                 // 生命值
    speed = 1;                                // 初始速度（原来3.2，可能为错误注释）
    beAttackedPointL = 215;                   // 攻击判定左边界
    beAttackedPointR = 260;                   // 攻击判定右边界
    level = 2;                                // 僵尸等级
    sunNum = 75;                              // 击杀后获得阳光数

    QString path = "Zombies/PoleVaultingZombie/"; // 动画资源路径
    cardGif = "Card/Zombies/PoleVaultingZombie.png"; // 卡片图片
    staticGif = path + "0.gif";               // 静态站立动画
    normalGif = path + "PoleVaultingZombie.gif"; // 持杆行走动画
    attackGif = path + "PoleVaultingZombieAttack.gif"; // 持杆攻击动画
    lostHeadGif = path + "PoleVaultingZombieLostHead.gif"; // 失头持杆动画
    lostHeadAttackGif = path + "PoleVaultingZombieLostHeadAttack.gif"; // 失头持杆攻击动画
    headGif = path + "PoleVaultingZombieHead.gif"; // 头部飞溅动画
    dieGif = path + "PoleVaultingZombieDie.gif"; // 死亡动画
    boomDieGif = path + "BoomDie.gif";        // 爆炸死亡动画
    walkGif = path + "PoleVaultingZombieWalk.gif"; // 丢弃撑杆后行走动画
    lostHeadWalkGif = path + "PoleVaultingZombieLostHeadWalk.gif"; // 失头后行走动画
    jumpGif1 = path + "PoleVaultingZombieJump.gif"; // 起跳动画
    jumpGif2 = path + "PoleVaultingZombieJump2.gif"; // 落地动画
    standGif = path + "1.gif";                // 站立动画
}

// 撑杆僵尸实例类构造函数
PoleVaultingZombieInstance::PoleVaultingZombieInstance(const Zombie *zombie)
    : ZombieInstance(zombie)
{
    judgeAttackOrig = false;                  // 是否使用原始攻击判定逻辑
    lostPole = false;                         // 是否丢弃撑杆
    beginCrushed = false;                     // 是否开始被压碎
}

// 获取撑杆僵尸原型（向下转型）
const PoleVaultingZombie *PoleVaultingZombieInstance::getZombieProtoType()
{
    return static_cast<const PoleVaultingZombie *>(zombieProtoType);
}

// 获取阴影位置（调整撑杆僵尸阴影）
QPointF PoleVaultingZombieInstance::getShadowPos()
{
    return QPointF(zombieProtoType->beAttackedPointL - 20, zombieProtoType->height - 35);
}

// 获取死亡时头部位置
QPointF PoleVaultingZombieInstance::getDieingHeadPos()
{
    return QPointF(X, picture->y() - 20);
}

// 重写攻击判定函数
void PoleVaultingZombieInstance::judgeAttack()
{
    if (judgeAttackOrig)                      // 已丢弃撑杆时使用基类判定逻辑
        ZombieInstance::judgeAttack();
    else {
        // 检测前方三列的植物（提前预判）
        int colEnd = zombieProtoType->scene->getCoordinate().getCol(ZX);
        for (int col = colEnd - 2; col <= colEnd; ++col) {
            if (col > 9) continue;
            QMap<int, PlantInstance *> plants = zombieProtoType->scene->getPlant(col, row);
            // 从高优先级到低优先级检查植物
            for (int i = 2; i >= 0; --i) {
                if (!plants.contains(i)) continue;
                PlantInstance *plant = plants[i];
                // 检测到可攻击植物时准备跳跃
                if (plant->attackedRX >= ZX - 74 && plant->attackedLX < ZX && plant->plantProtoType->canEat) {
                    judgeAttackOrig = true;     // 标记为已触发跳跃
                    posX = plant->attackedLX;   // 记录植物左边界位置
                    normalAttack(plant);        // 执行跳跃攻击
                    break;
                }
            }
        }
    }
}

// 重写攻击函数（实现跳跃机制）
void PoleVaultingZombieInstance::normalAttack(PlantInstance *plantInstance)
{
    if (lostPole)                               // 已丢弃撑杆时使用基类攻击逻辑
        ZombieInstance::normalAttack(plantInstance);
    else {
        QSound::play(":/audio/grassstep.wav");  // 播放准备跳跃音效
        picture->setMovie(getZombieProtoType()->jumpGif1); // 设置起跳动画
        picture->start();
        shadowPNG->setVisible(false);           // 隐藏阴影（模拟腾空）
        isAttacking = true;                     // 标记为攻击状态
        altitude = 2;                           // 设置高度（空中）

        // 0.5秒后播放跳跃音效
        (new Timer(picture, 500, [] { QSound::play(":/audio/polevault.wav"); }))->start();

        QUuid plantUuid = plantInstance->uuid;  // 记录目标植物UUID

        // 1秒后处理跳跃结果
        (new Timer(picture, 1000, [this, plantUuid] {
            PlantInstance *plant = zombieProtoType->scene->getPlant(plantUuid);
            if (plant && plant->plantProtoType->stature > 0) {
                // 遇到高个子植物（如墙果）时直接跳过
                attackedLX = ZX = plant->attackedRX;
                X = attackedLX - zombieProtoType->beAttackedPointL;
                attackedRX = X + zombieProtoType->beAttackedPointR;
                picture->setX(X);
                picture->setMovie(getZombieProtoType()->walkGif); // 设置丢弃撑杆后行走动画
                picture->start();
                shadowPNG->setVisible(true);    // 显示阴影
                isAttacking = 0;                // 取消攻击状态
                altitude = 1;                   // 高度恢复正常
                orignSpeed = speed = 1.6;       // 速度提升（丢弃撑杆后）
                normalGif = getZombieProtoType()->walkGif; // 更新普通动画
                lostHeadGif = getZombieProtoType()->lostHeadWalkGif; // 更新失头动画
                lostPole = true;                // 标记为已丢弃撑杆
                judgeAttackOrig = true;         // 使用原始攻击判定逻辑
            }
            else {
                // 遇到矮个子植物或空位置时执行落地动画
                attackedRX = posX;
                X = attackedRX - zombieProtoType->beAttackedPointR;
                attackedLX = ZX = X + zombieProtoType->beAttackedPointL;
                picture->setX(X);
                picture->setMovie(getZombieProtoType()->jumpGif2); // 设置落地动画
                picture->start();
                shadowPNG->setVisible(true);

                // 0.8秒后完成跳跃，更新状态
                (new Timer(picture, 800, [this]{
                    picture->setMovie(getZombieProtoType()->walkGif);
                    picture->start();
                    isAttacking = 0;
                    altitude = 1;
                    orignSpeed = speed = 1.6;
                    normalGif = getZombieProtoType()->walkGif;
                    lostHeadGif = getZombieProtoType()->lostHeadWalkGif;
                    lostPole = true;
                    judgeAttackOrig = true;
                }))->start();
            }
        }))->start();
    }
}

Zombie *ZombieFactory(GameScene *scene, const QString &ename)
{
    Zombie *zombie = nullptr;
    if (ename == "oZombie")
        zombie = new Zombie1;
    if (ename == "oZombie2")
        zombie = new Zombie2;
    if (ename == "oZombie3")
        zombie = new Zombie3;
    if (ename == "oFlagZombie")
        zombie = new FlagZombie;
    if (ename == "oConeheadZombie")
        zombie = new ConeheadZombie;
    if (ename == "oBucketheadZombie")
        zombie = new BucketheadZombie;
    if (ename == "oPoleVaultingZombie")
        zombie = new PoleVaultingZombie;
    if (ename == "oScreenDoorZombie") {
        zombie = new ScreenDoorZombie;
    }
    if (zombie) {
        zombie->scene = scene; //僵尸获得游戏场景的坐标系，以及将僵尸添加到游戏场景的接口
        zombie->update(); //正确更新僵尸的宽高
    }
    return zombie;
}

ZombieInstance *ZombieInstanceFactory(const Zombie *zombie)
{
    if (zombie->eName == "oConeheadZombie")
        return new ConeheadZombieInstance(zombie);
    if (zombie->eName == "oBucketheadZombie")
        return new BucketheadZombieInstance(zombie);
    if (zombie->eName == "oPoleVaultingZombie")
        return new PoleVaultingZombieInstance(zombie);
    if (zombie->eName == "oScreenDoorZombie") {
        return new ScreenDoorZombieInstance(zombie);
    }
    return new ZombieInstance(zombie);//没有匹配的特殊类，返回普通的zombieInstance对象
}
