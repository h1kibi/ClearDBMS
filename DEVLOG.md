# ClearDBMS 开发日志

## 2026-04-30

### 实现缓冲池管理器核心方法 & 线程安全

- **`BufferPoolManager::find_victim_page`**: 实现淘汰页选取逻辑 — free_list 非空时从头部取，已满时通过 LRU 策略淘汰。
- **`BufferPoolManager::update_page`**: 页面更新逻辑，脏页回写磁盘、更新 page table、重置页数据。
- **`BufferPoolManager::fetch_page`**: 页面获取完整流程 — 查 page table 缓存命中直接返回，未命中则分配帧、读盘、固定页。
- **`DiskManager::allocate_page`**: 添加 `std::scoped_lock` 互斥锁，保证多线程下页面分配安全。
- 代码风格修正 (`lru_replacer.cpp`)。

### 更新 Replacer & Disk Manager

- 整理 LRU Replacer 的淘汰逻辑结构。
- Disk Manager 初始化及文件管理基本框架。

## 2026-04-29 (约)

### 项目初始化

- 搭建 ClearDBMS 项目骨架：CMake 构建系统、目录结构 (src/docs/deps/build)。
- 添加 .gitignore、.clang-format、LICENSE (MIT)。
- 项目目标：基于 RUCBase 实验项目开发的教学型关系数据库管理系统，支持 SQL 解析、执行、索引、事务与并发控制。
