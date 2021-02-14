//
// H264CommonFunc.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __H264_COMMON_FUNC_H__
#define __H264_COMMON_FUNC_H__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "Bitstream.h"

#define    H264_MAX_DECODED_PICTURE_BUFFER_COUNT    16    // DPB[16]
#define    H264_MAX_REF_PIC_LIST_COUNT    16    // RefPicList0[16]

#define    NA    -1
#define    MB_WIDTH     16
#define    MB_HEIGHT    16

#define    PICTURE_FORMAT_YUV420    0
#define    PICTURE_FORMAT_YUV422    1
#define    PICTURE_FORMAT_YUV444    2


/*
 * T-REC-H.264-201704-S!!PDF-E.pdf
 * Table 7-1 – NAL unit type codes
 */
typedef enum _H264_NAL_
{
    H264_NAL_UNSPECIFIED = 0,
    H264_NAL_SLICE = 1,
    H264_NAL_DPA = 2,
    H264_NAL_DPB = 3,
    H264_NAL_DPC = 4,
    H264_NAL_IDR_SLICE = 5,
    H264_NAL_SEI = 6,
    H264_NAL_SPS = 7,
    H264_NAL_PPS = 8,
    H264_NAL_AUD = 9,
    H264_NAL_END_SEQUENCE = 10,
    H264_NAL_END_STREAM = 11,
    H264_NAL_FILLER_DATA = 12,
    H264_NAL_SPS_EXT = 13,
    H264_NAL_PREFIX = 14,
    H264_NAL_SUB_SPS = 15,
    H264_NAL_DPS = 16,
    H264_NAL_RESERVED17 = 17,
    H264_NAL_RESERVED18 = 18,
    H264_NAL_AUXILIARY_SLICE = 19,
    H264_NAL_EXTEN_SLICE = 20,
    H264_NAL_DEPTH_EXTEN_SLICE = 21,
    H264_NAL_RESERVED22 = 22,
    H264_NAL_RESERVED23 = 23,
    H264_NAL_UNSPECIFIED24 = 24,
    H264_NAL_UNSPECIFIED25 = 25,
    H264_NAL_UNSPECIFIED26 = 26,
    H264_NAL_UNSPECIFIED27 = 27,
    H264_NAL_UNSPECIFIED28 = 28,
    H264_NAL_UNSPECIFIED29 = 29,
    H264_NAL_UNSPECIFIED30 = 30,
    H264_NAL_UNSPECIFIED31 = 31,
}H264_NAL;


typedef enum _H264_PICTURE_CODED_TYPE_
{
    H264_PICTURE_CODED_TYPE_UNKNOWN = 0,
    H264_PICTURE_CODED_TYPE_FRAME = 1, //帧
    H264_PICTURE_CODED_TYPE_FIELD = 2, //场
    H264_PICTURE_CODED_TYPE_TOP_FIELD = 3, //顶场
    H264_PICTURE_CODED_TYPE_BOTTOM_FIELD = 4, //底场
    H264_PICTURE_CODED_TYPE_COMPLEMENTARY_FIELD_PAIR = 5, //互补场对
}H264_PICTURE_CODED_TYPE;


typedef enum _H264_PICTURE_TYPE_
{
    H264_PICTURE_TYPE_UNKNOWN = 0,
    H264_PICTURE_TYPE_I = 1, //I 帧（I帧不一定是IDR帧）
    H264_PICTURE_TYPE_P = 2, //P 帧
    H264_PICTURE_TYPE_B = 3, //B 帧
    H264_PICTURE_TYPE_IDR = 4, //IDR 帧（IDR帧一定是I帧）
}H264_PICTURE_TYPE;


//宏块残差幅值类型
typedef enum _MB_RESIDUAL_LEVEL_
{
    MB_RESIDUAL_UNKOWN = -1,
    MB_RESIDUAL_Intra16x16DCLevel = 0,
    MB_RESIDUAL_Intra16x16ACLevel = 1,
    MB_RESIDUAL_LumaLevel4x4 = 2,
    MB_RESIDUAL_ChromaDCLevel = 3,
    MB_RESIDUAL_ChromaACLevel = 4,
    MB_RESIDUAL_LumaLevel8x8 = 5,
    MB_RESIDUAL_CbIntra16x16DCLevel = 6,
    MB_RESIDUAL_CbIntra16x16ACLevel = 7,
    MB_RESIDUAL_CbLevel4x4 = 8,
    MB_RESIDUAL_CbLevel8x8 = 9,
    MB_RESIDUAL_CrIntra16x16DCLevel = 10,
    MB_RESIDUAL_CrIntra16x16ACLevel = 11,
    MB_RESIDUAL_CrLevel4x4 = 12,
    MB_RESIDUAL_CrLevel8x8 = 13,
    MB_RESIDUAL_ChromaDCLevelCb = 14,
    MB_RESIDUAL_ChromaDCLevelCr = 15,
    MB_RESIDUAL_ChromaACLevelCb = 16,
    MB_RESIDUAL_ChromaACLevelCr = 17,
}MB_RESIDUAL_LEVEL;


