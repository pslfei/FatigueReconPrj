#pragma once
extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
#include <opencv2/opencv.hpp>
#include <cstddef>
#include <iostream>

class FfmpegDecoder {
public:
    FfmpegDecoder()
        : codec_ctx(nullptr), frame(nullptr), packet(nullptr), sws_ctx(nullptr),
          current_codec(AV_CODEC_ID_NONE), last_w(0), last_h(0), last_pix_fmt(AV_PIX_FMT_NONE) {}
    ~FfmpegDecoder() { cleanup(); }

    bool init() {
        cleanup();
        frame = av_frame_alloc();
        packet = av_packet_alloc();
        if (!frame || !packet) {
            std::cerr << "[FFmpeg] Failed to allocate decoder buffers" << std::endl;
            cleanup();
            return false;
        }
        return true;
    }

    void reset() {
        if (packet) av_packet_unref(packet);
        if (frame) av_frame_unref(frame);
        releaseCodec();
    }

    bool decode(const unsigned char* data, int size, cv::Mat& out_img) {
        out_img.release();
        if (!data || size <= 0 || !frame || !packet) return false;

        const AVCodecID detected_codec = detectCodecId(data, static_cast<std::size_t>(size));
        if (detected_codec != AV_CODEC_ID_NONE && detected_codec != current_codec) {
            if (!openCodec(detected_codec)) return false;
        }
        if (!codec_ctx) return false;

        av_packet_unref(packet);
        packet->data = const_cast<uint8_t*>(data);
        packet->size = size;

        if (avcodec_send_packet(codec_ctx, packet) < 0) return false;

        if (avcodec_receive_frame(codec_ctx, frame) == 0) {
            const AVPixelFormat source_format = static_cast<AVPixelFormat>(frame->format);
            if (frame->width <= 0 || frame->height <= 0 || source_format == AV_PIX_FMT_NONE) return false;

            if (!sws_ctx || last_w != frame->width || last_h != frame->height || last_pix_fmt != source_format) {
                if (sws_ctx) sws_freeContext(sws_ctx);
                sws_ctx = sws_getContext(frame->width, frame->height, source_format,
                                         frame->width, frame->height, AV_PIX_FMT_BGR24,
                                         SWS_BILINEAR, nullptr, nullptr, nullptr);
                if (!sws_ctx) {
                    std::cerr << "[FFmpeg] Failed to create pixel format converter" << std::endl;
                    last_w = 0;
                    last_h = 0;
                    last_pix_fmt = AV_PIX_FMT_NONE;
                    return false;
                }
                last_w = frame->width; 
                last_h = frame->height;
                last_pix_fmt = source_format;
            }
            out_img = cv::Mat(frame->height, frame->width, CV_8UC3);
            uint8_t* dest[4] = { out_img.data, nullptr, nullptr, nullptr };
            int dest_linesize[4] = { (int)out_img.step[0], 0, 0, 0 };
            if (sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height, dest, dest_linesize) <= 0) {
                out_img.release();
                return false;
            }
            return true;
        }
        return false;
    }

    static AVCodecID detectCodecId(const unsigned char* data, std::size_t size) {
        if (!data || size < 4) return AV_CODEC_ID_NONE;

        AVCodecID detected_codec = AV_CODEC_ID_NONE;
        std::size_t search_offset = 0;
        std::size_t start_offset = 0;
        std::size_t start_code_size = 0;

        while (findStartCode(data, size, search_offset, start_offset, start_code_size)) {
            const std::size_t nal_offset = start_offset + start_code_size;
            std::size_t next_offset = 0;
            std::size_t next_start_code_size = 0;
            const bool has_next = findStartCode(data, size, nal_offset, next_offset, next_start_code_size);
            const std::size_t nal_end = has_next ? next_offset : size;
            const AVCodecID nal_codec = detectNalCodec(data + nal_offset, nal_end - nal_offset);

            if (nal_codec != AV_CODEC_ID_NONE) {
                if (detected_codec != AV_CODEC_ID_NONE && detected_codec != nal_codec) {
                    return AV_CODEC_ID_NONE;
                }
                detected_codec = nal_codec;
            }

            if (!has_next) break;
            search_offset = next_offset;
        }
        return detected_codec;
    }

