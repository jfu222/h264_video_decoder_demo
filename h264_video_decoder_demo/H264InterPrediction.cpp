//
// H264InterPrediction.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264PictureBase.h"
#include "H264PicturesGOP.h"
#include "CommonFunction.h"


#define FLD    0
#define FRM    1
#define AFRM   2



//8.5.1 Specification of transform decoding process for 4x4 luma residual blocks
//This specification applies when transform_size_8x8_flag is equal to 0.
int CH264PictureBase::transform_decoding_process_for_4x4_luma_residual_blocks_inter(int32_t isChroma, int32_t isChromaCb, int32_t BitDepth, int32_t PicWidthInSamples, uint8_t *pic_buff)
{
    int ret = 0;

    if (m_mbs[CurrMbAddr].m_mb_pred_mode != Intra_16x16)
    {
        ret = scaling_functions(isChroma, isChromaCb);
        RETURN_IF_FAILED(ret != 0, ret);
        
        int32_t isMbAff = (m_h264_slice_header.MbaffFrameFlag == 1 && m_mbs[CurrMbAddr].mb_field_decoding_flag == 1) ? 1 : 0;

        for (int32_t luma4x4BlkIdx = 0; luma4x4BlkIdx <= 15; luma4x4BlkIdx++) // or CbLevel4x4 or CrLevel4x4
        {
            //8.5.6 Inverse scanning process for 4x4 transform coefficients and scaling lists
            
            int32_t c[4][4] = {0};
            int32_t r[4][4] = {0};

            ret = Inverse_scanning_process_for_4x4_transform_coefficients_and_scaling_lists(m_mbs[CurrMbAddr].LumaLevel4x4[ luma4x4BlkIdx ], c,
                                                m_mbs[CurrMbAddr].field_pic_flag | m_mbs[CurrMbAddr].mb_field_decoding_flag);
            RETURN_IF_FAILED(ret != 0, ret);

            ret = Scaling_and_transformation_process_for_residual_4x4_blocks(c, r, isChroma, isChromaCb);
            RETURN_IF_FAILED(ret != 0, ret);

            if (m_mbs[CurrMbAddr].TransformBypassModeFlag == 1 
                && m_mbs[CurrMbAddr].m_mb_pred_mode == Intra_4x4
                && (m_mbs[CurrMbAddr].Intra4x4PredMode[ luma4x4BlkIdx ] == 0 || m_mbs[CurrMbAddr].Intra4x4PredMode[ luma4x4BlkIdx ] == 1)
                )
            {
                //8.5.15 Intra residual transform-bypass decoding process
                int32_t nW = 4;
                int32_t nH = 4;
                int32_t horPredFlag = m_mbs[CurrMbAddr].Intra4x4PredMode[ luma4x4BlkIdx ];
                
                int32_t f[4][4] = {0};
                for (int32_t i = 0; i <= nH - 1; i++)
                {
                    for (int32_t j = 0; j <= nW - 1; j++)
                    {
                        f[i][j] = r[i][j];
                    }
                }

                if (horPredFlag == 0)
                {
                    for (int32_t i = 0; i <= nH - 1; i++)
                    {
                        for (int32_t j = 0; j <= nW - 1; j++)
                        {
                            r[i][j] = 0;
                            for (int32_t k = 0; k <= i; k++)
                            {
                                r[i][j] += f[k][j];
                            }
                        }
                    }
                }
                else //if (horPredFlag == 1)
                {
                    for (int32_t i = 0; i <= nH - 1; i++)
                    {
                        for (int32_t j = 0; j <= nW - 1; j++)
                        {
                            r[i][j] = 0;
                            for (int32_t k = 0; k <= j; k++)
                            {
                                r[i][j] += f[i][k];
                            }
                        }
                    }
                }
            }

            //------------------------------------------------------
            //6.4.3 Inverse 4x4 luma block scanning process
            //InverseRasterScan = (a % (d / b) ) * b;    if e == 0;
            //InverseRasterScan = (a / (d / b) ) * c;    if e == 1;
            int32_t xO = InverseRasterScan(luma4x4BlkIdx / 4, 8, 8, 16, 0) + InverseRasterScan(luma4x4BlkIdx % 4, 4, 4, 8, 0);
            int32_t yO = InverseRasterScan(luma4x4BlkIdx / 4, 8, 8, 16, 1) + InverseRasterScan(luma4x4BlkIdx % 4, 4, 4, 8, 1);
            
            //--------帧内预测------------
//            ret = Intra_4x4_sample_prediction(luma4x4BlkIdx, pic_buff, isChroma, BitDepth);
//            RETURN_IF_FAILED(ret != 0, ret);

            int32_t u[16] = {0};

            for (int32_t i = 0; i <=3; i++)
            {
                for (int32_t j = 0; j <= 3; j++)
                {
                    //uij = Clip1Y( predL[ xO + j, yO + i ] + rij ) = Clip3( 0, ( 1 << BitDepthY ) − 1, x );
                    //u[i * 4 + j] = CLIP3( 0, ( 1 << BitDepth ) - 1,  pic_buff[(mb_y * 16 + (yO + i)) * PicWidthInSamples + (mb_x * 16 + (xO + j))] + r[i][j] );
                    u[i * 4 + j] = CLIP3(0, (1 << BitDepth) - 1, pic_buff[(m_mbs[CurrMbAddr].m_mb_position_y + (yO + (i)) * (1 + isMbAff)) * PicWidthInSamples + (m_mbs[CurrMbAddr].m_mb_position_x + (xO + (j)))] + r[i][j]);
                }
            }

            ret = Picture_construction_process_prior_to_deblocking_filter_process(u, 4, 4, luma4x4BlkIdx, isChroma, PicWidthInSamples, pic_buff);
            RETURN_IF_FAILED(ret != 0, ret);
        }
    }

    return 0;
}


//8.5.3 Specification of transform decoding process for 8x8 luma residual blocks
//This specification applies when transform_size_8x8_flag is equal to 1.
int CH264PictureBase::transform_decoding_process_for_8x8_luma_residual_blocks_inter(int32_t isChroma, int32_t isChromaCb, int32_t BitDepth, int32_t PicWidthInSamples, 
        int32_t Level8x8[4][64], uint8_t *pic_buff)
{
    int ret = 0;
    
    ret = scaling_functions(isChroma, isChromaCb);
    
    int32_t isMbAff = (m_h264_slice_header.MbaffFrameFlag == 1 && m_mbs[CurrMbAddr].mb_field_decoding_flag == 1) ? 1 : 0;

    for (int32_t luma8x8BlkIdx = 0; luma8x8BlkIdx <= 3; luma8x8BlkIdx++) // or cb8x8BlkIdx or cr8x8BlkIdx
    {
        int32_t c[8][8] = {0};
        int32_t r[8][8] = {0};

        ret = Inverse_scanning_process_for_8x8_transform_coefficients_and_scaling_lists(Level8x8[luma8x8BlkIdx], c, m_mbs[CurrMbAddr].field_pic_flag | m_mbs[CurrMbAddr].mb_field_decoding_flag);
        RETURN_IF_FAILED(ret != 0, ret);
        
        ret = Scaling_and_transformation_process_for_residual_8x8_blocks(c, r, isChroma, isChromaCb);
        RETURN_IF_FAILED(ret != 0, ret);
        
        if (m_mbs[CurrMbAddr].TransformBypassModeFlag == 1
            && m_mbs[CurrMbAddr].m_mb_pred_mode == Intra_8x8
            && (m_mbs[CurrMbAddr].Intra8x8PredMode[luma8x8BlkIdx] == 0 || m_mbs[CurrMbAddr].Intra8x8PredMode[luma8x8BlkIdx] == 1)
            )
        {
            //8.5.15 Intra residual transform-bypass decoding process
            int32_t nW = 8;
            int32_t nH = 8;
            int32_t horPredFlag = m_mbs[CurrMbAddr].Intra8x8PredMode[luma8x8BlkIdx];

            int32_t f[8][8];
            for (int32_t i = 0; i <= nH - 1; i++)
            {
                for (int32_t j = 0; j <= nW - 1; j++)
                {
                    f[i][j] = r[i][j];
                }
            }

            if (horPredFlag == 0)
            {
                for (int32_t i = 0; i <= nH - 1; i++)
                {
                    for (int32_t j = 0; j <= nW - 1; j++)
                    {
                        r[i][j] = 0;
                        for (int32_t k = 0; k <= i; k++)
                        {
                            r[i][j] += f[k][j];
                        }
                    }
                }
            }
            else //if (horPredFlag == 1)
            {
                for (int32_t i = 0; i <= nH - 1; i++)
                {
                    for (int32_t j = 0; j <= nW - 1; j++)
                    {
                        r[i][j] = 0;
                        for (int32_t k = 0; k <= j; k++)
                        {
                            r[i][j] += f[i][k];
                        }
                    }
                }
            }
        }

        //6.4.5 Inverse 8x8 luma block scanning process
        //InverseRasterScan = (a % (d / b) ) * b;    if e == 0;
        //InverseRasterScan = (a / (d / b) ) * c;    if e == 1;
        int32_t xO = InverseRasterScan(luma8x8BlkIdx, 8, 8, 16, 0);
        int32_t yO = InverseRasterScan(luma8x8BlkIdx, 8, 8, 16, 1);
        
        //--------帧内预测------------
//        ret = Intra_8x8_sample_prediction(luma8x8BlkIdx, pic_buff, isChroma, BitDepth);
//        RETURN_IF_FAILED(ret != 0, ret);

        int32_t u[64] = {0};

        for (int32_t i = 0; i <= 7; i++)
        {
            for (int32_t j = 0; j <= 7; j++)
            {
                //uij = Clip1Y( predL[ xO + j, yO + i ] + rij ) = Clip3( 0, ( 1 << BitDepthY ) − 1, x );
                //u[i * 8 + j] = CLIP3(0, (1 << BitDepth) - 1, pic_buff[(mb_y * 16 + (yO + i)) * PicWidthInSamples + (mb_x * 16 + (xO + j))] + r[i][j]);
                u[i * 8 + j] = CLIP3(0, (1 << BitDepth) - 1, pic_buff[(m_mbs[CurrMbAddr].m_mb_position_y + (yO + (i)) * (1 + isMbAff)) * PicWidthInSamples + (m_mbs[CurrMbAddr].m_mb_position_x + (xO + (j)))] + r[i][j]);
            }
        }

        ret = Picture_construction_process_prior_to_deblocking_filter_process(u, 8, 8, luma8x8BlkIdx, isChroma, PicWidthInSamples, pic_buff);
        RETURN_IF_FAILED(ret != 0, ret);
    }

    return 0;
}

//8.5.4 Specification of transform decoding process for chroma samples
//This process is invoked for each chroma component Cb and Cr separately when ChromaArrayType is not equal to 0.
int CH264PictureBase::transform_decoding_process_for_chroma_samples_inter(int32_t isChromaCb, int32_t PicWidthInSamples, uint8_t *pic_buff)
{
    int ret = 0;
    CH264SliceHeader & slice_header = m_h264_slice_header;

    if (slice_header.m_sps.ChromaArrayType == 0)
    {
        LOG_ERROR("This process is invoked for each chroma component Cb and Cr separately when ChromaArrayType is not equal to 0.");
        return -1;
    }
    
    if (slice_header.m_sps.ChromaArrayType == 3)
    {
        //8.5.5 Specification of transform decoding process for chroma samples with ChromaArrayType equal to 3
        ret = transform_decoding_process_for_chroma_samples_with_ChromaArrayType_equal_to_3(isChromaCb, PicWidthInSamples, pic_buff);
        RETURN_IF_FAILED(ret != 0, ret);
    }
    else //if (slice_header.m_sps.ChromaArrayType != 3)
    {
        int32_t iCbCr = (isChromaCb == 1) ? 0 : 1;
        int32_t dcC[4][2] = {0};

        int32_t numChroma4x4Blks = (MbWidthC / 4) * (MbHeightC / 4);

        if (slice_header.m_sps.ChromaArrayType == 1) //YUV420
        {
            int32_t c[2][2] = {0};
            c[0][0] = m_mbs[CurrMbAddr].ChromaDCLevel[iCbCr][0];
            c[0][1] = m_mbs[CurrMbAddr].ChromaDCLevel[iCbCr][1];
            c[1][0] = m_mbs[CurrMbAddr].ChromaDCLevel[iCbCr][2];
            c[1][1] = m_mbs[CurrMbAddr].ChromaDCLevel[iCbCr][3];

            ret = Scaling_and_transformation_process_for_chroma_DC_transform_coefficients(isChromaCb, c, 2, 2, dcC);
            RETURN_IF_FAILED(ret != 0, ret);
        }
        else if (slice_header.m_sps.ChromaArrayType == 2) //YUV422
        {
            int32_t c[4][2] = {0};
            c[0][0] = m_mbs[CurrMbAddr].ChromaDCLevel[iCbCr][0];
            c[0][1] = m_mbs[CurrMbAddr].ChromaDCLevel[iCbCr][1];
            c[1][0] = m_mbs[CurrMbAddr].ChromaDCLevel[iCbCr][2];
            c[1][1] = m_mbs[CurrMbAddr].ChromaDCLevel[iCbCr][3];
            c[2][0] = m_mbs[CurrMbAddr].ChromaDCLevel[iCbCr][4];
            c[2][1] = m_mbs[CurrMbAddr].ChromaDCLevel[iCbCr][5];
            c[3][0] = m_mbs[CurrMbAddr].ChromaDCLevel[iCbCr][6];
            c[3][1] = m_mbs[CurrMbAddr].ChromaDCLevel[iCbCr][7];

            ret = Scaling_and_transformation_process_for_chroma_DC_transform_coefficients(isChromaCb, c, 2, 4, dcC);
            RETURN_IF_FAILED(ret != 0, ret);
        }

        //---------------------------------
        //raster scan
        int32_t dcC_to_chroma_index[8] =
        {
            dcC[0][0], dcC[0][1],
            dcC[1][0], dcC[1][1],
            dcC[2][0], dcC[2][1],
            dcC[3][0], dcC[3][1],
        };
    
        int32_t rMb[16][16] = {0}; //本应该是rMb[MbHeightC][MbWidthC], 此处按最大的16x16尺寸来申请数组
        
        int32_t isMbAff = (m_h264_slice_header.MbaffFrameFlag == 1 && m_mbs[CurrMbAddr].mb_field_decoding_flag == 1) ? 1 : 0;

        for (int32_t chroma4x4BlkIdx = 0; chroma4x4BlkIdx <= numChroma4x4Blks - 1; chroma4x4BlkIdx++)
        {
            int32_t chromaList[16] = {0};

            chromaList[0] = dcC_to_chroma_index[chroma4x4BlkIdx]; //注意：这是直流系数

            for (int32_t k = 1; k <= 15; k++)
            {
                chromaList[k] = m_mbs[CurrMbAddr].ChromaACLevel[iCbCr][ chroma4x4BlkIdx ][ k - 1 ];
            }

            //-----------------
            int32_t c[4][4] = {0};
            int32_t r[4][4] = {0};

            ret = Inverse_scanning_process_for_4x4_transform_coefficients_and_scaling_lists(chromaList, c, m_mbs[CurrMbAddr].field_pic_flag | m_mbs[CurrMbAddr].mb_field_decoding_flag);
            RETURN_IF_FAILED(ret != 0, ret);
        
            int32_t isChroma = 1;
            ret = Scaling_and_transformation_process_for_residual_4x4_blocks(c, r, isChroma, isChromaCb);
            RETURN_IF_FAILED(ret != 0, ret);

            //6.4.7 Inverse 4x4 chroma block scanning process
            int32_t xO = InverseRasterScan( chroma4x4BlkIdx, 4, 4, 8, 0 );
            int32_t yO = InverseRasterScan( chroma4x4BlkIdx, 4, 4, 8, 1 );

            for (int32_t i = 0; i <= 3; i++)
            {
                for (int32_t j = 0; j <= 3; j++)
                {
                    rMb[yO + i][xO + j] = r[i][j];
                }
            }
        }
        
        //--------------------------------------------------
        if (m_mbs[CurrMbAddr].TransformBypassModeFlag == 1
            && (m_mbs[CurrMbAddr].m_mb_pred_mode == Intra_4x4 || m_mbs[CurrMbAddr].m_mb_pred_mode == Intra_8x8 
                    || (m_mbs[CurrMbAddr].m_mb_pred_mode == Intra_16x16 && m_mbs[CurrMbAddr].intra_chroma_pred_mode == 1 || m_mbs[CurrMbAddr].intra_chroma_pred_mode == 2)
                )
            )
        {
            //8.5.15 Intra residual transform-bypass decoding process
            int32_t nW = MbWidthC;
            int32_t nH = MbHeightC;
            int32_t horPredFlag = 2 - m_mbs[CurrMbAddr].intra_chroma_pred_mode;

            int32_t f[16][16] = {0};
            for (int32_t i = 0; i <= nH - 1; i++)
            {
                for (int32_t j = 0; j <= nW - 1; j++)
                {
                    f[i][j] = rMb[i][j];
                }
            }

            if (horPredFlag == 0)
            {
                for (int32_t i = 0; i <= nH - 1; i++)
                {
                    for (int32_t j = 0; j <= nW - 1; j++)
                    {
                        rMb[i][j] = 0;
                        for (int32_t k = 0; k <= i; k++)
                        {
                            rMb[i][j] += f[k][j];
                        }
                    }
                }
            }
            else //if (horPredFlag == 1)
            {
                for (int32_t i = 0; i <= nH - 1; i++)
                {
                    for (int32_t j = 0; j <= nW - 1; j++)
                    {
                        rMb[i][j] = 0;
                        for (int32_t k = 0; k <= j; k++)
                        {
                            rMb[i][j] += f[i][k];
                        }
                    }
                }
            }
        }

        //--------------------------------------
        //--------帧内预测------------
//        ret = Intra_chroma_sample_prediction(pic_buff);
//        RETURN_IF_FAILED(ret != 0, ret);

        int32_t u[16 * 16] = { 0 };

        for (int32_t i = 0; i <= MbHeightC - 1; i++)
        {
            for (int32_t j = 0; j <= MbWidthC - 1; j++)
            {
                //uij = Clip1C( predC[ j, i ] + rMb[ j, i ] );
                //u[i * MbHeightC + j] = CLIP3(0, (1 << m_h264_slice_header.m_sps.BitDepthC) - 1, pic_buff[(mb_y * MbHeightC + i) * PicWidthInSamples + (mb_x * MbWidthC + j)] + rMb[i][j]);
                u[i * MbHeightC + j] = CLIP3(0, (1 << m_h264_slice_header.m_sps.BitDepthC) - 1, 
                    pic_buff[(((m_mbs[CurrMbAddr].m_mb_position_y >> 4) * MbHeightC) + (m_mbs[CurrMbAddr].m_mb_position_y % 2) + (i) * (1 + isMbAff)) * PicWidthInSamples 
                    + ((m_mbs[CurrMbAddr].m_mb_position_x >> 4 ) * MbWidthC + (j))] + rMb[i][j]);
            }
        }

        int32_t BlkIdx = 0;
        int32_t isChroma = 1;
        ret = Picture_construction_process_prior_to_deblocking_filter_process(u, MbWidthC, MbHeightC, BlkIdx, isChroma, PicWidthInSamples, pic_buff);
        RETURN_IF_FAILED(ret != 0, ret);
    }

    return 0;
}


