#include "audiodecodethd.h"

#include <QDebug>

CAudioDecodeThd::CAudioDecodeThd(QObject *parent) : QThread(parent)
{

}

void CAudioDecodeThd::run()
{
    this->_PlayMp3();
}

void CAudioDecodeThd::_PlayMp3()
{
//    ::av_register_all();
    AVFormatContext *pFormatContex = nullptr;
    QString sFileName = "C:\\Users\\MMK\\Desktop\\testFFmpeg+SDL\\test.mp4";
    int iRet = ::avformat_open_input(&pFormatContex, sFileName.toLocal8Bit().data(), NULL, NULL);
    if (!pFormatContex)
    {
       qDebug() << "avformat_open_input fail";
       return;
    }
    qDebug() << "avformat_open_input success";

    if (avformat_find_stream_info(pFormatContex, NULL) < 0)
    {
       qDebug() << "could not find stream information";
       return;
    }

    qDebug() << "avformat_find_stream_info success";
    av_dump_format(pFormatContex, 0, sFileName.toLocal8Bit().data(), 0);

    int iAudioStreamIndex = -1;     // 音频流索引
    int iVideoStreamIndex = -1;     // 视频流索引

    double dAudioUnit = 0, dVieoUnit = 0;
    for (uint i = 0; i < pFormatContex->nb_streams; i++)
    {
       if (pFormatContex->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
       {
            iAudioStreamIndex = i;
            dAudioUnit = av_q2d(pFormatContex->streams[i]->time_base) * 1000;  // 算出是以秒为单位的时间刻度, 乘以1000算出以毫秒为单位时间刻度
            qDebug() << QString("Find a audio stream, index : %1, Time Unit: %2").arg(iAudioStreamIndex).arg(dAudioUnit);
       }

       if (pFormatContex->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
       {
            iVideoStreamIndex = i;
            dVieoUnit = av_q2d(pFormatContex->streams[i]->time_base) * 1000;
            qDebug() << QString("Find a video stream, index : %1, Time Unit: %2").arg(iVideoStreamIndex).arg(dVieoUnit);
       }
    }

    // 音频
    AVFrame *pFrameOrg = ::av_frame_alloc();;
    uint8_t * pAudioBuffer = nullptr;   ///< 用于重采样的音频数据缓存
    int iAudioBufferSize;
    if (iAudioStreamIndex == -1)
    {
        qDebug() << "Cant find audio stream";
    }
    else{
        this->_InitAudioFormat(pFormatContex, iAudioStreamIndex);
    }

    AVFrame * pVideoFrameRGB = av_frame_alloc();
    if (iVideoStreamIndex == -1)
    {
        qDebug() << "Cant find video stream";
    }
    else{
        this->_InitViewFormat(pFormatContex, iVideoStreamIndex, pVideoFrameRGB);
    }

    AVPacket *pPack = av_packet_alloc();
    unsigned int iCurrentAudioBuffer = 0;
    int iCurrentTargetSample = 0;
    while (::av_read_frame(pFormatContex, pPack) == 0)
    {
        // 解析出视频格式
        if (pPack->stream_index == iAudioStreamIndex)
        {
            iRet = ::avcodec_send_packet(m_pAudioCodeContext, pPack);
            if (iRet != 0)
            {
                qDebug() << "audio avcodec_send_packet() failed:" << iRet;
                return;
            }

            while (0 == avcodec_receive_frame(m_pAudioCodeContext, pFrameOrg))
            {
                int iTargetSamples = swr_get_out_samples(m_pAudioSwrCtx, pFrameOrg->nb_samples);     ///< 获取目标的采样数
                iAudioBufferSize = ::av_samples_get_buffer_size(NULL, av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO),
                                                           iTargetSamples, AV_SAMPLE_FMT_S16, 1);
                pAudioBuffer = (uint8_t*)::av_fast_realloc(pAudioBuffer, &iCurrentAudioBuffer, iAudioBufferSize);

//                if (pAudioBuffer == nullptr || iCurrentTargetSample < iTargetSamples)
//                {
//                    qDebug() << "#############################" << iCurrentTargetSample << iTargetSamples;
//                    iCurrentTargetSample = iTargetSamples;

//                }
                // 音频重采样
                int iNbsamples = ::swr_convert(m_pAudioSwrCtx, &pAudioBuffer, iAudioBufferSize,  (const uint8_t **)pFrameOrg->data,  pFrameOrg->nb_samples);
                if (iNbsamples < 0)
                {
                    qDebug() << "audio swr_convert_frame() failed, ret : " << AVERROR(iRet);
                    continue;
                }
                else{
                    int outsize = av_samples_get_buffer_size(NULL, av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO), iNbsamples,
                                                             AV_SAMPLE_FMT_S16, 1);
                    int iOutSize = iNbsamples * 2 * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
                    // 解码重采样出44100 双通道 16位 PCM流
                    QByteArray baAudio(reinterpret_cast<const char*>(pAudioBuffer), outsize);

                    this->_PushAudioPack(baAudio, TPackInfo::EPACKINFOTYPE_PCM, dAudioUnit * pFrameOrg->pts);
                    qDebug() << "##################################success:" << iNbsamples << outsize << iOutSize << dAudioUnit * pFrameOrg->pts;
                }
            }
        }
        else if(pPack->stream_index == iVideoStreamIndex)
        {
            iRet = ::avcodec_send_packet(m_pCodeContext, pPack);
            if (iRet != 0)
            {
                qDebug() << "avcodec_receive_frame() failed:" << iRet;
                return;
            }

           while (0 == ::avcodec_receive_frame(m_pCodeContext, pFrameOrg))
           {
               ::sws_scale(m_pVideoSwsCtx, (const uint8_t * const*)pFrameOrg->data, pFrameOrg->linesize, 0,
                           m_pCodeContext->height, pVideoFrameRGB->data, pVideoFrameRGB->linesize);
               QImage image(pVideoFrameRGB->data[0], m_pCodeContext->width, m_pCodeContext->height,
                            pVideoFrameRGB->linesize[0], QImage::Format_RGB888);
               this->_PushVideoPack(image.copy(), TPackInfo::EPACKINFOTYPE_RGB, dVieoUnit * pFrameOrg->pts);
//               qDebug() << "####################################Video RGB get sucess" << dVieoUnit * pFrameOrg->pts;
           }
        }
        ::av_packet_unref(pPack);    // 清空包数据
    }
    // 释放帧数据包
    ::av_packet_free(&pPack);
    // 释放buffer缓冲区
    ::av_free(pAudioBuffer);
    // 清空帧结构体
    ::av_frame_free(&pFrameOrg);
    // 释放转码器
    ::swr_free(&m_pAudioSwrCtx);
    // 释放编解码上下文
    ::avcodec_close(m_pAudioCodeContext);
    ::avcodec_free_context(&m_pAudioCodeContext);
    // 关闭格式上下文
    ::avformat_close_input(&pFormatContex);
}

