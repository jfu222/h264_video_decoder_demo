//
// H264MacroBlock.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __H264_MACRO_BLOCK_H__
#define __H264_MACRO_BLOCK_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Bitstream.h"
#include "H264SliceHeader.h"
#include "H264SliceData.h"
#include "H264CommonFunc.h"
#include "H264Cabac.h"


class CH264PictureBase;


//--------------------------
//Table 7-11 – Macroblock types for I slices
//Name of mb_type    transform_size_8x8_flag    MbPartPredMode(mb_type, 0)    Intra16x16PredMode    CodedBlockPatternChroma    CodedBlockPatternLuma
typedef struct _MB_TYPE_I_SLICES_T_
{
    int32_t                   mb_type;
    H264_MB_TYPE              name_of_mb_type;
    int32_t                   transform_size_8x8_flag;
    H264_MB_PART_PRED_MODE    MbPartPredMode;
    int32_t                   Intra16x16PredMode;
    int32_t                   CodedBlockPatternChroma;
    int32_t                   CodedBlockPatternLuma;
}MB_TYPE_I_SLICES_T;

//Table 7-12 – Macroblock type with value 0 for SI slices
//mb_type    Name of mb_type     MbPartPredMode(mb_type, 0)    Intra16x16PredMode    CodedBlockPatternChroma    CodedBlockPatternLuma
typedef struct _MB_TYPE_SI_SLICES_T_
{
    int32_t                   mb_type;
    H264_MB_TYPE              name_of_mb_type;
    H264_MB_PART_PRED_MODE    MbPartPredMode;
    int32_t                   Intra16x16PredMode;
    int32_t                   CodedBlockPatternChroma;
    int32_t                   CodedBlockPatternLuma;
}MB_TYPE_SI_SLICES_T;

//Table 7-13 – Macroblock type values 0 to 4 for P and SP slices
//mb_type    Name of mb_type    NumMbPart(mb_type)    MbPartPredMode(mb_type, 0)    MbPartPredMode(mb_type, 1)    MbPartWidth(mb_type)    MbPartHeight(mb_type)
typedef struct _MB_TYPE_P_SP_SLICES_T_
{
    int32_t                   mb_type;
    H264_MB_TYPE              name_of_mb_type;
    int32_t                   NumMbPart;
    H264_MB_PART_PRED_MODE    MbPartPredMode0;
    H264_MB_PART_PRED_MODE    MbPartPredMode1;
    int32_t                   MbPartWidth;
    int32_t                   MbPartHeight;
}MB_TYPE_P_SP_SLICES_T;

//Table 7-14 – Macroblock type values 0 to 22 for B slices
//mb_type    Name of mb_type    NumMbPart(mb_type)    MbPartPredMode(mb_type, 0)    MbPartPredMode(mb_type, 1)    MbPartWidth(mb_type)    MbPartHeight(mb_type)
typedef struct _MB_TYPE_B_SLICES_T_
{
    int32_t                   mb_type;
    H264_MB_TYPE              name_of_mb_type;
    int32_t                   NumMbPart;
    H264_MB_PART_PRED_MODE    MbPartPredMode0;
    H264_MB_PART_PRED_MODE    MbPartPredMode1;
    int32_t                   MbPartWidth;
    int32_t                   MbPartHeight;
}MB_TYPE_B_SLICES_T;

//--------------------------------------
//Table 7-17 – Sub-macroblock types in P macroblocks
//sub_mb_type[mbPartIdx]    Name of sub_mb_type[mbPartIdx]    NumSubMbPart(sub_mb_type[mbPartIdx])    SubMbPredMode(sub_mb_type[mbPartIdx])    SubMbPartWidth(sub_mb_type[ mbPartIdx])    SubMbPartHeight(sub_mb_type[mbPartIdx])
typedef struct _SUB_MB_TYPE_P_MBS_T_
{
    int32_t                   sub_mb_type;
    H264_MB_TYPE              name_of_sub_mb_type;
    int32_t                   NumSubMbPart;
    H264_MB_PART_PRED_MODE    SubMbPredMode;
    int32_t                   SubMbPartWidth;
    int32_t                   SubMbPartHeight;
}SUB_MB_TYPE_P_MBS_T;

