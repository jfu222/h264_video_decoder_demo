//
// H264MacroBlock.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264MacroBlock.h"
#include "H264Golomb.h"
#include "CommonFunction.h"
#include "H264CommonFunc.h"
#include "H264ResidualBlockCavlc.h"
#include "H264PictureBase.h"


/*
//Table 7-11 – Macroblock types for I slices
//Name of mb_type    transform_size_8x8_flag    MbPartPredMode(mb_type, 0)    Intra16x16PredMode    CodedBlockPatternChroma    CodedBlockPatternLuma
struct MB_TYPE_I_SLICES_T
{
    int32_t                   mb_type;
    H264_MB_TYPE              name_of_mb_type;
    int32_t                   transform_size_8x8_flag;
    H264_MB_PART_PRED_MODE    MbPartPredMode;
    int32_t                   Intra16x16PredMode;
    int32_t                   CodedBlockPatternChroma;
    int32_t                   CodedBlockPatternLuma;
};
*/
MB_TYPE_I_SLICES_T mb_type_I_slices_define[27] = 
{
    { 0,     I_NxN,            0,     Intra_4x4,      NA,  -1,   -1 },
    { 0,     I_NxN,            1,     Intra_8x8,      NA,  -1,   -1 },
    { 1,     I_16x16_0_0_0,    NA,    Intra_16x16,    0,    0,    0 },
    { 2,     I_16x16_1_0_0,    NA,    Intra_16x16,    1,    0,    0 },
    { 3,     I_16x16_2_0_0,    NA,    Intra_16x16,    2,    0,    0 },
    { 4,     I_16x16_3_0_0,    NA,    Intra_16x16,    3,    0,    0 },
    { 5,     I_16x16_0_1_0,    NA,    Intra_16x16,    0,    1,    0 },
    { 6,     I_16x16_1_1_0,    NA,    Intra_16x16,    1,    1,    0 },
    { 7,     I_16x16_2_1_0,    NA,    Intra_16x16,    2,    1,    0 },
    { 8,     I_16x16_3_1_0,    NA,    Intra_16x16,    3,    1,    0 },
    { 9,     I_16x16_0_2_0,    NA,    Intra_16x16,    0,    2,    0 },
    { 10,    I_16x16_1_2_0,    NA,    Intra_16x16,    1,    2,    0 },
    { 11,    I_16x16_2_2_0,    NA,    Intra_16x16,    2,    2,    0 },
    { 12,    I_16x16_3_2_0,    NA,    Intra_16x16,    3,    2,    0 },
    { 13,    I_16x16_0_0_1,    NA,    Intra_16x16,    0,    0,    15 },
    { 14,    I_16x16_1_0_1,    NA,    Intra_16x16,    1,    0,    15 },
    { 15,    I_16x16_2_0_1,    NA,    Intra_16x16,    2,    0,    15 },
    { 16,    I_16x16_3_0_1,    NA,    Intra_16x16,    3,    0,    15 },
    { 17,    I_16x16_0_1_1,    NA,    Intra_16x16,    0,    1,    15 },
    { 18,    I_16x16_1_1_1,    NA,    Intra_16x16,    1,    1,    15 },
    { 19,    I_16x16_2_1_1,    NA,    Intra_16x16,    2,    1,    15 },
    { 20,    I_16x16_3_1_1,    NA,    Intra_16x16,    3,    1,    15 },
    { 21,    I_16x16_0_2_1,    NA,    Intra_16x16,    0,    2,    15 },
    { 22,    I_16x16_1_2_1,    NA,    Intra_16x16,    1,    2,    15 },
    { 23,    I_16x16_2_2_1,    NA,    Intra_16x16,    2,    2,    15 },
    { 24,    I_16x16_3_2_1,    NA,    Intra_16x16,    3,    2,    15 },
    { 25,    I_PCM,            NA,    Intra_NA,       NA,   NA,   NA }
};

/*
//Table 7-12 – Macroblock type with value 0 for SI slices
//mb_type    Name of mb_type     MbPartPredMode(mb_type, 0)    Intra16x16PredMode    CodedBlockPatternChroma    CodedBlockPatternLuma
struct MB_TYPE_SI_SLICES_T
{
    int32_t                   mb_type;
    H264_MB_TYPE              name_of_mb_type;
    H264_MB_PART_PRED_MODE    MbPartPredMode;
    int32_t                   Intra16x16PredMode;
    int32_t                   CodedBlockPatternChroma;
    int32_t                   CodedBlockPatternLuma;
};
*/
MB_TYPE_SI_SLICES_T mb_type_SI_slices_define[1] = 
{
    { 0,     SI,     Intra_4x4,    NA,    NA,    NA }
};

/*
//Table 7-13 – Macroblock type values 0 to 4 for P and SP slices
//mb_type    Name of mb_type    NumMbPart(mb_type)    MbPartPredMode(mb_type, 0)    MbPartPredMode(mb_type, 1)    MbPartWidth(mb_type)    MbPartHeight(mb_type)
struct MB_TYPE_P_SP_SLICES_T
{
    int32_t                   mb_type;
    H264_MB_TYPE              name_of_mb_type;
    int32_t                   NumMbPart;
    H264_MB_PART_PRED_MODE    MbPartPredMode0;
    H264_MB_PART_PRED_MODE    MbPartPredMode1;
    int32_t                   MbPartWidth;
    int32_t                   MbPartHeight;
};
*/
MB_TYPE_P_SP_SLICES_T mb_type_P_SP_slices_define[6] = 
{
    { 0,    P_L0_16x16,      1,    Pred_L0,    Pred_NA,    16,    16 },
    { 1,    P_L0_L0_16x8,    2,    Pred_L0,    Pred_L0,    16,    8, },
    { 2,    P_L0_L0_8x16,    2,    Pred_L0,    Pred_L0,    8,     16 },
    { 3,    P_8x8,           4,    Pred_NA,    Pred_NA,    8,     8 },
    { 4,    P_8x8ref0,       4,    Pred_NA,    Pred_NA,    8,     8 },
    { 5,    P_Skip,          1,    Pred_L0,    Pred_NA,    16,    16}
};

/*
//Table 7-14 – Macroblock type values 0 to 22 for B slices
//mb_type    Name of mb_type    NumMbPart(mb_type)    MbPartPredMode(mb_type, 0)    MbPartPredMode(mb_type, 1)    MbPartWidth(mb_type)    MbPartHeight(mb_type)
struct MB_TYPE_B_SLICES_T
{
    int32_t                   mb_type;
    H264_MB_TYPE              name_of_mb_type;
    int32_t                   NumMbPart;
    H264_MB_PART_PRED_MODE    MbPartPredMode0;
    H264_MB_PART_PRED_MODE    MbPartPredMode1;
    int32_t                   MbPartWidth;
    int32_t                   MbPartHeight;
};
*/
MB_TYPE_B_SLICES_T mb_type_B_slices_define[24] = 
{
    { 0,     B_Direct_16x16,   NA,   Direct,     Pred_NA,    8,     8 },
    { 1,     B_L0_16x16,       1,    Pred_L0,    Pred_NA,    16,    16 },
    { 2,     B_L1_16x16,       1,    Pred_L1,    Pred_NA,    16,    16 },
    { 3,     B_Bi_16x16,       1,    BiPred,     Pred_NA,    16,    16 },
    { 4,     B_L0_L0_16x8,     2,    Pred_L0,    Pred_L0,    16,    8 },
    { 5,     B_L0_L0_8x16,     2,    Pred_L0,    Pred_L0,    8,     16 },
    { 6,     B_L1_L1_16x8,     2,    Pred_L1,    Pred_L1,    16,    8 },
    { 7,     B_L1_L1_8x16,     2,    Pred_L1,    Pred_L1,    8,     16 },
    { 8,     B_L0_L1_16x8,     2,    Pred_L0,    Pred_L1,    16,    8 },
    { 9,     B_L0_L1_8x16,     2,    Pred_L0,    Pred_L1,    8,     16 },
    { 10,    B_L1_L0_16x8,     2,    Pred_L1,    Pred_L0,    16,    8 },
    { 11,    B_L1_L0_8x16,     2,    Pred_L1,    Pred_L0,    8,     16 },
    { 12,    B_L0_Bi_16x8,     2,    Pred_L0,    BiPred,     16,    8 },
    { 13,    B_L0_Bi_8x16,     2,    Pred_L0,    BiPred,     8,     16 },
    { 14,    B_L1_Bi_16x8,     2,    Pred_L1,    BiPred,     16,    8 },
    { 15,    B_L1_Bi_8x16,     2,    Pred_L1,    BiPred,     8,     16 },
    { 16,    B_Bi_L0_16x8,     2,    BiPred,     Pred_L0,    16,    8 },
    { 17,    B_Bi_L0_8x16,     2,    BiPred,     Pred_L0,    8,     16 },
    { 18,    B_Bi_L1_16x8,     2,    BiPred,     Pred_L1,    16,    8 },
    { 19,    B_Bi_L1_8x16,     2,    BiPred,     Pred_L1,    8,     16 },
    { 20,    B_Bi_Bi_16x8,     2,    BiPred,     BiPred,     16,    8 },
    { 21,    B_Bi_Bi_8x16,     2,    BiPred,     BiPred,     8,     16 },
    { 22,    B_8x8,            4,    Pred_NA,    Pred_NA,    8,     8 },
    { 23,    B_Skip,           NA,   Direct,     Pred_NA,    8,     8 }
};

/*
//--------------------------------------
//Table 7-17 – Sub-macroblock types in P macroblocks
//sub_mb_type[mbPartIdx]    Name of sub_mb_type[mbPartIdx]    NumSubMbPart(sub_mb_type[mbPartIdx])    SubMbPredMode(sub_mb_type[mbPartIdx])    SubMbPartWidth(sub_mb_type[ mbPartIdx])    SubMbPartHeight(sub_mb_type[mbPartIdx])
struct SUB_MB_TYPE_P_MBS_T
{
    int32_t                   sub_mb_type;
    H264_MB_TYPE              name_of_sub_mb_type;
    int32_t                   NumSubMbPart;
    H264_MB_PART_PRED_MODE    SubMbPredMode;
    int32_t                   SubMbPartWidth;
    int32_t                   SubMbPartHeight;
};
*/
SUB_MB_TYPE_P_MBS_T sub_mb_type_P_mbs_define[4] = 
{
//    { inferred,    NA,         NA,   NA,         NA,   NA },
    { 0,    P_L0_8x8,    1,    Pred_L0,    8,    8 },
    { 1,    P_L0_8x4,    2,    Pred_L0,    8,    4 },
    { 2,    P_L0_4x8,    2,    Pred_L0,    4,    8 },
    { 3,    P_L0_4x4,    4,    Pred_L0,    4,    4 }
};

/*
//Table 7-18 – Sub-macroblock types in B macroblocks
//sub_mb_type[mbPartIdx]    Name of sub_mb_type[mbPartIdx]    NumSubMbPart(sub_mb_type[mbPartIdx])    SubMbPredMode(sub_mb_type[mbPartIdx])    SubMbPartWidth(sub_mb_type[mbPartIdx])    SubMbPartHeight(sub_mb_type[mbPartIdx])
struct SUB_MB_TYPE_B_MBS_T
{
    int32_t                   sub_mb_type;
    H264_MB_TYPE              name_of_sub_mb_type;
    int32_t                   NumSubMbPart;
    H264_MB_PART_PRED_MODE    SubMbPredMode;
    int32_t                   SubMbPartWidth;
    int32_t                   SubMbPartHeight;
};
*/
SUB_MB_TYPE_B_MBS_T sub_mb_type_B_mbs_define[13] = 
{
//    { inferred,    mb_type,    4,    Direct,    4,    4 },
    { 0,     B_Direct_8x8,    4,    Direct,     4,    4 },
    { 1,     B_L0_8x8,        1,    Pred_L0,    8,    8 },
    { 2,     B_L1_8x8,        1,    Pred_L1,    8,    8 },
    { 3,     B_Bi_8x8,        1,    BiPred,     8,    8 },
    { 4,     B_L0_8x4,        2,    Pred_L0,    8,    4 },
    { 5,     B_L0_4x8,        2,    Pred_L0,    4,    8 },
    { 6,     B_L1_8x4,        2,    Pred_L1,    8,    4 },
    { 7,     B_L1_4x8,        2,    Pred_L1,    4,    8 },
    { 8,     B_Bi_8x4,        2,    BiPred,     8,    4 },
    { 9,     B_Bi_4x8,        2,    BiPred,     4,    8 },
    { 10,    B_L0_4x4,        4,    Pred_L0,    4,    4 },
    { 11,    B_L1_4x4,        4,    Pred_L1,    4,    4 },
    { 12,    B_Bi_4x4,        4,    BiPred,     4,    4 }
};


