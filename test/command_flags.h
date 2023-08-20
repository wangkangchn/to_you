/* **************************************************************
 * Copyright © 2022 Yisa Inc.
 * 文件名   : command_flags.h
 * 作者	  	: wkangk <wangkang@yisa.com>
 * 版本	   	: v1.0
 * 描述	   	: 命令行参数
 * 时间	   	: 2022-08-06 22:05
 * **************************************************************/
#pragma once
#ifndef __terrace_MXBASE_WKANGK_COMMAND_FLAGS_H__
#define __terrace_MXBASE_WKANGK_COMMAND_FLAGS_H__
#include <gflags/gflags.h>


/* 设备相关 */
DEFINE_int32(device_id, 1, "设备 id");
DEFINE_int32(num_threads, 2, "线程数");
DEFINE_int32(batch, 1, "batch 数");

/* 日志相关 */
DEFINE_string(log_fn, "./logs/logger.log", "待输出的日志文件");
DEFINE_string(log_level, "info", "日志输出的等级, 默认 INFO, 可选 [debug|info|warn|error]");
DEFINE_string(image, "../../../../images/c060342e-174e-11ed-ba71-5b707e07e69c.jpg", "image path");
DEFINE_string(video_url, "../../../../video/vlc-record-2022-09-02-08h37m42s-test.mp4-.mp4", "video url");

DEFINE_bool(memory_leak, false, "是否开启内存泄漏检查");
DEFINE_bool(online, false, "实时流");

#endif