/*
 * Figure 6-14 – Determination of the neighbouring macroblock, blocks, and partitions (informative)
 *    D    B    C
 *    A    Current Macroblock or Partition or Block
 */
typedef enum _MB_ADDR_TYPE_
{
    MB_ADDR_TYPE_UNKOWN = 0,
    MB_ADDR_TYPE_mbAddrA = 1,
    MB_ADDR_TYPE_mbAddrB = 2,
    MB_ADDR_TYPE_mbAddrC = 3,
    MB_ADDR_TYPE_mbAddrD = 4,
    MB_ADDR_TYPE_CurrMbAddr = 5,
    MB_ADDR_TYPE_mbAddrA_add_1 = 6,
    MB_ADDR_TYPE_mbAddrB_add_1 = 7,
    MB_ADDR_TYPE_mbAddrC_add_1 = 8,
    MB_ADDR_TYPE_mbAddrD_add_1 = 9,
    MB_ADDR_TYPE_CurrMbAddr_minus_1 = 10,
}MB_ADDR_TYPE;


//Table 8-8 – Specification of mbAddrCol, yM, and vertMvScale
typedef enum _H264_VERT_MV_SCALE_
{
    H264_VERT_MV_SCALE_UNKNOWN = 0,
    H264_VERT_MV_SCALE_One_To_One = 1,
    H264_VERT_MV_SCALE_Frm_To_Fld = 2,
    H264_VERT_MV_SCALE_Fld_To_Frm = 3,
}H264_VERT_MV_SCALE;


//------------------------------------------------
//Table 7-6 – Name association to slice_type
typedef enum _H264_SLIECE_TYPE_
{
    H264_SLIECE_TYPE_P   = 0,
    H264_SLIECE_TYPE_B   = 1,
    H264_SLIECE_TYPE_I   = 2,
    H264_SLIECE_TYPE_SP  = 3,
    H264_SLIECE_TYPE_SI  = 4,
    H264_SLIECE_TYPE_P2  = 5,
    H264_SLIECE_TYPE_B2  = 6,
    H264_SLIECE_TYPE_I2  = 7,
    H264_SLIECE_TYPE_SP2 = 8,
    H264_SLIECE_TYPE_SI2 = 9,
}H264_SLIECE_TYPE;


//------------------------------------------------
//Table 7-9 – Memory management control operation (memory_management_control_operation) values
typedef enum _H264_PICTURE_MARKED_AS_
{
    H264_PICTURE_MARKED_AS_unkown = 0,
    H264_PICTURE_MARKED_AS_used_for_reference = 1,
    H264_PICTURE_MARKED_AS_used_for_short_term_reference = 2,
    H264_PICTURE_MARKED_AS_used_for_long_term_reference = 3,
    H264_PICTURE_MARKED_AS_non_existing = 4,
    H264_PICTURE_MARKED_AS_unused_for_reference = 5,
    H264_PICTURE_MARKED_AS_output_display = 6,
}H264_PICTURE_MARKED_AS;