//---------------------------------------
CH264MacroBlock::CH264MacroBlock()
{
    mb_type = 0;
    pcm_alignment_zero_bit = 0;
    memset(pcm_sample_luma, 0, sizeof(int32_t) * 256);
    memset(pcm_sample_chroma, 0, sizeof(int32_t) * 256);
    transform_size_8x8_flag = 0;
    coded_block_pattern = 0;
    mb_qp_delta = 0;

    memset(prev_intra4x4_pred_mode_flag, 0, sizeof(int32_t) * 16);
    memset(rem_intra4x4_pred_mode, 0, sizeof(int32_t) * 16);
    memset(prev_intra8x8_pred_mode_flag, 0, sizeof(int32_t) * 4);
    memset(rem_intra8x8_pred_mode, 0, sizeof(int32_t) * 4);
    intra_chroma_pred_mode = 0;
    memset(ref_idx_l0, 0, sizeof(int32_t) * 4);
    memset(ref_idx_l1, 0, sizeof(int32_t) * 4);
    memset(mvd_l0, 0, sizeof(int32_t) * 4 * 4 * 2);
    memset(mvd_l1, 0, sizeof(int32_t) * 4 * 4 * 2);
    
    memset(sub_mb_type, 0, sizeof(int32_t) * 4);
    
    CodedBlockPatternChroma = -1;
    CodedBlockPatternLuma = -1;
    QPY = 0;
    QP1Y = 0;
    QPCb = 0;
    QP1Cb = 0;
    QPCr = 0;
    QP1Cr = 0;
    QSCb = 0;
    QS1Cb = 0;
    QSCr = 0;
    QS1Cr = 0;
    TransformBypassModeFlag = 0;

    memset(mb_luma_4x4_non_zero_count_coeff, 0, sizeof(uint8_t) * 16);
    memset(mb_chroma_4x4_non_zero_count_coeff, 0, sizeof(uint8_t) * 2 * 16);
    memset(mb_luma_8x8_non_zero_count_coeff, 0, sizeof(uint8_t) * 4);
    memset(Intra4x4PredMode, 0, sizeof(uint8_t) * 16);
    memset(Intra8x8PredMode, 0, sizeof(uint8_t) * 4);
    Intra16x16PredMode = 0;
    field_pic_flag = 0;
    bottom_field_flag = 0;
    mb_skip_flag = 0;
    mb_field_decoding_flag = 0;
    MbaffFrameFlag = 0;
    disable_deblocking_filter_idc = 0;
    constrained_intra_pred_flag = 0;
    CurrMbAddr = 0;
    slice_id = 0;
    slice_number = 0;
    m_slice_type = 0;
    MbPartWidth = 0;
    MbPartHeight = 0;
    m_NumMbPart = 0;
    FilterOffsetA = 0;
    FilterOffsetB = 0;
    coded_block_flag_DC_pattern = 0x07;
    coded_block_flag_AC_pattern[0] = 0xFFFF; //cabac: coded_block_flag-luma
    coded_block_flag_AC_pattern[1] = 0xFFFF; //cabac: coded_block_flag-cb
    coded_block_flag_AC_pattern[2] = 0xFFFF; //cabac: coded_block_flag-cr
    memset(NumSubMbPart, 0, sizeof(uint32_t) * 4);
    memset(SubMbPartWidth, 0, sizeof(uint32_t) * 4);
    memset(SubMbPartHeight, 0, sizeof(uint32_t) * 4);
    memset(m_PredFlagL0, 0, sizeof(uint32_t) * 4);
    memset(m_PredFlagL1, 0, sizeof(uint32_t) * 4);
    
    m_slice_type_fixed = -1;
    m_mb_type_fixed = -1;

    m_name_of_mb_type = MB_TYPE_NA;
    m_mb_pred_mode = MB_PRED_MODE_NA;
    memset(m_name_of_sub_mb_type, 0, sizeof(H264_MB_TYPE) * 4);
    memset(m_sub_mb_pred_mode, 0, sizeof(H264_MB_PART_PRED_MODE) * 4);
    
    memset(m_MvL0, 0, sizeof(int32_t) * 4 * 4 * 2);
    memset(m_MvL1, 0, sizeof(int32_t) * 4 * 4 * 2);

    memset(m_RefIdxL0, 0, sizeof(uint32_t) * 4);
    memset(m_RefIdxL1, 0, sizeof(uint32_t) * 4);
    memset(m_PredFlagL0, 0, sizeof(uint32_t) * 4);
    memset(m_PredFlagL1, 0, sizeof(uint32_t) * 4);
    
    memset(m_isDecoded, 0, sizeof(uint8_t) * 4 * 4);

    m_mb_position_x = -1;
    m_mb_position_y = -1;
}


CH264MacroBlock::~CH264MacroBlock()
{

}


int CH264MacroBlock::printInfo()
{
    printf("---------Macro Block info------------\n");
    printf("mb_type=%d;\n", mb_type);
    printf("pcm_alignment_zero_bit=%d;\n", pcm_alignment_zero_bit);
    printf("transform_size_8x8_flag=%d;\n", transform_size_8x8_flag);
    printf("coded_block_pattern=%d;\n", coded_block_pattern);
    printf("mb_qp_delta=%d;\n", mb_qp_delta);
    printf("intra_chroma_pred_mode=%d;\n", intra_chroma_pred_mode);
    printf("CodedBlockPatternChroma=%d;\n", CodedBlockPatternChroma);
    printf("CodedBlockPatternLuma=%d;\n", CodedBlockPatternLuma);
    printf("QPY=%d;\n", QPY);
    printf("TransformBypassModeFlag=%d;\n", TransformBypassModeFlag);

    return 0;
}


std::string CH264MacroBlock::getNameOfMbTypeStr(H264_MB_TYPE name_of_mb_type)
{
    switch (name_of_mb_type)
    {
    case MB_TYPE_NA: { return "MB_TYPE_NA"; break; }
    case I_NxN: { return "I_NxN"; break; }
    case I_16x16_0_0_0: { return "I_16x16_0_0_0"; break; }
    case I_16x16_1_0_0: { return "I_16x16_1_0_0"; break; }
    case I_16x16_2_0_0: { return "I_16x16_2_0_0"; break; }
    case I_16x16_3_0_0: { return "I_16x16_3_0_0"; break; }
    case I_16x16_0_1_0: { return "I_16x16_0_1_0"; break; }
    case I_16x16_1_1_0: { return "I_16x16_1_1_0"; break; }
    case I_16x16_2_1_0: { return "I_16x16_2_1_0"; break; }
    case I_16x16_3_1_0: { return "I_16x16_3_1_0"; break; }
    case I_16x16_0_2_0: { return "I_16x16_0_2_0"; break; }
    case I_16x16_1_2_0: { return "I_16x16_1_2_0"; break; }
    case I_16x16_2_2_0: { return "I_16x16_2_2_0"; break; }
    case I_16x16_3_2_0: { return "I_16x16_3_2_0"; break; }
    case I_16x16_0_0_1: { return "I_16x16_0_0_1"; break; }
    case I_16x16_1_0_1: { return "I_16x16_1_0_1"; break; }
    case I_16x16_2_0_1: { return "I_16x16_2_0_1"; break; }
    case I_16x16_3_0_1: { return "I_16x16_3_0_1"; break; }
    case I_16x16_0_1_1: { return "I_16x16_0_1_1"; break; }
    case I_16x16_1_1_1: { return "I_16x16_1_1_1"; break; }
    case I_16x16_2_1_1: { return "I_16x16_2_1_1"; break; }
    case I_16x16_3_1_1: { return "I_16x16_3_1_1"; break; }
    case I_16x16_0_2_1: { return "I_16x16_0_2_1"; break; }
    case I_16x16_1_2_1: { return "I_16x16_1_2_1"; break; }
    case I_16x16_2_2_1: { return "I_16x16_2_2_1"; break; }
    case I_16x16_3_2_1: { return "I_16x16_3_2_1"; break; }
    case I_PCM: { return "I_PCM"; break; }
    case SI: { return "SI"; break; }
    case P_L0_16x16: { return "P_L0_16x16"; break; }
    case P_L0_L0_16x8: { return "P_L0_L0_16x8"; break; }
    case P_L0_L0_8x16: { return "P_L0_L0_8x16"; break; }
    case P_8x8: { return "P_8x8"; break; }
    case P_8x8ref0: { return "P_8x8ref0"; break; }
    case P_Skip: { return "P_Skip"; break; }
    case B_Direct_16x16: { return "B_Direct_16x16"; break; }
    case B_L0_16x16: { return "B_L0_16x16"; break; }
    case B_L1_16x16: { return "B_L1_16x16"; break; }
    case B_Bi_16x16: { return "B_Bi_16x16"; break; }
    case B_L0_L0_16x8: { return "B_L0_L0_16x8"; break; }
    case B_L0_L0_8x16: { return "B_L0_L0_8x16"; break; }
    case B_L1_L1_16x8: { return "B_L1_L1_16x8"; break; }
    case B_L1_L1_8x16: { return "B_L1_L1_8x16"; break; }
    case B_L0_L1_16x8: { return "B_L0_L1_16x8"; break; }
    case B_L0_L1_8x16: { return "B_L0_L1_8x16"; break; }
    case B_L1_L0_16x8: { return "B_L1_L0_16x8"; break; }
    case B_L1_L0_8x16: { return "B_L1_L0_8x16"; break; }
    case B_L0_Bi_16x8: { return "B_L0_Bi_16x8"; break; }
    case B_L0_Bi_8x16: { return "B_L0_Bi_8x16"; break; }
    case B_L1_Bi_16x8: { return "B_L1_Bi_16x8"; break; }
    case B_L1_Bi_8x16: { return "B_L1_Bi_8x16"; break; }
    case B_Bi_L0_16x8: { return "B_Bi_L0_16x8"; break; }
    case B_Bi_L0_8x16: { return "B_Bi_L0_8x16"; break; }
    case B_Bi_L1_16x8: { return "B_Bi_L1_16x8"; break; }
    case B_Bi_L1_8x16: { return "B_Bi_L1_8x16"; break; }
    case B_Bi_Bi_16x8: { return "B_Bi_Bi_16x8"; break; }
    case B_Bi_Bi_8x16: { return "B_Bi_Bi_8x16"; break; }
    case B_8x8: { return "B_8x8"; break; }
    case B_Skip: { return "B_Skip"; break; }
    case P_L0_8x8: { return "P_L0_8x8"; break; }
    case P_L0_8x4: { return "P_L0_8x4"; break; }
    case P_L0_4x8: { return "P_L0_4x8"; break; }
    case P_L0_4x4: { return "P_L0_4x4"; break; }
    case B_Direct_8x8: { return "B_Direct_8x8"; break; }
    case B_L0_8x8: { return "B_L0_8x8"; break; }
    case B_L1_8x8: { return "B_L1_8x8"; break; }
    case B_Bi_8x8: { return "B_Bi_8x8"; break; }
    case B_L0_8x4: { return "B_L0_8x4"; break; }
    case B_L0_4x8: { return "B_L0_4x8"; break; }
    case B_L1_8x4: { return "B_L1_8x4"; break; }
    case B_L1_4x8: { return "B_L1_4x8"; break; }
    case B_Bi_8x4: { return "B_Bi_8x4"; break; }
    case B_Bi_4x8: { return "B_Bi_4x8"; break; }
    case B_L0_4x4: { return "B_L0_4x4"; break; }
    case B_L1_4x4: { return "B_L1_4x4"; break; }
    case B_Bi_4x4: { return "B_Bi_4x4"; break; }
    default: { return "MB_TYPE_NA"; break; }
    }

    return "MB_TYPE_NA";
}


