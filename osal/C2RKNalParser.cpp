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

#undef  ROCKCHIP_LOG_TAG
#define ROCKCHIP_LOG_TAG    "C2RKNalParser"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "C2RKNalParser.h"
#include "C2RKLog.h"
#include "mpp/rk_mpi.h"

#define NAL_MAX_LEN             32
#define HEVC_PROFILE_MAIN_10    2
#define H264_PROFILE_HIGH10     110

int32_t C2RKNalParser::getBits(void *ctx, int32_t pos) {
    uint8_t  temp[5] = {0};
    uint8_t *curChar = NULL;
    uint8_t  nbyte  = 0;
    uint8_t  shift  = 0;
    uint32_t result = 0;
    uint64_t ret    = 0;

    MyBitCtx_t *bitCtx = (MyBitCtx_t *)ctx;

    if (pos > NAL_MAX_LEN) {
        pos = NAL_MAX_LEN;
    }

    if ((bitCtx->bitPos + pos) > bitCtx->totalBit) {
        pos = bitCtx->totalBit - bitCtx->bitPos;
    }

    curChar = bitCtx->buf + (bitCtx->bitPos >> 3);
    nbyte   = (bitCtx->curBitPos + pos + 7) >> 3;
    shift   = (8 - (bitCtx->curBitPos + pos)) & 0x07;

    memcpy(&temp[5 - nbyte], curChar, nbyte);
    ret = (uint32_t)temp[0] << 24;
    ret = ret << 8;
    ret = ((uint32_t)temp[1] << 24)|((uint32_t)temp[2] << 16) \
                        | ((uint32_t)temp[3] << 8)| temp[4];

    ret = (ret >> shift) & (((uint64_t)1 << pos) - 1);

    result = ret;
    bitCtx->bitPos += pos;
    bitCtx->curBitPos = bitCtx->bitPos & 0x7;

    return result;
}

void C2RKNalParser::skipBits(void *ctx, int32_t pos) {
    MyBitCtx_t *bitCtx = (MyBitCtx_t *)ctx;
    bitCtx->bitPos     += pos;
    bitCtx->curBitPos  = bitCtx->bitPos & 0x7;
}

int32_t C2RKNalParser::getAvcBitDepth(void * ctx) {
    int32_t profileIdc = 0;

    profileIdc = getBits(ctx, 8);
    if (profileIdc == H264_PROFILE_HIGH10) {
        return 10;
    }

    return 8;
}

int32_t C2RKNalParser::getHevcBitDepth(void * ctx) {
    int profileIdc = 0;

    skipBits(ctx, 8);  // nal header
    skipBits(ctx, 4);  // vps_id
    skipBits(ctx, 3);  // max_sub_layers
    skipBits(ctx, 1);
    skipBits(ctx, 2);  // profile_space
    skipBits(ctx, 1);  // tier_flag
    profileIdc = getBits(ctx, 5);
    if (profileIdc == HEVC_PROFILE_MAIN_10) {
        return 10;
    }

    return 8;
}

void* C2RKNalParser::createBitCtx(void *buf, int32_t size) {
    MyBitCtx_t *bitCtx = NULL;

    bitCtx = (MyBitCtx_t *)malloc(sizeof(MyBitCtx_t));
    if (bitCtx == NULL) {
        c2_err("failed to alloc bitCtx");
        goto exit;
    }

    bitCtx->bitPos    = 0;
    bitCtx->curBitPos = 0;
    bitCtx->bufSize   = size;
    bitCtx->buf       = (uint8_t *)malloc(size);
    if (bitCtx->buf == NULL) {
        c2_err("failed to alloc bitCtx buf");
        goto exit;
    }

    memcpy(bitCtx->buf, buf, size);
    bitCtx->totalBit = bitCtx->bufSize << 3;

    return (void *)bitCtx;

exit:
    freeBitCtx(bitCtx);
    return NULL;
}

void C2RKNalParser::freeBitCtx(void *ctx) {
    MyBitCtx_t *bitCtx = (MyBitCtx_t *)ctx;

    if (bitCtx != NULL) {
        if (bitCtx->buf != NULL) {
            free(bitCtx->buf);
            bitCtx->buf = NULL;
        }
        free(bitCtx);
        bitCtx = NULL;
    }
}

int32_t C2RKNalParser::getBitDepth(uint8_t *src, int32_t size, int32_t codingType) {
    void     *ctx = NULL;
    int32_t   i   = 0;
    int32_t   ret = 0;

    if ((codingType != MPP_VIDEO_CodingAVC && codingType != MPP_VIDEO_CodingHEVC) || size < 4) {
        return 8;
    }

    for (i = 0; i <= size - 4; i++) {
        if (codingType == MPP_VIDEO_CodingAVC) {
            if (src[i] == 0x00 && src[i + 1] == 0x00 &&
                src[i + 2] == 0x01 && src[i + 3] == 0x67) {
                i += 4;
                c2_info("find h264 sps");
                break;
            }
        } else if (codingType == MPP_VIDEO_CodingHEVC) {
            if (src[i] == 0x00 && src[i + 1] == 0x00 &&
                src[i + 2] == 0x01 && ((src[i + 3] & 0x7f) >> 1) == 33) {
                i += 4;
                c2_info("find h265 sps");
                break;
            }
       }
    }
    if (i == (size - 3)) {
        return 8;   // default 8
    }

    ctx = createBitCtx(src + i, size - i);
    if (ctx == NULL) {
        c2_err("failed to create bitCtx, set default 8");
        return 8;
    }

    if (codingType == MPP_VIDEO_CodingAVC) {
        ret = getAvcBitDepth(ctx);
    } else if (codingType == MPP_VIDEO_CodingHEVC) {
        ret = getHevcBitDepth(ctx);
    }

    freeBitCtx(ctx);

    return ret;
}