//8.4 Inter prediction process
//This process is invoked when decoding P and B macroblock types.
int CH264PictureBase::Inter_prediction_process()
{
    int ret = 0;
    int32_t MvCnt = 0;

    CH264SliceHeader & slice_header = m_h264_slice_header;
    
    int32_t refIdxL0 = -1;
    int32_t refIdxL1 = -1;
    int32_t mvL0[2] = {0};
    int32_t mvL1[2] = {0};
    int32_t mvCL0[2] = {0};
    int32_t mvCL1[2] = {0};
    int32_t subMvCnt = 0;
    int32_t predFlagL0 = 0;
    int32_t predFlagL1 = 0;

    int32_t NumMbPart = 0;
    int32_t NumSubMbPart = 0;
    int32_t partWidth = 0;
    int32_t partHeight = 0;
    int32_t partWidthC = 0;
    int32_t partHeightC = 0;
    int32_t SubMbPartWidth = 0;
    int32_t SubMbPartHeight = 0;
    H264_MB_PART_PRED_MODE SubMbPredMode = MB_PRED_MODE_NA;

    int32_t i = 0;
    int32_t j = 0;

    CH264MacroBlock &mb = m_mbs[CurrMbAddr];
    
    int32_t isMbAff = (slice_header.MbaffFrameFlag == 1 && mb.mb_field_decoding_flag == 1) ? 1 : 0;

    //--------------------------------------------
    if (mb.m_name_of_mb_type == B_Skip || mb.m_name_of_mb_type == B_Direct_16x16)
    {
        NumMbPart = 4;
    }
    else
    {
        NumMbPart = mb.m_NumMbPart;
    }

    //--------------------------------------------
    for (int mbPartIdx = 0; mbPartIdx <= NumMbPart - 1; mbPartIdx++)
    {
        ret = CH264MacroBlock::SubMbPredModeFunc(m_h264_slice_header.slice_type, mb.sub_mb_type[ mbPartIdx ], 
                NumSubMbPart, SubMbPredMode, SubMbPartWidth, SubMbPartHeight);
        RETURN_IF_FAILED(ret != 0, ret);

        if (mb.m_name_of_mb_type != P_8x8 
            && mb.m_name_of_mb_type != P_8x8ref0 
            && mb.m_name_of_mb_type != B_Skip 
            && mb.m_name_of_mb_type != B_Direct_16x16 
            && mb.m_name_of_mb_type != B_8x8)
        {
            NumSubMbPart = 1;
            partWidth = mb.MbPartWidth;
            partHeight = mb.MbPartHeight;
        }
        else if (mb.m_name_of_mb_type == P_8x8 
                || mb.m_name_of_mb_type == P_8x8ref0 
                || (mb.m_name_of_mb_type == B_8x8 && mb.m_name_of_sub_mb_type[ mbPartIdx ] != B_Direct_8x8)
                )
        {
            NumSubMbPart = mb.NumSubMbPart[mbPartIdx];
            partWidth = mb.SubMbPartWidth[mbPartIdx];
            partHeight = mb.SubMbPartHeight[mbPartIdx];
        }
        else //if (mb.m_name_of_mb_type == B_Skip 
                //|| mb.m_name_of_mb_type == B_Direct_16x16 
                //|| (mb.m_name_of_mb_type == B_8x8 && mb.m_name_of_sub_mb_type[ mbPartIdx ] == B_Direct_8x8)
                //)
        {
            NumSubMbPart = 4;
            partWidth = 4;
            partHeight = 4;
        }

        //-----------------
        if (slice_header.m_sps.ChromaArrayType != 0)
        {
            partWidthC = partWidth / slice_header.m_sps.SubWidthC;
            partHeightC = partHeight / slice_header.m_sps.SubHeightC;
        }

        //--------------------------------------
        //6.4.2.1 Inverse macroblock partition scanning process
        int32_t xP = InverseRasterScan(mbPartIdx, mb.MbPartWidth, mb.MbPartHeight, 16, 0);
        int32_t yP = InverseRasterScan(mbPartIdx, mb.MbPartWidth, mb.MbPartHeight, 16, 1);
        
        //--------------------------
        for (int subMbPartIdx = 0; subMbPartIdx <= NumSubMbPart - 1; subMbPartIdx++)
        {
            //------------------------------------
            CH264PictureBase * refPicL0 = NULL;
            CH264PictureBase * refPicL1 = NULL;

            //1. The derivation process for motion vector components and reference indices as specified in clause 8.4.1 is invoked.
            //8.4.1 Derivation process for motion vector components and reference indices
            ret = Derivation_process_for_motion_vector_components_and_reference_indices(mbPartIdx, subMbPartIdx,
                refIdxL0, refIdxL1, mvL0, mvL1, mvCL0, mvCL1, subMvCnt, predFlagL0, predFlagL1, refPicL0, refPicL1);
            RETURN_IF_FAILED(ret != 0, ret);

            //2. The variable MvCnt is incremented by subMvCnt.
            MvCnt += subMvCnt;

            //3. When (weighted_pred_flag is equal to 1 and (slice_type % 5) is equal to 0 or 3) or (weighted_bipred_idc is greater 
            //than 0 and (slice_type % 5) is equal to 1), the derivation process for prediction weights as specified in clause 8.4.3 is invoked.
            int32_t logWDL = 0;
            int32_t w0L = 1;
            int32_t w1L = 1;
            int32_t o0L = 0;
            int32_t o1L = 0;
            int32_t logWDCb = 0;
            int32_t w0Cb = 1;
            int32_t w1Cb = 1;
            int32_t o0Cb = 0;
            int32_t o1Cb = 0;
            int32_t logWDCr = 0;
            int32_t w0Cr = 1;
            int32_t w1Cr = 1;
            int32_t o0Cr = 0;
            int32_t o1Cr = 0;

            if ((slice_header.m_pps.weighted_pred_flag == 1 && ((slice_header.slice_type % 5) == 0 || (slice_header.slice_type % 5) == 3)) //H264_SLIECE_TYPE_P=0
                || (slice_header.m_pps.weighted_bipred_idc > 0 && (slice_header.slice_type % 5) == 1) //H264_SLIECE_TYPE_B=1
                )
            {
                //8.4.3 Derivation process for prediction weights
                ret = Derivation_process_for_prediction_weights(refIdxL0, refIdxL1, predFlagL0, predFlagL1,
                    logWDL, w0L, w1L, o0L, o1L, logWDCb, w0Cb, w1Cb, o0Cb, o1Cb, logWDCr, w0Cr, w1Cr, o0Cr, o1Cr);
                RETURN_IF_FAILED(ret != 0, ret);
            }

            //4. The decoding process for Inter prediction samples as specified in clause 8.4.2 is invoked.
            uint8_t predPartL[16 * 16] = {0};
            uint8_t predPartCb[16 * 16] = {0};
            uint8_t predPartCr[16 * 16] = {0};
            int32_t xS = 0;
            int32_t yS = 0;

            //--------------------------------------
            //6.4.2.2 Inverse sub-macroblock partition scanning process
            //int32_t xS = InverseRasterScan( subMbPartIdx, mb.SubMbPartWidth[subMbPartIdx], mb.SubMbPartHeight[subMbPartIdx], 8, 0 );
            //int32_t yS = InverseRasterScan( subMbPartIdx, mb.SubMbPartWidth[subMbPartIdx], mb.SubMbPartHeight[subMbPartIdx], 8, 1 );
            
            if (mb.m_name_of_mb_type == P_8x8 
                || mb.m_name_of_mb_type == P_8x8ref0 
                || mb.m_name_of_mb_type == B_8x8
               )
            {
                xS = InverseRasterScan( subMbPartIdx, partWidth, partHeight, 8, 0 );
                yS = InverseRasterScan( subMbPartIdx, partWidth, partHeight, 8, 1 );
            }
            else
            {
                xS = InverseRasterScan( subMbPartIdx, 4, 4, 8, 0 );
                yS = InverseRasterScan( subMbPartIdx, 4, 4, 8, 1 );
            }

            int32_t xAL = mb.m_mb_position_x + xP + xS;
            int32_t yAL = mb.m_mb_position_y + yP + yS;

            if (isMbAff)
            {
                yAL = (mb.m_mb_position_y / 2 + (yP + yS));
            }

            //8.4.2 Decoding process for Inter prediction samples
            ret = Decoding_process_for_Inter_prediction_samples(mbPartIdx, subMbPartIdx, partWidth, partHeight, partWidthC, partHeightC,
                        xAL, yAL,
                        mvL0, mvL1, mvCL0, mvCL1, refPicL0, refPicL1, predFlagL0, predFlagL1,
                        logWDL, w0L, w1L, o0L, o1L,
                        logWDCb, w0Cb, w1Cb, o0Cb, o1Cb,
                        logWDCr, w0Cr, w1Cr, o0Cr, o1Cr,
                        predPartL, predPartCb, predPartCr);
            RETURN_IF_FAILED(ret != 0, -1);

            //----------------------------------
            mb.m_MvL0[ mbPartIdx ][ subMbPartIdx ][0] = mvL0[0];
            mb.m_MvL0[ mbPartIdx ][ subMbPartIdx ][1] = mvL0[1];

            mb.m_MvL1[ mbPartIdx ][ subMbPartIdx ][0] = mvL1[0];
            mb.m_MvL1[ mbPartIdx ][ subMbPartIdx ][1] = mvL1[1];

            mb.m_RefIdxL0[ mbPartIdx ] = refIdxL0;
            mb.m_RefIdxL1[ mbPartIdx ] = refIdxL1;

            mb.m_PredFlagL0[ mbPartIdx ] = predFlagL0;
            mb.m_PredFlagL1[ mbPartIdx ] = predFlagL1;

            //--------------------------------------
            if (mb.mb_field_decoding_flag == 0)
            {
                for (i = 0; i <= partHeight - 1; i++)
                {
                    for (j = 0; j <= partWidth - 1; j++)
                    {
                        //predL[ xP + xS + x, yP + yS + y ] = predPartL[ x, y ];
                        m_pic_buff_luma[(mb_y * MbHeightL + yP + yS + i) * PicWidthInSamplesL + (mb_x * MbWidthL + xP + xS + j)] = predPartL[i * partWidth + j];
                    }
                }

                if (slice_header.m_sps.ChromaArrayType != 0)
                {
                    for (i = 0; i <= partHeightC - 1; i++)
                    {
                        for (j = 0; j <= partWidthC - 1; j++)
                        {
                            //predC[ xP / SubWidthC + xS / SubWidthC + x, yP / SubHeightC + yS / SubHeightC + y ] = predPartC[ x, y ];
                            m_pic_buff_cb[(mb_y * MbHeightC + yP / slice_header.m_sps.SubHeightC + yS / slice_header.m_sps.SubHeightC + i) * PicWidthInSamplesC 
                                + (mb_x * MbWidthC + xP / slice_header.m_sps.SubWidthC + xS / slice_header.m_sps.SubWidthC + j)] = predPartCb[i * partWidthC + j];

                            m_pic_buff_cr[(mb_y * MbHeightC + yP / slice_header.m_sps.SubHeightC + yS / slice_header.m_sps.SubHeightC + i) * PicWidthInSamplesC 
                                + (mb_x * MbWidthC + xP / slice_header.m_sps.SubWidthC + xS / slice_header.m_sps.SubWidthC + j)] = predPartCr[i * partWidthC + j];
                        }
                    }
                }
            }
            else //if (mb.mb_field_decoding_flag == 1)
            {
                for (i = 0; i <= partHeight - 1; i++)
                {
                    for (j = 0; j <= partWidth - 1; j++)
                    {
                        //predL[ xP + xS + x, yP + yS + y ] = predPartL[ x, y ];
                        m_pic_buff_luma[(mb_y % 2) * PicWidthInSamplesL + ((mb_y / 2) * MbHeightL + yP + yS + i) * PicWidthInSamplesL * 2 + (mb_x * MbWidthL + xP + xS + j)] = predPartL[i * partWidth + j];
                    }
                }

                if (slice_header.m_sps.ChromaArrayType != 0)
                {
                    for (i = 0; i <= partHeightC - 1; i++)
                    {
                        for (j = 0; j <= partWidthC - 1; j++)
                        {
                            //predC[ xP / SubWidthC + xS / SubWidthC + x, yP / SubHeightC + yS / SubHeightC + y ] = predPartC[ x, y ];
                            m_pic_buff_cb[(mb_y % 2) * PicWidthInSamplesC + ((mb_y / 2) * MbHeightC + yP / slice_header.m_sps.SubHeightC + yS / slice_header.m_sps.SubHeightC + i) * PicWidthInSamplesC * 2 
                                + (mb_x * MbWidthC + xP / slice_header.m_sps.SubWidthC + xS / slice_header.m_sps.SubWidthC + j)] = predPartCb[i * partWidthC + j];

                            m_pic_buff_cr[(mb_y % 2) * PicWidthInSamplesC + ((mb_y / 2) * MbHeightC + yP / slice_header.m_sps.SubHeightC + yS / slice_header.m_sps.SubHeightC + i) * PicWidthInSamplesC * 2 
                                + (mb_x * MbWidthC + xP / slice_header.m_sps.SubWidthC + xS / slice_header.m_sps.SubWidthC + j)] = predPartCr[i * partWidthC + j];
                        }
                    }
                }
            }

            //-------------------
            mb.m_isDecoded[mbPartIdx][subMbPartIdx] = 1; //标记为已解码
        }
    }

    return 0;
}


//8.4.1 Derivation process for motion vector components and reference indices
int CH264PictureBase::Derivation_process_for_motion_vector_components_and_reference_indices(int32_t mbPartIdx, int32_t subMbPartIdx,
        int32_t &refIdxL0, int32_t &refIdxL1, int32_t (&mvL0)[2], int32_t (&mvL1)[2], int32_t (&mvCL0)[2], int32_t (&mvCL1)[2], 
        int32_t &subMvCnt, int32_t &predFlagL0, int32_t &predFlagL1, CH264PictureBase *&refPicL0, CH264PictureBase *&refPicL1)
{
    int ret = 0;

    CH264SliceHeader & slice_header = m_h264_slice_header;
    
    int32_t mvpL0[2] = {0};
    int32_t mvpL1[2] = {0};
    H264_MB_TYPE currSubMbType = MB_TYPE_NA;
    int32_t listSuffixFlag = 0;
    int32_t refPicLSetFlag = 0;

    if (m_mbs[CurrMbAddr].m_name_of_mb_type == P_Skip)
    {
        //8.4.1.1 Derivation process for luma motion vectors for skipped macroblocks in P and SP slices
        refIdxL0 = 0;
        m_mbs[CurrMbAddr].m_PredFlagL0[0] = 1;
        m_mbs[CurrMbAddr].m_PredFlagL0[1] = 1;
        m_mbs[CurrMbAddr].m_PredFlagL0[2] = 1;
        m_mbs[CurrMbAddr].m_PredFlagL0[3] = 1;

        int32_t mbAddrN_A= 0;
        int32_t mvLXN_A[2] = {0};
        int32_t refIdxLXN_A = 0;
        
        int32_t mbAddrN_B= 0;
        int32_t mvLXN_B[2] = {0};
        int32_t refIdxLXN_B = 0;
        
        int32_t mbAddrN_C= 0;
        int32_t mvLXN_C[2] = {0};
        int32_t refIdxLXN_C = 0;

        //8.4.1.3.2 Derivation process for motion data of neighbouring partitions
        ret = Derivation_process_for_motion_data_of_neighbouring_partitions(mbPartIdx, subMbPartIdx, currSubMbType, listSuffixFlag, 
                mbAddrN_A, mvLXN_A, refIdxLXN_A, mbAddrN_B, mvLXN_B, refIdxLXN_B, mbAddrN_C, mvLXN_C, refIdxLXN_C);
        RETURN_IF_FAILED(ret != 0, ret);

        //--------------------------------------
        if (mbAddrN_A < 0 || mbAddrN_B < 0
            || (refIdxLXN_A == 0 && mvLXN_A[0] == 0 && mvLXN_A[1] == 0)
            || (refIdxLXN_B == 0 && mvLXN_B[0] == 0 && mvLXN_B[1] == 0)
            )
        {
            mvL0[0] = 0;
            mvL0[1] = 0;
        }
        else
        {
            //the derivation process for luma motion vector prediction as specified in clause 8.4.1.3 is invoked 
            //with mbPartIdx = 0, subMbPartIdx = 0, refIdxL0, and currSubMbType = "na" as inputs and the output is 
            //assigned to mvL0.

            //8.4.1.3 Derivation process for luma motion vector prediction
            ret = Derivation_process_for_luma_motion_vector_prediction(mbPartIdx, subMbPartIdx, currSubMbType, listSuffixFlag, refIdxL0, mvL0);
            RETURN_IF_FAILED(ret != 0, ret);
        }

        //and predFlagL0 is set equal to 1. mvL1 and refIdxL1 are marked as not available and predFlagL1 
        //is set equal to 0. The motion vector count variable subMvCnt is set equal to 1.
        predFlagL0 = 1;
        predFlagL1 = 0;
        mvL1[0] = NA;
        mvL1[1] = NA;
        subMvCnt = 1;
    }
    else if (m_mbs[CurrMbAddr].m_name_of_mb_type == B_Skip 
            || m_mbs[CurrMbAddr].m_name_of_mb_type == B_Direct_16x16 
            || m_mbs[CurrMbAddr].m_name_of_sub_mb_type[ mbPartIdx ] == B_Direct_8x8
            )
    {
        //8.4.1.2 Derivation process for luma motion vectors for B_Skip, B_Direct_16x16, and B_Direct_8x8
        ret = Derivation_process_for_luma_motion_vectors_for_B_Skip_or_B_Direct_16x16_or_B_Direct_8x8(mbPartIdx, subMbPartIdx,
                    refIdxL0, refIdxL1, mvL0, mvL1, subMvCnt, predFlagL0, predFlagL1);
        RETURN_IF_FAILED(ret != 0, ret);
    }
    else
    {
        int32_t NumSubMbPart = 0;
        H264_MB_PART_PRED_MODE SubMbPredMode = MB_PRED_MODE_NA;
        int32_t SubMbPartWidth = 0;
        int32_t SubMbPartHeight = 0;
        H264_MB_PART_PRED_MODE mb_pred_mode = MB_PRED_MODE_NA;

        ret = CH264MacroBlock::MbPartPredMode2(m_mbs[CurrMbAddr].m_name_of_mb_type, mbPartIdx, m_mbs[CurrMbAddr].transform_size_8x8_flag, mb_pred_mode);
        RETURN_IF_FAILED(ret != 0, ret);

        ret = CH264MacroBlock::SubMbPredModeFunc(m_h264_slice_header.slice_type, m_mbs[CurrMbAddr].sub_mb_type[ mbPartIdx ], 
                NumSubMbPart, SubMbPredMode, SubMbPartWidth, SubMbPartHeight);
        RETURN_IF_FAILED(ret != 0, ret);

        //If MbPartPredMode( mb_type, mbPartIdx ) or SubMbPredMode( sub_mb_type[ mbPartIdx ] ) is equal to Pred_LX or to BiPred,
        if (mb_pred_mode == Pred_L0 || mb_pred_mode == BiPred
           || SubMbPredMode == Pred_L0 || SubMbPredMode == BiPred
          )
        {
            refIdxL0 = m_mbs[CurrMbAddr].ref_idx_l0[ mbPartIdx ];
            predFlagL0 = 1;
        }
        else
        {
            refIdxL0 = -1;
            predFlagL0 = 0;
        }
        
        //If MbPartPredMode( mb_type, mbPartIdx ) or SubMbPredMode( sub_mb_type[ mbPartIdx ] ) is equal to Pred_LX or to BiPred,
        if (mb_pred_mode == Pred_L1 || mb_pred_mode == BiPred
           || SubMbPredMode == Pred_L1 || SubMbPredMode == BiPred
           )
        {
            refIdxL1 = m_mbs[CurrMbAddr].ref_idx_l1[ mbPartIdx ];
            predFlagL1 = 1;
        }
        else
        {
            refIdxL1 = -1;
            predFlagL1 = 0;
        }

        subMvCnt = predFlagL0 + predFlagL1;

        if (m_mbs[CurrMbAddr].m_name_of_mb_type == B_8x8)
        {
            currSubMbType = m_mbs[CurrMbAddr].m_name_of_sub_mb_type[ mbPartIdx ];
        }
        else
        {
            currSubMbType = MB_TYPE_NA;
        }

        //-----------------------
        if (predFlagL0 == 1)
        {
            refPicLSetFlag = 1;
            refPicL0 = NULL;

            //8.4.2.1 Reference picture selection process
            ret = Reference_picture_selection_process(refIdxL0, m_RefPicList0, m_RefPicList0Length, refPicL0);
            RETURN_IF_FAILED(ret, -1);

            //---------------------
            listSuffixFlag = 0;

            //8.4.1.3 Derivation process for luma motion vector prediction
            ret = Derivation_process_for_luma_motion_vector_prediction(mbPartIdx, subMbPartIdx, currSubMbType, listSuffixFlag, refIdxL0, mvpL0);
            RETURN_IF_FAILED(ret != 0, ret);

            mvL0[ 0 ] = mvpL0[ 0 ] + m_mbs[CurrMbAddr].mvd_l0[ mbPartIdx ][ subMbPartIdx ][ 0 ];
            mvL0[ 1 ] = mvpL0[ 1 ] + m_mbs[CurrMbAddr].mvd_l0[ mbPartIdx ][ subMbPartIdx ][ 1 ];
        }
        
        if (predFlagL1 == 1)
        {
            refPicLSetFlag = 1;
            refPicL1 = NULL;

            //8.4.2.1 Reference picture selection process
            ret = Reference_picture_selection_process(refIdxL1, m_RefPicList1, m_RefPicList1Length, refPicL1);
            RETURN_IF_FAILED(ret, -1);
            
            //---------------------
            listSuffixFlag = 1;

            //8.4.1.3 Derivation process for luma motion vector prediction
            ret = Derivation_process_for_luma_motion_vector_prediction(mbPartIdx, subMbPartIdx, currSubMbType, listSuffixFlag, refIdxL1, mvpL1);
            RETURN_IF_FAILED(ret != 0, ret);

            mvL1[ 0 ] = mvpL1[ 0 ] + m_mbs[CurrMbAddr].mvd_l1[ mbPartIdx ][ subMbPartIdx ][ 0 ];
            mvL1[ 1 ] = mvpL1[ 1 ] + m_mbs[CurrMbAddr].mvd_l1[ mbPartIdx ][ subMbPartIdx ][ 1 ];
        }
    }
    
    //-----------------------------
    if (refPicLSetFlag == 0)
    {
        if (predFlagL0 == 1)
        {
            refPicLSetFlag = 1;
            refPicL0 = NULL;

            //8.4.2.1 Reference picture selection process
            ret = Reference_picture_selection_process(refIdxL0, m_RefPicList0, m_RefPicList0Length, refPicL0);
            RETURN_IF_FAILED(ret, -1);
        }
        
        if (predFlagL1 == 1)
        {
            refPicLSetFlag = 1;
            refPicL1 = NULL;

            //8.4.2.1 Reference picture selection process
            ret = Reference_picture_selection_process(refIdxL1, m_RefPicList1, m_RefPicList1Length, refPicL1);
            RETURN_IF_FAILED(ret, -1);
        }
    }

    //-----------------------------
    if (slice_header.m_sps.ChromaArrayType != 0)
    {
        if (predFlagL0 == 1)
        {
            //8.4.1.4 Derivation process for chroma motion vectors
            ret = Derivation_process_for_chroma_motion_vectors(slice_header.m_sps.ChromaArrayType, mvL0, refPicL0, mvCL0);
            RETURN_IF_FAILED(ret != 0, ret);
        }

        if (predFlagL1 == 1)
        {
            //8.4.1.4 Derivation process for chroma motion vectors
            ret = Derivation_process_for_chroma_motion_vectors(slice_header.m_sps.ChromaArrayType, mvL1, refPicL1, mvCL1);
            RETURN_IF_FAILED(ret != 0, ret);
        }
    }

    return 0;
}