int CH264MacroBlock::fix_mb_type(const int32_t slice_type_raw, const int32_t mb_type_raw, int32_t &slice_type_fixed, int32_t &mb_type_fixed)
{
    int ret = 0;
    
    slice_type_fixed = slice_type_raw;
    mb_type_fixed = mb_type_raw;

    if ((slice_type_raw % 5) == H264_SLIECE_TYPE_I)
    {
        //不需要修正
    }
    else if ((slice_type_raw % 5) == H264_SLIECE_TYPE_SI)
    {
        //The macroblock types for SI slices are specified in Tables 7-12 and 7-11. The mb_type value 0 is specified 
        //in Table 7-12 and the mb_type values 1 to 26 are specified in Table 7-11, indexed by subtracting 1 from the value of mb_type.
        if (mb_type_raw == 0)
        {
            //不需要修正
        }
        else if (mb_type_raw >= 1 && mb_type_raw <= 26)
        {
            slice_type_fixed = H264_SLIECE_TYPE_I;
            mb_type_fixed = mb_type_raw - 1; //说明 SI slices 中含有I宏块
        }
        else
        {
            LOG_ERROR("SI slices: mb_type_raw=%d; Must be in [0..26]\n", mb_type_raw);
            return -1;
        }
    }
    else if ((slice_type_raw % 5) == H264_SLIECE_TYPE_P || (slice_type_raw % 5) == H264_SLIECE_TYPE_SP)
    {
        //The macroblock types for P and SP slices are specified in Tables 7-13 and 7-11. mb_type values 0 to 4 are specified 
        //in Table 7-13 and mb_type values 5 to 30 are specified in Table 7-11, indexed by subtracting 5 from the value of mb_type.
        if (mb_type_raw >= 0 && mb_type_raw <= 4)
        {
            //不需要修正
        }
        else if (mb_type_raw >= 5 && mb_type_raw <= 30)
        {
            slice_type_fixed = H264_SLIECE_TYPE_I;
            mb_type_fixed = mb_type_raw - 5; //说明 P and SP slices 中含有I宏块
        }
        else
        {
            LOG_ERROR("P and SP slices: mb_type_raw=%d; Must be in [0..30]\n", mb_type_raw);
            return -1;
        }
    }
    else if ((slice_type_raw % 5) == H264_SLIECE_TYPE_B)
    {
        //The macroblock types for B slices are specified in Tables 7-14 and 7-11. The mb_type values 0 to 22 are specified 
        //in Table 7-14 and the mb_type values 23 to 48 are specified in Table 7-11, indexed by subtracting 23 from the value of mb_type.
        if (mb_type_raw >= 0 && mb_type_raw <= 22)
        {
            //不需要修正
        }
        else if (mb_type_raw >= 23 && mb_type_raw <= 48)
        {
            slice_type_fixed = H264_SLIECE_TYPE_I;
            mb_type_fixed = mb_type_raw - 23; //说明 B slices 中含有I宏块
        }
        else
        {
            LOG_ERROR("B slices: mb_type_raw=%d; Must be in [0..48]\n", mb_type_raw);
            return -1;
        }
    }

    return 0;
}


int CH264MacroBlock::getMbPartWidthAndHeight(H264_MB_TYPE name_of_mb_type, int32_t &_MbPartWidth, int32_t &_MbPartHeight)
{
    int ret = 0;
    
    if (name_of_mb_type >= P_L0_16x16 && name_of_mb_type <= P_Skip)
    {
        _MbPartWidth = mb_type_P_SP_slices_define[name_of_mb_type - P_L0_16x16].MbPartWidth;
        _MbPartHeight = mb_type_P_SP_slices_define[name_of_mb_type - P_L0_16x16].MbPartHeight;
    }
    else if (name_of_mb_type >= B_Direct_16x16 && name_of_mb_type <= B_Skip)
    {
        _MbPartWidth = mb_type_B_slices_define[name_of_mb_type - B_Direct_16x16].MbPartWidth;
        _MbPartHeight = mb_type_B_slices_define[name_of_mb_type - B_Direct_16x16].MbPartHeight;
    }
    else if (name_of_mb_type >= P_L0_8x8 && name_of_mb_type <= P_L0_4x4)
    {
        _MbPartWidth = sub_mb_type_P_mbs_define[name_of_mb_type - P_L0_8x8].SubMbPartWidth;
        _MbPartHeight = sub_mb_type_P_mbs_define[name_of_mb_type - P_L0_8x8].SubMbPartHeight;
    }
    else if (name_of_mb_type >= B_Direct_8x8 && name_of_mb_type <= B_Bi_4x4)
    {
        _MbPartWidth = sub_mb_type_B_mbs_define[name_of_mb_type - B_Direct_8x8].SubMbPartWidth;
        _MbPartHeight = sub_mb_type_B_mbs_define[name_of_mb_type - B_Direct_8x8].SubMbPartHeight;
    }
    else
    {
        ret = -1;
    }

    return ret;
}


int CH264MacroBlock::MbPartPredMode(int32_t slice_type, int32_t transform_size_8x8_flag, int32_t _mb_type, int32_t index, int32_t &NumMbPart, 
        int32_t &CodedBlockPatternChroma, int32_t &CodedBlockPatternLuma, int32_t &_Intra16x16PredMode, H264_MB_TYPE &name_of_mb_type, H264_MB_PART_PRED_MODE &mb_pred_mode)
{
    int ret = 0;
    
    if ((slice_type % 5) == H264_SLIECE_TYPE_I)
    {
        if (_mb_type == 0)
        {
            if (transform_size_8x8_flag == 0)
            {
                name_of_mb_type = mb_type_I_slices_define[0].name_of_mb_type;
                mb_pred_mode = mb_type_I_slices_define[0].MbPartPredMode;
            }
            else //if (transform_size_8x8_flag == 1)
            {
                name_of_mb_type = mb_type_I_slices_define[1].name_of_mb_type;
                mb_pred_mode = mb_type_I_slices_define[1].MbPartPredMode;
            }
        }
        else if (_mb_type >= 1 && _mb_type <= 25)
        {
            name_of_mb_type = mb_type_I_slices_define[_mb_type + 1].name_of_mb_type;
            CodedBlockPatternChroma = mb_type_I_slices_define[_mb_type + 1].CodedBlockPatternChroma;
            CodedBlockPatternLuma = mb_type_I_slices_define[_mb_type + 1].CodedBlockPatternLuma;
            _Intra16x16PredMode = mb_type_I_slices_define[_mb_type + 1].Intra16x16PredMode;
            mb_pred_mode = mb_type_I_slices_define[_mb_type + 1].MbPartPredMode;
        }
        else
        {
            LOG_ERROR("mb_type_I_slices_define: _mb_type=%d; Must be in [0..25]\n", _mb_type);
            return -1;
        }
    }
    else if ((slice_type % 5) == H264_SLIECE_TYPE_SI)
    {
        if (_mb_type == 0)
        {
            name_of_mb_type = mb_type_SI_slices_define[0].name_of_mb_type;
            mb_pred_mode = mb_type_SI_slices_define[0].MbPartPredMode;
        }
        else
        {
            LOG_ERROR("mb_type_SI_slices_define: _mb_type=%d; Must be in [0..0]\n", _mb_type);
            return -1;
        }
    }
    else if ((slice_type % 5) == H264_SLIECE_TYPE_P || (slice_type % 5) == H264_SLIECE_TYPE_SP)
    {
        if (_mb_type >= 0 && _mb_type <= 5)
        {
            name_of_mb_type = mb_type_P_SP_slices_define[_mb_type].name_of_mb_type;
            NumMbPart = mb_type_P_SP_slices_define[_mb_type].NumMbPart;
            if (index == 0)
            {
                mb_pred_mode = mb_type_P_SP_slices_define[_mb_type].MbPartPredMode0;
            }
            else //if (index == 1)
            {
                mb_pred_mode = mb_type_P_SP_slices_define[_mb_type].MbPartPredMode1;
            }
        }
        else
        {
            LOG_ERROR("mb_type_P_SP_slices_define: _mb_type=%d; Must be in [0..5]\n", _mb_type);
            return -1;
        }
    }
    else if ((slice_type % 5) == H264_SLIECE_TYPE_B)
    {
        if (_mb_type >= 0 && _mb_type <= 23)
        {
            name_of_mb_type = mb_type_B_slices_define[_mb_type].name_of_mb_type;
            NumMbPart = mb_type_B_slices_define[_mb_type].NumMbPart;
            if (index == 0)
            {
                mb_pred_mode = mb_type_B_slices_define[_mb_type].MbPartPredMode0;
            }
            else //if (index == 1)
            {
                mb_pred_mode = mb_type_B_slices_define[_mb_type].MbPartPredMode1;
            }
        }
        else
        {
            LOG_ERROR("mb_type_B_slices_define: _mb_type=%d; Must be in [0..23]\n", _mb_type);
            return -1;
        }
    }
    else
    {
        LOG_ERROR("Unknown slice_type=%d;\n", slice_type);
        return -1;
    }

    return ret;
}