void CAudioDecodeThd::_PushAudioPack(const QByteArray &_cbytePack, TPackInfo::ePackInfoType _ePackType, double _iStamp)
{
    TPackInfo tPack;
    tPack.bData = _cbytePack;
    tPack.eType = _ePackType;
    tPack.dTimeStamp = _iStamp;
    m_queueAudioPack.enqueue(tPack);
}

void CAudioDecodeThd::_PushVideoPack(const QImage &_coImage, TPackInfo::ePackInfoType _ePackType, double _iStamp)
{
    TPackInfo tPack;
    tPack.oImage = _coImage;
    tPack.eType = _ePackType;
    tPack.dTimeStamp = _iStamp;
    m_queueVideoPack.enqueue(tPack);
}

void CAudioDecodeThd::_InitAudioFormat(AVFormatContext *_pAVFormatContext, int _iStreamIndex)
{
    // 获取音频流编码信息
    AVCodecParameters *pAudioCodeParam = _pAVFormatContext->streams[_iStreamIndex]->codecpar;
    // 获取音频的编码格式
    const AVCodec *pAudioCodec = ::avcodec_find_decoder(pAudioCodeParam->codec_id);
    // 申请音频编解码器内存
    m_pAudioCodeContext = ::avcodec_alloc_context3(pAudioCodec);

    // 初始化音频解码器
    int iRet = ::avcodec_parameters_to_context(m_pAudioCodeContext, pAudioCodeParam);
    if (iRet < 0)
    {
        qDebug() << "audio avcodec_parameters_to_context() failed, ret:" << iRet;
        return;
    }

    iRet = ::avcodec_open2(m_pAudioCodeContext, pAudioCodec, NULL);
    if (iRet < 0)
    {
        qDebug() << "audio avcodec_open2() failed, ret:" << iRet;
        return;
    }

    // 初始化格式转换器
    m_pAudioSwrCtx = swr_alloc_set_opts(NULL, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, 44100,
                                      m_pAudioCodeContext->channel_layout, m_pAudioCodeContext->sample_fmt,
                                      m_pAudioCodeContext->sample_rate, 0, NULL);
    qDebug() << "##################################pAudioCodeContext->sample_rate:" << m_pAudioCodeContext->sample_rate;
    // 初始化重采样上下文
    ::swr_init(m_pAudioSwrCtx);
}

void CAudioDecodeThd::_InitViewFormat(AVFormatContext *_pAVFormatContext, int _iStreamIndex, AVFrame * pFrameOutRGB)
{
    // 获取视频流编码信息
    AVCodecParameters *pCodeParam = _pAVFormatContext->streams[_iStreamIndex]->codecpar;

    // 获取编码格式
    const AVCodec *pCodec = ::avcodec_find_decoder(pCodeParam->codec_id);
    if (pCodec == NULL)
    {
        qDebug() << "Cannt find video codec!\n";
        return;
    }

    // 编码上下文
    m_pCodeContext = ::avcodec_alloc_context3(pCodec);

    // 根据视频流编码信息编码视频格式上下文
    int iRet = ::avcodec_parameters_to_context(m_pCodeContext, pCodeParam);
    if (iRet < 0)
    {
        qDebug() << "avcodec_parameters_to_context() failed, ret:" << iRet;
        return;
    }

    // 根据 AVCodec对象初始化 AVCodecContext对象
    iRet = ::avcodec_open2(m_pCodeContext, pCodec, NULL);
    if (iRet < 0)
    {
        qDebug() << "avcodec_open2() failed, ret:" << iRet;
        return;
    }

    // 给转码的帧 分配内存
    int iBuffSize = ::av_image_get_buffer_size(AV_PIX_FMT_RGB24, m_pCodeContext->width, m_pCodeContext->height, 1);
    uint8_t * buffer = (uint8_t*)::av_malloc(iBuffSize);
    ::av_image_fill_arrays(pFrameOutRGB->data, pFrameOutRGB->linesize, buffer, AV_PIX_FMT_RGB24,
                           m_pCodeContext->width, m_pCodeContext->height, 1);
    m_pVideoSwsCtx = ::sws_getContext(m_pCodeContext->width, m_pCodeContext->height, m_pCodeContext->pix_fmt,
                             m_pCodeContext->width, m_pCodeContext->height, AV_PIX_FMT_RGB24,
                             SWS_BICUBIC, NULL, NULL, NULL);
}
