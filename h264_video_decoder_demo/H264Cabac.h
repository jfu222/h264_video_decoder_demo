//
// H264Cabac.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __H264_CABAC_H__
#define __H264_CABAC_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Bitstream.h"
#include "CommonFunction.h"
#include "H264CommonFunc.h"

class CH264PictureBase;


//------------------------------------------------------------
//Context-Based Adaptive Binary Arithmetic Coding (CABAC)
//基于上下文自适应二进制算术熵编码
class CH264Cabac
{
public:
    int32_t m_codIRange;
    int32_t m_codIOffset;

    int32_t m_pStateIdxs[1024];
    int32_t m_valMPSs[1024];

public:
    CH264Cabac();
    ~CH264Cabac();
    
    int get_m_and_n(int32_t ctxIdx, H264_SLIECE_TYPE slice_type, int32_t cabac_init_idc, int32_t &m, int32_t &n);

    int Initialisation_process_for_context_variables(H264_SLIECE_TYPE slice_type, int32_t cabac_init_idc, int32_t SliceQPY); //9.3.1.1
    int Initialisation_process_for_the_arithmetic_decoding_engine(CBitstream &bs); //9.3.1.2
    
    int Derivation_process_of_ctxIdxInc_for_the_syntax_element_mb_skip_flag(CH264PictureBase &picture, int32_t _CurrMbAddr, int32_t &ctxIdxInc); //9.3.3.1.1.1
    int Derivation_process_of_ctxIdxInc_for_the_syntax_element_mb_field_decoding_flag(CH264PictureBase &picture, int32_t &ctxIdxInc); //9.3.3.1.1.2
    int Derivation_process_of_ctxIdxInc_for_the_syntax_element_mb_type(CH264PictureBase &picture, int32_t ctxIdxOffset, int32_t &ctxIdxInc); //9.3.3.1.1.3
    int Derivation_process_of_ctxIdxInc_for_the_syntax_element_coded_block_pattern(CH264PictureBase &picture, int32_t binIdx, int32_t binValues, 
            int32_t ctxIdxOffset, int32_t &ctxIdxInc); //9.3.3.1.1.4
    int Derivation_process_of_ctxIdxInc_for_the_syntax_element_mb_qp_delta(CH264PictureBase &picture, int32_t &ctxIdxInc); //9.3.3.1.1.5
    int Derivation_process_of_ctxIdxInc_for_the_syntax_elements_ref_idx_l0_and_ref_idx_l1(CH264PictureBase &picture, int32_t is_ref_idx_10, 
            int32_t mbPartIdx, int32_t &ctxIdxInc); //9.3.3.1.1.6
    int Derivation_process_of_ctxIdxInc_for_the_syntax_elements_mvd_l0_and_mvd_l1(CH264PictureBase &picture, int32_t is_mvd_10, int32_t mbPartIdx, int32_t subMbPartIdx, 
            int32_t isChroma, int32_t ctxIdxOffset, int32_t &ctxIdxInc); //9.3.3.1.1.7
    int Derivation_process_of_ctxIdxInc_for_the_syntax_element_intra_chroma_pred_mode(CH264PictureBase &picture, int32_t &ctxIdxInc); //9.3.3.1.1.8
    int Derivation_process_of_ctxIdxInc_for_the_syntax_element_coded_block_flag(CH264PictureBase &picture, int32_t ctxBlockCat, 
            int32_t BlkIdx, int32_t iCbCr, int32_t &ctxIdxInc); //9.3.3.1.1.9
    int Derivation_process_of_ctxIdxInc_for_the_syntax_element_transform_size_8x8_flag(CH264PictureBase &picture, int32_t &ctxIdxInc); //9.3.3.1.1.10

    //-----------------------------------------
    int DecodeBin(CBitstream &bs, int32_t bypassFlag, int32_t ctxIdx, int32_t &binVal); //9.3.3.2
    int DecodeDecision(CBitstream &bs, int32_t ctxIdx, int32_t &binVal); //9.3.3.2.1
    int RenormD(CBitstream &bs, int32_t &codIRange, int32_t &codIOffset); //9.3.3.2.2
    int DecodeBypass(CBitstream &bs, int32_t &binVal); //9.3.3.2.3
    int DecodeTerminate(CBitstream &bs, int32_t &binVal); //9.3.3.2.4
    
