#ifndef PLANTS_VS_ZOMBIES_GAMELEVELDATA_H
#define PLANTS_VS_ZOMBIES_GAMELEVELDATA_H

#include <QtCore>

// 前向声明游戏场景类
class GameScene;

/**
 * @brief 僵尸生成数据类
 * 存储单个僵尸类型的生成配置信息
 */
class ZombieData
{
public:
    QString eName;       // 僵尸英文名称
    int num;             // 生成数量
    int firstFlag;       // 首次出现的旗帜编号
    QList<int> flagList; // 出现波次对应的旗帜列表
};

/**
 * @brief 游戏关卡数据基类
 * 包含关卡配置数据和核心游戏逻辑接口
 */
class GameLevelData
{
    Q_DECLARE_TR_FUNCTIONS(GameLevelData) // Qt
public:
    GameLevelData();
    virtual ~GameLevelData() {}

    // 基础信息
    QString eName, cName;  // 英文名和中文名

    // 卡片配置
    QList<QString> pName, zName;  // 植物/僵尸名称列表
    int cardKind;           // 卡片类型: 0-植物卡 1-僵尸卡
    int dKind;              // 难度级别
    int sunNum;             // 初始阳光值

    // 场景配置
    QString backgroundImage; // 背景图片路径
    QList<int> LF;          // 特殊标志列表
    bool canSelectCard;     // 是否允许选择卡片
    bool staticCard;        // 是否静态卡片布局
    bool showScroll;        // 是否显示滚动条
    bool produceSun;        // 是否生产阳光
    bool hasShovel;         // 是否提供铲子
    int maxSelectedCards;   // 最大可选卡片数
    int coord;              // 坐标系标识

    // 波次配置
    int flagNum;            // 总波次数
    QList<int> largeWaveFlag; // 大波次标志列表(红字提示波次)
    QPair<QList<int>, QList<int>> flagToSumNum; // 波次与生成数量的映射
    QMap<int, std::function<void(GameScene *)>> flagToMonitor; // 波次监控回调函数

    // 僵尸生成数据
    QList<ZombieData> zombieData; // 本关卡所有僵尸配置

    // 音频配置
    QString backgroundMusic; // 背景音乐路径

    // 虚函数接口(需子类实现)
    virtual void loadAccess(GameScene *gameScene);    // 加载资源
    virtual void initLawnMower(GameScene *gameScene); // 初始化割草机
    virtual void startGame(GameScene *gameScene);     // 开始游戏逻辑
    virtual void endGame(GameScene *gameScene);       // 结束游戏逻辑
};

/**
 * @brief 具体关卡实现类(示例)
 * 继承基类并实现特定关卡逻辑
 */
class GameLevelData_1 : public GameLevelData
{
    Q_DECLARE_TR_FUNCTIONS(GameLevelData_1)
public:
    GameLevelData_1(); // 第一关特定配置
};

/**
 * @brief 关卡工厂函数
 * @param eName 关卡英文标识
 * @return 对应关卡实例指针
 */
GameLevelData * GameLevelDataFactory(const QString &eName);

#endif //PLANTS_VS_ZOMBIES_GAMELEVEL_H
