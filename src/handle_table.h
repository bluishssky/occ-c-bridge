// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <typeinfo>
#include <unordered_map>

#include "occ_c_types.h"

// 错误状态：thread_local 但必须跨编译单元共享（inline 变量），否则
// ocb_last_status/ocb_last_error 读不到其他 cpp 里设置的错误。
inline OcbStatus& bridgeErrStatus() {
    thread_local OcbStatus s = OCB_OK;
    return s;
}
inline std::string& bridgeErrText() {
    thread_local std::string s;
    return s;
}
inline void setError(OcbStatus status, const char* message) {
    bridgeErrStatus() = status;
    bridgeErrText() = message ? message : "";
}
inline void clearError() {
    bridgeErrStatus() = OCB_OK;
    bridgeErrText().clear();
}

// 句柄表：把 OCC 对象的所有权收进中间层，对外只暴露 uint64_t ID。
// 好处：
//   - 无效句柄可检测（get 返回 nullptr），不会悬空崩溃
//   - 不会双重释放
//   - 可统计存活数量，做泄漏检测
class HandleTable {
public:
    static HandleTable& instance() {
        static HandleTable table;
        return table;
    }

    template <class T>
    uint64_t add(T obj) {
        std::lock_guard<std::mutex> lock(mutex_);
        const uint64_t id = next_++;
        T* raw = new T(std::move(obj));
        table_[id] = std::shared_ptr<void>(raw, [](void* p) {
            delete static_cast<T*>(p);
        });
        kinds_[id] = typeid(T).hash_code();
        return id;
    }

    template <class T>
    T* get(uint64_t id) {
        if (id == 0) {
            return nullptr;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        const auto it = table_.find(id);
        if (it == table_.end()) {
            return nullptr;
        }
        const auto kind = kinds_.find(id);
        if (kind == kinds_.end() || kind->second != typeid(T).hash_code()) {
            return nullptr;
        }
        return static_cast<T*>(it->second.get());
    }

    bool release(uint64_t id) {
        if (id == 0) {
            return false;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        const auto it = table_.find(id);
        if (it == table_.end()) {
            return false;
        }
        table_.erase(it);
        kinds_.erase(id);
        return true;
    }

    uint64_t liveCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return static_cast<uint64_t>(table_.size());
    }

private:
    HandleTable() = default;
    ~HandleTable() = default;
    HandleTable(const HandleTable&) = delete;
    HandleTable& operator=(const HandleTable&) = delete;

    mutable std::mutex mutex_;
    uint64_t next_ = 1;
    std::unordered_map<uint64_t, std::shared_ptr<void>> table_;
    std::unordered_map<uint64_t, size_t> kinds_;
};