//Table 7-18 – Sub-macroblock types in B macroblocks
//sub_mb_type[mbPartIdx]    Name of sub_mb_type[mbPartIdx]    NumSubMbPart(sub_mb_type[mbPartIdx])    SubMbPredMode(sub_mb_type[mbPartIdx])    SubMbPartWidth(sub_mb_type[mbPartIdx])    SubMbPartHeight(sub_mb_type[mbPartIdx])
typedef struct _SUB_MB_TYPE_B_MBS_T_
{
    int32_t                   sub_mb_type;
    H264_MB_TYPE              name_of_sub_mb_type;
    int32_t                   NumSubMbPart;
    H264_MB_PART_PRED_MODE    SubMbPredMode;
    int32_t                   SubMbPartWidth;
    int32_t                   SubMbPartHeight;
}SUB_MB_TYPE_B_MBS_T;


//-----------------------------------------
/*
 * T-REC-H.264-201704-S!!PDF-E.pdf
 * Page 56/78/812
 * 7.3.5 Macroblock layer syntax
 */
class CH264MacroBlock
{
public:
    int32_t     mb_type; //2 ue(v) | ae(v)
    int32_t     pcm_alignment_zero_bit; // 3 f(1)
    int32_t     pcm_sample_luma[256]; //3 u(v)
    int32_t     pcm_sample_chroma[256]; //3 u(v)
    int32_t     transform_size_8x8_flag; //2 u(1) | ae(v)
    int32_t     coded_block_pattern; //2 me(v) | ae(v)
    int32_t     mb_qp_delta; //2 se(v) | ae(v)
    
    //mb_pred
    int32_t     prev_intra4x4_pred_mode_flag[16];
    int32_t     rem_intra4x4_pred_mode[16];
    int32_t     prev_intra8x8_pred_mode_flag[4];
    int32_t     rem_intra8x8_pred_mode[4];
    int32_t     intra_chroma_pred_mode; //2 se(v) | ae(v)
    int32_t     ref_idx_l0[4];
    int32_t     ref_idx_l1[4];
    int32_t     mvd_l0[4][4][2];
    int32_t     mvd_l1[4][4][2];

    //sub_mb_pred
    int32_t     sub_mb_type[4];
    
    int32_t     CodedBlockPatternChroma; //CodedBlockPatternLuma = coded_block_pattern % 16; CodedBlockPatternChroma = coded_block_pattern / 16;
    int32_t     CodedBlockPatternLuma; //specifies which of the four 8x8 luma blocks and associated chroma blocks of a macroblock may contain non-zero transform coefficient levels.
    int32_t     QPY;//QPY = ( ( QPY_prev + mb_qp_delta + 52 + 2 * QpBdOffsetY ) % ( 52 + QpBdOffsetY ) ) − QpBdOffsetY (7-37)
    int32_t     QSY; //slice_header.QSY
    int32_t     QP1Y; //QP′Y = QPY + QpBdOffsetY
    int32_t     QPCb;
    int32_t     QP1Cb; //QP′C
    int32_t     QPCr;
    int32_t     QP1Cr; //QP′C
    int32_t     QSCb; //When the current slice is an SP or SI slice, QSC is derived using the above process, substituting QPY with QSY and QPC with QSC.
    int32_t     QS1Cb; //QS′C
    int32_t     QSCr;
    int32_t     QS1Cr; //QS′C
    int32_t     TransformBypassModeFlag;
    