int CH264MacroBlock::MbPartPredMode2(H264_MB_TYPE name_of_mb_type, int32_t mbPartIdx, int32_t transform_size_8x8_flag, H264_MB_PART_PRED_MODE &mb_pred_mode)
{
    int ret = 0;

    if (name_of_mb_type == I_NxN)
    {
        if (mbPartIdx == 0)
        {
            if (transform_size_8x8_flag == 0)
            {
                mb_pred_mode = mb_type_I_slices_define[0].MbPartPredMode;
            }
            else //if (transform_size_8x8_flag == 1)
            {
                mb_pred_mode = mb_type_I_slices_define[1].MbPartPredMode;
            }
        }
        else
        {
            LOG_ERROR("mbPartIdx=%d; Must be in [0..0]\n", mbPartIdx);
            return -1;
        }
    }
    else if (name_of_mb_type >= I_16x16_0_0_0 && name_of_mb_type<= I_16x16_3_2_1)
    {
        if (mbPartIdx == 0)
        {
            mb_pred_mode = mb_type_I_slices_define[mbPartIdx - I_16x16_0_0_0].MbPartPredMode;
        }
        else
        {
            LOG_ERROR("mbPartIdx=%d; Must be in [0..0]\n", mbPartIdx);
            return -1;
        }
    }

    else if (name_of_mb_type == SI)
    {
        if (mbPartIdx == 0)
        {
            mb_pred_mode = mb_type_SI_slices_define[0].MbPartPredMode;
        }
        else
        {
            LOG_ERROR("mbPartIdx=%d; Must be in [0..0]\n", mbPartIdx);
            return -1;
        }
    }

    else if (name_of_mb_type == P_L0_16x16 || name_of_mb_type == P_Skip)
    {
        if (mbPartIdx == 0)
        {
            mb_pred_mode = Pred_L0;
        }
        else
        {
            LOG_ERROR("mbPartIdx=%d; Must be in [0..0]\n", mbPartIdx);
            return -1;
        }
    }
    else if (name_of_mb_type >= P_L0_L0_16x8 && name_of_mb_type <= P_L0_L0_8x16)
    {
        if (mbPartIdx == 0)
        {
            mb_pred_mode = mb_type_P_SP_slices_define[name_of_mb_type - P_L0_16x16].MbPartPredMode0;
        }
        else if (mbPartIdx == 1)
        {
            mb_pred_mode = mb_type_P_SP_slices_define[name_of_mb_type - P_L0_16x16].MbPartPredMode1;
        }
        else
        {
            LOG_ERROR("mbPartIdx=%d; Must be in [0..1]\n", mbPartIdx);
            return -1;
        }
    }
    else if (name_of_mb_type >= P_8x8 && name_of_mb_type <= P_8x8ref0)
    {
        if (mbPartIdx >= 0 && mbPartIdx <= 3)
        {
            mb_pred_mode = Pred_L0;
        }
        else
        {
            LOG_ERROR("mbPartIdx=%d; Must be in [0..3]\n", mbPartIdx);
            return -1;
        }
    }
    
    else if (name_of_mb_type == B_L0_16x16)
    {
        if (mbPartIdx == 0)
        {
            mb_pred_mode = Pred_L0;
        }
        else
        {
            LOG_ERROR("mbPartIdx=%d; Must be in [0..0]\n", mbPartIdx);
            return -1;
        }
    }
    else if (name_of_mb_type == B_L1_16x16)
    {
        if (mbPartIdx == 0)
        {
            mb_pred_mode = Pred_L1;
        }
        else
        {
            LOG_ERROR("mbPartIdx=%d; Must be in [0..0]\n", mbPartIdx);
            return -1;
        }
    }
    else if (name_of_mb_type == B_Bi_16x16)
    {
        if (mbPartIdx == 0)
        {
            mb_pred_mode = BiPred;
        }
        else
        {
            LOG_ERROR("mbPartIdx=%d; Must be in [0..0]\n", mbPartIdx);
            return -1;
        }
    }
    else if (name_of_mb_type >= B_Direct_16x16 && name_of_mb_type <= B_Skip)
    {
        if (mbPartIdx == 0)
        {
            mb_pred_mode = mb_type_B_slices_define[name_of_mb_type - B_Direct_16x16].MbPartPredMode0;
        }
        else if (mbPartIdx == 1)
        {
            mb_pred_mode = mb_type_B_slices_define[name_of_mb_type - B_Direct_16x16].MbPartPredMode1;
        }
        else if (mbPartIdx == 2 || mbPartIdx == 3)
        {
            mb_pred_mode = MB_PRED_MODE_NA;
        }
        else
        {
            LOG_ERROR("mbPartIdx=%d; Must be in [0..3]\n", mbPartIdx);
            return -1;
        }
    }
    else
    {
        LOG_ERROR("Invaild value: name_of_mb_type=%d;\n", name_of_mb_type);
        return -1;
    }

    return ret;
}


int CH264MacroBlock::SubMbPredModeFunc(int32_t slice_type, int32_t sub_mb_type, int32_t &NumSubMbPart, 
        H264_MB_PART_PRED_MODE &SubMbPredMode, int32_t &SubMbPartWidth, int32_t &SubMbPartHeight)
{
    if (slice_type == H264_SLIECE_TYPE_I)
    {
        LOG_ERROR("Unknown slice_type=%d;\n", slice_type);
        return -1;
    }
    else if (slice_type == H264_SLIECE_TYPE_P)
    {
        if (sub_mb_type >= 0 && sub_mb_type <= 4)
        {
            NumSubMbPart = sub_mb_type_P_mbs_define[sub_mb_type].NumSubMbPart;
            SubMbPredMode = sub_mb_type_P_mbs_define[sub_mb_type].SubMbPredMode;
            SubMbPartWidth = sub_mb_type_P_mbs_define[sub_mb_type].SubMbPartWidth;
            SubMbPartHeight = sub_mb_type_P_mbs_define[sub_mb_type].SubMbPartHeight;
        }
        else
        {
            LOG_ERROR("sub_mb_type_P_mbs_define: sub_mb_type=%d; Must be in [-1..5]\n", sub_mb_type);
            return -1;
        }
    }
    else if (slice_type == H264_SLIECE_TYPE_B)
    {
        if (sub_mb_type >= 0 && sub_mb_type <= 12)
        {
            NumSubMbPart = sub_mb_type_B_mbs_define[sub_mb_type].NumSubMbPart;
            SubMbPredMode = sub_mb_type_B_mbs_define[sub_mb_type].SubMbPredMode;
            SubMbPartWidth = sub_mb_type_B_mbs_define[sub_mb_type].SubMbPartWidth;
            SubMbPartHeight = sub_mb_type_B_mbs_define[sub_mb_type].SubMbPartHeight;
        }
        else
        {
            LOG_ERROR("sub_mb_type_B_mbs_define: sub_mb_type=%d; Must be in [0..12]\n", sub_mb_type);
            return -1;
        }
    }
    else
    {
        LOG_ERROR("Unknown slice_type=%d;\n", slice_type);
        return -1;
    }

    return 0;
}


int CH264MacroBlock::set_mb_type_X_slice_info()
{
    int32_t mbPartIdx = 0;

    if ((m_slice_type_fixed % 5) == H264_SLIECE_TYPE_I)
    {
        if (m_mb_type_fixed == 0)
        {
            if (transform_size_8x8_flag == 0)
            {
                mb_type_I_slice = mb_type_I_slices_define[0];
            }
            else //if (transform_size_8x8_flag == 1)
            {
                mb_type_I_slice = mb_type_I_slices_define[1];
            }
        }
        else if (m_mb_type_fixed >= 1 && m_mb_type_fixed <= 25)
        {
            mb_type_I_slice = mb_type_I_slices_define[m_mb_type_fixed + 1];
        }
        else
        {
            LOG_ERROR("mb_type_I_slices_define: m_mb_type_fixed=%d; Must be in [0..25]\n", m_mb_type_fixed);
            return -1;
        }
    }
    else if ((m_slice_type_fixed % 5) == H264_SLIECE_TYPE_SI)
    {
        if (m_mb_type_fixed == 0)
        {
            mb_type_SI_slice = mb_type_SI_slices_define[0];
        }
        else
        {
            LOG_ERROR("mb_type_SI_slices_define: m_mb_type_fixed=%d; Must be in [0..0]\n", m_mb_type_fixed);
            return -1;
        }
    }
    else if ((m_slice_type_fixed % 5) == H264_SLIECE_TYPE_P || (m_slice_type_fixed % 5) == H264_SLIECE_TYPE_SP)
    {
        if (m_mb_type_fixed >= 0 && m_mb_type_fixed <= 5)
        {
            mb_type_P_SP_slice = mb_type_P_SP_slices_define[m_mb_type_fixed];
        }
        else
        {
            LOG_ERROR("mb_type_P_SP_slices_define: m_mb_type_fixed=%d; Must be in [0..5]\n", m_mb_type_fixed);
            return -1;
        }

        MbPartWidth = mb_type_P_SP_slice.MbPartWidth;
        MbPartHeight = mb_type_P_SP_slice.MbPartHeight;
        m_NumMbPart = mb_type_P_SP_slice.NumMbPart;

        //---------------------------------------------
        //for (mbPartIdx = 0; mbPartIdx < m_NumMbPart; mbPartIdx++)
        //{
            //sub_mb_type_P_slice[mbPartIdx] = sub_mb_type_P_mbs_define[sub_mb_type[ mbPartIdx ]];
            //NumSubMbPart[mbPartIdx] = sub_mb_type_P_mbs_define[sub_mb_type[ mbPartIdx ]].NumSubMbPart;
            //SubMbPartWidth[mbPartIdx] = sub_mb_type_P_mbs_define[sub_mb_type[ mbPartIdx ]].SubMbPartWidth;
            //SubMbPartHeight[mbPartIdx] = sub_mb_type_P_mbs_define[sub_mb_type[ mbPartIdx ]].SubMbPartHeight;
        //}
    }
    else if ((m_slice_type_fixed % 5) == H264_SLIECE_TYPE_B)
    {
        if (m_mb_type_fixed >= 0 && m_mb_type_fixed <= 23)
        {
            mb_type_B_slice = mb_type_B_slices_define[m_mb_type_fixed];
        }
        else
        {
            LOG_ERROR("mb_type_B_slices_define: m_mb_type_fixed=%d; Must be in [0..23]\n", m_mb_type_fixed);
            return -1;
        }

        MbPartWidth = mb_type_B_slice.MbPartWidth;
        MbPartHeight = mb_type_B_slice.MbPartHeight;
        m_NumMbPart = mb_type_B_slice.NumMbPart;

        //---------------------------------------------
        //for (mbPartIdx = 0; mbPartIdx < m_NumMbPart; mbPartIdx++)
        //{
            //sub_mb_type_B_slice[mbPartIdx] = sub_mb_type_B_mbs_define[sub_mb_type[ mbPartIdx ]];
            //NumSubMbPart[mbPartIdx] = sub_mb_type_B_mbs_define[sub_mb_type[ mbPartIdx ]].NumSubMbPart;
            //SubMbPartWidth[mbPartIdx] = sub_mb_type_B_mbs_define[sub_mb_type[ mbPartIdx ]].SubMbPartWidth;
            //SubMbPartHeight[mbPartIdx] = sub_mb_type_B_mbs_define[sub_mb_type[ mbPartIdx ]].SubMbPartHeight;
        //}
    }
    else
    {
        LOG_ERROR("Unknown mb_type=%d; m_mb_type_fixed=%d;\n", mb_type, m_mb_type_fixed);
        return -1;
    }
    
    return 0;
}