//8.4.1.2 Derivation process for luma motion vectors for B_Skip, B_Direct_16x16, and B_Direct_8x8
int CH264PictureBase::Derivation_process_for_luma_motion_vectors_for_B_Skip_or_B_Direct_16x16_or_B_Direct_8x8(int32_t mbPartIdx, int32_t subMbPartIdx,
        int32_t &refIdxL0, int32_t &refIdxL1, int32_t (&mvL0)[2], int32_t (&mvL1)[2], int32_t &subMvCnt, int32_t &predFlagL0, int32_t &predFlagL1)
{
    int ret = 0;
    
    CH264SliceHeader & slice_header = m_h264_slice_header;

    //-----------------------------------
    if (slice_header.direct_spatial_mv_pred_flag == 1) //the mode in which the outputs of this process are derived is referred to as spatial direct prediction mode.
    {
        //8.4.1.2.2 Derivation process for spatial direct luma motion vector and reference index prediction mode
        ret = Derivation_process_for_spatial_direct_luma_motion_vector_and_reference_index_prediction_mode(mbPartIdx, subMbPartIdx,
                    refIdxL0, refIdxL1, mvL0, mvL1, subMvCnt, predFlagL0, predFlagL1);
        RETURN_IF_FAILED(ret != 0, ret);
    }
    else //(temporal direct prediction mode is used) mode in which the outputs of this process are derived is referred to as temporal direct prediction mode.
    {
        //8.4.1.2.3 Derivation process for temporal direct luma motion vector and reference index prediction mode
        ret = Derivation_process_for_temporal_direct_luma_motion_vector_and_reference_index_prediction_mode(mbPartIdx, subMbPartIdx,
                    refIdxL0, refIdxL1, mvL0, mvL1, subMvCnt, predFlagL0, predFlagL1);
        RETURN_IF_FAILED(ret != 0, ret);

        if (subMbPartIdx == 0)
        {
            subMvCnt = 2;
        }
        else
        {
            subMvCnt = 0;
        }
    }
    
    return 0;
}


//8.4.1.2.1 Derivation process for the co-located 4x4 sub-macroblock partitions
int CH264PictureBase::Derivation_process_for_the_co_located_4x4_sub_macroblock_partitions(int32_t mbPartIdx, int32_t subMbPartIdx, 
        CH264PictureBase *&colPic, int32_t &mbAddrCol, int32_t (&mvCol)[2], int32_t &refIdxCol, int32_t &vertMvScale)
{
    int ret = 0;
    
    CH264SliceHeader & slice_header = m_h264_slice_header;

    CH264PictureBase * firstRefPicL1Top = NULL;
    CH264PictureBase * firstRefPicL1Bottom = NULL;
    
    int32_t topAbsDiffPOC = 0;
    int32_t bottomAbsDiffPOC = 0;

    if (m_RefPicList1[0]->m_picture_coded_type_marked_as_refrence == H264_PICTURE_CODED_TYPE_FRAME
        || m_RefPicList1[0]->m_picture_coded_type_marked_as_refrence == H264_PICTURE_CODED_TYPE_COMPLEMENTARY_FIELD_PAIR
      )
    {
        firstRefPicL1Top = &m_RefPicList1[0]->m_picture_top_filed;
        firstRefPicL1Bottom = &m_RefPicList1[0]->m_picture_bottom_filed;

        topAbsDiffPOC = ABS( DiffPicOrderCnt( firstRefPicL1Top, this ) );
        bottomAbsDiffPOC = ABS( DiffPicOrderCnt( firstRefPicL1Bottom, this ) );
    }
    else
    {
        //
    }

    //------------------------------
    //Table 8-6 – Specification of the variable colPic
    colPic = NULL;

    if (slice_header.field_pic_flag == 1)
    {
        if (m_RefPicList1[0]->m_is_decode_finished == 1 //RefPicList1[0] is a field of a decoded frame
           && (m_RefPicList1[0]->m_picture_coded_type_marked_as_refrence == H264_PICTURE_CODED_TYPE_TOP_FIELD
               || m_RefPicList1[0]->m_picture_coded_type_marked_as_refrence == H264_PICTURE_CODED_TYPE_BOTTOM_FIELD
              )
          )
        {
            colPic = &m_RefPicList1[0]->m_picture_frame;
        }
        else //RefPicList1[0] is a decoded field
        {
            if (m_RefPicList1[0]->m_picture_coded_type_marked_as_refrence == H264_PICTURE_CODED_TYPE_TOP_FIELD)
            {
                colPic = &m_RefPicList1[0]->m_picture_top_filed;
            }
            else if (m_RefPicList1[0]->m_picture_coded_type_marked_as_refrence == H264_PICTURE_CODED_TYPE_BOTTOM_FIELD)
            {
                colPic = &m_RefPicList1[0]->m_picture_bottom_filed;
            }
            else
            {
                //
            }
        }
    }
    else //if (slice_header.field_pic_flag == 0)
    {
        if (m_RefPicList1[0]->m_is_decode_finished == 1 //RefPicList1[0] is a decoded frame
            && m_RefPicList1[0]->m_picture_coded_type_marked_as_refrence == H264_PICTURE_CODED_TYPE_FRAME
          )
        {
            colPic = &m_RefPicList1[0]->m_picture_frame;
        }
        else if (m_RefPicList1[0]->m_picture_coded_type_marked_as_refrence == H264_PICTURE_CODED_TYPE_COMPLEMENTARY_FIELD_PAIR)
        {
            if (m_h264_slice_data.mb_field_decoding_flag == 0)
            {
                if (topAbsDiffPOC < bottomAbsDiffPOC)
                {
                    colPic = firstRefPicL1Top;
                }
                else //if (topAbsDiffPOC >= bottomAbsDiffPOC)
                {
                    colPic = firstRefPicL1Bottom;
                }
            }
            else //if (m_h264_slice_data.mb_field_decoding_flag == 1)
            {
                if (( CurrMbAddr & 1 ) == 0)
                {
                    colPic = firstRefPicL1Top;
                }
                else //(CurrMbAddr & 1 ) != 0
                {
                    colPic = firstRefPicL1Bottom;
                }
            }
        }
    }

    RETURN_IF_FAILED(colPic == NULL, -1);

    //----------------------------------------------------
    //Table 8-7 – Specification of PicCodingStruct( X )
    int32_t PicCodingStruct_CurrPic = FLD;

    if (slice_header.field_pic_flag == 1)
    {
        PicCodingStruct_CurrPic = FLD;
    }
    else //if (slice_header.field_pic_flag == 0)
    {
        if (slice_header.m_sps.mb_adaptive_frame_field_flag == 0)
        {
            PicCodingStruct_CurrPic = FRM;
        }
        else //if (slice_header.m_sps.mb_adaptive_frame_field_flag == 1)
        {
            PicCodingStruct_CurrPic = AFRM;
        }
    }
    
    //----------------------------------------------------
    //Table 8-7 – Specification of PicCodingStruct( X )
    int32_t PicCodingStruct_colPic = FLD;

    if (colPic->m_h264_slice_header.field_pic_flag == 1)
    {
        PicCodingStruct_colPic = FLD;
    }
    else //if (colPic->m_slice_header.field_pic_flag == 0)
    {
        if (colPic->m_h264_slice_header.m_sps.mb_adaptive_frame_field_flag == 0)
        {
            PicCodingStruct_colPic = FRM;
        }
        else //if (colPic->m_h264_slice_header.m_sps.mb_adaptive_frame_field_flag == 1)
        {
            PicCodingStruct_colPic = AFRM;
        }
    }

    //NOTE – It is not possible for CurrPic and colPic picture coding types to be 
    //either (FRM, AFRM) or (AFRM, FRM) because these picture coding types must be 
    //separated by an IDR picture.
    RETURN_IF_FAILED(PicCodingStruct_CurrPic == FRM && PicCodingStruct_colPic == AFRM, -1);
    RETURN_IF_FAILED(PicCodingStruct_CurrPic == AFRM && PicCodingStruct_colPic == FRM, -1);

    //--------------------------------
    int32_t luma4x4BlkIdx = 0;

    if (slice_header.m_sps.direct_8x8_inference_flag == 0)
    {
        luma4x4BlkIdx = (4 * mbPartIdx + subMbPartIdx);
    }
    else //if (slice_header.m_sps.direct_8x8_inference_flag == 1)
    {
        luma4x4BlkIdx = 5 * mbPartIdx;
    }

    //------------------------------------------------------
    //6.4.3 Inverse 4x4 luma block scanning process
    //InverseRasterScan = (a % (d / b) ) * b;    if e == 0;
    //InverseRasterScan = (a / (d / b) ) * c;    if e == 1;
    int32_t xCol = InverseRasterScan(luma4x4BlkIdx / 4, 8, 8, 16, 0) + InverseRasterScan(luma4x4BlkIdx % 4, 4, 4, 8, 0);
    int32_t yCol = InverseRasterScan(luma4x4BlkIdx / 4, 8, 8, 16, 1) + InverseRasterScan(luma4x4BlkIdx % 4, 4, 4, 8, 1);

    //-----------------
    int32_t fieldDecodingFlagX = 0;
    mbAddrCol = CurrMbAddr;
    int32_t yM = 0;
    vertMvScale = H264_VERT_MV_SCALE_UNKNOWN;

    //Table 8-8 – Specification of mbAddrCol, yM, and vertMvScale
    if (PicCodingStruct_CurrPic == FLD)
    {
        if (PicCodingStruct_colPic == FLD)
        {
            mbAddrCol = CurrMbAddr;
            yM = yCol;
            vertMvScale = H264_VERT_MV_SCALE_One_To_One;
        }
        else if (PicCodingStruct_colPic == FRM)
        {
            int32_t mbAddrCol1 = 2 * PicWidthInMbs * ( CurrMbAddr / PicWidthInMbs ) + ( CurrMbAddr % PicWidthInMbs ) + PicWidthInMbs * ( yCol / 8 );
            mbAddrCol = mbAddrCol1;
            yM = ( 2 * yCol ) % 16;
            vertMvScale = H264_VERT_MV_SCALE_Frm_To_Fld;
        }
        else if (PicCodingStruct_colPic == AFRM)
        {
            int32_t mbAddrX = 2 * CurrMbAddr;
            if (colPic->m_mbs[mbAddrX].mb_field_decoding_flag == 0) //Otherwise (the macroblock mbAddrX in the picture colPic is a frame macroblock), fieldDecodingFlagX is set equal to 0.
            {
                fieldDecodingFlagX = 0;
                int32_t mbAddrCol2 = 2 * CurrMbAddr + ( yCol / 8 );
                mbAddrCol = mbAddrCol2;
                yM = ( 2 * yCol ) % 16;
                vertMvScale = H264_VERT_MV_SCALE_Frm_To_Fld;
            }
            else //if (colPic->m_mbs[mbAddrX].mb_field_decoding_flag == 1) //If the macroblock mbAddrX in the picture colPic is a field macroblock, fieldDecodingFlagX is set equal to 1.
            {
                fieldDecodingFlagX = 1;
                int32_t mbAddrCol3 = 2 * CurrMbAddr + this->m_mbs[CurrMbAddr].bottom_field_flag;
                mbAddrCol = mbAddrCol3;
                yM = yCol;
                vertMvScale = H264_VERT_MV_SCALE_One_To_One;
            }
        }
    }
    else if (PicCodingStruct_CurrPic == FRM)
    {
        if (PicCodingStruct_colPic == FLD)
        {
            int32_t mbAddrCol4 = PicWidthInMbs * ( CurrMbAddr / ( 2 * PicWidthInMbs ) ) + ( CurrMbAddr % PicWidthInMbs );
            mbAddrCol = mbAddrCol4;
            yM = 8 * ( (CurrMbAddr / PicWidthInMbs ) % 2) + 4 * ( yCol / 8 );
            vertMvScale = H264_VERT_MV_SCALE_Fld_To_Frm;
        }
        else if (PicCodingStruct_colPic == FRM)
        {
            mbAddrCol = CurrMbAddr;
            yM = yCol;
            vertMvScale = H264_VERT_MV_SCALE_One_To_One;
        }
    }
    else if (PicCodingStruct_CurrPic == AFRM)
    {
        if (PicCodingStruct_colPic == FLD)
        {
            int32_t mbAddrCol5 = CurrMbAddr / 2;
            if (m_h264_slice_data.mb_field_decoding_flag == 0)
            {
                mbAddrCol = mbAddrCol5;
                yM = 8 * ( CurrMbAddr % 2 ) + 4 * ( yCol / 8 );
                vertMvScale = H264_VERT_MV_SCALE_Fld_To_Frm;
            }
            else //if (m_h264_slice_data.mb_field_decoding_flag == 1)
            {
                mbAddrCol = mbAddrCol5;
                yM = yCol;
                vertMvScale = H264_VERT_MV_SCALE_One_To_One;
            }
        }
        else if (PicCodingStruct_colPic == AFRM)
        {
            int32_t mbAddrX = CurrMbAddr;
            if (m_h264_slice_data.mb_field_decoding_flag == 0)
            {
                if (colPic->m_mbs[mbAddrX].mb_field_decoding_flag == 0) //Otherwise (the macroblock mbAddrX in the picture colPic is a frame macroblock), fieldDecodingFlagX is set equal to 0.
                {
                    fieldDecodingFlagX = 0;
                    mbAddrCol = CurrMbAddr;
                    yM = yCol;
                    vertMvScale = H264_VERT_MV_SCALE_One_To_One;
                }
                else //if (colPic->m_mbs[mbAddrX].mb_field_decoding_flag == 1) //If the macroblock mbAddrX in the picture colPic is a field macroblock, fieldDecodingFlagX is set equal to 1.
                {
                    fieldDecodingFlagX = 1;
                    int32_t mbAddrCol6 = 2 * ( CurrMbAddr / 2 ) + ( ( topAbsDiffPOC < bottomAbsDiffPOC ) ? 0 : 1 );
                    mbAddrCol = mbAddrCol6;
                    yM = 8 * ( CurrMbAddr % 2 ) + 4 * ( yCol / 8 );
                    vertMvScale = H264_VERT_MV_SCALE_Fld_To_Frm;
                }
            }
            else //if (m_h264_slice_data.mb_field_decoding_flag == 1)
            {
                if (colPic->m_mbs[mbAddrX].mb_field_decoding_flag == 0) //Otherwise (the macroblock mbAddrX in the picture colPic is a frame macroblock), fieldDecodingFlagX is set equal to 0.
                {
                    fieldDecodingFlagX = 0;
                    int32_t mbAddrCol7 = 2 * ( CurrMbAddr / 2 ) + ( yCol / 8 );
                    mbAddrCol = mbAddrCol7;
                    yM = ( 2 * yCol ) % 16;
                    vertMvScale = H264_VERT_MV_SCALE_Frm_To_Fld;
                }
                else //if (colPic->m_mbs[mbAddrX].mb_field_decoding_flag == 1) //If the macroblock mbAddrX in the picture colPic is a field macroblock, fieldDecodingFlagX is set equal to 1.
                {
                    fieldDecodingFlagX = 1;
                    mbAddrCol = CurrMbAddr;
                    yM = yCol;
                    vertMvScale = H264_VERT_MV_SCALE_One_To_One;
                }
            }
        }
    }

    //--------------------------
    H264_MB_TYPE mbTypeCol = colPic->m_mbs[mbAddrCol].m_name_of_mb_type;
    H264_MB_TYPE subMbTypeCol[4] = {MB_TYPE_NA, MB_TYPE_NA, MB_TYPE_NA, MB_TYPE_NA};

    if (mbTypeCol == P_8x8 || mbTypeCol ==  P_8x8ref0 || mbTypeCol == B_8x8)
    {
        subMbTypeCol[0] = colPic->m_mbs[mbAddrCol].m_name_of_sub_mb_type[0];
        subMbTypeCol[1] = colPic->m_mbs[mbAddrCol].m_name_of_sub_mb_type[1];
        subMbTypeCol[2] = colPic->m_mbs[mbAddrCol].m_name_of_sub_mb_type[2];
        subMbTypeCol[3] = colPic->m_mbs[mbAddrCol].m_name_of_sub_mb_type[3];
    }

    int32_t mbPartIdxCol = 0;
    int32_t subMbPartIdxCol = 0;

    //6.4.13.4 Derivation process for macroblock and sub-macroblock partition indices
    ret = Derivation_process_for_macroblock_and_sub_macroblock_partition_indices(mbTypeCol, subMbTypeCol, xCol, yM, mbPartIdxCol, subMbPartIdxCol);
    RETURN_IF_FAILED(ret != 0, ret);

    //-----------------------------
    refIdxCol = -1;

    if (IS_INTRA_Prediction_Mode(colPic->m_mbs[mbAddrCol].m_mb_pred_mode) == true)
    {
        mvCol[0] = 0;
        mvCol[1] = 0;
        refIdxCol = -1;
    }
    else
    {
        int32_t predFlagL0Col = colPic->m_mbs[mbAddrCol].m_PredFlagL0[ mbPartIdxCol ];
        int32_t predFlagL1Col = colPic->m_mbs[mbAddrCol].m_PredFlagL1[ mbPartIdxCol ];

        if (predFlagL0Col == 1)
        {
            mvCol[0] = colPic->m_mbs[mbAddrCol].m_MvL0[ mbPartIdxCol ][ subMbPartIdxCol ][0]; //MvL0[ mbPartIdxCol ][ subMbPartIdxCol ]
            mvCol[1] = colPic->m_mbs[mbAddrCol].m_MvL0[ mbPartIdxCol ][ subMbPartIdxCol ][1];
            refIdxCol = colPic->m_mbs[mbAddrCol].m_RefIdxL0[ mbPartIdxCol ]; //RefIdxL0[ mbPartIdxCol ]
        }
        else //if (predFlagL0Col == 0 && predFlagL1Col == 1)
        {
            mvCol[0] = colPic->m_mbs[mbAddrCol].m_MvL1[ mbPartIdxCol ][ subMbPartIdxCol ][0]; //MvL1[ mbPartIdxCol ][ subMbPartIdxCol ]
            mvCol[1] = colPic->m_mbs[mbAddrCol].m_MvL1[ mbPartIdxCol ][ subMbPartIdxCol ][1];
            refIdxCol = colPic->m_mbs[mbAddrCol].m_RefIdxL1[ mbPartIdxCol ]; //RefIdxL1[ mbPartIdxCol ]
        }
    }

    return 0;
}