//------------------------------------------------
// Table 7-11 – Macroblock types for I slices
typedef enum _H264_MB_TYPE_
{
    MB_TYPE_NA,

    //Macroblock types for I slices
    I_NxN, //            0
    I_16x16_0_0_0, //    1
    I_16x16_1_0_0, //    2
    I_16x16_2_0_0, //    3
    I_16x16_3_0_0, //    4
    I_16x16_0_1_0, //    5
    I_16x16_1_1_0, //    6
    I_16x16_2_1_0, //    7
    I_16x16_3_1_0, //    8
    I_16x16_0_2_0, //    9
    I_16x16_1_2_0, //    10
    I_16x16_2_2_0, //    11
    I_16x16_3_2_0, //    12
    I_16x16_0_0_1, //    13
    I_16x16_1_0_1, //    14
    I_16x16_2_0_1, //    15
    I_16x16_3_0_1, //    16
    I_16x16_0_1_1, //    17
    I_16x16_1_1_1, //    18
    I_16x16_2_1_1, //    19
    I_16x16_3_1_1, //    20
    I_16x16_0_2_1, //    21
    I_16x16_1_2_1, //    22
    I_16x16_2_2_1, //    23
    I_16x16_3_2_1, //    24
    I_PCM, //            25

    //Macroblock type with value 0 for SI slices
    SI, //              0

    //Macroblock type values 0 to 4 for P and SP slices
    P_L0_16x16, //      0
    P_L0_L0_16x8, //    1
    P_L0_L0_8x16, //    2
    P_8x8, //           3
    P_8x8ref0, //       4
    P_Skip, //         -1

    //Macroblock type values 0 to 22 for B slices
    B_Direct_16x16, //  0
    B_L0_16x16, //      1
    B_L1_16x16, //      2
    B_Bi_16x16, //      3
    B_L0_L0_16x8, //    4
    B_L0_L0_8x16, //    5
    B_L1_L1_16x8, //    6
    B_L1_L1_8x16, //    7
    B_L0_L1_16x8, //    8
    B_L0_L1_8x16, //    9
    B_L1_L0_16x8, //    10
    B_L1_L0_8x16, //    11
    B_L0_Bi_16x8, //    12
    B_L0_Bi_8x16, //    13
    B_L1_Bi_16x8, //    14
    B_L1_Bi_8x16, //    15
    B_Bi_L0_16x8, //    16
    B_Bi_L0_8x16, //    17
    B_Bi_L1_16x8, //    18
    B_Bi_L1_8x16, //    19
    B_Bi_Bi_16x8, //    20
    B_Bi_Bi_8x16, //    21
    B_8x8, //           22
    B_Skip, //          -1

    //Sub-macroblock types in P macroblocks
    P_L0_8x8, //    0
    P_L0_8x4, //    1
    P_L0_4x8, //    2
    P_L0_4x4, //    3

    //Sub-macroblock types in B macroblocks
    B_Direct_8x8, //    0
    B_L0_8x8, //    1
    B_L1_8x8, //    2
    B_Bi_8x8, //    3
    B_L0_8x4, //    4
    B_L0_4x8, //    5
    B_L1_8x4, //    6
    B_L1_4x8, //    7
    B_Bi_8x4, //    8
    B_Bi_4x8, //    9
    B_L0_4x4, //    10
    B_L1_4x4, //    11
    B_Bi_4x4, //    12
}H264_MB_TYPE;


#define    IS_INTRA_Prediction_Mode(v)    (v != MB_PRED_MODE_NA && (v == Intra_4x4 || v == Intra_8x8 || v == Intra_16x16))
#define    IS_INTER_Prediction_Mode(v)    (v != MB_PRED_MODE_NA && (v == Pred_L0 || v == Pred_L1 || v == BiPred))


typedef enum _H264_MB_PART_PRED_MODE_
{
    MB_PRED_MODE_NA,

    Intra_NA, //        -1
    Intra_4x4, //        0
    Intra_8x8, //        1
    Intra_16x16, //      2
    Inter, //            3

    Pred_NA, //   -1
    Pred_L0, //    0
    Pred_L1, //    1
    BiPred, //     2
    Direct, //     3
}H264_MB_PART_PRED_MODE;


//--------------------------
//Table 8-2 – Specification of Intra4x4PredMode[ luma4x4BlkIdx ] and associated names
#define    Prediction_Mode_Intra_4x4_Vertical               0
#define    Prediction_Mode_Intra_4x4_Horizontal             1
#define    Prediction_Mode_Intra_4x4_DC                     2
#define    Prediction_Mode_Intra_4x4_Diagonal_Down_Left     3
#define    Prediction_Mode_Intra_4x4_Diagonal_Down_Right    4
#define    Prediction_Mode_Intra_4x4_Vertical_Right         5
#define    Prediction_Mode_Intra_4x4_Horizontal_Down        6
#define    Prediction_Mode_Intra_4x4_Vertical_Left          7
#define    Prediction_Mode_Intra_4x4_Horizontal_Up          8

//--------------------------
//Table 8-3 – Specification of Intra8x8PredMode[ luma8x8BlkIdx ] and associated names
#define    Prediction_Mode_Intra_8x8_Vertical               0
#define    Prediction_Mode_Intra_8x8_Horizontal             1
#define    Prediction_Mode_Intra_8x8_DC                     2
#define    Prediction_Mode_Intra_8x8_Diagonal_Down_Left     3
#define    Prediction_Mode_Intra_8x8_Diagonal_Down_Right    4
#define    Prediction_Mode_Intra_8x8_Vertical_Right         5
#define    Prediction_Mode_Intra_8x8_Horizontal_Down        6
#define    Prediction_Mode_Intra_8x8_Vertical_Left          7
#define    Prediction_Mode_Intra_8x8_Horizontal_Up          8

