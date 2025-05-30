#ifndef PLANTS_VS_ZOMBIES_PLANT_H
#define PLANTS_VS_ZOMBIES_PLANT_H

#include <QtCore>
#include <QtWidgets>
#include <QtMultimedia>

class MoviePixmapItem;
class GameScene;
class ZombieInstance;
class Trigger;

class Plant
{
    Q_DECLARE_TR_FUNCTIONS(Plant)

public:
    Plant();
    virtual  ~Plant() {}

    QString eName, cName;
    int width, height;
    int hp, pKind, bKind;
    int beAttackedPointL, beAttackedPointR;
    int zIndex;
    QString cardGif, staticGif, normalGif,attackGif;
    bool canEat, canSelect, night;
    double coolTime;
    int stature, sleep;
    int sunNum;
    QString toolTip;

    virtual double getDX() const;
    virtual double getDY(int x, int y) const;
    virtual bool canGrow(int x, int y) const;

    GameScene *scene;
    void update();
};

class PlantInstance
{
public:
    PlantInstance(const Plant *plant);
    virtual ~PlantInstance();

    virtual void birth(int c, int r);
    virtual void initTrigger();
    virtual void triggerCheck(ZombieInstance *zombieInstance, Trigger *trigger);
    virtual void normalAttack(ZombieInstance *zombieInstance);
    virtual void getHurt(ZombieInstance *zombie, int aKind, int attack);

    bool contains(const QPointF &pos);

    const Plant *plantProtoType;

    QUuid uuid;
    int row, col;
    int hp;
    bool canTrigger;
    qreal attackedLX, attackedRX;
    QMap<int, QList<Trigger *> > triggers;

    QGraphicsPixmapItem *shadowPNG;
    MoviePixmapItem *picture;
};

//wdp
//辣椒的头文件代码
class Jalapeno : public Plant
{
    Q_DECLARE_TR_FUNCTIONS(Jalapeno)
public:
    Jalapeno();
};

class JalapenoInstance : public PlantInstance
{
public:
    JalapenoInstance(const Plant *plant);
    virtual void birth(int c, int r) override;

private:
    void explode();
    void moveAndExplode();
};
//
//squash
class Squash : public Plant {
public:
    Squash();
    QString leftGif;
    QString rightGif,attackGif;
};

class SquashInstance : public PlantInstance {
public:
    explicit SquashInstance(const Plant *plant);
    void birth(int c, int r) override;
    void initTrigger() override;
    void triggerCheck(ZombieInstance *zombieInstance, Trigger *trigger) override;
private slots:
    void onJumpTimelineFrameChanged(int frame);
    void onJumpTimelineFinished();
private:
    void jumpAndCrush(bool jumpLeft);
    qreal initialY; // 存储初始Y坐标
};
//
class Cactus : public Plant {
public:
    Cactus();
};

class CactusInstance : public PlantInstance {
public:
    CactusInstance(const Plant* plant);
    void normalAttack(ZombieInstance* zombie) override;

};
//end

//
class Threepeater: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(Threepeater)
public:
    Threepeater();
};

class ThreepeaterInstance: public PlantInstance
{
public:
    ThreepeaterInstance(const Plant *plant);
    virtual void normalAttack(ZombieInstance *zombieInstance);
};
class Peashooter: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(Peashooter)
public:
    Peashooter();
};

class PeashooterInstance: public PlantInstance
{
public:
    PeashooterInstance(const Plant *plant);
    virtual void normalAttack(ZombieInstance *zombieInstance);
};
class Repeater: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(Repeater)
public:
    Repeater();
};

class RepeaterInstance: public PlantInstance
{
public:
    RepeaterInstance(const Plant *plant);
    virtual void normalAttack(ZombieInstance *zombieInstance);
};

class SnowPea: public Peashooter
{
    Q_DECLARE_TR_FUNCTIONS(SnowPea)
public:
    SnowPea();
};

class SnowPeaInstance: public PlantInstance
{
public:
    SnowPeaInstance(const Plant *plant);
    virtual void normalAttack(ZombieInstance *zombieInstance);
};

class SunFlower: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(SunFlower)
public:
    SunFlower();
};

class SunFlowerInstance: public PlantInstance
{
public:
    SunFlowerInstance(const Plant *plant);
    virtual void initTrigger();
private:
    QString lightedGif;
};

class WallNut: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(WallNut)
public:
    WallNut();
    virtual bool canGrow(int x, int y) const;
};

class WallNutInstance: public PlantInstance
{
public:
    WallNutInstance(const Plant *plant);
    virtual void initTrigger();
    virtual void getHurt(ZombieInstance *zombie, int aKind, int attack);
private:
    int hurtStatus;
    QString crackedGif1, crackedGif2;
};


class LawnCleaner: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(LawnCleaner)
public:
    LawnCleaner();
};

class LawnCleanerInstance: public PlantInstance
{
public:
    LawnCleanerInstance(const Plant *plant);
    virtual void initTrigger();
    virtual void triggerCheck(ZombieInstance *zombieInstance, Trigger *trigger);
    virtual void normalAttack(ZombieInstance *zombieInstance);
};

class PoolCleaner: public LawnCleaner
{
    Q_DECLARE_TR_FUNCTIONS(PoolCleaner)
public:
    PoolCleaner();
};

class Bullet
{
public:
    Bullet(GameScene *scene, int type, int row, qreal from, qreal x, qreal y, qreal zvalue,  int direction);
    ~Bullet();
    void start();
private:
    void move();

    GameScene *scene;
    int count, type, row, direction;
    QUuid uuid;
    qreal from;
    QGraphicsPixmapItem *picture;
};

class PumpkinHead: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(PumpkinHead)
public:
    PumpkinHead();
    virtual double getDY(int x, int y) const;
    virtual bool canGrow(int x, int y) const;
};

class PumpkinHeadInstance: public PlantInstance
{
public:
    PumpkinHeadInstance(const Plant *plant);
    virtual ~PumpkinHeadInstance();
    virtual void birth(int c, int r);
    virtual void getHurt(ZombieInstance *zombie, int aKind, int attack);

    int hurtStatus;
    MoviePixmapItem *picture2;
};

class Torchwood: public Plant
{
    Q_DECLARE_TR_FUNCTIONS(Torchwood)
public:
    Torchwood();
};

class TorchwoodInstance: public PlantInstance
{
public:
    TorchwoodInstance(const Plant *plant);
    virtual void initTrigger();
};

class TallNut: public WallNut
{
    Q_DECLARE_TR_FUNCTIONS(TallNut)
public:
    TallNut();
    virtual bool canGrow(int x, int y) const;
};

class TallNutInstance: public WallNutInstance
{
public:
    TallNutInstance(const Plant *plant);
    virtual void getHurt(ZombieInstance *zombie, int aKind, int attack);
private:
    int hurtStatus;
};


Plant *PlantFactory(GameScene *scene, const QString &eName);
PlantInstance *PlantInstanceFactory(const Plant *plant);

#endif //PLANTS_VS_ZOMBIES_PLANT_H