//8.4.1.2.2 Derivation process for spatial direct luma motion vector and reference index prediction mode
int CH264PictureBase::Derivation_process_for_spatial_direct_luma_motion_vector_and_reference_index_prediction_mode(int32_t mbPartIdx, int32_t subMbPartIdx,
        int32_t &refIdxL0, int32_t &refIdxL1, int32_t (&mvL0)[2], int32_t (&mvL1)[2], int32_t &subMvCnt, int32_t &predFlagL0, int32_t &predFlagL1)
{
    int ret = 0;
    
    //--------------
    H264_MB_TYPE currSubMbType = m_mbs[CurrMbAddr].m_name_of_sub_mb_type[ mbPartIdx ];
    
    int32_t mbAddrA = 0;
    int32_t mvL0A[2] = { 0 };
    int32_t refIdxL0A = 0;

    int32_t mbAddrB = 0;
    int32_t mvL0B[2] = { 0 };
    int32_t refIdxL0B = 0;

    int32_t mbAddrC = 0;
    int32_t mvL0C[2] = { 0 };
    int32_t refIdxL0C = 0;

    int32_t mbPartIdx_ = 0;
    int32_t subMbPartIdx_ = 0;
    int32_t listSuffixFlag = 0;
    
    //8.4.1.3.2 Derivation process for motion data of neighbouring partitions
    ret = Derivation_process_for_motion_data_of_neighbouring_partitions(mbPartIdx_, subMbPartIdx_, currSubMbType, listSuffixFlag,
        mbAddrA, mvL0A, refIdxL0A, mbAddrB, mvL0B, refIdxL0B, mbAddrC, mvL0C, refIdxL0C);
    RETURN_IF_FAILED(ret != 0, ret);
    
    //--------------
    mbAddrA = 0;
    int32_t mvL1A[2] = { 0 };
    int32_t refIdxL1A = 0;

    mbAddrB = 0;
    int32_t mvL1B[2] = { 0 };
    int32_t refIdxL1B = 0;

    mbAddrC = 0;
    int32_t mvL1C[2] = { 0 };
    int32_t refIdxL1C = 0;

    listSuffixFlag = 1;
    
    //8.4.1.3.2 Derivation process for motion data of neighbouring partitions
    ret = Derivation_process_for_motion_data_of_neighbouring_partitions(mbPartIdx_, subMbPartIdx_, currSubMbType, listSuffixFlag,
        mbAddrA, mvL1A, refIdxL1A, mbAddrB, mvL1B, refIdxL1B, mbAddrC, mvL1C, refIdxL1C);
    RETURN_IF_FAILED(ret != 0, ret);

    //--------------
#define MinPositive( x, y )    ((x) >= 0 && (y) >= 0) ? (MIN((x), (y))) : (MAX((x), (y)))

    refIdxL0 = MinPositive( refIdxL0A, MinPositive( refIdxL0B, refIdxL0C ) );
    refIdxL1 = MinPositive( refIdxL1A, MinPositive( refIdxL1B, refIdxL1C ) );
    int32_t directZeroPredictionFlag = 0;

#undef MinPositive

    if (refIdxL0 < 0 && refIdxL1 < 0)
    {
        refIdxL0 = 0;
        refIdxL1 = 0;
        directZeroPredictionFlag = 1;
    }

    //-------------------------------------
    //8.4.1.2.1 Derivation process for the co-located 4x4 sub-macroblock partitions
    CH264PictureBase *colPic = NULL;
    int32_t mbAddrCol = 0;
    int32_t mvCol[2] = {0};
    int32_t refIdxCol = 0;
    int32_t vertMvScale = 0;
    int32_t colZeroFlag = 0;

    ret = Derivation_process_for_the_co_located_4x4_sub_macroblock_partitions(mbPartIdx, subMbPartIdx, colPic, mbAddrCol, mvCol, refIdxCol, vertMvScale);
    RETURN_IF_FAILED(ret != 0, ret);

    //------------------------
    if (m_RefPicList1[ 0 ]->reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
        && refIdxCol == 0
        && ((m_mbs[mbAddrCol].mb_field_decoding_flag == 0 && (mvCol[0] >= -1 && mvCol[0] <= 1) && (mvCol[1] >= -1 && mvCol[1] <= 1))
             || (m_mbs[mbAddrCol].mb_field_decoding_flag == 1 && (mvCol[0] >= -1 && mvCol[0] <= 1) && (mvCol[1] >= -1 && mvCol[1] <= 1)))
        /* FIXME:
            both motion vector components mvCol[ 0 ] and mvCol[ 1 ] lie in the range of −1 to 1 in units specified as follows:
            – If the co-located macroblock is a frame macroblock, the units of mvCol[ 0 ] and mvCol[ 1 ] are units of
              quarter luma frame samples.
            – Otherwise (the co-located macroblock is a field macroblock), the units of mvCol[ 0 ] and mvCol[ 1 ] are
              units of quarter luma field samples.
        */
        )
    {
        colZeroFlag = 1;
    }
    else
    {
        colZeroFlag = 0;
    }

    //--------------------------
    if (directZeroPredictionFlag == 1
        || refIdxL0 < 0
        || (refIdxL0 == 0 && colZeroFlag == 1)
       )
    {
        mvL0[0] = 0;
        mvL0[1] = 0;
    }
    else
    {
        mbPartIdx_ = 0;
        subMbPartIdx_ = 0;
        listSuffixFlag = 0;

        //8.4.1.3 Derivation process for luma motion vector prediction
        ret = Derivation_process_for_luma_motion_vector_prediction(mbPartIdx_, subMbPartIdx_, currSubMbType, listSuffixFlag, refIdxL0, mvL0);
        RETURN_IF_FAILED(ret != 0, ret);
    }
    
    //--------------------------
    if (directZeroPredictionFlag == 1
        || refIdxL1 < 0
        || (refIdxL1 == 0 && colZeroFlag == 1)
       )
    {
        mvL1[0] = 0;
        mvL1[1] = 0;
    }
    else
    {
        mbPartIdx_ = 0;
        subMbPartIdx_ = 0;
        listSuffixFlag = 1;

        //8.4.1.3 Derivation process for luma motion vector prediction
        ret = Derivation_process_for_luma_motion_vector_prediction(mbPartIdx_, subMbPartIdx_, currSubMbType, listSuffixFlag, refIdxL1, mvL1);
        RETURN_IF_FAILED(ret != 0, ret);
    }

    //---------------------------
    if (refIdxL0 >= 0 && refIdxL1 >= 0)
    {
        predFlagL0 = 1;
        predFlagL1 = 1;
    }
    else if (refIdxL0 >= 0 && refIdxL1 < 0)
    {
        predFlagL0 = 1;
        predFlagL1 = 0;
    }
    else if (refIdxL0 < 0 && refIdxL1 >= 0)
    {
        predFlagL0 = 0;
        predFlagL1 = 1;
    }
    else //if (refIdxL0 < 0 && refIdxL1 < 0)
    {
        RETURN_IF_FAILED(0, -1);
    }

    //----------------------
    if (subMbPartIdx != 0)
    {
        subMvCnt = 0;
    }
    else
    {
        subMvCnt = predFlagL0 + predFlagL1;
    }

    return 0;
}


//8.4.1.2.3 Derivation process for temporal direct luma motion vector and reference index prediction mode
//This process is invoked when direct_spatial_mv_pred_flag is equal to 0 and any of the following conditions are true:
//– mb_type is equal to B_Skip,
//– mb_type is equal to B_Direct_16x16,
//– sub_mb_type[ mbPartIdx ] is equal to B_Direct_8x8.
int CH264PictureBase::Derivation_process_for_temporal_direct_luma_motion_vector_and_reference_index_prediction_mode(int32_t mbPartIdx, int32_t subMbPartIdx,
        int32_t &refIdxL0, int32_t &refIdxL1, int32_t (&mvL0)[2], int32_t (&mvL1)[2], int32_t &subMvCnt, int32_t &predFlagL0, int32_t &predFlagL1)
{
    int ret = 0;
    
    //8.4.1.2.1 Derivation process for the co-located 4x4 sub-macroblock partitions
    CH264PictureBase *colPic = NULL;
    int32_t mbAddrCol = 0;
    int32_t mvCol[2] = {0};
    int32_t refIdxCol = 0;
    int32_t vertMvScale = 0;

    ret = Derivation_process_for_the_co_located_4x4_sub_macroblock_partitions(mbPartIdx, subMbPartIdx, colPic, mbAddrCol, mvCol, refIdxCol, vertMvScale);
    RETURN_IF_FAILED(ret != 0, ret);
    
    CH264SliceHeader & slice_header = m_h264_slice_header;

    //---------------------------------
    int32_t refIdxL0_temp = 0;

    if (refIdxCol < 0)
    {
        refIdxL0_temp = 0;
    }
    else
    {
        //refIdxL0_temp = MapColToList0( refIdxCol ) );
        int32_t refIdxL0Frm = NA;

        for (int i = 0; i < H264_MAX_REF_PIC_LIST_COUNT; i++)
        {
            if ((m_RefPicList0[i]->m_picture_coded_type == H264_PICTURE_CODED_TYPE_FRAME && (&m_RefPicList0[i]->m_picture_frame == colPic))
                || (m_RefPicList0[i]->m_picture_coded_type == H264_PICTURE_CODED_TYPE_COMPLEMENTARY_FIELD_PAIR
                        && (&m_RefPicList0[i]->m_picture_top_filed == colPic || &m_RefPicList0[i]->m_picture_bottom_filed == colPic))
                )
            {
                refIdxL0Frm = i;
                break;
            }
        }

        RETURN_IF_FAILED(refIdxL0Frm == NA, -1);

        if (vertMvScale == H264_VERT_MV_SCALE_One_To_One)
        {
            if (slice_header.field_pic_flag == 0 && m_mbs[CurrMbAddr].mb_field_decoding_flag == 1) //If field_pic_flag is equal to 0 and the current macroblock is a field macroblock
            {
                /* FIXME:
                Let refIdxL0Frm be the lowest valued reference index in the current reference picture list RefPicList0 that 
                references the frame or complementary field pair that contains the field refPicCol. RefPicList0 shall contain 
                a frame or complementary field pair that contains the field refPicCol. The return value of MapColToList0( ) is 
                specified as follows:
                – If the field referred to by refIdxCol has the same parity as the current macroblock, MapColToList0( refIdxCol ) 
                   returns the reference index ( refIdxL0Frm << 1 ).
                – Otherwise (the field referred by refIdxCol has the opposite parity of the current macroblock), MapColToList0( refIdxCol ) 
                   returns the reference index ( ( refIdxL0Frm << 1 ) + 1 ).
                */
                if ((colPic->m_picture_coded_type == H264_PICTURE_CODED_TYPE_TOP_FIELD && CurrMbAddr % 2 == 0)
                    || (colPic->m_picture_coded_type == H264_PICTURE_CODED_TYPE_BOTTOM_FIELD && CurrMbAddr % 2 == 1)
                    )
                {
                    refIdxL0_temp = refIdxL0Frm << 1;
                }
                else
                {
                    refIdxL0_temp = ( refIdxL0Frm << 1 ) + 1;
                }
            }
            else //field_pic_flag is equal to 1 or the current macroblock is a frame macroblock
            {
                //FIXME:
                //MapColToList0( refIdxCol ) returns the lowest valued reference index refIdxL0 in the current reference picture list 
                //RefPicList0 that references refPicCol. RefPicList0 shall contain refPicCol.
                refIdxL0_temp = refIdxL0Frm;
            }
        }
        else if (vertMvScale == H264_VERT_MV_SCALE_Frm_To_Fld) //if vertMvScale is equal to Frm_To_Fld, the following applies
        {
            if (m_mbs[mbAddrCol].field_pic_flag == 0)
            {
                //let refIdxL0Frm be the lowest valued reference index in the current reference picture list RefPicList0 that references 
                //refPicCol. MapColToList0( refIdxCol ) returns the reference index ( refIdxL0Frm << 1 ). RefPicList0 shall contain refPicCol
                refIdxL0_temp = refIdxL0Frm << 1;
            }
            else
            {
                //MapColToList0( refIdxCol ) returns the lowest valued reference index refIdxL0 in the current reference picture list RefPicList0 
                //that references the field of refPicCol with the same parity as the current picture CurrPic. RefPicList0 shall contain the field 
                //of refPicCol with the same parity as the current picture CurrPic.
                if (colPic->m_picture_coded_type == this->m_picture_coded_type)
                {
                    refIdxL0_temp = refIdxL0Frm;
                }
                else
                {
                    RETURN_IF_FAILED(0, -1);
                }
            }
        }
        else if (vertMvScale == H264_VERT_MV_SCALE_Fld_To_Frm) //if vertMvScale is equal to Frm_To_Fld, the following applies
        {
            //MapColToList0( refIdxCol ) returns the lowest valued reference index refIdxL0 in the current reference picture list RefPicList0 that 
            //references the frame or complementary field pair that contains refPicCol. RefPicList0 shall contain a frame or complementary field 
            //pair that contains the field refPicCol.
            refIdxL0_temp = refIdxL0Frm;
        }
    }

    //---------------------------------
    refIdxL0 = refIdxL0_temp; //refIdxL0 = ( ( refIdxCol < 0 ) ? 0 : MapColToList0( refIdxCol ) );
    refIdxL1 = 0;

    //----------------------------------
    if (vertMvScale == H264_VERT_MV_SCALE_Frm_To_Fld)
    {
        mvCol[ 1 ] = mvCol[ 1 ] / 2;
    }
    else if (vertMvScale == H264_VERT_MV_SCALE_Fld_To_Frm)
    {
        mvCol[ 1 ] = mvCol[ 1 ] * 2;
    }
    else //if (vertMvScale == H264_VERT_MV_SCALE_One_To_One)
    {
        //mvCol[ 1 ] = mvCol[ 1 ]; //mvCol[ 1 ] remains unchanged
    }

    //---------------------------
    CH264PictureBase * pic0 = NULL;
    CH264PictureBase * pic1 = NULL;
    CH264PictureBase * currPicOrField = NULL;

    if (slice_header.field_pic_flag == 0 && m_mbs[CurrMbAddr].mb_field_decoding_flag == 1) //If field_pic_flag is equal to 0 and the current macroblock is a field macroblock
    {
        //1. currPicOrField is the field of the current picture CurrPic that has the same parity as the current macroblock.
        //2. pic1 is the field of RefPicList1[ 0 ] that has the same parity as the current macroblock.
        //3. The variable pic0 is derived as follows:
        //   – If refIdxL0 % 2 is equal to 0, pic0 is the field of RefPicList0[ refIdxL0 / 2 ] that has the same parity as the current macroblock.
        //   – Otherwise (refIdxL0 % 2 is not equal to 0), pic0 is the field of RefPicList0[ refIdxL0 / 2 ] that has the opposite parity of the current macroblock.
        
        if (CurrMbAddr % 2 == 0) //currPicOrField is the field of the current picture CurrPic that has the same parity as the current macroblock.
        {
            currPicOrField = &(m_parent->m_picture_top_filed);
        }
        else
        {
            currPicOrField = &(m_parent->m_picture_bottom_filed);
        }

        if (CurrMbAddr % 2 == 0) //pic1 is the field of RefPicList1[ 0 ] that has the same parity as the current macroblock.
        {
            pic1 = &(m_RefPicList1[0]->m_picture_top_filed);
        }
        else
        {
            pic1 = &(m_RefPicList1[0]->m_picture_bottom_filed);
        }

        if (refIdxL0 % 2 == 0)
        {
            if (CurrMbAddr % 2 == 0) //pic0 is the field of RefPicList0[ refIdxL0 / 2 ] that has the same parity as the current macroblock.
            {
                pic0 = &(m_RefPicList0[refIdxL0 / 2]->m_picture_top_filed);
            }
            else
            {
                pic0 = &(m_RefPicList0[refIdxL0 / 2]->m_picture_bottom_filed);
            }
        }
        else //if (refIdxL0 % 2 != 0)
        {
            if (CurrMbAddr % 2 == 0) //pic0 is the field of RefPicList0[ refIdxL0 / 2 ] that has the opposite parity of the current macroblock.
            {
                pic0 = &(m_RefPicList0[refIdxL0 / 2]->m_picture_bottom_filed);
            }
            else
            {
                pic0 = &(m_RefPicList0[refIdxL0 / 2]->m_picture_top_filed);
            }
        }
    }
    else //field_pic_flag is equal to 1 or the current macroblock is a frame macroblock
    {
        //currPicOrField is the current picture CurrPic, pic1 is the decoded reference picture RefPicList1[ 0 ], and pic0 is 
        //the decoded reference picture RefPicList0[ refIdxL0 ].

        currPicOrField = &(m_parent->m_picture_frame);
        pic0 = &(m_RefPicList0[refIdxL0]->m_picture_frame);
        pic1 = &(m_RefPicList1[0]->m_picture_frame);
    }

    //---------------------------
    if (m_RefPicList0[ refIdxL0 ]->reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference
        || DiffPicOrderCnt( pic1, pic0 )
        )
    {
        mvL0[0] = mvCol[0];
        mvL0[1] = mvCol[1];
        mvL1[0] = 0;
        mvL1[1] = 0;
    }
    else
    {
        int32_t tb = CLIP3( -128, 127, DiffPicOrderCnt( currPicOrField, pic0 ) );
        int32_t td = CLIP3( -128, 127, DiffPicOrderCnt( pic1, pic0 ) );

        int32_t tx = ( 16384 + ABS( td / 2 ) ) / td;
        int32_t DistScaleFactor = CLIP3( -1024, 1023, ( tb * tx + 32 ) >> 6 );

        mvL0[0] = ( DistScaleFactor * mvCol[0] + 128 ) >> 8;
        mvL0[1] = ( DistScaleFactor * mvCol[1] + 128 ) >> 8;
        mvL1[0] = mvL0[0] - mvCol[0];
        mvL1[1] = mvL0[1] - mvCol[1];
    }

    predFlagL0 = 1;
    predFlagL1 = 1;

    return 0;
}


