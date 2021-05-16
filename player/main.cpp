#include "mainwindow.h"

#include <iostream>
#include <QApplication>
#include <QDebug>
#include "audiodecodethd.h"
#include "playerthread.h"
#include "SDL2/SDL.h"

int64_t iTimeStart = 0;     // 存放开始播放音频的时间 计算偏移量

//extern "C"
//{
//#include "libavcodec/avcodec.h"
//#include "libavformat/avformat.h"
//#include "libswscale/swscale.h"
//#include "libavdevice/avdevice.h"
//}

#undef main

//#define SDL_AUDIO_FRAME_SAMPLE_SIZE  1024         // 一帧的样本数
//#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000 // 一秒钟 48000 32位 ——> 48000次采样 * 4字节

//struct TPacketQueue{
//    AVPacketList *pfirst_packt = nullptr, *plast_packt = nullptr;   // 头指针 尾指针
//    int inum_packs; // 队列里一共有多少个包
//    int isize;  // 所有包 内存的总和
//    SDL_mutex *mutex;
//    SDL_cond *condition;    // SDL的条件变量实现生产者消费者模式
//};

//// 分配解码过程中 AVFrame的缓存
//AVFrame *pout_frame = ::av_frame_alloc();
//TPacketQueue *paudio_queue = nullptr;

//// 初始化
//void packet_queue_init(TPacketQueue *_ppack_queue)
//{
//    ::memset(_ppack_queue, 0, sizeof(_ppack_queue));
//    _ppack_queue->mutex = ::SDL_CreateMutex();
//    _ppack_queue->condition = ::SDL_CreateCond();
//}

//// 生产者
//int packet_queue_put(TPacketQueue *_ppacket_queue, AVPacket *_ppack)
//{
//    AVPacketList *ppack_list;
//    if (::av_packet_make_writable(_ppack) < 0)  // 判断 pack 是否有独立的内存空间, 没有就分配
//    {
//        return -1;
//    }
//    ppack_list = (AVPacketList*)av_malloc(sizeof(AVPacketList));
//    if (!ppack_list)
//    {
//        return -1;
//    }
//    ppack_list->pkt = *_ppack;
//    ppack_list->next = nullptr;
//    ::SDL_LockMutex(paudio_queue->mutex);

//    if (paudio_queue->plast_packt == nullptr)
//    {
//        paudio_queue->pfirst_packt = ppack_list;
//    }
//    else{
//        paudio_queue->plast_packt->next = ppack_list;
//    }
//    paudio_queue->plast_packt = ppack_list;
//    paudio_queue->inum_packs++;
//    paudio_queue->isize += _ppack->size;

//    ::SDL_CondSignal(paudio_queue->condition);
//    ::SDL_UnlockMutex(paudio_queue->mutex);
//    return 0;
//}

//// 消费者
//int packet_queue_get(TPacketQueue *_ppacket_queue, AVPacket *_pout_packet, bool _bblock)
//{
//    AVPacketList *ppack_list = nullptr;
//    int ret;
//    ::SDL_LockMutex(_ppacket_queue->mutex);
//    for (; ;)
//    {
//        ppack_list = _ppacket_queue->pfirst_packt;
//        if (ppack_list)
//        {
//            _ppacket_queue->pfirst_packt = _ppacket_queue->pfirst_packt->next;
//            paudio_queue->inum_packs--;
//            paudio_queue->isize -= ppack_list->pkt.size;
//            *_pout_packet = ppack_list->pkt;
//            ::av_free(ppack_list);
//            ret = 1;
//            goto packet_queue_get_finish;
//        }
//        else if (_bblock){
//            ret = 0;
//            goto packet_queue_get_finish;
//        }
//        else{
//            ::SDL_CondWait(_ppacket_queue->condition, _ppacket_queue->mutex);   // 等待消费者唤醒 重新上锁
//        }
//    }
//    ::SDL_UnlockMutex(_ppacket_queue->mutex);
//packet_queue_get_finish:
//    return ret;
//}

//// 解码
//int audio_decode_frame(AVCodecContext *_pcodec_context, uint8_t *_paudio_buf, int _ibuf_size)
//{
////    static AVPacket pkt;
////    static uint8_t *audio_pkt_data = NULL;
////    static int audio_pkt_size = 0;
////    int len1, data_size;

////    for(;;)
////    {
////        if(packet_queue_get(paudio_queue, &pkt, 1) < 0)
////        {
////            return -1;
////        }
////        audio_pkt_data = pkt.data;
////        audio_pkt_size = pkt.size;
////        while(audio_pkt_size > 0)
////        {
////            int got_picture;

////            int ret = ::avcodec_decode_audio4( _pcodec_context, pout_frame, &got_picture, &pkt);
////            if( ret < 0 ) {
////                printf("Error in decoding audio frame.\n");
////                exit(0);
////            }

////            if( got_picture ) {
////                int in_samples = audioFrame->nb_samples;
////                short *sample_buffer = (short*)malloc(audioFrame->nb_samples * 2 * 2);
////                memset(sample_buffer, 0, audioFrame->nb_samples * 4);

////                int i=0;
////                float *inputChannel0 = (float*)(audioFrame->extended_data[0]);

////                // Mono
////                if( audioFrame->channels == 1 ) {
////                    for( i=0; i<in_samples; i++ ) {
////                        float sample = *inputChannel0++;
////                        if( sample < -1.0f ) {
////                            sample = -1.0f;
////                        } else if( sample > 1.0f ) {
////                            sample = 1.0f;
////                        }

////                        sample_buffer[i] = (int16_t)(sample * 32767.0f);
////                    }
////                } else { // Stereo
////                    float* inputChannel1 = (float*)(audioFrame->extended_data[1]);
////                    for( i=0; i<in_samples; i++) {
////                        sample_buffer[i*2] = (int16_t)((*inputChannel0++) * 32767.0f);
////                        sample_buffer[i*2+1] = (int16_t)((*inputChannel1++) * 32767.0f);
////                    }
////                }
//////                fwrite(sample_buffer, 2, in_samples*2, pcmOutFp);
////                memcpy(audio_buf,sample_buffer,in_samples*4);
////                free(sample_buffer);
////            }

////            audio_pkt_size -= ret;

////            if (audioFrame->nb_samples <= 0)
////            {
////                continue;
////            }

////            data_size = audioFrame->nb_samples * 4;

////            return data_size;
////        }
////        if(pkt.data)
////            av_free_packet(&pkt);
////   }
//}

//// SDL 定时器回调
void audio_callback(void *_pin_user_data, uint8_t *_pout_stream, int _iout_stream_len)
{
//    CAudioDecodeThd *pThd = (CAudioDecodeThd*)_pin_user_data;
//    static int iIndex = 0;  // 正在读取位置
//    static TPackInfo & tPackInfo;
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CAudioDecodeThd thd;

    CPlayerThread thdPlayer(&thd);

    MainWindow w(&thdPlayer);
    w.show();
    thd.start();
    thdPlayer.start();

    return a.exec();;
}