private:
    static bool findStartCode(const unsigned char* data, std::size_t size, std::size_t offset,
                              std::size_t& start_offset, std::size_t& start_code_size) {
        if (!data || offset >= size) return false;
        for (std::size_t i = offset; i + 2 < size; ++i) {
            if (data[i] != 0x00 || data[i + 1] != 0x00) continue;
            if (data[i + 2] == 0x01) {
                start_offset = i;
                start_code_size = 3;
                return true;
            }
            if (i + 3 < size && data[i + 2] == 0x00 && data[i + 3] == 0x01) {
                start_offset = i;
                start_code_size = 4;
                return true;
            }
        }
        return false;
    }

    static AVCodecID detectNalCodec(const unsigned char* nal, std::size_t size) {
        if (!nal || size < 2 || (nal[0] & 0x80) != 0) return AV_CODEC_ID_NONE;

        const unsigned int hevc_type = (nal[0] >> 1) & 0x3F;
        const unsigned int hevc_layer_id = ((nal[0] & 0x01) << 5) | (nal[1] >> 3);
        const unsigned int hevc_temporal_id = nal[1] & 0x07;
        if (size >= 3 && hevc_layer_id == 0 && hevc_temporal_id != 0 &&
            (hevc_type == 32 || hevc_type == 33 || hevc_type == 34)) {
            return AV_CODEC_ID_HEVC;
        }

        const unsigned int h264_type = nal[0] & 0x1F;
        const unsigned int h264_ref_idc = (nal[0] >> 5) & 0x03;
        if (h264_type == 7 && h264_ref_idc != 0 && size >= 4 &&
            nal[1] != 0 && (nal[2] & 0x03) == 0) {
            return AV_CODEC_ID_H264;
        }
        // A canonical PPS is useful when SPS and PPS arrive in separate callbacks. Requiring
        // nal_ref_idc=3 avoids confusing a common HEVC type-20 IDR header (0x28) with H.264 PPS.
        if (h264_type == 8 && h264_ref_idc == 3 && size >= 2) {
            return AV_CODEC_ID_H264;
        }
        return AV_CODEC_ID_NONE;
    }

    bool openCodec(AVCodecID codec_id) {
        if (codec_ctx && current_codec == codec_id) return true;

        if (packet) av_packet_unref(packet);
        if (frame) av_frame_unref(frame);
        releaseCodec();

        const AVCodec* codec = avcodec_find_decoder(codec_id);
        if (!codec) {
            std::cerr << "[FFmpeg] Decoder not found for " << avcodec_get_name(codec_id) << std::endl;
            return false;
        }

        codec_ctx = avcodec_alloc_context3(codec);
        if (!codec_ctx) {
            std::cerr << "[FFmpeg] Failed to allocate " << avcodec_get_name(codec_id)
                      << " decoder context" << std::endl;
            return false;
        }
        const int open_result = avcodec_open2(codec_ctx, codec, nullptr);
        if (open_result < 0) {
            std::cerr << "[FFmpeg] Failed to open " << avcodec_get_name(codec_id)
                      << " decoder (" << open_result << ")" << std::endl;
            avcodec_free_context(&codec_ctx);
            return false;
        }

        current_codec = codec_id;
        std::cout << "[FFmpeg] Detected and opened " << avcodec_get_name(codec_id) << " stream" << std::endl;
        return true;
    }

    void releaseCodec() {
        if (codec_ctx) avcodec_free_context(&codec_ctx);
        if (sws_ctx) {
            sws_freeContext(sws_ctx);
            sws_ctx = nullptr;
        }
        current_codec = AV_CODEC_ID_NONE;
        last_w = 0;
        last_h = 0;
        last_pix_fmt = AV_PIX_FMT_NONE;
    }

    void cleanup() {
        releaseCodec();
        if (frame) av_frame_free(&frame);
        if (packet) av_packet_free(&packet);
    }
    AVCodecContext* codec_ctx;
    AVFrame* frame;
    AVPacket* packet;
    SwsContext* sws_ctx;
    AVCodecID current_codec;
    int last_w;
    int last_h;
    AVPixelFormat last_pix_fmt;
};