//8.4.1.3 Derivation process for luma motion vector prediction
int CH264PictureBase::Derivation_process_for_luma_motion_vector_prediction(int32_t mbPartIdx, int32_t subMbPartIdx, H264_MB_TYPE currSubMbType, int32_t listSuffixFlag, int32_t refIdxLX, int32_t (&mvpLX)[2])
{
    int ret = 0;
    
    int32_t mbAddrN_A = 0;
    int32_t mvLXN_A[2] = { 0 };
    int32_t refIdxLXN_A = 0;

    int32_t mbAddrN_B = 0;
    int32_t mvLXN_B[2] = { 0 };
    int32_t refIdxLXN_B = 0;

    int32_t mbAddrN_C = 0;
    int32_t mvLXN_C[2] = { 0 };
    int32_t refIdxLXN_C = 0;

    //8.4.1.3.2 Derivation process for motion data of neighbouring partitions
    ret = Derivation_process_for_motion_data_of_neighbouring_partitions(mbPartIdx, subMbPartIdx, currSubMbType, listSuffixFlag,
        mbAddrN_A, mvLXN_A, refIdxLXN_A, mbAddrN_B, mvLXN_B, refIdxLXN_B, mbAddrN_C, mvLXN_C, refIdxLXN_C);
    RETURN_IF_FAILED(ret != 0, ret);

    //------------------
    int32_t MbPartWidth = m_mbs[CurrMbAddr].MbPartWidth;
    int32_t MbPartHeight = m_mbs[CurrMbAddr].MbPartHeight;

    if (MbPartWidth == 16 && MbPartHeight == 8 && mbPartIdx == 0 && refIdxLXN_B == refIdxLX)
    {
        mvpLX[0] = mvLXN_B[0];
        mvpLX[1] = mvLXN_B[1];
    }
    else if (MbPartWidth == 16 && MbPartHeight == 8 && mbPartIdx == 1 && refIdxLXN_A == refIdxLX)
    {
        mvpLX[0] = mvLXN_A[0];
        mvpLX[1] = mvLXN_A[1];
    }
    else if (MbPartWidth == 8 && MbPartHeight == 16 && mbPartIdx == 0 && refIdxLXN_A == refIdxLX)
    {
        mvpLX[0] = mvLXN_A[0];
        mvpLX[1] = mvLXN_A[1];
    }
    else if (MbPartWidth == 8 && MbPartHeight == 16 && mbPartIdx == 1 && refIdxLXN_C == refIdxLX)
    {
        mvpLX[0] = mvLXN_C[0];
        mvpLX[1] = mvLXN_C[1];
    }
    else
    {
        //8.4.1.3.1 Derivation process for median luma motion vector prediction

        //When both partitions mbAddrB\mbPartIdxB\subMbPartIdxB and mbAddrC\mbPartIdxC\subMbPartIdxC are not 
        //available and mbAddrA\mbPartIdxA\subMbPartIdxA is available
        if (mbAddrN_B < 0 && mbAddrN_C < 0 && mbAddrN_A >= 0)
        {
            mvLXN_B[0] = mvLXN_A[0];
            mvLXN_B[1] = mvLXN_A[1];
            mvLXN_C[0] = mvLXN_A[0];
            mvLXN_C[1] = mvLXN_A[1];
            refIdxLXN_B = refIdxLXN_A;
            refIdxLXN_C = refIdxLXN_A;
        }

        //------------
        //If one and only one of the reference indices refIdxLXA, refIdxLXB, or refIdxLXC is equal to the reference 
        //index refIdxLX of the current partition, the following applies. Let refIdxLXN be the reference index that is 
        //equal to refIdxLX, the motion vector mvLXN is assigned to the motion vector prediction mvpLX:
        if (refIdxLXN_A == refIdxLX && refIdxLXN_B != refIdxLX && refIdxLXN_C != refIdxLX)
        {
            mvpLX[0] = mvLXN_A[0];
            mvpLX[1] = mvLXN_A[1];
        }
        else if (refIdxLXN_A != refIdxLX && refIdxLXN_B == refIdxLX && refIdxLXN_C != refIdxLX)
        {
            mvpLX[0] = mvLXN_B[0];
            mvpLX[1] = mvLXN_B[1];
        }
        else if (refIdxLXN_A != refIdxLX && refIdxLXN_B != refIdxLX && refIdxLXN_C == refIdxLX)
        {
            mvpLX[0] = mvLXN_C[0];
            mvpLX[1] = mvLXN_C[1];
        }
        else
        {
            int32_t MIN0 = MIN(mvLXN_A[0], MIN(mvLXN_B[0], mvLXN_C[0]));
            int32_t MAX0 = MAX(mvLXN_A[0], MAX(mvLXN_B[0], mvLXN_C[0]));

            int32_t MIN1 = MIN(mvLXN_A[1], MIN(mvLXN_B[1], mvLXN_C[1]));
            int32_t MAX1 = MAX(mvLXN_A[1], MAX(mvLXN_B[1], mvLXN_C[1]));

            mvpLX[0] = mvLXN_A[0] + mvLXN_B[0]+ mvLXN_C[0] - MIN0 - MAX0; //mvpLX[0] = Median(( mvLXN_A[0], mvLXN_B[0], mvLXN_C[0] );
            mvpLX[1] = mvLXN_A[1] + mvLXN_B[1]+ mvLXN_C[1] - MIN1 - MAX1; //mvpLX[1] = Median(( mvLXN_A[1], mvLXN_B[1], mvLXN_C[1] );
        }
    }

    return 0;
}


//8.4.1.3.2 Derivation process for motion data of neighbouring partitions
int CH264PictureBase::Derivation_process_for_motion_data_of_neighbouring_partitions(int32_t mbPartIdx, int32_t subMbPartIdx, H264_MB_TYPE currSubMbType, int32_t listSuffixFlag, 
    int32_t &mbAddrN_A, int32_t (&mvLXN_A)[2], int32_t &refIdxLXN_A, int32_t &mbAddrN_B, int32_t (&mvLXN_B)[2], int32_t &refIdxLXN_B, int32_t &mbAddrN_C, int32_t (&mvLXN_C)[2], int32_t &refIdxLXN_C)
{
    int ret = 0;
    
    CH264SliceHeader & slice_header = m_h264_slice_header;

//    int32_t mbAddrN_A= 0;
    int32_t mbPartIdxN_A = 0;
    int32_t subMbPartIdxN_A = 0;

//    int32_t mbAddrN_B = 0;
    int32_t mbPartIdxN_B = 0;
    int32_t subMbPartIdxN_B = 0;

//    int32_t mbAddrN_C = 0;
    int32_t mbPartIdxN_C = 0;
    int32_t subMbPartIdxN_C = 0;

    int32_t mbAddrN_D = 0;
    int32_t mbPartIdxN_D = 0;
    int32_t subMbPartIdxN_D = 0;

    //--------------------------------
    //6.4.2.1 Inverse macroblock partition scanning process
    int32_t MbPartWidth = m_mbs[CurrMbAddr].MbPartWidth;
    int32_t MbPartHeight = m_mbs[CurrMbAddr].MbPartHeight;
    int32_t SubMbPartWidth = m_mbs[CurrMbAddr].SubMbPartWidth[mbPartIdx];
    int32_t SubMbPartHeight = m_mbs[CurrMbAddr].SubMbPartHeight[mbPartIdx];

    int32_t x = InverseRasterScan( mbPartIdx, MbPartWidth, MbPartHeight, 16, 0 );
    int32_t y = InverseRasterScan( mbPartIdx, MbPartWidth, MbPartHeight, 16, 1 );

    //--------------------
    int32_t xS = 0;
    int32_t yS = 0;

    if (m_mbs[CurrMbAddr].m_name_of_mb_type == P_8x8 || m_mbs[CurrMbAddr].m_name_of_mb_type == P_8x8ref0 || m_mbs[CurrMbAddr].m_name_of_mb_type == B_8x8)
    {
        //6.4.2.2 Inverse sub-macroblock partition scanning process
        xS = InverseRasterScan( subMbPartIdx, SubMbPartWidth, SubMbPartHeight, 8, 0 );
        yS = InverseRasterScan( subMbPartIdx, SubMbPartWidth, SubMbPartHeight, 8, 1 );
        
        //Otherwise (mb_type is not equal to P_8x8, P_8x8ref0, or B_8x8),
        //xS = InverseRasterScan( subMbPartIdx, 4, 4, 8, 0 );
        //yS = InverseRasterScan( subMbPartIdx, 4, 4, 8, 1 );
    }
    else
    {
        xS = 0;
        yS = 0;
    }

    //--------------------------
    int32_t predPartWidth = 0;

    if (m_mbs[CurrMbAddr].m_name_of_mb_type == P_Skip || m_mbs[CurrMbAddr].m_name_of_mb_type ==  B_Skip || m_mbs[CurrMbAddr].m_name_of_mb_type == B_Direct_16x16)
    {
        predPartWidth = 16;
    }
    else if (m_mbs[CurrMbAddr].m_name_of_mb_type == B_8x8)
    {
        if (currSubMbType == B_Direct_8x8)
        {
            predPartWidth = 16;
        }
        else
        {
            predPartWidth = SubMbPartWidth; //SubMbPartWidth( sub_mb_type[ mbPartIdx ] );
        }
    }
    else if (m_mbs[CurrMbAddr].m_name_of_mb_type == P_8x8 || m_mbs[CurrMbAddr].m_name_of_mb_type == P_8x8ref0)
    {
        predPartWidth = SubMbPartWidth; //SubMbPartWidth( sub_mb_type[ mbPartIdx ] );
    }
    else
    {
        predPartWidth = MbPartWidth; //MbPartWidth( mb_type );
    }

    //--------------------------------
    int32_t isChroma = 0;

    //6.4.11.7 Derivation process for neighbouring partitions
    ret = Derivation_process_for_neighbouring_partitions(x + xS - 1, y + yS + 0, mbPartIdx, currSubMbType, subMbPartIdx, isChroma, mbAddrN_A, mbPartIdxN_A, subMbPartIdxN_A);
    RETURN_IF_FAILED(ret != 0, ret);
    
    ret = Derivation_process_for_neighbouring_partitions(x + xS + 0, y + yS - 1, mbPartIdx, currSubMbType, subMbPartIdx, isChroma, mbAddrN_B, mbPartIdxN_B, subMbPartIdxN_B);
    RETURN_IF_FAILED(ret != 0, ret);
    
    ret = Derivation_process_for_neighbouring_partitions(x + xS + predPartWidth, y + yS - 1, mbPartIdx, currSubMbType, subMbPartIdx, isChroma, mbAddrN_C, mbPartIdxN_C, subMbPartIdxN_C);
    RETURN_IF_FAILED(ret != 0, ret);
    
    ret = Derivation_process_for_neighbouring_partitions(x + xS - 1, y + yS - 1, mbPartIdx, currSubMbType, subMbPartIdx, isChroma, mbAddrN_D, mbPartIdxN_D, subMbPartIdxN_D);
    RETURN_IF_FAILED(ret != 0, ret);

    //-----------------------------------
    if (mbAddrN_C < 0 || mbPartIdxN_C < 0 || subMbPartIdxN_C < 0) //When the partition mbAddrC\mbPartIdxC\subMbPartIdxC is not available
    {
        mbAddrN_C = mbAddrN_D;
        mbPartIdxN_C = mbPartIdxN_D;
        subMbPartIdxN_C = subMbPartIdxN_D;
    }

    //----------------mbAddrN_A---------------
    if (mbAddrN_A < 0 || mbPartIdxN_A < 0 || subMbPartIdxN_A < 0
        || IS_INTRA_Prediction_Mode(m_mbs[mbAddrN_A].m_mb_pred_mode) == true
        || (listSuffixFlag == 0 && m_mbs[mbAddrN_A].m_PredFlagL0[mbPartIdxN_A] == 0)
        || (listSuffixFlag == 1 && m_mbs[mbAddrN_A].m_PredFlagL1[mbPartIdxN_A] == 0)
        )
    {
        mvLXN_A[0] = 0;
        mvLXN_A[1] = 0;
        refIdxLXN_A = -1;
    }
    else
    {
        //The motion vector mvLXN and reference index refIdxLXN are set equal to 
        //MvLX[ mbPartIdxN ][ subMbPartIdxN ] and RefIdxLX[ mbPartIdxN ], respectively, which are the motion 
        //vector mvLX and reference index refIdxLX that have been assigned to the (sub-)macroblock partition 
        //mbAddrN\mbPartIdxN\subMbPartIdxN.
        if (listSuffixFlag == 0)
        {
            mvLXN_A[0] = m_mbs[mbAddrN_A].m_MvL0[ mbPartIdxN_A ][ subMbPartIdxN_A ][0];
            mvLXN_A[1] = m_mbs[mbAddrN_A].m_MvL0[ mbPartIdxN_A ][ subMbPartIdxN_A ][1];
            refIdxLXN_A = m_mbs[mbAddrN_A].m_RefIdxL0[ mbPartIdxN_A ];
        }
        else //if (listSuffixFlag == 1)
        {
            mvLXN_A[0] = m_mbs[mbAddrN_A].m_MvL1[ mbPartIdxN_A ][ subMbPartIdxN_A ][0];
            mvLXN_A[1] = m_mbs[mbAddrN_A].m_MvL1[ mbPartIdxN_A ][ subMbPartIdxN_A ][1];
            refIdxLXN_A = m_mbs[mbAddrN_A].m_RefIdxL1[ mbPartIdxN_A ];
        }

        if (m_mbs[CurrMbAddr].mb_field_decoding_flag == 1 && m_mbs[mbAddrN_A].mb_field_decoding_flag == 0) //If the current macroblock is a field macroblock and the macroblock mbAddrN is a frame macroblock
        {
            mvLXN_A[ 1 ] = mvLXN_A[ 1 ] / 2;
            refIdxLXN_A = refIdxLXN_A * 2;
        }
        else if (m_mbs[CurrMbAddr].mb_field_decoding_flag == 0 && m_mbs[mbAddrN_A].mb_field_decoding_flag == 1)//if the current macroblock is a frame macroblock and the macroblock mbAddrN is a field macroblock
        {
            mvLXN_A[ 1 ] = mvLXN_A[ 1 ] * 2;
            refIdxLXN_A = refIdxLXN_A / 2;
        }
        else
        {
            //the vertical motion vector component mvLXN[ 1 ] and the reference index refIdxLXN remain unchanged.
        }
    }
    
    //----------------mbAddrN_B---------------
    if (mbAddrN_B < 0 || mbPartIdxN_B < 0 || subMbPartIdxN_B < 0
        || IS_INTRA_Prediction_Mode(m_mbs[mbAddrN_B].m_mb_pred_mode) == true
        || (listSuffixFlag == 0 && m_mbs[mbAddrN_B].m_PredFlagL0[mbPartIdxN_B] == 0)
        || (listSuffixFlag == 1 && m_mbs[mbAddrN_B].m_PredFlagL1[mbPartIdxN_B] == 0)
        )
    {
        mvLXN_B[0] = 0;
        mvLXN_B[1] = 0;
        refIdxLXN_B = -1;
    }
    else
    {
        //The motion vector mvLXN and reference index refIdxLXN are set equal to 
        //MvLX[ mbPartIdxN ][ subMbPartIdxN ] and RefIdxLX[ mbPartIdxN ], respectively, which are the motion 
        //vector mvLX and reference index refIdxLX that have been assigned to the (sub-)macroblock partition 
        //mbAddrN\mbPartIdxN\subMbPartIdxN.
        if (listSuffixFlag == 0)
        {
            mvLXN_B[0] = m_mbs[mbAddrN_B].m_MvL0[ mbPartIdxN_B ][ subMbPartIdxN_B ][0];
            mvLXN_B[1] = m_mbs[mbAddrN_B].m_MvL0[ mbPartIdxN_B ][ subMbPartIdxN_B ][1];
            refIdxLXN_B = m_mbs[mbAddrN_B].m_RefIdxL0[ mbPartIdxN_B ];
        }
        else //if (listSuffixFlag == 1)
        {
            mvLXN_B[0] = m_mbs[mbAddrN_B].m_MvL1[ mbPartIdxN_B ][ subMbPartIdxN_B ][0];
            mvLXN_B[1] = m_mbs[mbAddrN_B].m_MvL1[ mbPartIdxN_B ][ subMbPartIdxN_B ][1];
            refIdxLXN_B = m_mbs[mbAddrN_B].m_RefIdxL1[ mbPartIdxN_B ];
        }

        if (m_mbs[CurrMbAddr].mb_field_decoding_flag == 1 && m_mbs[mbAddrN_B].mb_field_decoding_flag == 0) //If the current macroblock is a field macroblock and the macroblock mbAddrN is a frame macroblock
        {
            mvLXN_B[ 1 ] = mvLXN_B[ 1 ] / 2;
            refIdxLXN_B = refIdxLXN_B * 2;
        }
        else if (m_mbs[CurrMbAddr].mb_field_decoding_flag == 0 && m_mbs[mbAddrN_B].mb_field_decoding_flag == 1)//if the current macroblock is a frame macroblock and the macroblock mbAddrN is a field macroblock
        {
            mvLXN_B[ 1 ] = mvLXN_B[ 1 ] * 2;
            refIdxLXN_B = refIdxLXN_B / 2;
        }
        else
        {
            //the vertical motion vector component mvLXN[ 1 ] and the reference index refIdxLXN remain unchanged.
        }
    }
    
    //----------------mbAddrN_C---------------
    if (mbAddrN_C < 0 || mbPartIdxN_C < 0 || subMbPartIdxN_C < 0
        || IS_INTRA_Prediction_Mode(m_mbs[mbAddrN_C].m_mb_pred_mode) == true
        || (listSuffixFlag == 0 && m_mbs[mbAddrN_C].m_PredFlagL0[mbPartIdxN_C] == 0)
        || (listSuffixFlag == 1 && m_mbs[mbAddrN_C].m_PredFlagL1[mbPartIdxN_C] == 0)
        )
    {
        mvLXN_C[0] = 0;
        mvLXN_C[1] = 0;
        refIdxLXN_C = -1;
    }
    else
    {
        //The motion vector mvLXN and reference index refIdxLXN are set equal to 
        //MvLX[ mbPartIdxN ][ subMbPartIdxN ] and RefIdxLX[ mbPartIdxN ], respectively, which are the motion 
        //vector mvLX and reference index refIdxLX that have been assigned to the (sub-)macroblock partition 
        //mbAddrN\mbPartIdxN\subMbPartIdxN.
        if (listSuffixFlag == 0)
        {
            mvLXN_C[0] = m_mbs[mbAddrN_C].m_MvL0[ mbPartIdxN_C ][ subMbPartIdxN_C ][0];
            mvLXN_C[1] = m_mbs[mbAddrN_C].m_MvL0[ mbPartIdxN_C ][ subMbPartIdxN_C ][1];
            refIdxLXN_C = m_mbs[mbAddrN_C].m_RefIdxL0[ mbPartIdxN_C ];
        }
        else //if (listSuffixFlag == 1)
        {
            mvLXN_C[0] = m_mbs[mbAddrN_C].m_MvL1[ mbPartIdxN_C ][ subMbPartIdxN_C ][0];
            mvLXN_C[1] = m_mbs[mbAddrN_C].m_MvL1[ mbPartIdxN_C ][ subMbPartIdxN_C ][1];
            refIdxLXN_C = m_mbs[mbAddrN_C].m_RefIdxL1[ mbPartIdxN_C ];
        }

        if (m_mbs[CurrMbAddr].mb_field_decoding_flag == 1 && m_mbs[mbAddrN_C].mb_field_decoding_flag == 0) //If the current macroblock is a field macroblock and the macroblock mbAddrN is a frame macroblock
        {
            mvLXN_C[ 1 ] = mvLXN_C[ 1 ] / 2;
            refIdxLXN_C = refIdxLXN_C * 2;
        }
        else if (m_mbs[CurrMbAddr].mb_field_decoding_flag == 0 && m_mbs[mbAddrN_C].mb_field_decoding_flag == 1)//if the current macroblock is a frame macroblock and the macroblock mbAddrN is a field macroblock
        {
            mvLXN_C[ 1 ] = mvLXN_C[ 1 ] * 2;
            refIdxLXN_C = refIdxLXN_C / 2;
        }
        else
        {
            //the vertical motion vector component mvLXN[ 1 ] and the reference index refIdxLXN remain unchanged.
        }
    }

    return 0;
}


//8.4.1.4 Derivation process for chroma motion vectors
//This process is only invoked when ChromaArrayType is not equal to 0.
int CH264PictureBase::Derivation_process_for_chroma_motion_vectors(int32_t ChromaArrayType, int32_t mvLX[2], CH264PictureBase *refPic, int32_t (&mvCLX)[2])
{
    int ret = 0;
    
    if (ChromaArrayType == 0)
    {
        RETURN_IF_FAILED(0, -1);
    }
    else if (ChromaArrayType != 1 || m_mbs[CurrMbAddr].mb_field_decoding_flag == 0)
    {
        mvCLX[ 0 ] = mvLX[ 0 ];
        mvCLX[ 1 ] = mvLX[ 1 ];
    }
    else //if (ChromaArrayType == 1 && m_mbs[CurrMbAddr].mb_field_decoding_flag == 1) // ChromaArrayType is equal to 1 and the current macroblock is a field macroblock
    {
        mvCLX[ 0 ] = mvLX[ 0 ];

        //Table 8-10 – Derivation of the vertical component of the chroma vector in field coding mode
        if (refPic && refPic->m_picture_coded_type == H264_PICTURE_CODED_TYPE_TOP_FIELD
            && (this->m_picture_coded_type == H264_PICTURE_CODED_TYPE_BOTTOM_FIELD //bottom field
                || mb_y % 2 == 1 //bottom field macroblock
               )
            )
        {
            mvCLX[ 1 ] = mvLX[ 1 ] + 2;
        }
        else if (refPic && refPic->m_picture_coded_type == H264_PICTURE_CODED_TYPE_BOTTOM_FIELD
            && (this->m_picture_coded_type == H264_PICTURE_CODED_TYPE_TOP_FIELD //top field
                || mb_y % 2 == 0 //top field macroblock
               )
            )
        {
            mvCLX[ 1 ] = mvLX[ 1 ] - 2;
        }
        else
        {
            mvCLX[ 1 ] = mvLX[ 1 ];
        }
    }

    return 0;
}


