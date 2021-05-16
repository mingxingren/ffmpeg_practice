#ifndef PACKETINFO_H
#define PACKETINFO_H

#include <QByteArray>
#include <QImage>

struct TPackInfo
{
    enum ePackInfoType
    {
        EPACKINFOTYPE_PCM,
        EPACKINFOTYPE_RGB,
    };

    int64_t dTimeStamp = -1; ///< 时间戳 同步音视频 以毫秒为单位
    QByteArray bData;   ///< store PCM
    QImage oImage;      ///< store RGB
    ePackInfoType eType;        ///< 包的类型
};

#endif // PACKETINFO_H