    int32_t     i16x16DClevel[16]; //DCT变换后的直流系数
    int32_t     i16x16AClevel[16][16]; //DCT变换后的交流系数
    int32_t     level4x4[16][16];
    int32_t     level8x8[4][64];
    
    int32_t     Intra16x16DCLevel[16];
    int32_t     Intra16x16ACLevel[16][16];
    int32_t     LumaLevel4x4[16][16];
    int32_t     LumaLevel8x8[4][64];
    
    int32_t     CbIntra16x16DCLevel[16];
    int32_t     CbIntra16x16ACLevel[16][16];
    int32_t     CbLevel4x4[16][16];
    int32_t     CbLevel8x8[4][64];
    
    int32_t     CrIntra16x16DCLevel[16];
    int32_t     CrIntra16x16ACLevel[16][16];
    int32_t     CrLevel4x4[16][16];
    int32_t     CrLevel8x8[4][64];

    int32_t     ChromaDCLevel[2][16];
    int32_t     ChromaACLevel[2][16][16];

    uint8_t     mb_luma_4x4_non_zero_count_coeff[16]; //存储当前亮度宏块的16个4x4子宏块非零系数，范围[0,16]
    uint8_t     mb_chroma_4x4_non_zero_count_coeff[2][16]; //存储当前两个色度宏块的16个4x4子宏块非零系数，范围[0,16]
    uint8_t     mb_luma_8x8_non_zero_count_coeff[4]; //存储当前亮度宏块的4个8x8子宏块非零系数，范围[0,64]
    uint8_t     Intra4x4PredMode[16]; //存储当前宏块的16个4x4子宏块预测模式的值，范围[0,8]
    uint8_t     Intra8x8PredMode[4]; //存储当前宏块的4个8x8子宏块预测模式的值，范围[0,8]
    int32_t     Intra16x16PredMode; //存储当前宏块的1个16x16宏块预测模式的值，范围[0,4]
    int32_t     field_pic_flag;
    int32_t     bottom_field_flag;
    int32_t     mb_skip_flag;
    int32_t     mb_field_decoding_flag; //0-a frame macroblock pair; 1-a field macroblock pair;
    int32_t     MbaffFrameFlag; //MbaffFrameFlag = ( sps.mb_adaptive_frame_field_flag && !field_pic_flag );
    int32_t     constrained_intra_pred_flag;
    int32_t     disable_deblocking_filter_idc;
    int32_t     CurrMbAddr; //[0,PicSizeInMbs-1]
    int32_t     slice_id;
    int32_t     m_slice_type;
    int32_t     slice_number;
    int32_t     MbPartWidth;
    int32_t     MbPartHeight;
    int32_t     m_NumMbPart;
    int32_t     NumSubMbPart[4];
    int32_t     SubMbPartWidth[4];
    int32_t     SubMbPartHeight[4];
    int32_t     FilterOffsetA;
    int32_t     FilterOffsetB;
    uint8_t     coded_block_flag_DC_pattern; //3个bit位表示CABAC残差相应4x4子宏块中的DC直流系数block的coded_block_flag值，(b7,...,b2,b1,b0)=(x,...,cr,cb,luma)
    uint16_t    coded_block_flag_AC_pattern[3]; //16个bit位表示CABAC残差相应4x4子宏块中的AC交流系数block的coded_block_flag值(0或1)(全部默认为1)，[0]-luma,[1]-cb,[2]-cr

    MB_TYPE_I_SLICES_T       mb_type_I_slice;
    MB_TYPE_SI_SLICES_T      mb_type_SI_slice;
    MB_TYPE_P_SP_SLICES_T    mb_type_P_SP_slice;
    MB_TYPE_B_SLICES_T       mb_type_B_slice;
    SUB_MB_TYPE_P_MBS_T      sub_mb_type_P_slice[4];
    SUB_MB_TYPE_B_MBS_T      sub_mb_type_B_slice[4];
    