//8.4.2 Decoding process for Inter prediction samples
int CH264PictureBase::Decoding_process_for_Inter_prediction_samples(int32_t mbPartIdx, int32_t subMbPartIdx, int32_t partWidth, int32_t partHeight, int32_t partWidthC, int32_t partHeightC,
        int32_t xAL, int32_t yAL,
        int32_t (&mvL0)[2], int32_t (&mvL1)[2], int32_t (&mvCL0)[2], int32_t (&mvCL1)[2], CH264PictureBase *refPicL0, CH264PictureBase *refPicL1, int32_t predFlagL0, int32_t predFlagL1,
        int32_t logWDL, int32_t w0L, int32_t w1L, int32_t o0L, int32_t o1L,
        int32_t logWDCb, int32_t w0Cb, int32_t w1Cb, int32_t o0Cb, int32_t o1Cb,
        int32_t logWDCr, int32_t w0Cr, int32_t w1Cr, int32_t o0Cr, int32_t o1Cr,
        uint8_t *predPartL, //predPartL[partHeight][partWidth]
        uint8_t *predPartCb, //predPartCb[partHeightC][partWidthC]
        uint8_t *predPartCr //predPartCr[partHeightC][partWidthC]
        )
{
    int ret = 0;
    
    uint8_t predPartL0L[256] = {0};
    uint8_t predPartL1L[256] = {0};
    uint8_t predPartL0Cb[256] = {0};
    uint8_t predPartL1Cb[256] = {0};
    uint8_t predPartL0Cr[256] = {0};
    uint8_t predPartL1Cr[256] = {0};

    //---------------------------
    if (predFlagL0 == 1)
    {
//        CH264PictureBase * refPicL0 = NULL;

        //8.4.2.1 Reference picture selection process
//        ret = Reference_picture_selection_process(refIdxL0, m_RefPicList0, m_RefPicList0Length, refPicL0);
//        RETURN_IF_FAILED(ret, -1);

        //8.4.2.2 Fractional sample interpolation process
        ret = Fractional_sample_interpolation_process(mbPartIdx, subMbPartIdx, partWidth, partHeight, partWidthC, partHeightC, xAL, yAL,
           mvL0, mvCL0, refPicL0, predPartL0L, predPartL0Cb, predPartL0Cr);
        RETURN_IF_FAILED(ret, -1);
    }
    
    if (predFlagL1 == 1)
    {
//        CH264PictureBase * refPicL1 = NULL;

        //8.4.2.1 Reference picture selection process
//        ret = Reference_picture_selection_process(refIdxL1, m_RefPicList1, m_RefPicList1Length, refPicL1);
//        RETURN_IF_FAILED(ret, -1);

        //8.4.2.2 Fractional sample interpolation process
        ret = Fractional_sample_interpolation_process(mbPartIdx, subMbPartIdx, partWidth, partHeight, partWidthC, partHeightC, xAL, yAL,
           mvL1, mvCL1, refPicL1, predPartL1L, predPartL1Cb, predPartL1Cr);
        RETURN_IF_FAILED(ret, -1);
    }

    //----------------------------
    //8.4.2.3 Weighted sample prediction process
    ret = Weighted_sample_prediction_process(mbPartIdx, subMbPartIdx, predFlagL0, predFlagL1, partWidth, partHeight, partWidthC, partHeightC,
                logWDL, w0L, w1L, o0L, o1L,
                logWDCb, w0Cb, w1Cb, o0Cb, o1Cb,
                logWDCr, w0Cr, w1Cr, o0Cr, o1Cr,
                predPartL0L, predPartL0Cb, predPartL0Cr,
                predPartL1L, predPartL1Cb, predPartL1Cr,
                predPartL, predPartCb, predPartCr);
    RETURN_IF_FAILED(ret != 0, -1);

    return 0;
}


//8.4.2.1 Reference picture selection process
// Page 164/186/812
int CH264PictureBase::Reference_picture_selection_process(int32_t refIdxLX, CH264Picture *RefPicListX[16], int32_t RefPicListXLength, CH264PictureBase *&refPic)
{
    int ret = 0;
    CH264SliceHeader & slice_header = m_h264_slice_header;

    RETURN_IF_FAILED(refIdxLX < 0 || refIdxLX >= 32, -1);

    if (slice_header.field_pic_flag == 1)
    {
        //each entry of RefPicListX is a reference field or a field of a reference frame.
        for (int i = 0; i < RefPicListXLength; i++)
        {
            if (!(RefPicListX[i]->m_picture_coded_type_marked_as_refrence == H264_PICTURE_CODED_TYPE_TOP_FIELD 
                || RefPicListX[i]->m_picture_coded_type_marked_as_refrence == H264_PICTURE_CODED_TYPE_BOTTOM_FIELD)
                )
            {
                RETURN_IF_FAILED(-1, -1);
            }
        }
    }
    else //if (slice_header.field_pic_flag == 0)
    {
        //each entry of RefPicListX is a reference frame or a complementary reference field pair.
        for (int i = 0; i < RefPicListXLength; i++)
        {
            if (!(RefPicListX[i]->m_picture_coded_type_marked_as_refrence == H264_PICTURE_CODED_TYPE_FRAME 
                || RefPicListX[i]->m_picture_coded_type_marked_as_refrence == H264_PICTURE_CODED_TYPE_COMPLEMENTARY_FIELD_PAIR)
                )
            {
                RETURN_IF_FAILED(-1, -1);
            }
        }
    }

    //---------------------
    if (slice_header.field_pic_flag == 1)
    {
        if (RefPicListX[refIdxLX]->m_picture_coded_type_marked_as_refrence == H264_PICTURE_CODED_TYPE_TOP_FIELD)
        {
            refPic = &(RefPicListX[ refIdxLX ]->m_picture_top_filed);
        }
        else if (RefPicListX[refIdxLX]->m_picture_coded_type_marked_as_refrence == H264_PICTURE_CODED_TYPE_BOTTOM_FIELD)
        {
            refPic = &(RefPicListX[ refIdxLX ]->m_picture_bottom_filed);
        }
        else
        {
            RETURN_IF_FAILED(-1, -1);
        }
    }
    else //if (slice_header.field_pic_flag == 0)
    {
        if (m_mbs[CurrMbAddr].mb_field_decoding_flag == 0) //If the current macroblock is a frame macroblock
        {
            refPic = &(RefPicListX[ refIdxLX ]->m_picture_frame);
        }
        else //the current macroblock is a field macroblock
        {
            if (refIdxLX % 2 == 0)
            {
                if (mb_y % 2 == 0)
                {
                    refPic = &(RefPicListX[ refIdxLX / 2 ]->m_picture_top_filed);
                }
                else
                {
                    refPic = &(RefPicListX[ refIdxLX / 2 ]->m_picture_bottom_filed);
                }
            }
            else //if (refIdxLX % 2 == 1)
            {
                if (mb_y % 2 == 0)
                {
                    refPic = &(RefPicListX[ refIdxLX / 2 ]->m_picture_bottom_filed);
                }
                else
                {
                    refPic = &(RefPicListX[ refIdxLX / 2 ]->m_picture_top_filed);
                }
            }
        }
    }

    //-----------------------------
    if (slice_header.m_sps.separate_colour_plane_flag == 0)
    {
        //FIXME: 8.7 Deblocking filter process
    }
    else //if (slice_header.m_sps.separate_colour_plane_flag == 1)
    {
        if (slice_header.colour_plane_id == 0)
        {

        }
        else if (slice_header.colour_plane_id == 1)
        {

        }
        else //if (slice_header.colour_plane_id == 2)
        {

        }
    }

    RETURN_IF_FAILED(refPic == NULL, -1);

    return 0;
}


//8.4.2.2 Fractional sample interpolation process
int CH264PictureBase::Fractional_sample_interpolation_process(int32_t mbPartIdx, int32_t subMbPartIdx, int32_t partWidth, int32_t partHeight, int32_t partWidthC, int32_t partHeightC, 
            int32_t xAL, int32_t yAL, 
            int32_t (&mvLX)[2], int32_t (&mvCLX)[2], CH264PictureBase *refPicLX,
            uint8_t *predPartLXL, //predPartL[partHeight][partWidth]
            uint8_t *predPartLXCb, //predPartCb[partHeightC][partWidthC]
            uint8_t *predPartLXCr) //predPartCr[partHeightC][partWidthC]
{
    int ret = 0;
    CH264SliceHeader & slice_header = m_h264_slice_header;

//    int32_t xAL = mb_x * 16;
//    int32_t yAL = mb_y * 16;

    for (int32_t yL = 0; yL < partHeight; yL++)
    {
        for (int32_t xL = 0; xL < partWidth; xL++)
        {
            int32_t xIntL = xAL + ( mvLX[ 0 ] >> 2 ) + xL;
            int32_t yIntL = yAL + ( mvLX[ 1 ] >> 2 ) + yL;
            int32_t xFracL = mvLX[ 0 ] & 3;
            int32_t yFracL = mvLX[ 1 ] & 3;

            //8.4.2.2.1 Luma sample interpolation process
            uint8_t predPartLXL_xL_yL = 0;
            ret = Luma_sample_interpolation_process(xIntL, yIntL, xFracL, yFracL, refPicLX, predPartLXL_xL_yL);
            RETURN_IF_FAILED(ret != 0, -1);
            predPartLXL[ yL * partWidth + xL ] = predPartLXL_xL_yL;
        }
    }

    //-------------------------
    if (slice_header.m_sps.ChromaArrayType != 0)
    {
        int32_t xIntC = 0;
        int32_t yIntC = 0;
        int32_t xFracC = 0;
        int32_t yFracC = 0;
        int32_t isChromaCb = 1;

        for (int32_t yC = 0; yC < partHeightC; yC++)
        {
            for (int32_t xC = 0; xC < partWidthC; xC++)
            {
                if (slice_header.m_sps.ChromaArrayType == 1)
                {
                    xIntC = ( xAL / slice_header.m_sps.SubWidthC ) + ( mvCLX[ 0 ] >> 3 ) + xC;
                    yIntC = ( yAL / slice_header.m_sps.SubHeightC ) + ( mvCLX[ 1 ] >> 3 ) + yC;
                    xFracC = mvCLX[ 0 ] & 7;
                    yFracC = mvCLX[ 1 ] & 7;
                }
                else if (slice_header.m_sps.ChromaArrayType == 2)
                {
                    xIntC = ( xAL / slice_header.m_sps.SubWidthC ) + ( mvCLX[ 0 ] >> 3 ) + xC;
                    yIntC = ( yAL / slice_header.m_sps.SubHeightC ) + ( mvCLX[ 1 ] >> 2 ) + yC;
                    xFracC = mvCLX[ 0 ] & 7;
                    yFracC = ( mvCLX[ 1 ] & 3 ) << 1;
                }
                else //if (slice_header.m_sps.ChromaArrayType == 3)
                {
                    xIntC = xAL + ( mvLX[ 0 ] >> 2 ) + xC;
                    yIntC = yAL + ( mvLX[ 1 ] >> 2 ) + yC;
                    xFracC = ( mvCLX[ 0 ] & 3 ); //FIXME: mvCX[ 0 ] ?
                    yFracC = ( mvCLX[ 1 ] & 3 );
                }

                if (slice_header.m_sps.ChromaArrayType != 3)
                {
                    //8.4.2.2.2 Chroma sample interpolation process
                    uint8_t predPartLXCb_xC_yC = 0;
                    isChromaCb = 1;
                    ret = Chroma_sample_interpolation_process(xIntC, yIntC, xFracC, yFracC, refPicLX, isChromaCb, predPartLXCb_xC_yC);
                    RETURN_IF_FAILED(ret != 0, -1);
                    predPartLXCb[ yC * partWidthC + xC ] = predPartLXCb_xC_yC;
                    
                    //8.4.2.2.2 Chroma sample interpolation process
                    uint8_t predPartLXCr_xC_yC = 0;
                    isChromaCb = 0;
                    ret = Chroma_sample_interpolation_process(xIntC, yIntC, xFracC, yFracC, refPicLX, isChromaCb, predPartLXCr_xC_yC);
                    RETURN_IF_FAILED(ret != 0, -1);
                    predPartLXCr[ yC * partWidthC + xC ] = predPartLXCr_xC_yC;
                }
                else
                {
                    //8.4.2.2.1 Luma sample interpolation process
                    uint8_t predPartLXCb_xC_yC = 0;
                    ret = Luma_sample_interpolation_process(xIntC, yIntC, xFracC, yFracC, refPicLX, predPartLXCb_xC_yC);
                    RETURN_IF_FAILED(ret != 0, -1);
                    predPartLXCb[ yC * partWidthC + xC ] = predPartLXCb_xC_yC;
                    
                    //8.4.2.2.1 Luma sample interpolation process
                    uint8_t predPartLXCr_xC_yC = 0;
                    ret = Luma_sample_interpolation_process(xIntC, yIntC, xFracC, yFracC, refPicLX, predPartLXCr_xC_yC);
                    RETURN_IF_FAILED(ret != 0, -1);
                    predPartLXCr[ yC * partWidthC + xC ] = predPartLXCr_xC_yC;
                }
            }
        }
    }

    return 0;
}


//8.4.2.2.1 Luma sample interpolation process
//亮度像素值插值过程
//Figure 8-4
// 
// 口       口       A     aa    B       口       口
// 口       口       C     bb    D       口       口
// E        F        G  a  b  c  H       I        J 
//                   d  e  f  g                     
// cc       dd       h  i  j  k  m       ee       ff
//                   n  p  q  r                     
// K        L        M     s     N       P        Q 
// 口       口       R     gg    S       口       口
// 口       口       T     hh    U       口       口
int CH264PictureBase::Luma_sample_interpolation_process(int32_t xIntL, int32_t yIntL, int32_t xFracL, int32_t yFracL, CH264PictureBase *refPic, uint8_t &predPartLXL_xL_yL)
{
    int ret = 0;
    
    int32_t refPicHeightEffectiveL = 0;
    CH264SliceHeader & slice_header = m_h264_slice_header;

    if (slice_header.MbaffFrameFlag == 0 || m_h264_slice_data.mb_field_decoding_flag == 0)
    {
        refPicHeightEffectiveL = PicHeightInSamplesL;
    }
    else //if (slice_header.MbaffFrameFlag == 1 && m_h264_slice_data.mb_field_decoding_flag == 1)
    {
        refPicHeightEffectiveL = PicHeightInSamplesL / 2;
    }

    //------------------------
    //#define CLIP3(x, y, z)  (((z) < (x)) ? (x) : (((z) > (y)) ? (y) : (z)))

#define getLumaSample(xDZL, yDZL)    refPic->m_pic_buff_luma[CLIP3( 0, refPicHeightEffectiveL - 1, yIntL + (yDZL) ) * refPic->PicWidthInSamplesL + CLIP3( 0, refPic->PicWidthInSamplesL - 1, xIntL + (xDZL) )]

    int32_t A = getLumaSample( 0, -2);
    int32_t B = getLumaSample( 1, -2);
    int32_t C = getLumaSample( 0, -1);
    int32_t D = getLumaSample( 1, -1);
    int32_t E = getLumaSample(-2,  0);
    int32_t F = getLumaSample(-1,  0);
    int32_t G = getLumaSample( 0,  0); //坐标原点
    int32_t H = getLumaSample( 1,  0);
    int32_t I = getLumaSample( 2,  0);
    int32_t J = getLumaSample( 3,  0);
    int32_t K = getLumaSample(-2,  1);
    int32_t L = getLumaSample(-1,  1);
    int32_t M = getLumaSample( 0,  1);
    int32_t N = getLumaSample( 1,  1);
    int32_t P = getLumaSample( 2,  1);
    int32_t Q = getLumaSample( 3,  1);
    int32_t R = getLumaSample( 0,  2);
    int32_t S = getLumaSample( 1,  2);
    int32_t T = getLumaSample( 0,  3);
    int32_t U = getLumaSample( 1,  3);
    
    int32_t X11 = getLumaSample(-2, -2); //A所在的行与E所在的列的交点
    int32_t X12 = getLumaSample(-1, -2);
    int32_t X13 = getLumaSample( 2, -2);
    int32_t X14 = getLumaSample( 3, -2);

    int32_t X21 = getLumaSample(-2, -1);
    int32_t X22 = getLumaSample(-1, -1);
    int32_t X23 = getLumaSample( 2, -1);
    int32_t X24 = getLumaSample( 3, -1);
    
    int32_t X31 = getLumaSample(-2,  2);
    int32_t X32 = getLumaSample(-1,  2);
    int32_t X33 = getLumaSample( 2,  2);
    int32_t X34 = getLumaSample( 3,  2);
    
    int32_t X41 = getLumaSample(-2,  3);
    int32_t X42 = getLumaSample(-1,  3);
    int32_t X43 = getLumaSample( 2,  3);
    int32_t X44 = getLumaSample( 3,  3);

    //-----------------------
    // a 6-tap filter with tap values ( 1, −5, 20, 20, −5, 1 ).
    // 6抽头滤波器

#define a_6_tap_filter(v1, v2, v3, v4, v5, v6)     ( (v1) - 5 * (v2) + 20 * (v3) + 20 * (v4) - 5 * (v5) + (v6) )

//    int32_t b1 = ( E - 5 * F + 20 * G + 20 * H - 5 * I + J );
//    int32_t h1 = ( A - 5 * C + 20 * G + 20 * M - 5 * R + T );
    
    int32_t b1 = a_6_tap_filter(E, F, G, H, I, J);
    int32_t s1 = a_6_tap_filter(K, L, M, N, P, Q);
    int32_t h1 = a_6_tap_filter(A, C, G, M, R, T);
    int32_t m1 = a_6_tap_filter(B, D, H, N, S, U);

    //b = Clip1Y( ( b1 + 16 ) >> 5 ) = Clip3( 0, ( 1 << BitDepthY ) - 1, x );
    int32_t b = CLIP3( 0, ( 1 << slice_header.m_sps.BitDepthY ) - 1, ( b1 + 16 ) >> 5 );
    int32_t s = CLIP3( 0, ( 1 << slice_header.m_sps.BitDepthY ) - 1, ( s1 + 16 ) >> 5 );
    int32_t h = CLIP3( 0, ( 1 << slice_header.m_sps.BitDepthY ) - 1, ( h1 + 16 ) >> 5 );
    int32_t m = CLIP3( 0, ( 1 << slice_header.m_sps.BitDepthY ) - 1, ( m1 + 16 ) >> 5 );
    
    int32_t aa = a_6_tap_filter(X11, X12, A, B, X13, X14);
    int32_t bb = a_6_tap_filter(X21, X22, C, D, X23, X24);
    int32_t gg = a_6_tap_filter(X31, X32, R, S, X33, X34);
    int32_t hh = a_6_tap_filter(X41, X42, T, U, X43, X44);
    
    int32_t cc = a_6_tap_filter(X11, X21, E, K, X31, X41);
    int32_t dd = a_6_tap_filter(X12, X22, F, L, X32, X42);
    int32_t ee = a_6_tap_filter(X13, X23, I, P, X33, X43);
    int32_t ff = a_6_tap_filter(X14, X24, J, Q, X34, X44);

    //--------------------
//    int32_t j1 = cc - 5 * dd + 20 * h1 + 20 * m1 - 5 * ee + ff;
//    int32_t j2 = aa - 5 * bb + 20 * b1 + 20 * s1 - 5 * gg + hh;
    
    int32_t j1 = a_6_tap_filter(cc, dd, h1, m1, ee, ff);
//    int32_t j2 = a_6_tap_filter(aa, bb, b1, s1, gg, hh);

//    RETURN_IF_FAILED(j1 != j2, -1);

    //int32_t j = Clip1Y( ( j1 + 512 ) >> 10 );
    int32_t j = CLIP3( 0, ( 1 << slice_header.m_sps.BitDepthY ) - 1, ( j1 + 512 ) >> 10 );
    
    //--------------------
    int32_t a = ( G + b + 1 ) >> 1;
    int32_t c = ( H + b + 1 ) >> 1;
    int32_t d = ( G + h + 1 ) >> 1;
    int32_t n = ( M + h + 1 ) >> 1;
    int32_t f = ( b + j + 1 ) >> 1;
    int32_t i = ( h + j + 1 ) >> 1;
    int32_t k = ( j + m + 1 ) >> 1;
    int32_t q = ( j + s + 1 ) >> 1;

    int32_t e = ( b + h + 1 ) >> 1;
    int32_t g = ( b + m + 1 ) >> 1;
    int32_t p = ( h + s + 1 ) >> 1;
    int32_t r = ( m + s + 1 ) >> 1;
    
    //----------------------
#undef a_6_tap_filter
#undef getLumaSample

    //-------------------------
    //Table 8-12 – Assignment of the luma prediction sample predPartLXL[ xL, yL ]
    int32_t predPartLXLs[4][4] = 
    {
        {G, d, h, n},
        {a, e, i, p},
        {b, f, j, q},
        {c, g, k, r},
    };

    predPartLXL_xL_yL = predPartLXLs[xFracL][yFracL];

    return 0;
}


