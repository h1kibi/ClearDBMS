/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "rm_scan.h"
#include "rm_file_handle.h"

/**
 * @brief 初始化file_handle和rid
 * @param file_handle
 */
RmScan::RmScan(const RmFileHandle *file_handle) : file_handle_(file_handle) {
    // Todo:
    // 初始化file_handle和rid（指向第一个存放了记录的位置）

    if(file_handle_ == nullptr) { throw std::invalid_argument("RmScan: file_handle is null"); }

    // 1. 将游标放到“起跑线”前：第一页的第 -1 个槽位
    // RM_FIRST_RECORD_PAGE 在 defs.h 中通常定义为 1
    rid_.page_no = RM_FIRST_RECORD_PAGE; 
    rid_.slot_no = -1;

    // 2. 直接调用 next()。
    // next() 内部会自动将 slot_no 加 1 (变成 0)，然后开始在第一页寻找第一条真实存在的数据。
    // 如果整张表是空的，next() 会自动把 rid_.page_no 设为 RM_NO_PAGE。
    next();
}

/**
 * @brief 找到文件中下一个存放了记录的位置
 */
void RmScan::next() {
    // Todo:
    // 找到文件中下一个存放了记录的非空闲位置，用rid_来指向这个位置

    rid_.slot_no++;

    // 2. 获取表文件目前总共有多少页，以及每页最多能存多少条记录
    int max_page_no = file_handle_->file_hdr_.num_pages;
    int max_slot_no = file_handle_->file_hdr_.num_records_per_page;

    // 3. 外层循环：遍历文件中的每一页（从当前 rid_.page_no 开始）
    while (rid_.page_no < max_page_no) {
        
        // 从 Buffer Pool 中获取当前页的句柄 (这一步会把页面 Pin 在内存里)
        RmPageHandle page_handle = file_handle_->fetch_page_handle(rid_.page_no);

        // 4. 内层循环：在当前页中，从 rid_.slot_no 开始往后找
        while (rid_.slot_no < max_slot_no) {
            
            // 使用 Bitmap 工具类检查当前槽位是否为 1 (1 表示有真实数据，0 表示被删了或空着)
            if (Bitmap::is_set(page_handle.bitmap, rid_.slot_no)) {
                
                // 找到了数据，unpin页面后退出
                file_handle_->buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);
                return; // 找到了！保留当前 rid_ 状态，直接退出函数
            }
            
            // 当前槽位是空的，继续看下一个槽位
            rid_.slot_no++;
        }

        // 5. 当前页看完了，unpin当前页
        file_handle_->buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);

        // 6. 翻页操作：去下一页，并且把槽位重置为 0，准备从下一页的开头继续找
        rid_.page_no++;
        rid_.slot_no = 0;
    }

    // 7. 如果把所有的页 (while循环) 都找完了，还是没执行到 return，说明真的到表尾了。
    // 将状态设置为结束标志。RM_NO_PAGE 通常在 defs.h 中定义为 -1。
    rid_.page_no = RM_NO_PAGE;
    rid_.slot_no = -1;
}

/**
 * @brief ​ 判断是否到达文件末尾
 */
bool RmScan::is_end() const {
    // Todo: 修改返回值

    return rid_.page_no == RM_NO_PAGE;
}

/**
 * @brief RmScan内部存放的rid
 */
Rid RmScan::rid() const {
    return rid_;
}