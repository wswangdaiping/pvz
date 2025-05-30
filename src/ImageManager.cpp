// 图像管理器类的实现文件，负责图像的加载和缓存管理

#include "ImageManager.h"

// 全局图像管理器指针
ImageManager *gImageCache;

// 加载图像，如果图像未缓存则加载并缓存
QPixmap ImageManager::load(const QString &path)
{
    if (pixmaps.find(path) == pixmaps.end())
        pixmaps.insert(path, QPixmap(":/images/" + path));
    return pixmaps[path];
}

// 初始化图像管理器
void InitImageManager()
{
    gImageCache = new ImageManager;
}

// 销毁图像管理器
void DestoryImageManager()
{
    delete gImageCache;
}