//8.4.2.2.2 Chroma sample interpolation process
//This process is only invoked when ChromaArrayType is equal to 1 or 2.
int CH264PictureBase::Chroma_sample_interpolation_process(int32_t xIntC, int32_t yIntC, int32_t xFracC, int32_t yFracC, CH264PictureBase *refPic, int32_t isChromaCb, uint8_t &predPartLXC_xC_yC)
{
    int ret = 0;
    
    int32_t refPicHeightEffectiveC = 0;
    CH264SliceHeader & slice_header = m_h264_slice_header;

    if (slice_header.MbaffFrameFlag == 0 || m_h264_slice_data.mb_field_decoding_flag == 0)
    {
        refPicHeightEffectiveC = PicHeightInSamplesC;
    }
    else //if (slice_header.MbaffFrameFlag == 1 && m_h264_slice_data.mb_field_decoding_flag == 1)
    {
        refPicHeightEffectiveC = PicHeightInSamplesC / 2;
    }

    //----------------------
    uint8_t * refPicLC_pic_buff_cbcr = (isChromaCb == 1) ? refPic->m_pic_buff_cb : refPic->m_pic_buff_cr;

    int32_t xAC = CLIP3( 0, refPic->PicWidthInSamplesC - 1, xIntC ); // (8-262)
    int32_t xBC = CLIP3( 0, refPic->PicWidthInSamplesC - 1, xIntC + 1 ); // (8-263)
    int32_t xCC = CLIP3( 0, refPic->PicWidthInSamplesC - 1, xIntC ); // (8-264)
    int32_t xDC = CLIP3( 0, refPic->PicWidthInSamplesC - 1, xIntC + 1 ); // (8-265)

    int32_t yAC = CLIP3( 0, refPicHeightEffectiveC - 1, yIntC ); // (8-266)
    int32_t yBC = CLIP3( 0, refPicHeightEffectiveC - 1, yIntC ); // (8-267)
    int32_t yCC = CLIP3( 0, refPicHeightEffectiveC - 1, yIntC + 1 ); // (8-268)
    int32_t yDC = CLIP3( 0, refPicHeightEffectiveC - 1, yIntC + 1 ); // (8-269)

    int32_t A = refPicLC_pic_buff_cbcr[ yAC * refPic->PicWidthInSamplesC + xAC ];
    int32_t B = refPicLC_pic_buff_cbcr[ yBC * refPic->PicWidthInSamplesC + xBC ];
    int32_t C = refPicLC_pic_buff_cbcr[ yCC * refPic->PicWidthInSamplesC + xCC ];
    int32_t D = refPicLC_pic_buff_cbcr[ yDC * refPic->PicWidthInSamplesC + xDC ];

    predPartLXC_xC_yC = ( ( 8 - xFracC ) * ( 8 - yFracC ) * A + xFracC * ( 8 - yFracC ) * B + ( 8 - xFracC ) * yFracC * C + xFracC * yFracC * D + 32 ) >> 6;

    return 0;
}


//8.4.2.3 Weighted sample prediction process
int CH264PictureBase::Weighted_sample_prediction_process(int32_t mbPartIdx, int32_t subMbPartIdx, int32_t predFlagL0, int32_t predFlagL1, 
        int32_t partWidth, int32_t partHeight, int32_t partWidthC, int32_t partHeightC,
        int32_t logWDL, int32_t w0L, int32_t w1L, int32_t o0L, int32_t o1L,
        int32_t logWDCb, int32_t w0Cb, int32_t w1Cb, int32_t o0Cb, int32_t o1Cb,
        int32_t logWDCr, int32_t w0Cr, int32_t w1Cr, int32_t o0Cr, int32_t o1Cr,
        uint8_t *predPartL0L, //predPartLXL[partHeight][partWidth]
        uint8_t *predPartL0Cb, //predPartLXCb[partHeightC][partWidthC]
        uint8_t *predPartL0Cr, //predPartLXCr[partHeightC][partWidthC]
        uint8_t *predPartL1L, //predPartLXL[partHeight][partWidth]
        uint8_t *predPartL1Cb, //predPartLXCb[partHeightC][partWidthC]
        uint8_t *predPartL1Cr, //predPartLXCr[partHeightC][partWidthC]
        uint8_t *predPartL, //out: predPartL[partHeight][partWidth]
        uint8_t *predPartCb, //out: predPartLXCb[partHeightC][partWidthC]
        uint8_t *predPartCr //out: predPartLXCr[partHeightC][partWidthC]
        )
{
    int ret = 0;
    CH264SliceHeader & slice_header = m_h264_slice_header;
    
    if (predFlagL0 == 1 && (slice_header.slice_type % 5 == H264_SLIECE_TYPE_P || slice_header.slice_type % 5 == H264_SLIECE_TYPE_SP))
    {
        if (slice_header.m_pps.weighted_pred_flag == 0)
        {
            //8.4.2.3.1 Default weighted sample prediction process
            ret = Default_weighted_sample_prediction_process(predFlagL0, predFlagL1, partWidth, partHeight, partWidthC, partHeightC,
                    predPartL0L, predPartL0Cb, predPartL0Cr, predPartL1L, predPartL1Cb, predPartL1Cr, predPartL, predPartCb, predPartCr);
            RETURN_IF_FAILED(ret != 0, -1);
        }
        else //if (slice_header.m_pps.weighted_pred_flag == 1)
        {
            //8.4.2.3.2 Weighted sample prediction process
            ret = Weighted_sample_prediction_process_2(mbPartIdx, subMbPartIdx, predFlagL0, predFlagL1, partWidth, partHeight, partWidthC, partHeightC,
                logWDL, w0L, w1L, o0L, o1L,
                logWDCb, w0Cb, w1Cb, o0Cb, o1Cb,
                logWDCr, w0Cr, w1Cr, o0Cr, o1Cr,
                predPartL0L, predPartL0Cb, predPartL0Cr,
                predPartL1L, predPartL1Cb, predPartL1Cr,
                predPartL, predPartCb, predPartCr);
            RETURN_IF_FAILED(ret != 0, -1);
        }
    }
    else if ((predFlagL0 == 1 || predFlagL1 == 1) && (slice_header.slice_type % 5 == H264_SLIECE_TYPE_B))
    {
        if (slice_header.m_pps.weighted_bipred_idc == 0)
        {
            //8.4.2.3.1 Default weighted sample prediction process
            ret = Default_weighted_sample_prediction_process(predFlagL0, predFlagL1, partWidth, partHeight, partWidthC, partHeightC,
                    predPartL0L, predPartL0Cb, predPartL0Cr, predPartL1L, predPartL1Cb, predPartL1Cr, predPartL, predPartCb, predPartCr);
            RETURN_IF_FAILED(ret != 0, -1);
        }
        else if (slice_header.m_pps.weighted_bipred_idc == 1)
        {
            //8.4.2.3.2 Weighted sample prediction process
            ret = Weighted_sample_prediction_process_2(mbPartIdx, subMbPartIdx, predFlagL0, predFlagL1, partWidth, partHeight, partWidthC, partHeightC,
                logWDL, w0L, w1L, o0L, o1L,
                logWDCb, w0Cb, w1Cb, o0Cb, o1Cb,
                logWDCr, w0Cr, w1Cr, o0Cr, o1Cr,
                predPartL0L, predPartL0Cb, predPartL0Cr,
                predPartL1L, predPartL1Cb, predPartL1Cr,
                predPartL, predPartCb, predPartCr);
            RETURN_IF_FAILED(ret != 0, -1);
        }
        else //if (slice_header.m_pps.weighted_bipred_idc == 2)
        {
            if (predFlagL0 == 1 && predFlagL1 == 1)
            {
                //8.4.2.3.2 Weighted sample prediction process
                ret = Weighted_sample_prediction_process_2(mbPartIdx, subMbPartIdx, predFlagL0, predFlagL1, partWidth, partHeight, partWidthC, partHeightC,
                    logWDL, w0L, w1L, o0L, o1L,
                    logWDCb, w0Cb, w1Cb, o0Cb, o1Cb,
                    logWDCr, w0Cr, w1Cr, o0Cr, o1Cr,
                    predPartL0L, predPartL0Cb, predPartL0Cr,
                    predPartL1L, predPartL1Cb, predPartL1Cr,
                    predPartL, predPartCb, predPartCr);
                RETURN_IF_FAILED(ret != 0, -1);
            }
            else //if (predFlagL0 == 1 || predFlagL1 == 1)
            {
                //8.4.2.3.1 Default weighted sample prediction process
                ret = Default_weighted_sample_prediction_process(predFlagL0, predFlagL1, partWidth, partHeight, partWidthC, partHeightC,
                        predPartL0L, predPartL0Cb, predPartL0Cr, predPartL1L, predPartL1Cb, predPartL1Cr, predPartL, predPartCb, predPartCr);
                RETURN_IF_FAILED(ret != 0, -1);
            }
        }
    }
    
    return 0;
}


//8.4.2.3.1 Default weighted sample prediction process
int CH264PictureBase::Default_weighted_sample_prediction_process(int32_t predFlagL0, int32_t predFlagL1, 
        int32_t partWidth, int32_t partHeight, int32_t partWidthC, int32_t partHeightC,
        uint8_t *predPartL0L, uint8_t *predPartL0Cb, uint8_t *predPartL0Cr,
        uint8_t *predPartL1L, uint8_t *predPartL1Cb, uint8_t *predPartL1Cr,
        uint8_t *predPartL, uint8_t *predPartCb, uint8_t *predPartCr
        )
{
    int ret = 0;
    CH264SliceHeader & slice_header = m_h264_slice_header;

    if (predFlagL0 == 1 && predFlagL1 == 0)
    {
        for (int y = 0; y <= partHeight - 1; y++)
        {
            for (int x = 0; x <= partWidth - 1; x++)
            {
                predPartL[y * partWidth + x] = predPartL0L[y * partWidth + x];
            }
        }

        if (slice_header.m_sps.ChromaArrayType != 0)
        {
            for (int y = 0; y <= partHeightC - 1; y++)
            {
                for (int x = 0; x <= partWidthC - 1; x++)
                {
                    predPartCb[y * partWidthC + x] = predPartL0Cb[y * partWidthC + x];
                    predPartCr[y * partWidthC + x] = predPartL0Cr[y * partWidthC + x];
                }
            }
        }
    }
    else if (predFlagL0 == 0 && predFlagL1 == 1)
    {
        for (int y = 0; y <= partHeight - 1; y++)
        {
            for (int x = 0; x <= partWidth - 1; x++)
            {
                predPartL[y * partWidth + x] = predPartL1L[y * partWidth + x];
            }
        }

        if (slice_header.m_sps.ChromaArrayType != 0)
        {
            for (int y = 0; y <= partHeightC - 1; y++)
            {
                for (int x = 0; x <= partWidthC - 1; x++)
                {
                    predPartCb[y * partWidthC + x] = predPartL1Cb[y * partWidthC + x];
                    predPartCr[y * partWidthC + x] = predPartL1Cr[y * partWidthC + x];
                }
            }
        }
    }
    else //if (predFlagL0 == 1 && predFlagL1 == 1)
    {
        for (int y = 0; y <= partHeight - 1; y++)
        {
            for (int x = 0; x <= partWidth - 1; x++)
            {
                predPartL[y * partWidth + x] = (predPartL0L[y * partWidth + x] + predPartL1L[y * partWidth + x] + 1) >> 1;
            }
        }

        if (slice_header.m_sps.ChromaArrayType != 0)
        {
            for (int y = 0; y <= partHeightC - 1; y++)
            {
                for (int x = 0; x <= partWidthC - 1; x++)
                {
                    predPartCb[y * partWidthC + x] = (predPartL0Cb[y * partWidthC + x] + predPartL1Cb[y * partWidthC + x] + 1) >> 1;
                    predPartCr[y * partWidthC + x] = (predPartL0Cr[y * partWidthC + x] + predPartL1Cr[y * partWidthC + x] + 1) >> 1;
                }
            }
        }
    }
    
    return 0;
}


//8.4.2.3.2 Weighted sample prediction process
int CH264PictureBase::Weighted_sample_prediction_process_2(int32_t mbPartIdx, int32_t subMbPartIdx, int32_t predFlagL0, int32_t predFlagL1, 
        int32_t partWidth, int32_t partHeight, int32_t partWidthC, int32_t partHeightC,
        int32_t logWDL, int32_t w0L, int32_t w1L, int32_t o0L, int32_t o1L,
        int32_t logWDCb, int32_t w0Cb, int32_t w1Cb, int32_t o0Cb, int32_t o1Cb,
        int32_t logWDCr, int32_t w0Cr, int32_t w1Cr, int32_t o0Cr, int32_t o1Cr,
        uint8_t *predPartL0L, uint8_t *predPartL0Cb, uint8_t *predPartL0Cr,
        uint8_t *predPartL1L, uint8_t *predPartL1Cb, uint8_t *predPartL1Cr,
        uint8_t *predPartL, uint8_t *predPartCb, uint8_t *predPartCr
        )
{
    int ret = 0;
    CH264SliceHeader & slice_header = m_h264_slice_header;

    if (predFlagL0 == 1 && predFlagL1 == 0)
    {
        for (int y = 0; y <= partHeight - 1; y++)
        {
            for (int x = 0; x <= partWidth - 1; x++)
            {
                if ( logWDL >= 1 ) //Clip1Y( x ) = Clip3( 0, ( 1 << BitDepthY ) − 1, x )
                {
                    predPartL[y * partWidth + x] = CLIP3( 0, ( 1 << slice_header.m_sps.BitDepthY ) - 1, ( ( predPartL0L[y * partWidth + x] * w0L + h264_power2(logWDL - 1) ) >> logWDL ) + o0L );
                }
                else
                {
                    predPartL[y * partWidth + x] = CLIP3( 0, ( 1 << slice_header.m_sps.BitDepthY ) - 1, predPartL0L[y * partWidth + x] * w0L + o0L );
                }
            }
        }

        if (slice_header.m_sps.ChromaArrayType != 0)
        {
            for (int y = 0; y <= partHeightC - 1; y++)
            {
                for (int x = 0; x <= partWidthC - 1; x++)
                {
                    if ( logWDCb >= 1 ) //Clip1Y( x ) = Clip3( 0, ( 1 << BitDepthC ) − 1, x )
                    {
                        predPartCb[y * partWidthC + x] = CLIP3( 0, ( 1 << slice_header.m_sps.BitDepthC ) - 1, ( ( predPartL0Cb[y * partWidthC + x] * w0Cb + h264_power2(logWDCb - 1) ) >> logWDCb ) + o0Cb );
                    }
                    else
                    {
                        predPartCb[y * partWidthC + x] = CLIP3( 0, ( 1 << slice_header.m_sps.BitDepthC ) - 1, predPartL0Cb[y * partWidthC + x] * w0Cb + o0Cb );
                    }

                    if ( logWDCr >= 1 ) //Clip1Y( x ) = Clip3( 0, ( 1 << BitDepthC ) − 1, x )
                    {
                        predPartCr[y * partWidthC + x] = CLIP3( 0, ( 1 << slice_header.m_sps.BitDepthC ) - 1, ( ( predPartL0Cr[y * partWidthC + x] * w0Cr + h264_power2(logWDCr - 1) ) >> logWDCr ) + o0Cr );
                    }
                    else
                    {
                        predPartCr[y * partWidthC + x] = CLIP3( 0, ( 1 << slice_header.m_sps.BitDepthC ) - 1, predPartL0Cr[y * partWidthC + x] * w0Cr + o0Cr );
                    }
                }
            }
        }
    }
    else if (predFlagL0 == 0 && predFlagL1 == 1)
    {
        for (int y = 0; y <= partHeight - 1; y++)
        {
            for (int x = 0; x <= partWidth - 1; x++)
            {
                if ( logWDL >= 1 ) //Clip1Y( x ) = Clip3( 0, ( 1 << BitDepthY ) − 1, x )
                {
                    predPartL[y * partWidth + x] = CLIP3( 0, ( 1 << slice_header.m_sps.BitDepthY ) - 1, ( ( predPartL1L[y * partWidth + x] * w0L + h264_power2(logWDL - 1) ) >> logWDL ) + o0L );
                }
                else
                {
                    predPartL[y * partWidth + x] = CLIP3( 0, ( 1 << slice_header.m_sps.BitDepthY ) - 1, predPartL1L[y * partWidth + x] * w0L + o0L );
                }
            }
        }

        if (slice_header.m_sps.ChromaArrayType != 0)
        {
            for (int y = 0; y <= partHeightC - 1; y++)
            {
                for (int x = 0; x <= partWidthC - 1; x++)
                {
                    if ( logWDCb >= 1 ) //Clip1Y( x ) = Clip3( 0, ( 1 << BitDepthC ) − 1, x )
                    {
                        predPartCb[y * partWidthC + x] = CLIP3( 0, ( 1 << slice_header.m_sps.BitDepthC ) - 1, ( ( predPartL1Cb[y * partWidthC + x] * w1Cb + h264_power2(logWDCb - 1) ) >> logWDCb ) + o1Cb );
                    }
                    else
                    {
                        predPartCb[y * partWidthC + x] = CLIP3( 0, ( 1 << slice_header.m_sps.BitDepthC ) - 1, predPartL1Cb[y * partWidthC + x] * w1Cb + o1Cb );
                    }
                    
                    if ( logWDCr >= 1 ) //Clip1Y( x ) = Clip3( 0, ( 1 << BitDepthC ) − 1, x )
                    {
                        predPartCr[y * partWidthC + x] = CLIP3( 0, ( 1 << slice_header.m_sps.BitDepthC ) - 1, ( ( predPartL1Cr[y * partWidthC + x] * w1Cr + h264_power2(logWDCr - 1) ) >> logWDCr ) + o1Cr );
                    }
                    else
                    {
                        predPartCr[y * partWidthC + x] = CLIP3( 0, ( 1 << slice_header.m_sps.BitDepthC ) - 1, predPartL1Cr[y * partWidthC + x] * w1Cr + o1Cr );
                    }
                }
            }
        }
    }
    else if (predFlagL0 == 1 && predFlagL1 == 1) //处理B帧的双向预测结果
    {
        for (int y = 0; y <= partHeight - 1; y++)
        {
            for (int x = 0; x <= partWidth - 1; x++)
            {
                //predPartC[ x, y ] = Clip1( ( ( predPartL0C[ x, y ] * w0C + predPartL1C[ x, y ] * w1C + 2logWDC ) >> ( logWDC + 1 ) ) + ( ( o0C + o1C + 1 ) >> 1 ) );
                predPartL[y * partWidth + x] = CLIP3( 0, ( 1 << slice_header.m_sps.BitDepthY ) - 1, 
                    ( ( predPartL0L[y * partWidth + x] * w0L + predPartL1L[y * partWidth + x] * w1L + h264_power2(logWDL) ) >> ( logWDL + 1 ) ) + ( ( o0L + o1L + 1 ) >> 1 ) );
            }
        }

        if (slice_header.m_sps.ChromaArrayType != 0)
        {
            for (int y = 0; y <= partHeightC - 1; y++)
            {
                for (int x = 0; x <= partWidthC - 1; x++)
                {
                    predPartCb[y * partWidthC + x] = CLIP3( 0, ( 1 << slice_header.m_sps.BitDepthC ) - 1, 
                        ( ( predPartL0Cb[y * partWidthC + x] * w0Cb + predPartL1Cb[y * partWidthC + x] * w1Cb + h264_power2(logWDCb) ) >> ( logWDCb + 1 ) ) + ( ( o0Cb + o1Cb + 1 ) >> 1 ) );
                    
                    predPartCr[y * partWidthC + x] = CLIP3( 0, ( 1 << slice_header.m_sps.BitDepthC ) - 1, 
                        ( ( predPartL0Cr[y * partWidthC + x] * w0Cr + predPartL1Cr[y * partWidthC + x] * w1Cr + h264_power2(logWDCr) ) >> ( logWDCr + 1 ) ) + ( ( o0Cr + o1Cr + 1 ) >> 1 ) );
                }
            }
        }
    }

    return 0;
}