    int32_t                  m_slice_type_fixed;
    int32_t                  m_mb_type_fixed; //码流解码出来的原始mb_type值，需要修正一次才行，原因是有的P帧里面含有帧内编码的I宏块
    H264_MB_TYPE             m_name_of_mb_type;
    H264_MB_PART_PRED_MODE   m_mb_pred_mode;
    H264_MB_TYPE             m_name_of_sub_mb_type[4];
    H264_MB_PART_PRED_MODE   m_sub_mb_pred_mode[4];

    int32_t                  m_MvL0[4][4][2];
    int32_t                  m_MvL1[4][4][2];
    int32_t                  m_RefIdxL0[4];
    int32_t                  m_RefIdxL1[4];
    int32_t                  m_PredFlagL0[4];
    int32_t                  m_PredFlagL1[4];
    uint8_t                  m_isDecoded[4][4]; //the partition given by mbPartIdxN and subMbPartIdxN is not yet decoded,
    int32_t                  m_mb_position_x; //本宏块的左上角像素，相对于整张图片左上角像素的x坐标
    int32_t                  m_mb_position_y; //本宏块的左上角像素，相对于整张图片左上角像素的y坐标

public:
    CH264MacroBlock();
    ~CH264MacroBlock();
    
    int printInfo();
    
    static std::string getNameOfMbTypeStr(H264_MB_TYPE name_of_mb_type);

    static int fix_mb_type(const int32_t slice_type_raw, const int32_t mb_type_raw, int32_t &slice_type_fixed, int32_t &mb_type_fixed);

    static int getMbPartWidthAndHeight(H264_MB_TYPE name_of_mb_type, int32_t &_MbPartWidth, int32_t &_MbPartHeight);
    static int MbPartPredMode(int32_t slice_type, int32_t transform_size_8x8_flag, int32_t _mb_type, int32_t index, int32_t &NumMbPart, 
        int32_t &CodedBlockPatternChroma, int32_t &CodedBlockPatternLuma, int32_t &_Intra16x16PredMode, H264_MB_TYPE &name_of_mb_type, H264_MB_PART_PRED_MODE &mb_pred_mode);
    static int MbPartPredMode2(H264_MB_TYPE name_of_mb_type, int32_t mbPartIdx, int32_t transform_size_8x8_flag, H264_MB_PART_PRED_MODE &mb_pred_mode);
    static int SubMbPredModeFunc(int32_t slice_type, int32_t sub_mb_type, int32_t &NumSubMbPart, H264_MB_PART_PRED_MODE &SubMbPredMode, 
                 int32_t &SubMbPartWidth, int32_t &SubMbPartHeight);

    inline int set_mb_type_X_slice_info();

    int macroblock_layer(CBitstream &bs, CH264PictureBase &picture, const CH264SliceData &slice_data, CH264Cabac &cabac);
    int macroblock_layer_mb_skip(CH264PictureBase &picture, const CH264SliceData &slice_data, CH264Cabac &cabac);
    int mb_pred(CBitstream &bs, CH264PictureBase &picture, const CH264SliceData &slice_data, CH264Cabac &cabac);
    int sub_mb_pred(CBitstream &bs, CH264PictureBase &picture, const CH264SliceData &slice_data, CH264Cabac &cabac);
    
    int residual(CBitstream &bs, CH264PictureBase &picture, int32_t startIdx, int32_t endIdx, CH264Cabac &cabac);
    int residual_luma(CBitstream &bs, CH264PictureBase &picture, int32_t (&i16x16DClevel)[16], int32_t (&i16x16AClevel)[16][16], int32_t (&level4x4)[16][16], 
            int32_t (&level8x8)[4][64], int32_t startIdx, int32_t endIdx, MB_RESIDUAL_LEVEL mb_residual_level_dc, MB_RESIDUAL_LEVEL mb_residual_level_ac, CH264Cabac &cabac);
};

#endif //__H264_MACRO_BLOCK_H__
