/*
 * Copyright (C) 2021 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string.h>
#include <android/log.h>
#include "C2RKLog.h"
#include "C2RKEnv.h"

static uint32_t envValue = 0;
void _Rockchip_C2_Log_Init() {
    Rockchip_C2_GetEnvU32("vendor.c2.log.debug", &envValue, 0);
}


void _Rockchip_C2_Log(ROCKCHIP_LOG_LEVEL logLevel, uint32_t flag, const char *tag, const char *msg, ...)
{
    va_list argptr;

    va_start(argptr, msg);

    switch (logLevel) {
    case ROCKCHIP_LOG_TRACE: {
        if (envValue & C2_TRACE_ON) {
            __android_log_vprint(ANDROID_LOG_DEBUG, tag, msg, argptr);
        }
    }
    break;
    case ROCKCHIP_LOG_DEBUG: {
        if (envValue & flag) {
            __android_log_vprint(ANDROID_LOG_DEBUG, tag, msg, argptr);
        }
    } break;
    case ROCKCHIP_LOG_INFO:
        __android_log_vprint(ANDROID_LOG_INFO, tag, msg, argptr);
        break;
    case ROCKCHIP_LOG_WARNING:
        __android_log_vprint(ANDROID_LOG_WARN, tag, msg, argptr);
        break;
    case ROCKCHIP_LOG_ERROR:
        __android_log_vprint(ANDROID_LOG_ERROR, tag, msg, argptr);
        break;
    default:
        __android_log_vprint(ANDROID_LOG_VERBOSE, tag, msg, argptr);
    }

    va_end(argptr);
}


