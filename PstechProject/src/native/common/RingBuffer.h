#pragma once
#include <vector>
#include <atomic>
#include <cstring>
#include <iostream>

// 缓存秒数：30fps * 5s = 150帧
// 1080P: 1920*1080*3 * 150 = ~900MB
// 720P:  1280*720*3  * 150 = ~400MB
// 640P:  640*480*3   * 150 = ~130MB
#define BUFFER_CAPACITY 150 
#define MAX_WIDTH 1920
#define MAX_HEIGHT 1080
#define MAX_FRAME_SIZE (MAX_WIDTH * MAX_HEIGHT * 3)

struct FrameNode {
    std::vector<unsigned char> data;
    int width;
    int height;
    long long timestamp;
    
    FrameNode() {
        // 预分配最大内存，避免运行时 new/malloc 造成抖动
        data.resize(MAX_FRAME_SIZE);
        width = 0;
        height = 0;
        timestamp = 0;
    }
};

class RingBuffer {
private:
    std::vector<FrameNode> m_buffer;
    std::atomic<int> m_head; // 指向下一个写入位置
    
public:
    RingBuffer() : m_head(0) {
        // 初始化池子
        m_buffer.resize(BUFFER_CAPACITY);
    }
    
    // 写入一帧 (生产者)
    // 这是一个极快的内存拷贝操作
    void Write(const unsigned char* src, int w, int h, long long ts) {
        if (w * h * 3 > MAX_FRAME_SIZE) {
             // 超过预分配大小，丢弃或重新分配(这里选择忽略防止崩)
             return;
        }

        // 获取当前写入槽位 (原子加载，虽非必须但好习惯)
        int currentHead = m_head.load(std::memory_order_relaxed);
        
        FrameNode& node = m_buffer[currentHead];
        node.width = w;
        node.height = h;
        node.timestamp = ts;
        std::memcpy(node.data.data(), src, w * h * 3);
        
        // 移动指针 (原子操作)
        int nextHead = (currentHead + 1) % BUFFER_CAPACITY;
        m_head.store(nextHead, std::memory_order_release);
    }
    
    // 获取最新一帧 (消费者)
    // 返回指针。注意：如果消费者处理太慢(>5秒)，该指针指向的内容可能会被覆盖，产生撕裂。
    // 但对于实时分析，这种概率极低且可接受。
    unsigned char* GetLatest(int& w, int& h, long long& ts) {
        int currentHead = m_head.load(std::memory_order_acquire);
        
        // 回退一格，获取刚刚写完的那一帧
        int latestIdx = (currentHead - 1 + BUFFER_CAPACITY) % BUFFER_CAPACITY;
        
        FrameNode& node = m_buffer[latestIdx];
        if (node.width == 0) return nullptr; // 还没数据
        
        w = node.width;
        h = node.height;
        ts = node.timestamp;
        return node.data.data();
    }
    
    // TODO: GetRange(startTime, endTime) for video saving
};