//7.3.5 Macroblock layer syntax
int CH264MacroBlock::macroblock_layer(CBitstream &bs, CH264PictureBase &picture, const CH264SliceData &slice_data, CH264Cabac &cabac)
{
    int ret = 0;
    int32_t i = 0;
    CH264Golomb gb;
    int32_t mbPartIdx = 0;
    int32_t transform_size_8x8_flag_temp = 0;

    CH264SliceHeader & slice_header = picture.m_h264_slice_header;

    field_pic_flag = slice_header.field_pic_flag;
    bottom_field_flag = slice_header.bottom_field_flag;
    mb_skip_flag = slice_data.mb_skip_flag;
    mb_field_decoding_flag = slice_data.mb_field_decoding_flag;
    MbaffFrameFlag = slice_header.MbaffFrameFlag;
    constrained_intra_pred_flag = slice_header.m_pps.constrained_intra_pred_flag;
    disable_deblocking_filter_idc = slice_header.disable_deblocking_filter_idc;
    CurrMbAddr = slice_data.CurrMbAddr;
    slice_id = slice_data.slice_id;
    slice_number = slice_data.slice_number;
    m_slice_type = slice_header.slice_type;
    FilterOffsetA = slice_header.FilterOffsetA;
    FilterOffsetB = slice_header.FilterOffsetB;

    ret = picture.Inverse_macroblock_scanning_process(slice_header.MbaffFrameFlag, CurrMbAddr, mb_field_decoding_flag, picture.m_mbs[CurrMbAddr].m_mb_position_x, picture.m_mbs[CurrMbAddr].m_mb_position_y);
    RETURN_IF_FAILED(ret != 0, ret);

    int is_ae = slice_header.m_pps.entropy_coding_mode_flag; //ae(v)表示CABAC编码

    if (is_ae) // ae(v) 表示CABAC编码
    {
        ret = cabac.CABAC_decode_mb_type(picture, bs, mb_type); //2 ue(v) | ae(v)
        RETURN_IF_FAILED(ret != 0, ret);
    }
    else // ue(v) 表示CAVLC编码
    {
        mb_type = gb.get_ue_golomb(bs); //2 ue(v) | ae(v)
    }

    ret = fix_mb_type(slice_header.slice_type, mb_type, m_slice_type_fixed, m_mb_type_fixed); //需要立即修正mb_type的值
    RETURN_IF_FAILED(ret != 0, ret);

    ret = set_mb_type_X_slice_info(); //因CABAC会用到MbPartWidth/MbPartHeight信息，所以需要尽可能提前设置相关值
    RETURN_IF_FAILED(ret != 0, ret);

    ret = MbPartPredMode(m_slice_type_fixed, transform_size_8x8_flag, m_mb_type_fixed, 0, m_NumMbPart,
        CodedBlockPatternChroma, CodedBlockPatternLuma, Intra16x16PredMode, m_name_of_mb_type, m_mb_pred_mode);
    RETURN_IF_FAILED(ret != 0, ret);

    if (m_mb_type_fixed == 25) //I_PCM=25
    {
        while(!byte_aligned(bs))
        {
            pcm_alignment_zero_bit = bs.readBits(1); // 3 f(1)    is a bit equal to 0.
        }

        for (i = 0; i < 256; i++)
        {
            int32_t v = slice_header.m_sps.BitDepthY;
            pcm_sample_luma[ i ] = bs.readBits(v); //3 u(v)
        }

        for (i = 0; i < 2 * slice_header.m_sps.MbWidthC * slice_header.m_sps.MbHeightC; i++)
        {
            int32_t v = slice_header.m_sps.BitDepthC;
            pcm_sample_chroma[ i ] = bs.readBits(v); //3 u(v)
        }
    }
    else
    {
        int32_t noSubMbPartSizeLessThan8x8Flag = 1;

        //if (mb_type != I_NxN && MbPartPredMode(mb_type, 0) != Intra_16x16 && NumMbPart(mb_type) == 4)
        if (m_name_of_mb_type != I_NxN && m_mb_pred_mode != Intra_16x16 && m_NumMbPart == 4)
        {
            ret = sub_mb_pred(bs, picture, slice_data, cabac); //mb_type is one of 3=P_8x8, 4=P_8x8ref0, 22=B_8x8
            RETURN_IF_FAILED(ret != 0, ret);

            for (mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++)
            {
                if (m_name_of_sub_mb_type[ mbPartIdx ] != B_Direct_8x8)
                {
                    int NumSubMbPart = 0;
                    if (m_slice_type_fixed == H264_SLIECE_TYPE_P && sub_mb_type[ mbPartIdx ] >= 0 && sub_mb_type[ mbPartIdx ] <= 3)
                    {
                        NumSubMbPart = sub_mb_type_P_mbs_define[sub_mb_type[ mbPartIdx ]].NumSubMbPart;
                    }else if (m_slice_type_fixed == H264_SLIECE_TYPE_B && sub_mb_type[ mbPartIdx ] >= 0 && sub_mb_type[ mbPartIdx ] <= 3)
                    {
                        NumSubMbPart = sub_mb_type_B_mbs_define[sub_mb_type[ mbPartIdx ]].NumSubMbPart;
                    }
                    else
                    {
                        LOG_ERROR("m_slice_type=%d; m_slice_type_fixed=%d; sub_mb_type[%d]=%d;\n", m_slice_type, m_slice_type_fixed, mbPartIdx, sub_mb_type[ mbPartIdx ]);
                        return -1;
                    }

                    //-----mb_type is one of 3=P_8x8, 4=P_8x8ref0, 22=B_8x8----------
                    //if (NumSubMbPart(sub_mb_type[ mbPartIdx ]) > 1)
                    if (NumSubMbPart > 1)
                    {
                        noSubMbPartSizeLessThan8x8Flag = 0;
                    }
                }
                else if (!slice_header.m_sps.direct_8x8_inference_flag)
                {
                    noSubMbPartSizeLessThan8x8Flag = 0;
                }
            }
        }
        else
        {
            if (slice_header.m_pps.transform_8x8_mode_flag && m_name_of_mb_type == I_NxN)
            {
                if (is_ae) // ae(v) 表示CABAC编码
                {
                    ret = cabac.CABAC_decode_transform_size_8x8_flag(picture, bs, transform_size_8x8_flag_temp); //2 u(1) | ae(v)
                    RETURN_IF_FAILED(ret != 0, ret);
                }
                else // ue(v) 表示CAVLC编码
                {
                    transform_size_8x8_flag_temp = bs.readBits(1); //2 u(1) | ae(v)
                }

                if (transform_size_8x8_flag_temp != transform_size_8x8_flag)
                {
                    transform_size_8x8_flag = transform_size_8x8_flag_temp;
                    ret = MbPartPredMode(m_slice_type_fixed, transform_size_8x8_flag, m_mb_type_fixed, 0, m_NumMbPart, 
                        CodedBlockPatternChroma, CodedBlockPatternLuma, Intra16x16PredMode, m_name_of_mb_type, m_mb_pred_mode);
                    RETURN_IF_FAILED(ret != 0, ret);

                    if ((m_slice_type_fixed % 5) == H264_SLIECE_TYPE_I && m_mb_type_fixed == 0)
                    {
                        mb_type_I_slice = mb_type_I_slices_define[(transform_size_8x8_flag == 0) ? 0 : 1];
                    }
                }
            }

            ret = mb_pred(bs, picture, slice_data, cabac);
            RETURN_IF_FAILED(ret != 0, ret);
        }

        //if (MbPartPredMode(mb_type, 0) != Intra_16x16)
        if (m_mb_pred_mode != Intra_16x16)
        {
            if (is_ae) // ae(v) 表示CABAC编码
            {
                ret = cabac.CABAC_decode_coded_block_pattern(picture, bs, coded_block_pattern); //2 me(v) | ae(v)
                RETURN_IF_FAILED(ret != 0, ret);
            }
            else // ue(v) 表示CAVLC编码
            {
                coded_block_pattern = gb.get_me_golomb(bs, slice_header.m_sps.ChromaArrayType, m_mb_pred_mode); //2 me(v) | ae(v)
            }
            
            CodedBlockPatternLuma = coded_block_pattern % 16;
            CodedBlockPatternChroma = coded_block_pattern / 16;

            if (CodedBlockPatternLuma > 0 && slice_header.m_pps.transform_8x8_mode_flag && m_name_of_mb_type != I_NxN && noSubMbPartSizeLessThan8x8Flag 
                && (m_name_of_mb_type != B_Direct_16x16 || slice_header.m_sps.direct_8x8_inference_flag)
                )
            {
                if (is_ae) // ae(v) 表示CABAC编码
                {
                    ret = cabac.CABAC_decode_transform_size_8x8_flag(picture, bs, transform_size_8x8_flag_temp); //2 u(1) | ae(v)
                    RETURN_IF_FAILED(ret != 0, ret);
                }
                else // ue(v) 表示CAVLC编码
                {
                    transform_size_8x8_flag_temp = bs.readBits(1); //2 u(1) | ae(v)
                }
                
                if (transform_size_8x8_flag_temp != transform_size_8x8_flag)
                {
                    transform_size_8x8_flag = transform_size_8x8_flag_temp;
                    ret = MbPartPredMode(m_slice_type_fixed, transform_size_8x8_flag, m_mb_type_fixed, 0, m_NumMbPart, 
                        CodedBlockPatternChroma, CodedBlockPatternLuma, Intra16x16PredMode, m_name_of_mb_type, m_mb_pred_mode);
                    RETURN_IF_FAILED(ret != 0, ret);
                    
                    if ((m_slice_type_fixed % 5) == H264_SLIECE_TYPE_I && m_mb_type_fixed == 0)
                    {
                        mb_type_I_slice = mb_type_I_slices_define[(transform_size_8x8_flag == 0) ? 0 : 1];
                    }
                }
            }
        }

        if (CodedBlockPatternLuma > 0 || CodedBlockPatternChroma > 0 || m_mb_pred_mode == Intra_16x16) // MbPartPredMode(mb_type, 0) == Intra_16x16)
        {
            if (is_ae) // ae(v) 表示CABAC编码
            {
                ret = cabac.CABAC_decode_mb_qp_delta(picture, bs, mb_qp_delta); //2 se(v) | ae(v)
                RETURN_IF_FAILED(ret != 0, ret);
            }
            else // ue(v) 表示CAVLC编码
            {
                mb_qp_delta = gb.get_se_golomb(bs); //2 se(v) | ae(v)
            }
            
            ret = residual(bs, picture, 0, 15, cabac); // 3 | 4
            RETURN_IF_FAILED(ret != 0, ret);
        }
    }
    
    //---------------------------------------------------
    if (mb_qp_delta < (int32_t)(-(26 + (int32_t)slice_header.m_sps.QpBdOffsetY / 2)) || mb_qp_delta > (25 + (int32_t)slice_header.m_sps.QpBdOffsetY / 2))
    {
        LOG_WARN("mb_qp_delta=(%d) is out of range [%d,%d].\n", mb_qp_delta, (-(26 + (int32_t)slice_header.m_sps.QpBdOffsetY / 2)), (25 + (int32_t)slice_header.m_sps.QpBdOffsetY / 2));
        mb_qp_delta = CLIP3((-(26 + (int32_t)slice_header.m_sps.QpBdOffsetY / 2)), (25 + (int32_t)slice_header.m_sps.QpBdOffsetY / 2), mb_qp_delta);
    }

    QPY = ((slice_header.QPY_prev + mb_qp_delta + 52 + 2 * slice_header.m_sps.QpBdOffsetY) % (52 + slice_header.m_sps.QpBdOffsetY)) - slice_header.m_sps.QpBdOffsetY; // (7-37)
    QP1Y = QPY + slice_header.m_sps.QpBdOffsetY;
    slice_header.QPY_prev = QPY;

    if (slice_header.m_sps.qpprime_y_zero_transform_bypass_flag == 1 && QP1Y == 0)
    {
        TransformBypassModeFlag = 1;
    }
    else //if (slice_header.m_sps.qpprime_y_zero_transform_bypass_flag == 0 || QP1Y == 1)
    {
        TransformBypassModeFlag = 0;
    }

    //-------------------
//    int ret2 = printInfo();

    return 0;
}


int CH264MacroBlock::macroblock_layer_mb_skip(CH264PictureBase &picture, const CH264SliceData &slice_data, CH264Cabac &cabac)
{
    int ret = 0;
    int32_t i = 0;
    CH264Golomb gb;
    int32_t mbPartIdx = 0;
    
    CH264SliceHeader & slice_header = picture.m_h264_slice_header;
    
    field_pic_flag = slice_header.field_pic_flag;
    bottom_field_flag = slice_header.bottom_field_flag;
    mb_skip_flag = slice_data.mb_skip_flag;
    mb_field_decoding_flag = slice_data.mb_field_decoding_flag;
    MbaffFrameFlag = slice_header.MbaffFrameFlag;
    constrained_intra_pred_flag = slice_header.m_pps.constrained_intra_pred_flag;
    disable_deblocking_filter_idc = slice_header.disable_deblocking_filter_idc;
    CurrMbAddr = slice_data.CurrMbAddr;
    slice_id = slice_data.slice_id;
    slice_number = slice_data.slice_number;
    m_slice_type = slice_header.slice_type;
    FilterOffsetA = slice_header.FilterOffsetA;
    FilterOffsetB = slice_header.FilterOffsetB;
    
    ret = picture.Inverse_macroblock_scanning_process(slice_header.MbaffFrameFlag, CurrMbAddr, mb_field_decoding_flag, picture.m_mbs[CurrMbAddr].m_mb_position_x, picture.m_mbs[CurrMbAddr].m_mb_position_y);
    RETURN_IF_FAILED(ret != 0, ret);

//    int is_ae = slice_header.m_pps.entropy_coding_mode_flag; //ae(v)表示CABAC编码

    if (slice_header.slice_type == H264_SLIECE_TYPE_P || slice_header.slice_type == H264_SLIECE_TYPE_SP)
    {
        mb_type = 5; //inferred: P_Skip: no further data is present for the macroblock in the bitstream.
    }
    else if (slice_header.slice_type == H264_SLIECE_TYPE_B)
    {
        mb_type = 23; //inferred: B_Skip: no further data is present for the macroblock in the bitstream.
    }

//    ret = fix_mb_type(slice_header.slice_type, mb_type, m_slice_type_fixed, m_mb_type_fixed); //需要立即修正mb_type的值
//    RETURN_IF_FAILED(ret != 0, ret);

    m_slice_type_fixed = slice_header.slice_type;
    m_mb_type_fixed = mb_type;

    //-----------------------------------------------
    int32_t noSubMbPartSizeLessThan8x8Flag = 1;
    ret = MbPartPredMode(m_slice_type_fixed, transform_size_8x8_flag, m_mb_type_fixed, 0, m_NumMbPart,
        CodedBlockPatternChroma, CodedBlockPatternLuma, Intra16x16PredMode, m_name_of_mb_type, m_mb_pred_mode);
    RETURN_IF_FAILED(ret != 0, ret);

//    RETURN_IF_FAILED(m_name_of_mb_type == P_Skip || m_name_of_mb_type == B_Skip, -1);
    
    //---------------------------------------------------
    mb_qp_delta = 0;

    QPY = ((slice_header.QPY_prev + mb_qp_delta + 52 + 2 * slice_header.m_sps.QpBdOffsetY) % (52 + slice_header.m_sps.QpBdOffsetY)) - slice_header.m_sps.QpBdOffsetY; // (7-37)
    QP1Y = QPY + slice_header.m_sps.QpBdOffsetY;
    slice_header.QPY_prev = QPY;
    
    ret = set_mb_type_X_slice_info(); //因CABAC会用到MbPartWidth/MbPartHeight信息，所以需要尽可能提前设置相关值
    RETURN_IF_FAILED(ret != 0, ret);

    return 0;
}


