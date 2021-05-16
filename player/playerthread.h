#ifndef CPLAYERTHREAD_H
#define CPLAYERTHREAD_H

#include <QAudioOutput>
#include <QIODevice>
#include <QThread>
#include "audiodecodethd.h"

class CPlayerThread : public QThread
{
    Q_OBJECT
public:
    explicit CPlayerThread(CAudioDecodeThd *_pAudioDecodeThd, QObject *parent = nullptr);

protected:
    virtual void run() override;

signals:
    void SIGNAL_FrameRGB(const QImage &oImage);

private:
    CAudioDecodeThd * m_pDecodeThd; ///< 解码线程
    int64_t m_iStartTime;       ///< 记录开始时间

    QAudioOutput * m_pAudioOutput = nullptr;
    QIODevice * m_pAudioWriteDevice = nullptr;
};

#endif // CPLAYERTHREAD_H