//8.4.3 Derivation process for prediction weights
int CH264PictureBase::Derivation_process_for_prediction_weights(int32_t refIdxL0, int32_t refIdxL1, int32_t predFlagL0, int32_t predFlagL1,
        int32_t &logWDL, int32_t &w0L, int32_t &w1L, int32_t &o0L, int32_t &o1L,
        int32_t &logWDCb, int32_t &w0Cb, int32_t &w1Cb, int32_t &o0Cb, int32_t &o1Cb,
        int32_t &logWDCr, int32_t &w0Cr, int32_t &w1Cr, int32_t &o0Cr, int32_t &o1Cr)
{
    int ret = 0;

    CH264SliceHeader & slice_header = m_h264_slice_header;

    //-----------------------------
    int32_t implicitModeFlag = 0;
    int32_t explicitModeFlag = 0;

    if (slice_header.m_pps.weighted_bipred_idc == 2 && (slice_header.slice_type % 5) == 1 && predFlagL0 == 1 && predFlagL1 == 1)
    {
         implicitModeFlag = 1;
         explicitModeFlag = 0;
    }
    else if (slice_header.m_pps.weighted_bipred_idc == 1 && (slice_header.slice_type % 5) == 1 && (predFlagL0 + predFlagL1 == 1 || predFlagL0 + predFlagL1 == 2))
    {
         implicitModeFlag = 0;
         explicitModeFlag = 1;
    }
    else if (slice_header.m_pps.weighted_pred_flag == 1 && ((slice_header.slice_type % 5) == 0 || (slice_header.slice_type % 5) == 3) && predFlagL0 == 1)
    {
         implicitModeFlag = 0;
         explicitModeFlag = 1;
    }
    else
    {
         implicitModeFlag = 0;
         explicitModeFlag = 0;
    }

    //-----------------------------
    if (implicitModeFlag == 1)
    {
        logWDL = 5;
        o0L = 0;
        o1L = 0;

        if (slice_header.m_sps.ChromaArrayType != 0)
        {
            logWDCb = 5;
            o0Cb = 0;
            o1Cb = 0;

            logWDCr = 5;
            o0Cr = 0;
            o1Cr = 0;
        }
        
        //-----------------------------
        CH264PictureBase * currPicOrField = NULL;
        CH264PictureBase * pic0 = NULL;
        CH264PictureBase * pic1 = NULL;

        if (slice_header.field_pic_flag == 0 && m_mbs[CurrMbAddr].mb_field_decoding_flag == 1) //If field_pic_flag is equal to 0 and the current macroblock is a field macroblock
        {
            if (CurrMbAddr % 2 == 0) //currPicOrField is the field of the current picture CurrPic that has the same parity as the current macroblock.
            {
                currPicOrField = &(m_parent->m_picture_top_filed);
            }
            else
            {
                currPicOrField = &(m_parent->m_picture_bottom_filed);
            }

            if (refIdxL0 % 2 == 0)
            {
                if (CurrMbAddr % 2 == 0) //pic0 is the field of RefPicList0[ refIdxL0 / 2 ] that has the same parity as the current macroblock.
                {
                    pic0 = &(m_RefPicList0[ refIdxL0 / 2 ]->m_picture_top_filed);
                }
                else
                {
                    pic0 = &(m_RefPicList0[ refIdxL0 / 2 ]->m_picture_bottom_filed);
                }
            }
            else //if (refIdxL0 % 2 != 0)
            {
                if (CurrMbAddr % 2 == 0) //pic0 is the field of RefPicList0[ refIdxL0 / 2 ] that has the opposite parity of the current macroblock.
                {
                    pic0 = &(m_RefPicList0[ refIdxL0 / 2 ]->m_picture_bottom_filed);
                }
                else
                {
                    pic0 = &(m_RefPicList0[ refIdxL0 / 2 ]->m_picture_top_filed);
                }
            }
            
            if (refIdxL1 % 2 == 0)
            {
                if (CurrMbAddr % 2 == 0) //pic1 is the field of RefPicList1[ refIdxL1 / 2 ] that has the same parity as the current macroblock.
                {
                    pic1 = &(m_RefPicList1[ refIdxL1 / 2 ]->m_picture_top_filed);
                }
                else
                {
                    pic1 = &(m_RefPicList1[ refIdxL1 / 2 ]->m_picture_bottom_filed);
                }
            }
            else //if (refIdxL0 % 2 != 0)
            {
                if (CurrMbAddr % 2 == 0) //pic1 is the field of RefPicList1[ refIdxL1 / 2 ] that has the opposite parity of the current macroblock.
                {
                    pic1 = &(m_RefPicList1[ refIdxL1 / 2 ]->m_picture_bottom_filed);
                }
                else
                {
                    pic1 = &(m_RefPicList1[ refIdxL1 / 2 ]->m_picture_top_filed);
                }
            }
        }
        else //field_pic_flag is equal to 1 or the current macroblock is a frame macroblock
        {
            currPicOrField = &(m_parent->m_picture_frame);
            pic0 = &(m_RefPicList0[ refIdxL0 ]->m_picture_frame);
            pic1 = &(m_RefPicList1[ refIdxL1 ]->m_picture_frame);
        }

        //------------------------------------
        int32_t tb = CLIP3( -128, 127, DiffPicOrderCnt( currPicOrField, pic0 ) ); //(8-201)
        int32_t td = CLIP3( -128, 127, DiffPicOrderCnt( pic1, pic0 ) ); //(8-202)

        int32_t tx = ( 16384 + ABS( td / 2 ) ) / td; //(8-197)
        int32_t DistScaleFactor = CLIP3( -1024, 1023, ( tb * tx + 32 ) >> 6 ); //(8-198)

        if (DiffPicOrderCnt( pic1, pic0 ) == 0
            || pic0->reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference
            || pic1->reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference
            || ( DistScaleFactor >> 2 ) < -64 
            || ( DistScaleFactor >> 2 ) > 128 
            )
        {
            w0L = 32;
            w1L = 32;
            
            if (slice_header.m_sps.ChromaArrayType != 0)
            {
                w0Cb = 32;
                w1Cb = 32;
                w0Cr = 32;
                w1Cr = 32;
            }
        }
        else
        {
            w0L = 64 - (DistScaleFactor >> 2);
            w1L = DistScaleFactor >> 2;
            
            if (slice_header.m_sps.ChromaArrayType != 0)
            {
                w0Cb = 64 - (DistScaleFactor >> 2);
                w1Cb = DistScaleFactor >> 2;
                w0Cr = 64 - (DistScaleFactor >> 2);
                w1Cr = DistScaleFactor >> 2;
            }
        }
    }
    else if (explicitModeFlag == 1)
    {
        int32_t refIdxL0WP = 0;
        int32_t refIdxL1WP = 0;

        if (slice_header.MbaffFrameFlag == 1 && m_mbs[CurrMbAddr].mb_field_decoding_flag == 1) //If MbaffFrameFlag is equal to 1 and the current macroblock is a field macroblock
        {
            refIdxL0WP = refIdxL0 >> 1;
            refIdxL1WP = refIdxL1 >> 1;
        }
        else //Otherwise (MbaffFrameFlag is equal to 0 or the current macroblock is a frame macroblock)
        {
            refIdxL0WP = refIdxL0;
            refIdxL1WP = refIdxL1;
        }

        //If C is equal to L for luma samples
        logWDL = slice_header.luma_log2_weight_denom;
        w0L = slice_header.luma_weight_l0[ refIdxL0WP ];
        w1L = slice_header.luma_weight_l1[ refIdxL1WP ];
        o0L = slice_header.luma_offset_l0[ refIdxL0WP ] * ( 1 << ( slice_header.m_sps.BitDepthY - 8 ) );
        o1L = slice_header.luma_offset_l1[ refIdxL1WP ] * ( 1 << ( slice_header.m_sps.BitDepthY - 8 ) );

        //C is equal to Cb or Cr for chroma samples
        if (slice_header.m_sps.ChromaArrayType != 0)
        {
            logWDCb = slice_header.chroma_log2_weight_denom;
            w0Cb = slice_header.chroma_weight_l0[ refIdxL0WP ][ 0 ];
            w1Cb = slice_header.chroma_weight_l1[ refIdxL1WP ][ 0 ];
            o0Cb = slice_header.chroma_offset_l0[ refIdxL0WP ][ 0 ] * ( 1 << ( slice_header.m_sps.BitDepthC - 8 ) );
            o1Cb = slice_header.chroma_offset_l1[ refIdxL1WP ][ 0 ] * ( 1 << ( slice_header.m_sps.BitDepthC - 8 ) );
            
            logWDCr = slice_header.chroma_log2_weight_denom;
            w0Cr = slice_header.chroma_weight_l0[ refIdxL0WP ][ 1 ];
            w1Cr = slice_header.chroma_weight_l1[ refIdxL1WP ][ 1 ];
            o0Cr = slice_header.chroma_offset_l0[ refIdxL0WP ][ 1 ] * ( 1 << ( slice_header.m_sps.BitDepthC - 8 ) );
            o1Cr = slice_header.chroma_offset_l1[ refIdxL1WP ][ 1 ] * ( 1 << ( slice_header.m_sps.BitDepthC - 8 ) );
        }
    }
    else //if (implicitModeFlag == 0 && explicitModeFlag == 0)
    {
        //the variables logWDC, w0C, w1C, o0C, o1C are not used in the reconstruction process for the current macroblock.
    }

    //-----------------
    if (explicitModeFlag == 1 && predFlagL0 == 1 && predFlagL1 == 1)
    {
        //−128 <= w0C + w1C <= ( ( logWDC = = 7 ) ? 127 : 128 )
        RETURN_IF_FAILED(!(-128 <= w0L + w1L && w0L + w1L <= ( ( logWDL == 7 ) ? 127 : 128)), -1);
        if (slice_header.m_sps.ChromaArrayType != 0)
        {
            RETURN_IF_FAILED(!(-128 <= w0Cb + w1Cb && w0Cb + w1Cb <= ( ( logWDCb == 7 ) ? 127 : 128)), -1);
            RETURN_IF_FAILED(!(-128 <= w0Cr + w1Cr && w0Cb + w1Cr <= ( ( logWDCr == 7 ) ? 127 : 128)), -1);
        }
    }

    return 0;
}


//6.4.2.2 Inverse sub-macroblock partition scanning process
int CH264PictureBase::Inverse_sub_macroblock_partition_scanning_process(CH264MacroBlock *mb, int32_t mbPartIdx, int32_t subMbPartIdx, int32_t &x, int32_t &y)
{
    int ret = 0;
    CH264SliceHeader & slice_header = m_h264_slice_header;
    
    int32_t MbPartWidth = mb->MbPartWidth;
    int32_t MbPartHeight = mb->MbPartHeight;
    int32_t SubMbPartWidth = mb->SubMbPartWidth[mbPartIdx];
    int32_t SubMbPartHeight = mb->SubMbPartHeight[mbPartIdx];

    if (mb->m_name_of_mb_type == P_8x8 || mb->m_name_of_mb_type == P_8x8ref0 || mb->m_name_of_mb_type == B_8x8)
    {
        x = InverseRasterScan( subMbPartIdx, SubMbPartWidth, SubMbPartHeight, 8, 0 );
        y = InverseRasterScan( subMbPartIdx, SubMbPartWidth, SubMbPartHeight, 8, 1 );
    }
    else
    {
        x = InverseRasterScan( subMbPartIdx, 4, 4, 8, 0 );
        y = InverseRasterScan( subMbPartIdx, 4, 4, 8, 1 );
    }

    return 0;
}


//6.4.11.7 Derivation process for neighbouring partitions
int CH264PictureBase::Derivation_process_for_neighbouring_partitions(int32_t xN, int32_t yN, int32_t mbPartIdx, H264_MB_TYPE currSubMbType, int32_t subMbPartIdx, int32_t isChroma,
        int32_t &mbAddrN, int32_t &mbPartIdxN, int32_t &subMbPartIdxN)
{
    int ret = 0;
    CH264SliceHeader & slice_header = m_h264_slice_header;

    //---------------------------------------
    //1. The inverse macroblock partition scanning process as described in clause 6.4.2.1 is invoked 
    //with mbPartIdx as the input and ( x, y ) as the output.

    //2. The location of the upper-left luma sample inside a macroblock partition ( xS, yS ) is derived as follows:
    //   – If mb_type is equal to P_8x8, P_8x8ref0 or B_8x8, the inverse sub-macroblock partition scanning process 
    //     as described in clause 6.4.2.2 is invoked with subMbPartIdx as the input and ( xS, yS ) as the output.
    //   – Otherwise, ( xS, yS ) are set to ( 0, 0 ).

    //3. The variable predPartWidth in Table 6-2 is specified as follows:
    //   – If mb_type is equal to P_Skip, B_Skip, or B_Direct_16x16, predPartWidth = 16.
    //   – Otherwise, if mb_type is equal to B_8x8, the following applies:
    //     – If currSubMbType is equal to B_Direct_8x8, predPartWidth = 16.
    //     – Otherwise, predPartWidth = SubMbPartWidth( sub_mb_type[ mbPartIdx ] ).
    //   – Otherwise, if mb_type is equal to P_8x8 or P_8x8ref0, predPartWidth = SubMbPartWidth( sub_mb_type[ mbPartIdx ] ).
    //   – Otherwise, predPartWidth = MbPartWidth( mb_type ).

    //4. The difference of luma location ( xD, yD ) is set according to Table 6-2.

    //5. The neighbouring luma location ( xN, yN ) is specified by
    //       xN = x + xS + xD    (6-29)
    //       yN = y + yS + yD    (6-30)
    
    //6. The derivation process for neighbouring locations as specified in clause 6.4.12 is invoked for 
    //luma locations with ( xN, yN ) as the input and the output is assigned to mbAddrN and ( xW, yW ).

    //---------------------------------------
    //mbAddrA\mbPartIdxA\subMbPartIdxA
    //6.4.12 Derivation process for neighbouring locations
    int32_t xW = 0;
    int32_t yW = 0;
    int32_t maxW = 0;
    int32_t maxH = 0;

    MB_ADDR_TYPE mbAddrN_type = MB_ADDR_TYPE_UNKOWN;
    int32_t luma4x4BlkIdxN = 0;
    int32_t luma8x8BlkIdxN = 0;

    if (isChroma == 0)
    {
        maxW = 16;
        maxH = 16;
    }
    else //if (isChroma == 1)
    {
        maxW = MbWidthC;
        maxH = MbHeightC;
    }

    if (slice_header.MbaffFrameFlag == 0)
    {
        //6.4.12.1 Specification for neighbouring locations in fields and non-MBAFF frames
        ret = getMbAddrN_non_MBAFF_frames(xN, yN, maxW, maxH, CurrMbAddr, mbAddrN_type, mbAddrN, luma4x4BlkIdxN, luma8x8BlkIdxN, xW, yW, isChroma);
        RETURN_IF_FAILED(ret != 0, ret);
    }
    else //if (slice_header.MbaffFrameFlag == 1)
    {
        //6.4.12.2 Specification for neighbouring locations in MBAFF frames
        ret = getMbAddrN_MBAFF_frames(xN, yN, maxW, maxH, CurrMbAddr, mbAddrN_type, mbAddrN, luma4x4BlkIdxN, luma8x8BlkIdxN, xW, yW, isChroma);
        RETURN_IF_FAILED(ret != 0, ret);
    }
    
    //----------------
    if (mbAddrN < 0) //mbAddrN is not available
    {
        mbAddrN = NA;
        mbPartIdxN = NA;
        subMbPartIdxN = NA;
    }
    else //mbAddrN is available
    {
        //6.4.13.4 Derivation process for macroblock and sub-macroblock partition indices
        ret = Derivation_process_for_macroblock_and_sub_macroblock_partition_indices(m_mbs[mbAddrN].m_name_of_mb_type, m_mbs[mbAddrN].m_name_of_sub_mb_type, xW, yW, mbPartIdxN, subMbPartIdxN);
        RETURN_IF_FAILED(ret != 0, ret);

        //FIXME:
        //When the partition given by mbPartIdxN and subMbPartIdxN is not yet decoded, the macroblock
        //partition mbPartIdxN and the sub-macroblock partition subMbPartIdxN are marked as not available.
        if (m_mbs[mbAddrN].NumSubMbPart[mbPartIdxN] > subMbPartIdxN && m_mbs[mbAddrN].m_isDecoded[mbPartIdxN][subMbPartIdxN] == 0)
        {
            //mbAddrN = NA;
            //mbPartIdxN = NA;
            //subMbPartIdxN = NA;
        }
    }

    return 0;
}


//6.4.13.4 Derivation process for macroblock and sub-macroblock partition indices
int CH264PictureBase::Derivation_process_for_macroblock_and_sub_macroblock_partition_indices(H264_MB_TYPE mb_type_, H264_MB_TYPE subMbType_[4], int32_t xP, int32_t yP, int32_t &mbPartIdxN, int32_t &subMbPartIdxN)
{
    int ret = 0;
    CH264SliceHeader & slice_header = m_h264_slice_header;

    if (mb_type_ == MB_TYPE_NA)
    {
        LOG_ERROR("mb_type_ == MB_TYPE_NA\n", mb_type_);
        return -1;
    }

    //---------------------
    if (mb_type_ >= I_NxN && mb_type_ <= I_PCM) //If mbType specifies an I macroblock type
    {
        mbPartIdxN = 0;
    }
    else //mbType does not specify an I macroblock type
    {
        int32_t MbPartWidth = 0;
        int32_t MbPartHeight = 0;

        ret = CH264MacroBlock::getMbPartWidthAndHeight(mb_type_, MbPartWidth, MbPartHeight);
        RETURN_IF_FAILED(ret != 0, ret);

        //mbPartIdx = ( 16 / MbPartWidth( mbType ) ) * ( yP / MbPartHeight( mbType ) ) + ( xP / MbPartWidth( mbType ) );
        mbPartIdxN = (16 / MbPartWidth) * (yP / MbPartHeight) + (xP / MbPartWidth);
    }
    
    //If mbType is not equal to P_8x8, P_8x8ref0, B_8x8, B_Skip, or B_Direct_16x16, subMbPartIdx is set equal to 0.
    if (mb_type_ != P_8x8 
        && mb_type_ != P_8x8ref0 
        && mb_type_ != B_8x8 
        && mb_type_ != B_Skip 
        && mb_type_ != B_Direct_16x16)
    {
        subMbPartIdxN = 0;
    }
    else if (mb_type_ == B_Skip 
        || mb_type_ == B_Direct_16x16)
    {
        //subMbPartIdx = 2 * ( ( yP % 8 ) / 4 ) + ( ( xP % 8 ) / 4 );
        subMbPartIdxN = 2 * ((yP % 8) / 4) + ((xP % 8) / 4);
    }
    else //mbType is equal to P_8x8, P_8x8ref0, or B_8x8
    {
        int32_t SubMbPartWidth = 0;
        int32_t SubMbPartHeight = 0;
        
        ret = CH264MacroBlock::getMbPartWidthAndHeight(subMbType_[mbPartIdxN], SubMbPartWidth, SubMbPartHeight);
        RETURN_IF_FAILED(ret != 0, ret);

        //subMbPartIdx = ( 8 / SubMbPartWidth( subMbType[ mbPartIdx ] ) ) * ( ( yP % 8 ) / SubMbPartHeight( subMbType[ mbPartIdx ] ) ) + ( ( xP % 8 ) / SubMbPartWidth( subMbType[ mbPartIdx ] ) )
        subMbPartIdxN = (8 / SubMbPartWidth) * ((yP % 8) / SubMbPartHeight) + ((xP % 8) / SubMbPartWidth);
    }

    return 0;
}


//8.2.1 Decoding process for picture order count
int CH264PictureBase::PicOrderCntFunc(CH264PictureBase *picX) //POC: picture order count 图像序列号
{
    if (picX->m_picture_coded_type == H264_PICTURE_CODED_TYPE_FRAME 
        || picX->m_picture_coded_type == H264_PICTURE_CODED_TYPE_COMPLEMENTARY_FIELD_PAIR
        ) //picX is a frame or a complementary field pair  //当前图像为帧
    {
        picX->PicOrderCnt = MIN(picX->TopFieldOrderCnt, picX->BottomFieldOrderCnt); // of the frame or complementary field pair picX
    }
    else if (picX->m_picture_coded_type == H264_PICTURE_CODED_TYPE_TOP_FIELD) //当前图像为顶场 (picX is a top field)
    {
        picX->PicOrderCnt = picX->TopFieldOrderCnt; // of field picX
    }
    else if (picX->m_picture_coded_type == H264_PICTURE_CODED_TYPE_BOTTOM_FIELD)//当前图像为底场 if (picX is a bottom field)
    {
        picX->PicOrderCnt = picX->BottomFieldOrderCnt; // of field picX
    }
    else
    {
        RETURN_IF_FAILED(-1, -1);
    }

    return picX->PicOrderCnt;
}


int CH264PictureBase::DiffPicOrderCnt(CH264PictureBase *picA, CH264PictureBase *picB)
{
    int32_t DiffPicOrderCnt = PicOrderCntFunc(picA) - PicOrderCntFunc(picB);
    return DiffPicOrderCnt;
}