/*
 * Page 57/79/812
 * 7.3.5.1 Macroblock prediction syntax
 */
int CH264MacroBlock::mb_pred(CBitstream &bs, CH264PictureBase &picture, const CH264SliceData &slice_data, CH264Cabac &cabac)
{
    int ret = 0;

    CH264Golomb gb;
    int32_t NumMbPart = m_NumMbPart;
    int32_t luma4x4BlkIdx = 0;
    int32_t luma8x8BlkIdx = 0;
    int32_t mbPartIdx = 0;
    int32_t compIdx = 0;
    
    CH264SliceHeader & slice_header = picture.m_h264_slice_header;

    int is_ae = slice_header.m_pps.entropy_coding_mode_flag; //ae(v)表示CABAC编码

    //if (MbPartPredMode(mb_type, 0) == Intra_4x4 || MbPartPredMode(mb_type, 0) == Intra_8x8 || MbPartPredMode(mb_type, 0) == Intra_16x16)
    if (m_mb_pred_mode == Intra_4x4 || m_mb_pred_mode == Intra_8x8 || m_mb_pred_mode == Intra_16x16)
    {
        if (m_mb_pred_mode == Intra_4x4)
        {
            for (luma4x4BlkIdx=0; luma4x4BlkIdx < 16; luma4x4BlkIdx++)
            {
                if (is_ae) // ae(v) 表示CABAC编码
                {
                    ret = cabac.CABAC_decode_prev_intra4x4_pred_mode_flag_or_prev_intra8x8_pred_mode_flag(picture, bs, prev_intra4x4_pred_mode_flag[ luma4x4BlkIdx ]); //2 u(1) | ae(v)
                    RETURN_IF_FAILED(ret != 0, ret);
                }
                else // ue(v) 表示CAVLC编码
                {
                    prev_intra4x4_pred_mode_flag[ luma4x4BlkIdx ] = bs.readBits(1); //2 u(1) | ae(v)
                }

                if (!prev_intra4x4_pred_mode_flag[ luma4x4BlkIdx ])
                {
                    if (is_ae) // ae(v) 表示CABAC编码
                    {
                        ret = cabac.CABAC_decode_rem_intra4x4_pred_mode_or_rem_intra8x8_pred_mode(picture, bs, rem_intra4x4_pred_mode[ luma4x4BlkIdx ]); //2 u(3) | ae(v)
                        RETURN_IF_FAILED(ret != 0, ret);
                    }
                    else // ue(v) 表示CAVLC编码
                    {
                        rem_intra4x4_pred_mode[ luma4x4BlkIdx ] = bs.readBits(3); //2 u(3) | ae(v)
                    }
                }
            }
        }

        if (m_mb_pred_mode == Intra_8x8)
        {
            for (luma8x8BlkIdx=0; luma8x8BlkIdx < 4; luma8x8BlkIdx++)
            {
                if (is_ae) // ae(v) 表示CABAC编码
                {
                    ret = cabac.CABAC_decode_prev_intra4x4_pred_mode_flag_or_prev_intra8x8_pred_mode_flag(picture, bs, prev_intra8x8_pred_mode_flag[ luma8x8BlkIdx ]); //2 u(1) | ae(v)
                    RETURN_IF_FAILED(ret != 0, ret);
                }
                else // ue(v) 表示CAVLC编码
                {
                    prev_intra8x8_pred_mode_flag[ luma8x8BlkIdx ] = bs.readBits(1); //2 u(1) | ae(v)
                }

                if (!prev_intra8x8_pred_mode_flag[ luma8x8BlkIdx ])
                {
                    if (is_ae) // ae(v) 表示CABAC编码
                    {
                        ret = cabac.CABAC_decode_rem_intra4x4_pred_mode_or_rem_intra8x8_pred_mode(picture, bs, rem_intra8x8_pred_mode[ luma8x8BlkIdx ]); //2 u(3) | ae(v)
                        RETURN_IF_FAILED(ret != 0, ret);
                    }
                    else // ue(v) 表示CAVLC编码
                    {
                        rem_intra8x8_pred_mode[ luma8x8BlkIdx ] = bs.readBits(3); //2 u(3) | ae(v)
                    }
                }
            }
        }

        if (slice_header.m_sps.ChromaArrayType == 1 || slice_header.m_sps.ChromaArrayType == 2)
        {
            if (is_ae) // ae(v) 表示CABAC编码
            {
                ret = cabac.CABAC_decode_intra_chroma_pred_mode(picture, bs, intra_chroma_pred_mode); //2 ue(v) | ae(v)
                RETURN_IF_FAILED(ret != 0, ret);
            }
            else // ue(v) 表示CAVLC编码
            {
                intra_chroma_pred_mode = gb.get_ue_golomb(bs); //2 ue(v) | ae(v)
            }
        }
    }
    else if (m_mb_pred_mode != Direct)
    {
        H264_MB_PART_PRED_MODE mb_pred_mode = MB_PRED_MODE_NA;

        for (mbPartIdx = 0; mbPartIdx < NumMbPart; mbPartIdx++)
        {
            ret = MbPartPredMode2(m_name_of_mb_type, mbPartIdx, transform_size_8x8_flag, mb_pred_mode);
            RETURN_IF_FAILED(ret != 0, ret);

            if ((slice_header.num_ref_idx_l0_active_minus1 > 0 || slice_data.mb_field_decoding_flag != slice_header.field_pic_flag)
                && mb_pred_mode != Pred_L1) //MbPartPredMode(mb_type, mbPartIdx) != Pred_L1)
            {
                if (is_ae) // ae(v) 表示CABAC编码
                {
                    int32_t ref_idx_flag = 0;

                    ret = cabac.CABAC_decode_ref_idx_lX(picture, bs, ref_idx_flag, mbPartIdx, ref_idx_l0[ mbPartIdx ]); //2 te(v) | ae(v)
                    RETURN_IF_FAILED(ret != 0, ret);
                }
                else // ue(v) 表示CAVLC编码
                {
                    int range = picture.m_RefPicList0Length - 1;

                    if (slice_data.mb_field_decoding_flag == 1) //注意：此处是个坑，T-REC-H.264-201704-S!!PDF-E.pdf 文档中并未明确写出来
                    {
                        range = picture.m_RefPicList0Length * 2 - 1;
                    }

                    ref_idx_l0[ mbPartIdx ] = gb.get_te_golomb(bs, range); //2 te(v) | ae(v)
                }
            }
        }

        for (mbPartIdx = 0; mbPartIdx < NumMbPart; mbPartIdx++)
        {
            ret = MbPartPredMode2(m_name_of_mb_type, mbPartIdx, transform_size_8x8_flag, mb_pred_mode);
            RETURN_IF_FAILED(ret != 0, ret);

            if ((slice_header.num_ref_idx_l1_active_minus1 > 0 || slice_data.mb_field_decoding_flag != slice_header.field_pic_flag)
                && mb_pred_mode != Pred_L0) //MbPartPredMode(mb_type, mbPartIdx) != Pred_L0)
            {
                if (is_ae) // ae(v) 表示CABAC编码
                {
                    int32_t ref_idx_flag = 1;

                    ret = cabac.CABAC_decode_ref_idx_lX(picture, bs, ref_idx_flag, mbPartIdx, ref_idx_l1[ mbPartIdx ]); //2 te(v) | ae(v)
                    RETURN_IF_FAILED(ret != 0, ret);
                }
                else // ue(v) 表示CAVLC编码
                {
                    int range = picture.m_RefPicList1Length - 1;

                    if (slice_data.mb_field_decoding_flag == 1) //注意：此处是个坑，T-REC-H.264-201704-S!!PDF-E.pdf 文档中并未明确写出来
                    {
                        range = picture.m_RefPicList1Length * 2 - 1;
                    }

                    ref_idx_l1[ mbPartIdx ] = gb.get_te_golomb(bs, range); //2 te(v) | ae(v)
                }
            }
        }

        for (mbPartIdx = 0; mbPartIdx < NumMbPart; mbPartIdx++)
        {
            ret = MbPartPredMode2(m_name_of_mb_type, mbPartIdx, transform_size_8x8_flag, mb_pred_mode);
            RETURN_IF_FAILED(ret != 0, ret);

            //if (MbPartPredMode (mb_type, mbPartIdx) != Pred_L1)
            if (mb_pred_mode != Pred_L1)
            {
                for (compIdx = 0; compIdx < 2; compIdx++)
                {
                    if (is_ae) // ae(v) 表示CABAC编码
                    {
                        int32_t mvd_flag = compIdx;
                        int32_t subMbPartIdx = 0;
                        int32_t isChroma = 0;

                        ret = cabac.CABAC_decode_mvd_lX(picture, bs, mvd_flag, mbPartIdx, subMbPartIdx, isChroma, mvd_l0[ mbPartIdx ][ 0 ][ compIdx ]); //2 ue(v) | ae(v)
                        RETURN_IF_FAILED(ret != 0, ret);
                    }
                    else // ue(v) 表示CAVLC编码
                    {
                        mvd_l0[ mbPartIdx ][ 0 ][ compIdx ] = gb.get_se_golomb(bs); //2 se(v) | ae(v)
                    }
                }
            }
        }

        for (mbPartIdx = 0; mbPartIdx < NumMbPart; mbPartIdx++)
        {
            ret = MbPartPredMode2(m_name_of_mb_type, mbPartIdx, transform_size_8x8_flag, mb_pred_mode);
            RETURN_IF_FAILED(ret != 0, ret);

            //if (MbPartPredMode(mb_type, mbPartIdx) != Pred_L0)
            if (mb_pred_mode != Pred_L0)
            {
                for (compIdx = 0; compIdx < 2; compIdx++)
                {
                    if (is_ae) // ae(v) 表示CABAC编码
                    {
                        int32_t mvd_flag = 2 + compIdx;
                        int32_t subMbPartIdx = 0;
                        int32_t isChroma = 0;

                        ret = cabac.CABAC_decode_mvd_lX(picture, bs, mvd_flag, mbPartIdx, subMbPartIdx, isChroma, mvd_l1[ mbPartIdx ][ 0 ][ compIdx ]); //2 ue(v) | ae(v)
                        RETURN_IF_FAILED(ret != 0, ret);
                    }
                    else // ue(v) 表示CAVLC编码
                    {
                        mvd_l1[ mbPartIdx ][ 0 ][ compIdx ] = gb.get_se_golomb(bs); //2 se(v) | ae(v)
                    }
                }
            }
        }
    }

    return 0;
}