//--------------------------
//Table 8-4 – Specification of Intra16x16PredMode and associated names
#define    Prediction_Mode_Intra_16x16_Vertical             0
#define    Prediction_Mode_Intra_16x16_Horizontal           1
#define    Prediction_Mode_Intra_16x16_DC                   2
#define    Prediction_Mode_Intra_16x16_Plane                3

//--------------------------
//Table 8-5 – Specification of Intra chroma prediction modes and associated names
#define    Prediction_Mode_Intra_Chroma_DC                  0
#define    Prediction_Mode_Intra_Chroma_Horizontal          1
#define    Prediction_Mode_Intra_Chroma_Vertical            2
#define    Prediction_Mode_Intra_Chroma_Plane               3

//----------------------------------------
#define    H264_SLIECE_TYPE_TO_STR(slice_type)                \
    (slice_type == H264_SLIECE_TYPE_P)   ? "P"   :            \
    (slice_type == H264_SLIECE_TYPE_B)   ? "B"   :            \
    (slice_type == H264_SLIECE_TYPE_I)   ? "I"   :            \
    (slice_type == H264_SLIECE_TYPE_SP)  ? "SP"  :            \
    (slice_type == H264_SLIECE_TYPE_SI)  ? "SI"  :            \
    (slice_type == H264_SLIECE_TYPE_P2)  ? "P2"  :            \
    (slice_type == H264_SLIECE_TYPE_B2)  ? "B2"  :            \
    (slice_type == H264_SLIECE_TYPE_I2)  ? "I2"  :            \
    (slice_type == H264_SLIECE_TYPE_SP2) ? "SP2" :            \
    (slice_type == H264_SLIECE_TYPE_SI2) ? "SI2" : "UNKNOWN"

#define    H264_MB_PART_PRED_MODE_TO_STR(pred_mode)             \
    (pred_mode == MB_PRED_MODE_NA)   ? "MB_PRED_MODE_NA"   :    \
    (pred_mode == Intra_NA)          ? "Intra_NA"    :          \
    (pred_mode == Intra_4x4)         ? "Intra_4x4"   :          \
    (pred_mode == Intra_8x8)         ? "Intra_8x8"   :          \
    (pred_mode == Intra_16x16)       ? "Intra_16x16" :          \
    (pred_mode == Pred_NA)           ? "Pred_NA"   :            \
    (pred_mode == Pred_L0)           ? "Pred_L0"   :            \
    (pred_mode == Pred_L1)           ? "Pred_L1"   :            \
    (pred_mode == BiPred)            ? "BiPred"    :            \
    (pred_mode == BiPred)            ? "BiPred"    :            \
    (pred_mode == Direct)            ? "Direct" : "UNKNOWN"


