// 游戏关卡数据类的实现文件，负责管理游戏关卡的基本信息和游戏流程控制

#include "GameLevelData.h"
#include "GameScene.h"
#include "ImageManager.h"
#include "Timer.h"

// 游戏关卡数据基类构造函数，初始化关卡的基本属性
GameLevelData::GameLevelData() : cardKind(0), // 卡片类型: 0-植物卡 1-僵尸卡
                                 //dKind(1),// 难度级别
                                 sunNum(50),
                                 backgroundImage("interface/background1.jpg"),
                                 LF{ 0, 1, 1, 1, 1, 1 }, // 特殊标志列表
                                 canSelectCard(true),
                                 staticCard(true),
                                 showScroll(true),
                                 produceSun(true),
                                 hasShovel(true),//铲子
                                 maxSelectedCards(8),//最大可选卡牌数
                                 coord(0),
                                 flagNum(0)//总波次数
{}

// 加载关卡资源完成后的处理
void  GameLevelData::loadAccess(GameScene *gameScene)
{
    gameScene->loadAcessFinished();
}

// 开始游戏的处理，包括初始化割草机、准备种植植物、开始背景音乐、监控、冷却和阳光生成等
void GameLevelData::startGame(GameScene *gameScene)
{
    initLawnMower(gameScene);
    gameScene->prepareGrowPlants( [gameScene] {
        gameScene->beginBGM();
        gameScene->beginMonitor();
        gameScene->beginCool();
        gameScene->beginSun(25);
        (new Timer(gameScene, 15000, [gameScene] {
            gameScene->beginZombies();
        }))->start();
    });
}

// 初始化割草机
void GameLevelData::initLawnMower(GameScene *gameScene)
{
    for (int i = 0; i < LF.size(); ++i) {
        if (LF[i] == 1)
            gameScene->customSpecial("oLawnCleaner", -1, i);//wdp
    }
}

// 结束游戏的处理
void GameLevelData::endGame(GameScene *gameScene)
{

}

// 游戏关卡数据 1 类构造函数，初始化关卡 1 的具体属性
GameLevelData_1::GameLevelData_1()
{
    backgroundImage = "interface/background1.jpg";
    backgroundMusic = "qrc:/audio/UraniwaNi.mp3";
    sunNum = 50;
    canSelectCard = true;
    showScroll = true;
    eName = "1";
    cName = tr("Level 1-1");
    // 可选植物卡片列表
        pName = {
            "oPeashooter",     // 豌豆射手
            "oSnowPea",        // 寒冰射手
            "oSunflower",      // 向日葵
            "oWallNut",        // 坚果墙
            "oPumpkinHead",    // 南瓜头
            "oTorchwood",      // 火炬树桩
            "oTallNut",        // 高坚果
            "oThreepeater",    // 三发豌豆
            "oCactus",         // 仙人掌
            "oRepeater",       // 双发射手
            "oJalapeno",       // 火爆辣椒
            "oSquash"          // 倭瓜
        };
    zombieData = { { "oZombie", 3, 1, {} },
                   { "oZombie2", 3, 1, {} },
                   { "oZombie3", 3, 1, {} },
                   { "oConeheadZombie", 5, 3, {} },
                   { "oPoleVaultingZombie", 5, 5, {} },
                   { "oBucketheadZombie", 5, 9, {} },
                   { "oScreenDoorZombie", 4, 5, {} }   };
    flagNum = 10;//波次
    largeWaveFlag = { 5,8};
    flagToSumNum = QPair<QList<int>, QList<int> >({ 3, 4, 8, 9, 10, 13, 15, 19 }, { 1, 3, 5, 7, 20, 8, 10, 14, 30 });
}

// 游戏关卡数据工厂函数，根据关卡名称创建对应的关卡数据对象
GameLevelData *GameLevelDataFactory(const QString &eName)
{
    if (eName == "1")
        return new GameLevelData_1;
    return nullptr;
}