/*
 * Page 58/80/812
 * 7.3.5.2 Sub-macroblock prediction syntax
 */
int CH264MacroBlock::sub_mb_pred(CBitstream &bs, CH264PictureBase &picture, const CH264SliceData &slice_data, CH264Cabac &cabac)
{
    int ret = 0;
    int i = 0;
    CH264Golomb gb;
    int32_t mbPartIdx = 0;
    int32_t subMbPartIdx = 0;
    int32_t compIdx = 0;
    
    CH264SliceHeader & slice_header = picture.m_h264_slice_header;

    int is_ae = slice_header.m_pps.entropy_coding_mode_flag; //ae(v)表示CABAC编码

    //--------------------------
    for (mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++)
    {
        if (is_ae) // ae(v) 表示CABAC编码
        {
            ret = cabac.CABAC_decode_sub_mb_type(picture, bs, sub_mb_type[ mbPartIdx ]); //2 ue(v) | ae(v)
            RETURN_IF_FAILED(ret != 0, ret);
        }
        else // ue(v) 表示CAVLC编码
        {
           sub_mb_type[ mbPartIdx ]= gb.get_ue_golomb(bs); //2 ue(v) | ae(v)
        }
        
        //--------------------------------------------------
        if (m_slice_type_fixed == H264_SLIECE_TYPE_P && sub_mb_type[ mbPartIdx ] >= 0 && sub_mb_type[ mbPartIdx ] <= 3)
        {
            m_name_of_sub_mb_type[mbPartIdx] = sub_mb_type_P_mbs_define[sub_mb_type[ mbPartIdx ]].name_of_sub_mb_type;
            m_sub_mb_pred_mode[mbPartIdx] = sub_mb_type_P_mbs_define[sub_mb_type[ mbPartIdx ]].SubMbPredMode;
            NumSubMbPart[mbPartIdx] = sub_mb_type_P_mbs_define[sub_mb_type[ mbPartIdx ]].NumSubMbPart;
            SubMbPartWidth[mbPartIdx] = sub_mb_type_P_mbs_define[sub_mb_type[ mbPartIdx ]].SubMbPartWidth;
            SubMbPartHeight[mbPartIdx] = sub_mb_type_P_mbs_define[sub_mb_type[ mbPartIdx ]].SubMbPartHeight;
        }
        else if (m_slice_type_fixed == H264_SLIECE_TYPE_B && sub_mb_type[ mbPartIdx ] >= 0 && sub_mb_type[ mbPartIdx ] <= 12)
        {
            m_name_of_sub_mb_type[mbPartIdx] = sub_mb_type_B_mbs_define[sub_mb_type[ mbPartIdx ]].name_of_sub_mb_type;
            m_sub_mb_pred_mode[mbPartIdx] = sub_mb_type_B_mbs_define[sub_mb_type[ mbPartIdx ]].SubMbPredMode;
            NumSubMbPart[mbPartIdx] = sub_mb_type_B_mbs_define[sub_mb_type[ mbPartIdx ]].NumSubMbPart;
            SubMbPartWidth[mbPartIdx] = sub_mb_type_B_mbs_define[sub_mb_type[ mbPartIdx ]].SubMbPartWidth;
            SubMbPartHeight[mbPartIdx] = sub_mb_type_B_mbs_define[sub_mb_type[ mbPartIdx ]].SubMbPartHeight;
        }
        else
        {
            LOG_ERROR("m_slice_type=%d; m_slice_type_fixed=%d; sub_mb_type[%d]=%d;\n", m_slice_type, m_slice_type_fixed, mbPartIdx, sub_mb_type[ mbPartIdx ]);
            return -1;
        }
    }

    for (mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++)
    {
        //-----mb_type is one of 3=P_8x8, 4=P_8x8ref0, 22=B_8x8----------
        if ((slice_header.num_ref_idx_l0_active_minus1 > 0 || slice_data.mb_field_decoding_flag != slice_header.field_pic_flag) 
            && m_name_of_mb_type != P_8x8ref0 && m_name_of_sub_mb_type[ mbPartIdx ] != B_Direct_8x8 
            && m_sub_mb_pred_mode[mbPartIdx] != Pred_L1) //SubMbPredMode(sub_mb_type[ mbPartIdx ]) != Pred_L1)
        {
            if (is_ae) // ae(v) 表示CABAC编码
            {
                int32_t ref_idx_flag = 0;

                ret = cabac.CABAC_decode_ref_idx_lX(picture, bs, ref_idx_flag, mbPartIdx, ref_idx_l0[mbPartIdx]); //2 te(v) | ae(v)
                RETURN_IF_FAILED(ret != 0, ret);
            }
            else // ue(v) 表示CAVLC编码
            {
                int range = picture.m_RefPicList0Length - 1;

                if (slice_data.mb_field_decoding_flag == 1) //注意：此处是个坑，T-REC-H.264-201704-S!!PDF-E.pdf 文档中并未明确写出来
                {
                    range = picture.m_RefPicList0Length * 2 - 1;
                }

                ref_idx_l0[ mbPartIdx ]= gb.get_te_golomb(bs, range); //2 te(v) | ae(v)
            }
        }
    }

    for (mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++)
    {
        //-----mb_type is one of 3=P_8x8, 4=P_8x8ref0, 22=B_8x8----------
        if ((slice_header.num_ref_idx_l1_active_minus1 > 0 || slice_data.mb_field_decoding_flag != slice_header.field_pic_flag) 
            && m_name_of_sub_mb_type[ mbPartIdx ] != B_Direct_8x8 && m_sub_mb_pred_mode[mbPartIdx] != Pred_L0) //SubMbPredMode(sub_mb_type[ mbPartIdx ]) != Pred_L0)
        {
            if (is_ae) // ae(v) 表示CABAC编码
            {
                int32_t ref_idx_flag = 1;

                ret = cabac.CABAC_decode_ref_idx_lX(picture, bs, ref_idx_flag, mbPartIdx, ref_idx_l1[mbPartIdx]); //2 te(v) | ae(v)
                RETURN_IF_FAILED(ret != 0, ret);
            }
            else // ue(v) 表示CAVLC编码
            {
                int range = picture.m_RefPicList1Length - 1;

                if (slice_data.mb_field_decoding_flag == 1) //注意：此处是个坑，T-REC-H.264-201704-S!!PDF-E.pdf 文档中并未明确写出来
                {
                    range = picture.m_RefPicList1Length * 2 - 1;
                }

                ref_idx_l1[ mbPartIdx ]= gb.get_te_golomb(bs, range); //2 te(v) | ae(v)
            }
        }
    }

    for (mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++)
    {
        //-----mb_type is one of 3=P_8x8, 4=P_8x8ref0, 22=B_8x8----------
        if (m_name_of_sub_mb_type[ mbPartIdx ] != B_Direct_8x8 && m_sub_mb_pred_mode[mbPartIdx] != Pred_L1) //SubMbPredMode(sub_mb_type[ mbPartIdx ]) != Pred_L1)
        {
            //for (subMbPartIdx = 0; subMbPartIdx < NumSubMbPart(sub_mb_type[ mbPartIdx ]); subMbPartIdx++)
            for (subMbPartIdx = 0; subMbPartIdx < NumSubMbPart[mbPartIdx]; subMbPartIdx++)
            {
                for (compIdx = 0; compIdx < 2; compIdx++)
                {
                    if (is_ae) // ae(v) 表示CABAC编码
                    {
                        int32_t mvd_flag = compIdx;
                        int32_t isChroma = 0;

                        ret = cabac.CABAC_decode_mvd_lX(picture, bs, mvd_flag, mbPartIdx, subMbPartIdx, isChroma, mvd_l0[ mbPartIdx ][ subMbPartIdx ][ compIdx ]); //2 ue(v) | ae(v)
                        RETURN_IF_FAILED(ret != 0, ret);
                    }
                    else // ue(v) 表示CAVLC编码
                    {
                         mvd_l0[ mbPartIdx ][ subMbPartIdx ][ compIdx ] = gb.get_se_golomb(bs); //2 se(v) | ae(v)
                    }
                }
            }
        }
    }

    for (mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++)
    {
        //-----mb_type is one of 3=P_8x8, 4=P_8x8ref0, 22=B_8x8----------
        if (m_name_of_sub_mb_type[ mbPartIdx ] != B_Direct_8x8 && m_sub_mb_pred_mode[mbPartIdx] != Pred_L0) //SubMbPredMode(sub_mb_type[ mbPartIdx ]) != Pred_L0)
        {
            //for (subMbPartIdx = 0; subMbPartIdx < NumSubMbPart(sub_mb_type[ mbPartIdx ]); subMbPartIdx++)
            for (subMbPartIdx = 0; subMbPartIdx < NumSubMbPart[mbPartIdx]; subMbPartIdx++)
            {
                for (compIdx = 0; compIdx < 2; compIdx++)
                {
                    if (is_ae) // ae(v) 表示CABAC编码
                    {
                        int32_t mvd_flag = 2 + compIdx;
                        int32_t isChroma = 0;

                        ret = cabac.CABAC_decode_mvd_lX(picture, bs, mvd_flag, mbPartIdx, subMbPartIdx, isChroma, mvd_l1[mbPartIdx][subMbPartIdx][compIdx]); //2 ue(v) | ae(v)
                        RETURN_IF_FAILED(ret != 0, ret);
                    }
                    else // ue(v) 表示CAVLC编码
                    {
                        mvd_l1[mbPartIdx][subMbPartIdx][compIdx] = gb.get_se_golomb(bs); //2 se(v) | ae(v)
                    }
                }
            }
        }
    }

    return 0;
}


/*
 * Page 59/81/812
 * 7.3.5.3 Residual data syntax
 */
