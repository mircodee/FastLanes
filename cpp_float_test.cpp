// ────────────────────────────────────────────────────────
// |                      FastLanes                       |
// ────────────────────────────────────────────────────────
// cpp_float_test.cpp - C++浮点数性能测试，与Java版本对比
// ────────────────────────────────────────────────────────

#include <iostream>
#include <chrono>
#include <vector>
#include <cstring>
#include <cmath>

using namespace std;
using namespace std::chrono;

class FloatBenchmark {
public:
    static void testFloat32(const vector<float>& data) {
        auto start = high_resolution_clock::now();
        vector<uint8_t> encoded = encodeFloat32(data);
        auto encode_end = high_resolution_clock::now();
        
        vector<float> decoded = decodeFloat32(encoded, data.size());
        auto decode_end = high_resolution_clock::now();
        
        double encode_time = duration_cast<microseconds>(encode_end - start).count() / 1000.0;
        double decode_time = duration_cast<microseconds>(decode_end - encode_end).count() / 1000.0;
        
        double ratio = (double)encoded.size() / (data.size() * 4);
        double encode_mbs = (data.size() * 4.0) / (encode_time / 1000.0) / 1024 / 1024;
        double decode_mbs = (data.size() * 4.0) / (decode_time / 1000.0) / 1024 / 1024;
        
        printf("Float32 - 编码: %.2fms (%.1f MB/s), 解码: %.2fms (%.1f MB/s), 压缩比: %.2f%%\n",
               encode_time, encode_mbs, decode_time, decode_mbs, ratio * 100);
    }
    
    static void testFloat64(const vector<double>& data) {
        auto start = high_resolution_clock::now();
        vector<uint8_t> encoded = encodeFloat64(data);
        auto encode_end = high_resolution_clock::now();
        
        vector<double> decoded = decodeFloat64(encoded, data.size());
        auto decode_end = high_resolution_clock::now();
        
        double encode_time = duration_cast<microseconds>(encode_end - start).count() / 1000.0;
        double decode_time = duration_cast<microseconds>(decode_end - encode_end).count() / 1000.0;
        
        double ratio = (double)encoded.size() / (data.size() * 8);
        double encode_mbs = (data.size() * 8.0) / (encode_time / 1000.0) / 1024 / 1024;
        double decode_mbs = (data.size() * 8.0) / (decode_time / 1000.0) / 1024 / 1024;
        
        printf("Float64 - 编码: %.2fms (%.1f MB/s), 解码: %.2fms (%.1f MB/s), 压缩比: %.2f%%\n",
               encode_time, encode_mbs, decode_time, decode_mbs, ratio * 100);
    }

private:
    static vector<uint8_t> encodeFloat32(const vector<float>& values) {
        vector<uint8_t> encoded(values.size() * 4 + 4);
        
        uint32_t first;
        memcpy(&first, &values[0], sizeof(float));
        encoded[0] = first & 0xFF;
        encoded[1] = (first >> 8) & 0xFF;
        encoded[2] = (first >> 16) & 0xFF;
        encoded[3] = (first >> 24) & 0xFF;
        
        size_t pos = 4;
        for (size_t i = 1; i < values.size(); i++) {
            uint32_t current, prev;
            memcpy(&current, &values[i], sizeof(float));
            memcpy(&prev, &values[i-1], sizeof(float));
            uint32_t xor_val = current ^ prev;
            
            encoded[pos++] = xor_val & 0xFF;
            encoded[pos++] = (xor_val >> 8) & 0xFF;
            encoded[pos++] = (xor_val >> 16) & 0xFF;
            encoded[pos++] = (xor_val >> 24) & 0xFF;
        }
        
        encoded.resize(pos);
        return encoded;
    }
    
    static vector<float> decodeFloat32(const vector<uint8_t>& data, size_t len) {
        vector<float> values(len);
        
        uint32_t first = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
        memcpy(&values[0], &first, sizeof(float));
        
        size_t pos = 4;
        for (size_t i = 1; i < len; i++) {
            uint32_t xor_val = data[pos] | (data[pos+1] << 8) | (data[pos+2] << 16) | (data[pos+3] << 24);
            uint32_t prev;
            memcpy(&prev, &values[i-1], sizeof(float));
            uint32_t current = prev ^ xor_val;
            memcpy(&values[i], &current, sizeof(float));
            pos += 4;
        }
        
        return values;
    }
    
    static vector<uint8_t> encodeFloat64(const vector<double>& values) {
        vector<uint8_t> encoded(values.size() * 8 + 8);
        
        uint64_t first;
        memcpy(&first, &values[0], sizeof(double));
        for (int i = 0; i < 8; i++) {
            encoded[i] = (first >> (i * 8)) & 0xFF;
        }
        
        size_t pos = 8;
        for (size_t i = 1; i < values.size(); i++) {
            uint64_t current, prev;
            memcpy(&current, &values[i], sizeof(double));
            memcpy(&prev, &values[i-1], sizeof(double));
            uint64_t xor_val = current ^ prev;
            
            for (int j = 0; j < 8; j++) {
                encoded[pos++] = (xor_val >> (j * 8)) & 0xFF;
            }
        }
        
        encoded.resize(pos);
        return encoded;
    }
    
    static vector<double> decodeFloat64(const vector<uint8_t>& data, size_t len) {
        vector<double> values(len);
        
        uint64_t first = 0;
        for (int i = 0; i < 8; i++) {
            first |= ((uint64_t)data[i]) << (i * 8);
        }
        memcpy(&values[0], &first, sizeof(double));
        
        size_t pos = 8;
        for (size_t i = 1; i < len; i++) {
            uint64_t xor_val = 0;
            for (int j = 0; j < 8; j++) {
                xor_val |= ((uint64_t)data[pos+j]) << (j * 8);
            }
            uint64_t prev;
            memcpy(&prev, &values[i-1], sizeof(double));
            uint64_t current = prev ^ xor_val;
            memcpy(&values[i], &current, sizeof(double));
            pos += 8;
        }
        
        return values;
    }
};

int main() {
    cout << "=== FastLanes C++ 性能测试 ===" << endl;
    
    // 测试数据 - 与Java版本相同
    const size_t size = 100000;
    vector<float> data32(size);
    for (size_t i = 0; i < size; i++) {
        data32[i] = 100.0f + sin(i * 0.1f) * 10.0f;
    }
    
    // 测试Float32
    FloatBenchmark::testFloat32(data32);
    
    // 测试Float64
    vector<double> data64(size);
    for (size_t i = 0; i < size; i++) {
        data64[i] = 100.0 + sin(i * 0.1) * 10.0;
    }
    FloatBenchmark::testFloat64(data64);
    
    return 0;
}