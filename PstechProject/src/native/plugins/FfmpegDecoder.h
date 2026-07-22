#pragma once
extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
#include <opencv2/opencv.hpp>
#include <iostream>

class FfmpegDecoder {
public:
    FfmpegDecoder() : codec_ctx(nullptr), frame(nullptr), packet(nullptr), sws_ctx(nullptr), last_w(0), last_h(0) {}
    ~FfmpegDecoder() { cleanup(); }

    bool init() {
        const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
        if (!codec) { std::cerr << "H.264 Codec not found" << std::endl; return false; }
        codec_ctx = avcodec_alloc_context3(codec);
        if (avcodec_open2(codec_ctx, codec, nullptr) < 0) return false;
        frame = av_frame_alloc();
        packet = av_packet_alloc();
        return true;
    }

    bool decode(const unsigned char* data, int size, cv::Mat& out_img) {
        if (!codec_ctx) return false;
        packet->data = (uint8_t*)data;
        packet->size = size;

        if (avcodec_send_packet(codec_ctx, packet) < 0) return false;

        if (avcodec_receive_frame(codec_ctx, frame) == 0) {
            if (!sws_ctx || last_w != frame->width || last_h != frame->height) {
                if (sws_ctx) sws_freeContext(sws_ctx);
                sws_ctx = sws_getContext(frame->width, frame->height, codec_ctx->pix_fmt,
                                         frame->width, frame->height, AV_PIX_FMT_BGR24,
                                         SWS_BILINEAR, nullptr, nullptr, nullptr);
                last_w = frame->width; 
                last_h = frame->height;
            }
            out_img = cv::Mat(frame->height, frame->width, CV_8UC3);
            uint8_t* dest[4] = { out_img.data, nullptr, nullptr, nullptr };
            int dest_linesize[4] = { (int)out_img.step[0], 0, 0, 0 };
            sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height, dest, dest_linesize);
            return true;
        }
        return false;
    }

private:
    void cleanup() {
        if (codec_ctx) avcodec_free_context(&codec_ctx);
        if (frame) av_frame_free(&frame);
        if (packet) av_packet_free(&packet);
        if (sws_ctx) sws_freeContext(sws_ctx);
    }
    AVCodecContext* codec_ctx;
    AVFrame* frame;
    AVPacket* packet;
    SwsContext* sws_ctx;
    int last_w;
    int last_h;
};