    //-----------------------------------------
    int CABAC_decode_mb_type(CH264PictureBase &picture, CBitstream &bs, int32_t &synElVal);
    int CABAC_decode_sub_mb_type(CH264PictureBase &picture, CBitstream &bs, int32_t &synElVal);
    int CABAC_decode_mb_type_in_I_slices(CH264PictureBase &picture, CBitstream &bs, int32_t ctxIdxOffset, int32_t &synElVal);
    int CABAC_decode_mb_type_in_SI_slices(CH264PictureBase &picture, CBitstream &bs, int32_t &synElVal);
    int CABAC_decode_mb_type_in_P_SP_slices(CH264PictureBase &picture, CBitstream &bs, int32_t &synElVal);
    int CABAC_decode_mb_type_in_B_slices(CH264PictureBase &picture, CBitstream &bs, int32_t &synElVal);
    int CABAC_decode_sub_mb_type_in_P_SP_slices(CH264PictureBase &picture, CBitstream &bs, int32_t &synElVal);
    int CABAC_decode_sub_mb_type_in_B_slices(CH264PictureBase &picture, CBitstream &bs, int32_t &synElVal);

    int CABAC_decode_mb_skip_flag(CH264PictureBase &picture, CBitstream &bs, int32_t _CurrMbAddr, int32_t &synElVal);
    int CABAC_decode_mvd_lX(CH264PictureBase &picture, CBitstream &bs, int32_t mvd_flag, int32_t mbPartIdx, int32_t subMbPartIdx, int32_t isChroma, int32_t &synElVal);
    int CABAC_decode_ref_idx_lX(CH264PictureBase &picture, CBitstream &bs, int32_t ref_idx_flag, int32_t mbPartIdx, int32_t &synElVal);
    int CABAC_decode_mb_qp_delta(CH264PictureBase &picture, CBitstream &bs, int32_t &synElVal);
    int CABAC_decode_intra_chroma_pred_mode(CH264PictureBase &picture, CBitstream &bs, int32_t &synElVal);
    int CABAC_decode_prev_intra4x4_pred_mode_flag_or_prev_intra8x8_pred_mode_flag(CH264PictureBase &picture, CBitstream &bs, int32_t &synElVal);
    int CABAC_decode_rem_intra4x4_pred_mode_or_rem_intra8x8_pred_mode(CH264PictureBase &picture, CBitstream &bs, int32_t &synElVal);
    int CABAC_decode_mb_field_decoding_flag(CH264PictureBase &picture, CBitstream &bs, int32_t &synElVal);
    int CABAC_decode_coded_block_pattern(CH264PictureBase &picture, CBitstream &bs, int32_t &synElVal);
    int CABAC_decode_coded_block_flag(CH264PictureBase &picture, CBitstream &bs, MB_RESIDUAL_LEVEL mb_block_level, int32_t BlkIdx, int32_t iCbCr, int32_t &synElVal);
    int CABAC_decode_significant_coeff_flag(CH264PictureBase &picture, CBitstream &bs, MB_RESIDUAL_LEVEL mb_block_level, 
            int32_t levelListIdx, int32_t last_flag, int32_t &synElVal);
    int CABAC_decode_coeff_abs_level_minus1(CH264PictureBase &picture, CBitstream &bs, MB_RESIDUAL_LEVEL mb_block_level, 
            int32_t numDecodAbsLevelEq1, int32_t numDecodAbsLevelGt1, int32_t &synElVal);
    int CABAC_decode_coeff_sign_flag(CH264PictureBase &picture, CBitstream &bs, int32_t &synElVal);
    int CABAC_decode_end_of_slice_flag(CH264PictureBase &picture, CBitstream &bs, int32_t &synElVal);
    int CABAC_decode_transform_size_8x8_flag(CH264PictureBase &picture, CBitstream &bs, int32_t &synElVal);

    //------------------------------------
    int residual_block_cabac(CH264PictureBase &picture, CBitstream &bs, int32_t coeffLevel[], int32_t startIdx, int32_t endIdx, 
            int32_t maxNumCoeff, MB_RESIDUAL_LEVEL mb_block_level, int32_t BlkIdx, int32_t iCbCr, int32_t &TotalCoeff);
};

#endif //__H264_CABAC_H__
