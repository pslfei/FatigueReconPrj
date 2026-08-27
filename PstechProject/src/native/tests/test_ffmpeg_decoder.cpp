#include "../plugins/FfmpegDecoder.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace {
int failures = 0;

void ExpectCodec(const std::string& name, const std::vector<unsigned char>& data, AVCodecID expected) {
    const AVCodecID actual = FfmpegDecoder::detectCodecId(data.data(), data.size());
    if (actual == expected) return;

    std::cerr << "[FAIL] " << name << ": expected " << avcodec_get_name(expected)
              << ", got " << avcodec_get_name(actual) << std::endl;
    ++failures;
}

void ExpectDecodedFile(const std::string& name, const std::string& path) {
    std::ifstream stream(path, std::ios::binary);
    const std::vector<unsigned char> data((std::istreambuf_iterator<char>(stream)),
                                          std::istreambuf_iterator<char>());
    if (!stream.is_open() || data.empty()) {
        std::cerr << "[FAIL] " << name << ": cannot read " << path << std::endl;
        ++failures;
        return;
    }

    FfmpegDecoder decoder;
    cv::Mat image;
    if (!decoder.init() || !decoder.decode(data.data(), static_cast<int>(data.size()), image) || image.empty()) {
        std::cerr << "[FAIL] " << name << ": decoder did not produce a frame" << std::endl;
        ++failures;
        return;
    }
    if (image.type() != CV_8UC3) {
        std::cerr << "[FAIL] " << name << ": expected CV_8UC3 output, got type "
                  << image.type() << std::endl;
        ++failures;
    }
}

void ExpectSwitchAndReset(const std::string& h264_path, const std::string& h265_path) {
    std::ifstream h264_stream(h264_path, std::ios::binary);
    std::ifstream h265_stream(h265_path, std::ios::binary);
    const std::vector<unsigned char> h264_data((std::istreambuf_iterator<char>(h264_stream)),
                                               std::istreambuf_iterator<char>());
    const std::vector<unsigned char> h265_data((std::istreambuf_iterator<char>(h265_stream)),
                                               std::istreambuf_iterator<char>());
    if (h264_data.empty() || h265_data.empty()) {
        std::cerr << "[FAIL] codec switch/reset smoke: input stream is empty" << std::endl;
        ++failures;
        return;
    }

    FfmpegDecoder decoder;
    cv::Mat image;
    if (!decoder.init() || !decoder.decode(h264_data.data(), static_cast<int>(h264_data.size()), image) ||
        image.empty()) {
        std::cerr << "[FAIL] codec switch/reset smoke: initial H.264 decode failed" << std::endl;
        ++failures;
        return;
    }
    if (!decoder.decode(h265_data.data(), static_cast<int>(h265_data.size()), image) || image.empty()) {
        std::cerr << "[FAIL] codec switch/reset smoke: H.264 to H.265 switch failed" << std::endl;
        ++failures;
        return;
    }

    decoder.reset();
    if (!decoder.decode(h264_data.data(), static_cast<int>(h264_data.size()), image) || image.empty()) {
        std::cerr << "[FAIL] codec switch/reset smoke: H.264 decode after reset failed" << std::endl;
        ++failures;
    }
}
}

int main(int argc, char** argv) {
    ExpectCodec("H.264 SPS with four-byte start code",
                {0x00, 0x00, 0x00, 0x01, 0x67, 0x64, 0x00, 0x1F, 0xAC}, AV_CODEC_ID_H264);
    ExpectCodec("H.264 PPS with three-byte start code",
                {0x00, 0x00, 0x01, 0x68, 0xEE, 0x3C, 0x80}, AV_CODEC_ID_H264);
    ExpectCodec("H.265 VPS with four-byte start code",
                {0x00, 0x00, 0x00, 0x01, 0x40, 0x01, 0x0C}, AV_CODEC_ID_HEVC);
    ExpectCodec("H.265 SPS with three-byte start code",
                {0x00, 0x00, 0x01, 0x42, 0x01, 0x01}, AV_CODEC_ID_HEVC);
    ExpectCodec("H.265 PPS with four-byte start code",
                {0x00, 0x00, 0x00, 0x01, 0x44, 0x01, 0xC0}, AV_CODEC_ID_HEVC);
    ExpectCodec("H.264 SPS after AUD",
                {0x00, 0x00, 0x01, 0x09, 0xF0,
                 0x00, 0x00, 0x00, 0x01, 0x67, 0x42, 0x00, 0x1E}, AV_CODEC_ID_H264);
    ExpectCodec("H.265 VPS after AUD",
                {0x00, 0x00, 0x01, 0x46, 0x01, 0x50,
                 0x00, 0x00, 0x01, 0x40, 0x01, 0x0C}, AV_CODEC_ID_HEVC);
    ExpectCodec("H.264 slice without parameter set",
                {0x00, 0x00, 0x00, 0x01, 0x65, 0x88, 0x84}, AV_CODEC_ID_NONE);
    ExpectCodec("H.265 type-20 IDR is not H.264 PPS",
                {0x00, 0x00, 0x00, 0x01, 0x28, 0x01, 0xAF}, AV_CODEC_ID_NONE);
    ExpectCodec("Invalid H.265 temporal id",
                {0x00, 0x00, 0x01, 0x42, 0x00, 0x01}, AV_CODEC_ID_NONE);
    ExpectCodec("Truncated H.264 SPS",
                {0x00, 0x00, 0x01, 0x67, 0x64}, AV_CODEC_ID_NONE);
    ExpectCodec("Data without Annex B start code",
                {0x67, 0x64, 0x00, 0x1F}, AV_CODEC_ID_NONE);
    ExpectCodec("Conflicting parameter sets",
                {0x00, 0x00, 0x01, 0x67, 0x64, 0x00, 0x1F,
                 0x00, 0x00, 0x01, 0x40, 0x01, 0x0C}, AV_CODEC_ID_NONE);

    if (argc == 3) {
        ExpectDecodedFile("H.264 decode smoke", argv[1]);
        ExpectDecodedFile("H.265 decode smoke", argv[2]);
        ExpectSwitchAndReset(argv[1], argv[2]);
    } else if (argc != 1) {
        std::cerr << "Usage: Test_FfmpegDecoder [h264-annex-b-file h265-annex-b-file]" << std::endl;
        return 2;
    }

    if (failures != 0) {
        std::cerr << failures << " codec detection test(s) failed" << std::endl;
        return 1;
    }
    std::cout << "All FFmpeg codec detection tests passed" << std::endl;
    return 0;
}
