#ifndef CAUDIODECODETHD_H
#define CAUDIODECODETHD_H

#include <QThread>
#include <QObject>
extern "C"{
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>
}
#include "packqueue.h"
#include "packetinfo.h"

// 此处是将解码 和 重采样 放在了一起
class CAudioDecodeThd : public QThread
{
    Q_OBJECT
public:
    explicit CAudioDecodeThd(QObject *parent = nullptr);
    PackQueue<TPackInfo> & GetAudioQueuePack()
    {
        return m_queueAudioPack;
    }

    PackQueue<TPackInfo> & GetVideoQueuePack()
    {
        return m_queueVideoPack;
    }

protected:
    void run();

private:
    void _PlayMp3();
    void _PushAudioPack(const QByteArray & _cbytePack, TPackInfo::ePackInfoType _ePackType, double _iStamp);
    void _PushVideoPack(const QImage & _coImage, TPackInfo::ePackInfoType _ePackType, double _iStamp);
    void _InitAudioFormat(AVFormatContext *_pAVFormatContext, int _iStreamIndex);
    /**
     * @brief _InitViewFormat
     * @param _pAVFormatContext
     * @param _iStreamIndex
     * @param pFrameOutRGB 给转码的目标图像分配内存
     */
    void _InitViewFormat(AVFormatContext *_pAVFormatContext, int _iStreamIndex, AVFrame * pFrameOutRGB);

private:
    PackQueue<TPackInfo> m_queueAudioPack;
    PackQueue<TPackInfo> m_queueVideoPack;

private:
    struct SwrContext * m_pAudioSwrCtx = nullptr;     ///< 音频重采样上下文
    AVCodecContext *m_pAudioCodeContext = nullptr;    ///< 音频编码器上下文

    AVCodecContext *m_pCodeContext = nullptr;     ///< 视频编码器上下文
    struct SwsContext * m_pVideoSwsCtx = nullptr;      ///< 图像转码上下文

};

#endif // CAUDIODECODETHD_H