//syntax element type
//H264语法元素类型，主要用于CABAC算术解码
//Table 9-11 – Association of ctxIdx and syntax elements for each slice type in the initialisation process
typedef enum _H264_SYNTAX_ELEMENT_TYPE_
{
    H264_SYNTAX_ELEMENT_TYPE_UNKNOWN,

    //slice_data( )
    H264_SYNTAX_ELEMENT_TYPE_mb_skip_flag,
    H264_SYNTAX_ELEMENT_TYPE_mb_field_decoding_flag,

    //macroblock_layer( )
    H264_SYNTAX_ELEMENT_TYPE_mb_type,
    H264_SYNTAX_ELEMENT_TYPE_transform_size_8x8_flag,
    H264_SYNTAX_ELEMENT_TYPE_coded_block_pattern,
    H264_SYNTAX_ELEMENT_TYPE_coded_block_pattern_luma,
    H264_SYNTAX_ELEMENT_TYPE_coded_block_pattern_chroma,
    H264_SYNTAX_ELEMENT_TYPE_mb_qp_delta,

    //mb_pred( )
    H264_SYNTAX_ELEMENT_TYPE_prev_intra4x4_pred_mode_flag,
    H264_SYNTAX_ELEMENT_TYPE_rem_intra4x4_pred_mode,
    H264_SYNTAX_ELEMENT_TYPE_prev_intra8x8_pred_mode_flag,
    H264_SYNTAX_ELEMENT_TYPE_rem_intra8x8_pred_mode,
    H264_SYNTAX_ELEMENT_TYPE_intra_chroma_pred_mode,

    //mb_pred( ) and sub_mb_pred( )
    H264_SYNTAX_ELEMENT_TYPE_ref_idx_l0,
    H264_SYNTAX_ELEMENT_TYPE_ref_idx_l1,
    H264_SYNTAX_ELEMENT_TYPE_mvd_l0_arr_arr_0,
    H264_SYNTAX_ELEMENT_TYPE_mvd_l1_arr_arr_0,
    H264_SYNTAX_ELEMENT_TYPE_mvd_l0_arr_arr_1,
    H264_SYNTAX_ELEMENT_TYPE_mvd_l1_arr_arr_1,
    H264_SYNTAX_ELEMENT_TYPE_ref_idx_lX,
    H264_SYNTAX_ELEMENT_TYPE_mvd_lX,

    //sub_mb_pred( )
    H264_SYNTAX_ELEMENT_TYPE_sub_mb_type_arr,

    //residual_block_cabac( )
    H264_SYNTAX_ELEMENT_TYPE_coded_block_flag,
    H264_SYNTAX_ELEMENT_TYPE_significant_coeff_flag,
    H264_SYNTAX_ELEMENT_TYPE_last_significant_coeff_flag,
    H264_SYNTAX_ELEMENT_TYPE_coeff_abs_level_minus1,
    H264_SYNTAX_ELEMENT_TYPE_significant_coeff_flag_arr,
    H264_SYNTAX_ELEMENT_TYPE_last_significant_coeff_flag_arr,
    H264_SYNTAX_ELEMENT_TYPE_coeff_abs_level_minus1_arr,
    H264_SYNTAX_ELEMENT_TYPE_coeff_sign_flag,
    H264_SYNTAX_ELEMENT_TYPE_end_of_slice_flag,
}H264_SYNTAX_ELEMENT_TYPE;


//----------Bitmap-------------------
typedef struct
{
//    unsigned short    bfType;
    unsigned int      bfSize;
    unsigned short    bfReserved1;
    unsigned short    bfReserved2;
    unsigned int      bfOffBits;
}MY_BitmapFileHeader;


typedef struct
{
    unsigned int     biSize;
    int              biWidth;
    int              biHeight;
    unsigned short   biPlanes;
    unsigned short   biBitCount;
    unsigned int     biCompression;
    unsigned int     biSizeImage;
    int              biXPelsPerMeter;
    int              biYPelsPerMeter;
    unsigned int     biClrUsed;
    unsigned int     biClrImportant;
}MY_BitmapInfoHeader;


typedef struct
{
    unsigned char rgbBlue; //该颜色的蓝色分量
    unsigned char rgbGreen; //该颜色的绿色分量
    unsigned char rgbRed; //该颜色的红色分量
    unsigned char rgbReserved; //保留值
}MY_RgbQuad;


typedef struct _MY_BITMAP_
{
    long                  bmType;
    long                  bmWidth;
    long                  bmHeight;
    long                  bmWidthBytes;
    unsigned short        bmPlanes;
    unsigned short        bmBitsPixel;
    void *                bmBits;
}MY_BITMAP;


//-----------------------------
int h264_log2(int32_t value);
int32_t h264_power2(int32_t value);
int scaling_list(CBitstream &bs, int32_t *scalingList, int sizeOfScalingList, int32_t &useDefaultScalingMatrixFlag);
int more_rbsp_data(CBitstream &bs);
int more_rbsp_trailing_data(CBitstream &bs);
int byte_aligned(CBitstream &bs);
int rbsp_trailing_bits(CBitstream &bs);
int access_unit_delimiter_rbsp(CBitstream &bs);
int end_of_seq_rbsp(CBitstream &bs);
int end_of_stream_rbsp(CBitstream &bs);
int filler_data_rbsp(CBitstream &bs);
int rbsp_slice_trailing_bits(CBitstream &bs, int32_t entropy_coding_mode_flag);

int InverseRasterScan(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e);


//-----------错误码-----------------
typedef enum _H264_DECODE_ERROR_CODE_
{
    H264_DECODE_ERROR_CODE_UNKNOWN = 0, //未知错误
    H264_DECODE_ERROR_CODE_FILE_END, //到达文件末尾，视频解码完毕
    H264_DECODE_ERROR_CODE_OPEN_FILE_FAILED, //打开文件失败
    H264_DECODE_ERROR_CODE_NEED_MORE_DATA, //需要更多的码流数据
    H264_DECODE_ERROR_CODE_NO_MORE_FRAMES, //没有更多的帧了
}H264_DECODE_ERROR_CODE;


#endif //__H264_COMMON_FUNC_H__