int CH264MacroBlock::residual(CBitstream &bs, CH264PictureBase &picture, int32_t startIdx, int32_t endIdx, CH264Cabac &cabac)
{
    int ret = 0;
    int32_t i = 0;
    int32_t iCbCr = 0;
    int32_t i8x8 = 0;
    int32_t i4x4 = 0;
    int32_t BlkIdx = 0;
    int32_t TotalCoeff = 0; //该 4x4 block的残差中，总共有多少个非零系数
    CH264ResidualBlockCavlc cavlc;
    
    CH264SliceHeader & slice_header = picture.m_h264_slice_header;

    int is_ae = slice_header.m_pps.entropy_coding_mode_flag; //ae(v)表示CABAC编码

//    if (!slice_header.m_pps.entropy_coding_mode_flag)
//    {
//        residual_block = residual_block_cavlc;
//    }
//    else
//    {
//        residual_block = residual_block_cabac;
//    }

    ret = residual_luma(bs, picture, i16x16DClevel, i16x16AClevel, level4x4, level8x8, startIdx, endIdx, MB_RESIDUAL_Intra16x16DCLevel, MB_RESIDUAL_Intra16x16ACLevel, cabac); //3 | 4
    RETURN_IF_FAILED(ret != 0, ret);

    memcpy(Intra16x16DCLevel, i16x16DClevel, sizeof(int32_t) * 16 );
    memcpy(Intra16x16ACLevel, i16x16AClevel, sizeof(int32_t) * 16 * 16);
    memcpy(LumaLevel4x4, level4x4, sizeof(int32_t) * 16 * 16);
    memcpy(LumaLevel8x8, level8x8, sizeof(int32_t) * 4 * 64);

    //-----------------------
    if (slice_header.m_sps.ChromaArrayType == 1 || slice_header.m_sps.ChromaArrayType == 2)
    {
        int32_t NumC8x8 = 4 / (slice_header.m_sps.SubWidthC * slice_header.m_sps.SubHeightC);
        for (iCbCr = 0; iCbCr < 2; iCbCr++)
        {
            if ((CodedBlockPatternChroma & 3) && startIdx == 0) //chroma DC residual present
            {
                BlkIdx = 0;
                TotalCoeff = 0;

                if (is_ae) // ae(v) 表示CABAC编码
                {
                    ret = cabac.residual_block_cabac(picture, bs, ChromaDCLevel[ iCbCr ], 0, 4 * NumC8x8 - 1, 4 * NumC8x8, MB_RESIDUAL_ChromaDCLevel, BlkIdx, iCbCr, TotalCoeff); //3 | 4
                    RETURN_IF_FAILED(ret != 0, ret);
                }
                else // ue(v) 表示CAVLC编码
                {
                    MB_RESIDUAL_LEVEL mb_level = (iCbCr == 0) ? MB_RESIDUAL_ChromaDCLevelCb : MB_RESIDUAL_ChromaDCLevelCr;

                    ret = cavlc.residual_block_cavlc(picture, bs, ChromaDCLevel[ iCbCr ], 0, 4 * NumC8x8 - 1, 4 * NumC8x8, mb_level, m_mb_pred_mode, BlkIdx, TotalCoeff); //3 | 4
                    RETURN_IF_FAILED(ret != 0, ret);
                }

                mb_chroma_4x4_non_zero_count_coeff[iCbCr][BlkIdx] = TotalCoeff;
            }
            else
            {
                for (i = 0; i < 4 * NumC8x8; i++)
                {
                    ChromaDCLevel[ iCbCr ][ i ] = 0;
                }
            }
        }

        for (iCbCr = 0; iCbCr < 2; iCbCr++)
        {
            for (i8x8 = 0; i8x8 < NumC8x8; i8x8++)
            {
                for (i4x4 = 0; i4x4 < 4; i4x4++)
                {
                    if (CodedBlockPatternChroma & 2) //chroma AC residual present
                    {
                        BlkIdx = i8x8 * 4 + i4x4;
                        TotalCoeff = 0;

                        if (is_ae) // ae(v) 表示CABAC编码
                        {
                            ret = cabac.residual_block_cabac(picture, bs, ChromaACLevel[ iCbCr ][ i8x8*4+i4x4 ], MAX(0, startIdx - 1), endIdx - 1, 15, 
                                            MB_RESIDUAL_ChromaACLevel, BlkIdx, iCbCr, TotalCoeff); //3 | 4
                            RETURN_IF_FAILED(ret != 0, ret);
                        }
                        else // ue(v) 表示CAVLC编码
                        {
                            MB_RESIDUAL_LEVEL mb_level = (iCbCr == 0) ? MB_RESIDUAL_ChromaACLevelCb : MB_RESIDUAL_ChromaACLevelCr;

                            ret = cavlc.residual_block_cavlc(picture, bs, ChromaACLevel[ iCbCr ][ i8x8*4+i4x4 ], MAX(0, startIdx - 1), endIdx - 1, 15, 
                                            mb_level, m_mb_pred_mode, BlkIdx, TotalCoeff); //3 | 4
                            RETURN_IF_FAILED(ret != 0, ret);
                        }

                        mb_chroma_4x4_non_zero_count_coeff[iCbCr][BlkIdx] = TotalCoeff;
                    }
                    else
                    {
                        for (i = 0; i < 15; i++)
                        {
                            ChromaACLevel[ iCbCr ][ i8x8*4+i4x4 ][ i ] = 0;
                        }
                    }
                }
            }
        }
    }
    else if (slice_header.m_sps.ChromaArrayType == 3)
    {
        ret = residual_luma(bs, picture, i16x16DClevel, i16x16AClevel, level4x4, level8x8, startIdx, endIdx, 
                    MB_RESIDUAL_CbIntra16x16DCLevel, MB_RESIDUAL_CbIntra16x16ACLevel, cabac); //3 | 4
        RETURN_IF_FAILED(ret != 0, ret);
        
        memcpy(CbIntra16x16DCLevel, i16x16DClevel, sizeof(int32_t) * 16);
        memcpy(CbIntra16x16ACLevel, i16x16AClevel, sizeof(int32_t) * 16 * 16);
        memcpy(CbLevel4x4, level4x4, sizeof(int32_t) * 16 * 16);
        memcpy(CbLevel8x8, level8x8, sizeof(int32_t) * 4 * 64);

        ret = residual_luma(bs, picture, i16x16DClevel, i16x16AClevel, level4x4, level8x8, startIdx, endIdx, 
                    MB_RESIDUAL_CrIntra16x16DCLevel, MB_RESIDUAL_CrIntra16x16ACLevel, cabac); //3 | 4
        RETURN_IF_FAILED(ret != 0, ret);

        memcpy(CrIntra16x16DCLevel, i16x16DClevel, sizeof(int32_t) * 16);
        memcpy(CrIntra16x16ACLevel, i16x16AClevel, sizeof(int32_t) * 16 * 16);
        memcpy(CrLevel4x4, level4x4, sizeof(int32_t) * 16 * 16);
        memcpy(CrLevel8x8, level8x8, sizeof(int32_t) * 4 * 64);
    }

    return 0;
}


/*
 * Page 60/82/812
 * 7.3.5.3.1 Residual luma syntax
 */
int CH264MacroBlock::residual_luma(CBitstream &bs, CH264PictureBase &picture, int32_t (&i16x16DClevel)[16], int32_t (&i16x16AClevel)[16][16], int32_t (&level4x4)[16][16], 
        int32_t (&level8x8)[4][64], int32_t startIdx, int32_t endIdx, MB_RESIDUAL_LEVEL mb_residual_level_dc, MB_RESIDUAL_LEVEL mb_residual_level_ac, CH264Cabac &cabac)
{
    int ret = 0;
    int32_t i = 0;
    int32_t i8x8 = 0;
    int32_t i4x4 = 0;
    int32_t BlkIdx = 0;
    int32_t TotalCoeff = 0; //该 4x4 block的残差中，总共有多少个非零系数
    H264_MB_TYPE name_of_mb_type2 = MB_TYPE_NA;
    CH264ResidualBlockCavlc cavlc;
    
    CH264SliceHeader & slice_header = picture.m_h264_slice_header;

    int is_ae = slice_header.m_pps.entropy_coding_mode_flag; //ae(v)表示CABAC编码

    if (startIdx == 0 && m_mb_pred_mode == Intra_16x16) //MbPartPredMode(mb_type, 0) == Intra_16x16)
    {
        BlkIdx = 0;
        TotalCoeff = 0;

        if (is_ae) // ae(v) 表示CABAC编码
        {
            ret = cabac.residual_block_cabac(picture, bs, i16x16DClevel, 0, 15, 16, mb_residual_level_dc, BlkIdx, -1, TotalCoeff); //3 | 4
            RETURN_IF_FAILED(ret != 0, ret);
        }
        else // ue(v) 表示CAVLC编码
        {
            ret = cavlc.residual_block_cavlc(picture, bs, i16x16DClevel, 0, 15, 16, mb_residual_level_dc, m_mb_pred_mode, BlkIdx, TotalCoeff); //3 | 4
            RETURN_IF_FAILED(ret != 0, ret);
        }

        mb_luma_4x4_non_zero_count_coeff[BlkIdx] = TotalCoeff;
    }

    for (i8x8 = 0; i8x8 < 4; i8x8++)
    {
        if (!transform_size_8x8_flag || !slice_header.m_pps.entropy_coding_mode_flag)
        {
            for (i4x4 = 0; i4x4 < 4; i4x4++)
            {
                if (CodedBlockPatternLuma & (1 << i8x8))
                {
                    BlkIdx = i8x8 * 4 + i4x4;
                    TotalCoeff = 0;

                    if (m_mb_pred_mode == Intra_16x16)
                    {
                        if (is_ae) // ae(v) 表示CABAC编码
                        {
                            ret = cabac.residual_block_cabac(picture, bs, i16x16AClevel[ i8x8 * 4 + i4x4 ], MAX(0, startIdx - 1), endIdx - 1, 15, MB_RESIDUAL_Intra16x16ACLevel, BlkIdx, -1, TotalCoeff); //3 | 4
                            RETURN_IF_FAILED(ret != 0, ret);
                        }
                        else // ue(v) 表示CAVLC编码
                        {
                            ret = cavlc.residual_block_cavlc(picture, bs, i16x16AClevel[ i8x8 * 4 + i4x4 ], MAX(0, startIdx - 1), endIdx - 1, 15, MB_RESIDUAL_Intra16x16ACLevel, m_mb_pred_mode, BlkIdx, TotalCoeff); //3
                            RETURN_IF_FAILED(ret != 0, ret);
                        }
                    }
                    else // Intra_4x4 or Intra_8x8
                    {
                        if (is_ae) // ae(v) 表示CABAC编码
                        {
                            ret = cabac.residual_block_cabac(picture, bs, level4x4[ i8x8 * 4 + i4x4 ], startIdx, endIdx, 16, MB_RESIDUAL_LumaLevel4x4, BlkIdx, -1, TotalCoeff); //3 | 4
                            RETURN_IF_FAILED(ret != 0, ret);
                        }
                        else // ue(v) 表示CAVLC编码
                        {
                            ret = cavlc.residual_block_cavlc(picture, bs, level4x4[ i8x8 * 4 + i4x4 ], startIdx, endIdx, 16, MB_RESIDUAL_LumaLevel4x4, m_mb_pred_mode, BlkIdx, TotalCoeff); //3 | 4
                            RETURN_IF_FAILED(ret != 0, ret);
                        }
                    }

                    mb_luma_4x4_non_zero_count_coeff[BlkIdx] = TotalCoeff;
                    mb_luma_8x8_non_zero_count_coeff[i8x8] += mb_luma_4x4_non_zero_count_coeff[ i8x8 * 4 + i4x4 ];
                }
                else if (m_mb_pred_mode == Intra_16x16)
                {
                    for (i = 0; i < 15; i++)
                    {
                        i16x16AClevel[ i8x8 * 4 + i4x4 ][ i ] = 0;
                    }
                }
                else
                {
                    for (i = 0; i < 16; i++)
                    {
                        level4x4[ i8x8 * 4 + i4x4 ][ i ] = 0;
                    }
                }

                if (!slice_header.m_pps.entropy_coding_mode_flag && transform_size_8x8_flag)
                {
                    for (i = 0; i < 16; i++)
                    {
                        level8x8[ i8x8 ][ 4 * i + i4x4 ] = level4x4[ i8x8 * 4 + i4x4 ][ i ];
                    }
                    mb_luma_8x8_non_zero_count_coeff[i8x8] += mb_luma_4x4_non_zero_count_coeff[ i8x8 * 4 + i4x4 ];
                }
            }
        }
        else if (CodedBlockPatternLuma & (1 << i8x8))
        {
            BlkIdx = i8x8;
            TotalCoeff = 0;

            if (is_ae) // ae(v) 表示CABAC编码
            {
                ret = cabac.residual_block_cabac(picture, bs, level8x8[ i8x8 ], 4 * startIdx, 4 * endIdx + 3, 64, MB_RESIDUAL_LumaLevel8x8, BlkIdx, -1, TotalCoeff); //3 | 4
                RETURN_IF_FAILED(ret != 0, ret);
            }
            else // ue(v) 表示CAVLC编码
            {
                ret = cavlc.residual_block_cavlc(picture, bs, level8x8[ i8x8 ], 4 * startIdx, 4 * endIdx + 3, 64, MB_RESIDUAL_LumaLevel8x8, m_mb_pred_mode, BlkIdx, TotalCoeff); //3 | 4
                RETURN_IF_FAILED(ret != 0, ret);
            }

            mb_luma_8x8_non_zero_count_coeff[i8x8] = TotalCoeff;
        }
        else
        {
            for (i = 0; i < 64; i++)
            {
                level8x8[ i8x8 ][ i ] = 0;
            }
        }
    }

    return 0;
}
