/*
 * Copyright (C) 2020 Rockchip Electronics Co. LTD
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

#ifndef ANDROID_C2_RK_NAL_DEF_H__
#define ANDROID_C2_RK_NAL_DEF_H__

class C2RKNalParser {
public:
    static int32_t getBitDepth(uint8_t *src, int32_t size, int32_t codingType);

private:
    typedef struct {
        uint8_t *buf;
        int32_t  bufSize;
        int32_t  bitPos;
        int32_t  totalBit;
        int32_t  curBitPos;
    } MyBitCtx_t;

    static void* createBitCtx(void *buf, int32_t size);
    static void  freeBitCtx(void *ctx);

    static int32_t getBits(void *ctx, int32_t pos);
    static void    skipBits(void *ctx, int32_t pos);

    static int32_t getAvcBitDepth(void *ctx);
    static int32_t getHevcBitDepth(void *ctx);
};

#endif  // ANDROID_C2_RK_NAL_DEF_H__
