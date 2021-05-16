#include "playerthread.h"

#include <QDateTime>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QIODevice>
#include <QDebug>

CPlayerThread::CPlayerThread(CAudioDecodeThd *_pAudioDecodeThd, QObject *parent)
    : QThread(parent), m_pDecodeThd(_pAudioDecodeThd)
{
    QAudioFormat format;
    // Set up the format, eg.
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    m_pAudioOutput = new QAudioOutput(format, this);
    m_pAudioWriteDevice = m_pAudioOutput->start();
    qDebug() << "###########m_pAudioWriteDevice" << m_pAudioOutput->bytesFree() << m_pAudioOutput->bufferSize() <<
                 m_pAudioOutput->periodSize();;
}

void CPlayerThread::run()
{
    msleep(1000);
    m_iStartTime = QDateTime::currentMSecsSinceEpoch(); // 获取播放线程刚开始时间戳
    while (true)
    {
        int64_t iNowTime = QDateTime::currentMSecsSinceEpoch();
        int64_t iOffsetTime = iNowTime - m_iStartTime;
        if (m_pDecodeThd->GetAudioQueuePack().size() > 0)
        {
            if (m_pDecodeThd->GetAudioQueuePack().head().dTimeStamp < iOffsetTime)
            {
                TPackInfo tPack = m_pDecodeThd->GetAudioQueuePack().dequeue();
                int chunks = m_pAudioOutput->bytesFree() / m_pAudioOutput->periodSize();
                int iIndex = 0;
                while (chunks) {
                    qint64 len;
                    if (tPack.bData.size() - iIndex >= m_pAudioOutput->periodSize())
                    {
                        len = m_pAudioOutput->periodSize();
                    }
                    else{
                        len = tPack.bData.size() - iIndex;
                    }
                    if (len)
                    {
                       m_pAudioWriteDevice->write(tPack.bData.data() + iIndex, len);
                       iIndex += len;
                    }
                    if (len != m_pAudioOutput->periodSize())
                       break;
                    --chunks;
                }
            }
        }

        if (m_pDecodeThd->GetVideoQueuePack().size() > 0){
            if (m_pDecodeThd->GetVideoQueuePack().head().dTimeStamp < iOffsetTime)
            {
                TPackInfo tPack = m_pDecodeThd->GetVideoQueuePack().dequeue();
                emit SIGNAL_FrameRGB(tPack.oImage);
            }
        }

        msleep(10);
    }

    qDebug() << QDateTime::currentMSecsSinceEpoch() - m_iStartTime;
}
