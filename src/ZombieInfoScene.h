#ifndef ZOMBIEINFO_SCENE_H
#define ZOMBIEINFO_SCENE_H

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QMediaPlayer>
#include <QTimer>
#include "MouseEventPixmapItem.h"
#include "ImageManager.h"

class ZombieInfoScene : public QGraphicsScene {
    Q_OBJECT
public:
    ZombieInfoScene();
    ~ZombieInfoScene();

private slots:
    void backToSelector();

private:
    QGraphicsPixmapItem *background;
    MouseEventPixmapItem *exitButton;
    QMediaPlayer *backgroundMusic;
};

#endif // ZOMBIEINFO_SCENE_H
