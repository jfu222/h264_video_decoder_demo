//
// H264ResidualBlockCavlc.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264ResidualBlockCavlc.h"
#include "H264Golomb.h"
#include "CommonFunction.h"


CH264ResidualBlockCavlc::CH264ResidualBlockCavlc()
{
    memset(&levelVal, 0, sizeof(int32_t) * 16);
    memset(&runVal, 0, sizeof(int32_t) * 16);
}


CH264ResidualBlockCavlc::~CH264ResidualBlockCavlc()
{

}


int CH264ResidualBlockCavlc::printInfo()
{
    printf("---------h264 residual block cavlc info------------\n");

    return 0;
}


int CH264ResidualBlockCavlc::residual_block_cavlc(CH264PictureBase &picture, CBitstream &bs, int32_t *coeffLevel, int32_t startIdx, int32_t endIdx, 
                                    int32_t maxNumCoeff, MB_RESIDUAL_LEVEL mb_residual_level, int32_t MbPartPredMode, int32_t BlkIdx, int32_t &TotalCoeff)
{
    int ret = 0;
    int32_t i = 0;
    int32_t suffixLength = 0;
    int32_t levelSuffixSize = 0;
    int32_t levelCode = 0;
    int32_t zerosLeft = 0;
    int32_t coeffNum = 0;
    int32_t nC = 0;
    TotalCoeff = 0;
    
    //for (i = 0; i < maxNumCoeff; i++) { coeffLevel[ i ] = 0; }
    memset(coeffLevel, 0, sizeof(int32_t) * maxNumCoeff);

    // coeff_token; //3 | 4 ce(v)
    ret = get_nC(picture, mb_residual_level, MbPartPredMode, BlkIdx, nC);
    RETURN_IF_FAILED(ret != 0, -1);

    uint16_t coeff_token = bs.getBits(16); //先获取16bit数据（注意：并不是读取），因为coeff_token_table表里面最长的coeff_token为16bit，所以预先获取16bit数据就足够了

    int32_t coeff_token_bit_length = 0;
    int32_t TrailingOnes = 0;

    ret = coeff_token_table(nC, coeff_token, coeff_token_bit_length, TrailingOnes, TotalCoeff); //直接查表找到对应的TrailingOnes, TotalCoeff值
    RETURN_IF_FAILED(ret != 0, -1);

    uint16_t coeff_token2 = bs.readBits(coeff_token_bit_length); //此处才是读取coeff_token_bit_length bit数据
    
    //-------------------------------------------
    if (TotalCoeff > 0)
    {
        if (TotalCoeff > 10 && TrailingOnes < 3)
        {
            suffixLength = 1;
        }
        else // if (TotalCoeff <= 10 || TrailingOnes == 3)
        {
            suffixLength = 0;
        }

        for (i = 0; i < TotalCoeff; i++)
        {
            if (i < TrailingOnes)
            {
                trailing_ones_sign_flag = bs.readBits(1); //3 | 4 u(1)
                levelVal[ i ] = 1 - 2 * trailing_ones_sign_flag; //3个拖尾系数，只能是1或-1
            }
            else
            {
                // level_prefix; //3 | 4 ce(v)
                int32_t leadingZeroBits = -1;
                for (int32_t b = 0; !b; leadingZeroBits++ )
                {
                    b = bs.readBits( 1 );
                }
                level_prefix = leadingZeroBits;

                //---------------levelSuffixSize-----------------------------------
                if (level_prefix == 14 && suffixLength == 0)
                {
                    levelSuffixSize = 4;
                }else if (level_prefix >= 15)
                {
                    levelSuffixSize = level_prefix - 3;
                }else
                {
                    levelSuffixSize = suffixLength;
                }
                
                //-----------------level_suffix---------------------------------
                levelCode = (MIN(15, level_prefix) << suffixLength);
                if (suffixLength > 0 || level_prefix >= 14)
                {
                    // level_suffix ; //3 | 4 u(v)
                    if (levelSuffixSize > 0)
                    {
                        level_suffix = bs.readBits(levelSuffixSize);
                    }else //if (levelSuffixSize == 0)
                    {
                        level_suffix = 0;
                    }

                    levelCode += level_suffix;
                }

                //--------------------------------------------------
                if (level_prefix >= 15 && suffixLength == 0)
                {
                    levelCode += 15;
                }

                if (level_prefix >= 16)
                {
                    levelCode += (1 << (level_prefix - 3)) - 4096;
                }

                if (i == TrailingOnes && TrailingOnes < 3)
                {
                    levelCode += 2;
                }

                if (levelCode % 2 == 0)
                {
                    levelVal[ i ] = (levelCode + 2) >> 1;
                }
                else
                {
                    levelVal[ i ] = (-levelCode - 1) >> 1;
                }

                if (suffixLength == 0)
                {
                    suffixLength = 1;
                }

                if (ABS(levelVal[ i ]) > (3 << (suffixLength - 1)) && suffixLength < 6)
                {
                    suffixLength++;
                }
            }
        }
        
        //---------------total_zeros--------------------------
        if (TotalCoeff < endIdx - startIdx + 1)
        {
            // total_zeros; //3 | 4 ce(v)
            int32_t tzVlcIndex = TotalCoeff;
            ret = get_total_zeros(bs, maxNumCoeff, tzVlcIndex, total_zeros);
            RETURN_IF_FAILED(ret != 0, ret);

            zerosLeft = total_zeros;
        }else
        {
            zerosLeft = 0;
        }
        
        //--------------run_before---------------------------
        for (i = 0; i < TotalCoeff - 1; i++)
        {
            if (zerosLeft > 0)
            {
                // run_before; //3 | 4 ce(v)
                ret = get_run_before(bs, zerosLeft, run_before);
                RETURN_IF_FAILED(ret != 0, ret);

                runVal[i] = run_before;
            }else
            {
                runVal[i] = 0;
            }
            zerosLeft = zerosLeft - runVal[i];
        }

        runVal[TotalCoeff - 1] = zerosLeft;

        //---------------------
        coeffNum = -1;
        for (i = TotalCoeff - 1; i >= 0; i--)
        {
            coeffNum += runVal[i] + 1;
            coeffLevel[startIdx + coeffNum] = levelVal[i];
        }
    }

    if (picture.m_PicNumCnt == -1)
    {
        printf("%s(%d): mb_x=%d; mb_y=%d; TotalCoeff=%d;\n", __FUNCTION__, __LINE__, picture.mb_x, picture.mb_y, TotalCoeff);
        for (i = 0; i < TotalCoeff; i++)
        {
            printf(" %d", levelVal[i]);
        }
        if (TotalCoeff > 0)
        {
            printf("\n");
        }
    }

    return 0;
}


/*
 *    Figure 6-10 – Scan for 4x4 luma blocks
 *        0    1    4    5
 *        2    3    6    7
 *        8    9    12   13
 *        10   11   14   15
*/
int32_t   Scan_for_4x4_luma_blocks[16] =
{
    0,  1,  4,  5,
    2,  3,  6,  7,
    8,  9,  12, 13,
    10, 11, 14, 15,
};


/*
 *      |               |          
 *    D |       B       |    C     
 *  ----+---------------+----------
 *    A | Current       |
 *      | Macroblock    |
 *      | or Partition  |
 *      | or Block      |
*/
// 9.2.1 Parsing process for total number of non-zero transform coefficient levels and number of trailing ones
int CH264ResidualBlockCavlc::get_nC(CH264PictureBase &picture, MB_RESIDUAL_LEVEL mb_residual_level, int32_t MbPartPredMode, int32_t BlkIdx, int32_t &nC)
{
    int ret = 0;
    int32_t BlkIdxA = BlkIdx;
    int32_t BlkIdxB = BlkIdx;
    int32_t luma4x4BlkIdx = BlkIdx;
    int32_t chroma4x4BlkIdx = BlkIdx;
    int32_t cb4x4BlkIdx = BlkIdx;
    int32_t cr4x4BlkIdx = BlkIdx;
    int32_t mb_type_neighbouring_A = 0;
    int32_t mb_type_neighbouring_B = 0;
    
    MB_ADDR_TYPE mbAddrN_A_type = MB_ADDR_TYPE_UNKOWN;
    MB_ADDR_TYPE mbAddrN_B_type = MB_ADDR_TYPE_UNKOWN;
    int32_t mbAddrN_A = -1;
    int32_t mbAddrN_B = -1;
    int32_t luma4x4BlkIdxN_A = 0;
    int32_t luma4x4BlkIdxN_B = 0;
    int32_t luma8x8BlkIdxN_A = 0;
    int32_t luma8x8BlkIdxN_B = 0;
    int32_t cb4x4BlkIdxN_A = 0;
    int32_t cb4x4BlkIdxN_B = 0;
    int32_t cr4x4BlkIdxN_A = 0;
    int32_t cr4x4BlkIdxN_B = 0;
    int32_t chroma4x4BlkIdxN_A = 0;
    int32_t chroma4x4BlkIdxN_B = 0;

    int32_t CurrMbAddr = picture.CurrMbAddr;
    
    //Table 6-2 – Specification of input and output assignments for clauses 6.4.11.1 to 6.4.11.7
    int32_t xD_A = -1;
    int32_t yD_A = 0;
    int32_t xD_B = 0;
    int32_t yD_B = -1;
    int32_t x = 0;
    int32_t y = 0;
    int32_t maxW = 0;
    int32_t maxH = 0;
    int32_t xW = 0;
    int32_t yW = 0;
    int32_t isChroma = 0;

    CH264SliceHeader & slice_header = picture.m_h264_slice_header;

    if (mb_residual_level == MB_RESIDUAL_ChromaDCLevelCb || mb_residual_level == MB_RESIDUAL_ChromaDCLevelCr)
    {
        if (slice_header.m_sps.ChromaArrayType == 1)
        {
            nC = -1;
        }
        else //if (slice_header.m_sps.ChromaArrayType == 2)
        {
            nC = -2;
        }
    }
    else
    {
        if (mb_residual_level == MB_RESIDUAL_Intra16x16DCLevel)
        {
            luma4x4BlkIdx = 0;
        }
        if (mb_residual_level == MB_RESIDUAL_CbIntra16x16DCLevel)
        {
            cb4x4BlkIdx = 0;
        }
        if (mb_residual_level == MB_RESIDUAL_CrIntra16x16DCLevel)
        {
            cr4x4BlkIdx = 0;
        }
        
        if (mb_residual_level == MB_RESIDUAL_Intra16x16DCLevel || mb_residual_level == MB_RESIDUAL_Intra16x16ACLevel || mb_residual_level == MB_RESIDUAL_LumaLevel4x4)
        {
            //6.4.11.4 Derivation process for neighbouring 4x4 luma blocks
            
            //6.4.3 Inverse 4x4 luma block scanning process
            //InverseRasterScan = (a % (d / b) ) * b;    if e == 0;
            //InverseRasterScan = (a / (d / b) ) * c;    if e == 1;
            x = InverseRasterScan(luma4x4BlkIdx / 4, 8, 8, 16, 0) + InverseRasterScan(luma4x4BlkIdx % 4, 4, 4, 8, 0);
            y = InverseRasterScan(luma4x4BlkIdx / 4, 8, 8, 16, 1) + InverseRasterScan(luma4x4BlkIdx % 4, 4, 4, 8, 1);

            //6.4.12 Derivation process for neighbouring locations
            maxW = 16;
            maxH = 16;
            isChroma = 0;

            if (slice_header.MbaffFrameFlag == 0) //6.4.12.1 Specification for neighbouring locations in fields and non-MBAFF frames
            {
                ret = picture.getMbAddrN_non_MBAFF_frames(x - 1, y + 0, maxW, maxH, CurrMbAddr, mbAddrN_A_type, mbAddrN_A, luma4x4BlkIdxN_A, luma8x8BlkIdxN_A, xW, yW, isChroma);
                RETURN_IF_FAILED(ret != 0, ret);

                ret = picture.getMbAddrN_non_MBAFF_frames(x + 0, y - 1, maxW, maxH, CurrMbAddr, mbAddrN_B_type, mbAddrN_B, luma4x4BlkIdxN_B, luma8x8BlkIdxN_B, xW, yW, isChroma);
                RETURN_IF_FAILED(ret != 0, ret);
            }
            else //if (slice_header.MbaffFrameFlag == 1) //6.4.12.2 Specification for neighbouring locations in MBAFF frames
            {
                ret = picture.getMbAddrN_MBAFF_frames(x - 1, y + 0, maxW, maxH, CurrMbAddr, mbAddrN_A_type, mbAddrN_A, luma4x4BlkIdxN_A, luma8x8BlkIdxN_A, xW, yW, isChroma);
                RETURN_IF_FAILED(ret != 0, ret);
                
                mb_type_neighbouring_A = I_NxN;

                ret = picture.getMbAddrN_MBAFF_frames(x + 0, y - 1, maxW, maxH, CurrMbAddr, mbAddrN_B_type, mbAddrN_B, luma4x4BlkIdxN_B, luma8x8BlkIdxN_B, xW, yW, isChroma);
                RETURN_IF_FAILED(ret != 0, ret);
            }

            BlkIdxA = luma4x4BlkIdxN_A;
            BlkIdxB = luma4x4BlkIdxN_B;
        }
        else if (mb_residual_level == MB_RESIDUAL_CbIntra16x16DCLevel || mb_residual_level == MB_RESIDUAL_CbIntra16x16DCLevel || mb_residual_level == MB_RESIDUAL_CbLevel4x4)
        {
            //6.4.11.6 Derivation process for neighbouring 4x4 chroma blocks for ChromaArrayType equal to 3
            RETURN_IF_FAILED(slice_header.m_sps.ChromaArrayType != 3, -1);

            //6.4.11.4 Derivation process for neighbouring 4x4 luma blocks
            x = InverseRasterScan(cb4x4BlkIdx / 4, 8, 8, 16, 0) + InverseRasterScan(cb4x4BlkIdx % 4, 4, 4, 8, 0);
            y = InverseRasterScan(cb4x4BlkIdx / 4, 8, 8, 16, 1) + InverseRasterScan(cb4x4BlkIdx % 4, 4, 4, 8, 1);

            //6.4.12 Derivation process for neighbouring locations
            maxW = slice_header.m_sps.MbWidthC;
            maxH = slice_header.m_sps.MbHeightC;
            isChroma = 0;

            if (slice_header.MbaffFrameFlag == 0)
            {
                ret = picture.getMbAddrN_non_MBAFF_frames(x - 1, y + 0, maxW, maxH, CurrMbAddr, mbAddrN_A_type, mbAddrN_A, cb4x4BlkIdxN_A, luma8x8BlkIdxN_A, xW, yW, isChroma);
                RETURN_IF_FAILED(ret != 0, ret);

                ret = picture.getMbAddrN_non_MBAFF_frames(x + 0, y - 1, maxW, maxH, CurrMbAddr, mbAddrN_B_type, mbAddrN_B, cb4x4BlkIdxN_B, luma8x8BlkIdxN_B, xW, yW, isChroma);
                RETURN_IF_FAILED(ret != 0, ret);
            }
            else //if (slice_header.MbaffFrameFlag == 1) //6.4.12.2 Specification for neighbouring locations in MBAFF frames
            {
                ret = picture.getMbAddrN_MBAFF_frames(x - 1, y + 0, maxW, maxH, CurrMbAddr, mbAddrN_A_type, mbAddrN_A, cb4x4BlkIdxN_A, luma8x8BlkIdxN_A, xW, yW, isChroma);
                RETURN_IF_FAILED(ret != 0, ret);
                
                ret = picture.getMbAddrN_MBAFF_frames(x + 0, y - 1, maxW, maxH, CurrMbAddr, mbAddrN_B_type, mbAddrN_B, cb4x4BlkIdxN_B, luma8x8BlkIdxN_B, xW, yW, isChroma);
                RETURN_IF_FAILED(ret != 0, ret);
            }

            BlkIdxA = cb4x4BlkIdxN_A;
            BlkIdxB = cb4x4BlkIdxN_B;
        }
        else if (mb_residual_level == MB_RESIDUAL_CrIntra16x16DCLevel || mb_residual_level == MB_RESIDUAL_CrIntra16x16ACLevel || mb_residual_level == MB_RESIDUAL_CrLevel4x4)
        {
            //6.4.11.6 Derivation process for neighbouring 4x4 chroma blocks for ChromaArrayType equal to 3
            RETURN_IF_FAILED(slice_header.m_sps.ChromaArrayType != 3, -1);

            //6.4.11.4 Derivation process for neighbouring 4x4 luma blocks
            x = InverseRasterScan(cr4x4BlkIdx / 4, 8, 8, 16, 0) + InverseRasterScan(cr4x4BlkIdx % 4, 4, 4, 8, 0);
            y = InverseRasterScan(cr4x4BlkIdx / 4, 8, 8, 16, 1) + InverseRasterScan(cr4x4BlkIdx % 4, 4, 4, 8, 1);

            //6.4.12 Derivation process for neighbouring locations
            maxW = slice_header.m_sps.MbWidthC;
            maxH = slice_header.m_sps.MbHeightC;
            isChroma = 0;

            if (slice_header.MbaffFrameFlag == 0)
            {
                ret = picture.getMbAddrN_non_MBAFF_frames(x - 1, y + 0, maxW, maxH, CurrMbAddr, mbAddrN_A_type, mbAddrN_A, cr4x4BlkIdxN_A, luma8x8BlkIdxN_A, xW, yW, isChroma);
                RETURN_IF_FAILED(ret != 0, ret);

                ret = picture.getMbAddrN_non_MBAFF_frames(x + 0, y - 1, maxW, maxH, CurrMbAddr, mbAddrN_B_type, mbAddrN_B, cr4x4BlkIdxN_B, luma8x8BlkIdxN_B, xW, yW, isChroma);
                RETURN_IF_FAILED(ret != 0, ret);
            }
            else //if (slice_header.MbaffFrameFlag == 1) //6.4.12.2 Specification for neighbouring locations in MBAFF frames
            {
                ret = picture.getMbAddrN_MBAFF_frames(x - 1, y + 0, maxW, maxH, CurrMbAddr, mbAddrN_A_type, mbAddrN_A, cr4x4BlkIdxN_A, luma8x8BlkIdxN_A, xW, yW, isChroma);
                RETURN_IF_FAILED(ret != 0, ret);
                
                ret = picture.getMbAddrN_MBAFF_frames(x + 0, y - 1, maxW, maxH, CurrMbAddr, mbAddrN_B_type, mbAddrN_B, cr4x4BlkIdxN_B, luma8x8BlkIdxN_B, xW, yW, isChroma);
                RETURN_IF_FAILED(ret != 0, ret);
            }

            BlkIdxA = cr4x4BlkIdxN_A;
            BlkIdxB = cr4x4BlkIdxN_B;
        }
        else if (mb_residual_level == MB_RESIDUAL_ChromaACLevelCb || mb_residual_level == MB_RESIDUAL_ChromaACLevelCr)
        {
            //6.4.11.5 Derivation process for neighbouring 4x4 chroma blocks
            RETURN_IF_FAILED(slice_header.m_sps.ChromaArrayType != 1 && slice_header.m_sps.ChromaArrayType != 2, -1);

            //6.4.7 Inverse 4x4 chroma block scanning process
            x = InverseRasterScan( chroma4x4BlkIdx, 4, 4, 8, 0 );
            y = InverseRasterScan( chroma4x4BlkIdx, 4, 4, 8, 1 );

            maxW = slice_header.m_sps.MbWidthC;
            maxH = slice_header.m_sps.MbHeightC;
            isChroma = 1;

            if (slice_header.MbaffFrameFlag == 0)
            {
                ret = picture.getMbAddrN_non_MBAFF_frames(x - 1, y + 0, maxW, maxH, CurrMbAddr, mbAddrN_A_type, mbAddrN_A, chroma4x4BlkIdxN_A, luma8x8BlkIdxN_A, xW, yW, isChroma);
                RETURN_IF_FAILED(ret != 0, ret);

                ret = picture.getMbAddrN_non_MBAFF_frames(x + 0, y - 1, maxW, maxH, CurrMbAddr, mbAddrN_B_type, mbAddrN_B, chroma4x4BlkIdxN_B, luma8x8BlkIdxN_B, xW, yW, isChroma);
                RETURN_IF_FAILED(ret != 0, ret);
            }
            else //if (slice_header.MbaffFrameFlag == 1) //6.4.12.2 Specification for neighbouring locations in MBAFF frames
            {
                ret = picture.getMbAddrN_MBAFF_frames(x - 1, y + 0, maxW, maxH, CurrMbAddr, mbAddrN_A_type, mbAddrN_A, chroma4x4BlkIdxN_A, luma8x8BlkIdxN_A, xW, yW, isChroma);
                RETURN_IF_FAILED(ret != 0, ret);
                
                ret = picture.getMbAddrN_MBAFF_frames(x + 0, y - 1, maxW, maxH, CurrMbAddr, mbAddrN_B_type, mbAddrN_B, chroma4x4BlkIdxN_B, luma8x8BlkIdxN_B, xW, yW, isChroma);
                RETURN_IF_FAILED(ret != 0, ret);
            }

            BlkIdxA = chroma4x4BlkIdxN_A;
            BlkIdxB = chroma4x4BlkIdxN_B;
        }

        //----------------------------------------------------
        int32_t availableFlagN_A = 0;
        int32_t availableFlagN_B = 0;

        if (mbAddrN_A < 0 //mbAddrN is not available
            || (IS_INTRA_Prediction_Mode(MbPartPredMode) //the current macroblock is coded using an Intra macroblock prediction mode
                && slice_header.m_pps.constrained_intra_pred_flag == 1 //constrained_intra_pred_flag is equal to 1
                && !IS_INTRA_Prediction_Mode(picture.m_mbs[mbAddrN_A].m_mb_pred_mode) //mbAddrN is coded using an Inter macroblock prediction mode
                && slice_header.m_nal_unit.nal_unit_type >= 2 && slice_header.m_nal_unit.nal_unit_type <= 4) //slice data partitioning is in use (nal_unit_type is in the range of 2 to 4, inclusive)
            )
        {
            availableFlagN_A = 0;
        }
        else
        {
            availableFlagN_A = 1;
        }
    
        if (mbAddrN_B < 0 
            || (IS_INTRA_Prediction_Mode(MbPartPredMode)
                && slice_header.m_pps.constrained_intra_pred_flag == 1
                && !IS_INTRA_Prediction_Mode(picture.m_mbs[mbAddrN_B].m_mb_pred_mode) //mbAddrN is coded using an Inter macroblock prediction mode
                && slice_header.m_nal_unit.nal_unit_type >= 2 && slice_header.m_nal_unit.nal_unit_type <= 4)
            )
        {
            availableFlagN_B = 0;
        }
        else
        {
            availableFlagN_B = 1;
        }

        //-----------------------
        int32_t nA = 0;
        int32_t nB = 0;

        if (availableFlagN_A == 1)
        {
            if (picture.m_mbs[mbAddrN_A].m_mb_type_fixed == P_Skip || picture.m_mbs[mbAddrN_A].m_mb_type_fixed == B_Skip)
            {
                nA = 0;
            }
            else if (picture.m_mbs[mbAddrN_A].m_mb_type_fixed == I_PCM) //if mbAddrN is an I_PCM macroblock, nN is set equal to 16.
            {
                nA = 16;
            }
            else //nN is set equal to the value TotalCoeff( coeff_token ) of the neighbouring block blkN.
            {
                if (mb_residual_level == MB_RESIDUAL_Intra16x16DCLevel || mb_residual_level == MB_RESIDUAL_Intra16x16ACLevel || mb_residual_level == MB_RESIDUAL_LumaLevel4x4)
                {
                    nA = picture.m_mbs[mbAddrN_A].mb_luma_4x4_non_zero_count_coeff[BlkIdxA];
                }
                else if (mb_residual_level == MB_RESIDUAL_CbIntra16x16DCLevel || mb_residual_level == MB_RESIDUAL_CbIntra16x16DCLevel || mb_residual_level == MB_RESIDUAL_CbLevel4x4)
                {
                    nA = picture.m_mbs[mbAddrN_A].mb_chroma_4x4_non_zero_count_coeff[0][BlkIdxA];
                }
                else if (mb_residual_level == MB_RESIDUAL_CrIntra16x16DCLevel || mb_residual_level == MB_RESIDUAL_CrIntra16x16ACLevel || mb_residual_level == MB_RESIDUAL_CrLevel4x4)
                {
                    nA = picture.m_mbs[mbAddrN_A].mb_chroma_4x4_non_zero_count_coeff[1][BlkIdxA];
                }
                else if (mb_residual_level == MB_RESIDUAL_ChromaACLevelCb)
                {
                    nA = picture.m_mbs[mbAddrN_A].mb_chroma_4x4_non_zero_count_coeff[0][BlkIdxA];
                }
                else if (mb_residual_level == MB_RESIDUAL_ChromaACLevelCr)
                {
                    nA = picture.m_mbs[mbAddrN_A].mb_chroma_4x4_non_zero_count_coeff[1][BlkIdxA];
                }
            }
        }

        if (availableFlagN_B == 1)
        {
            if (picture.m_mbs[mbAddrN_B].m_mb_type_fixed == P_Skip || picture.m_mbs[mbAddrN_B].m_mb_type_fixed == B_Skip)
            {
                nB = 0;
            }
            else if (picture.m_mbs[mbAddrN_B].m_mb_type_fixed == I_PCM) //if mbAddrN is an I_PCM macroblock, nN is set equal to 16.
            {
                nB = 16;
            }
            else //nN is set equal to the value TotalCoeff( coeff_token ) of the neighbouring block blkN.
            {
                if (mb_residual_level == MB_RESIDUAL_Intra16x16DCLevel || mb_residual_level == MB_RESIDUAL_Intra16x16ACLevel || mb_residual_level == MB_RESIDUAL_LumaLevel4x4)
                {
                    nB = picture.m_mbs[mbAddrN_B].mb_luma_4x4_non_zero_count_coeff[BlkIdxB];
                }
                else if (mb_residual_level == MB_RESIDUAL_CbIntra16x16DCLevel || mb_residual_level == MB_RESIDUAL_CbIntra16x16DCLevel || mb_residual_level == MB_RESIDUAL_CbLevel4x4)
                {
                    nB = picture.m_mbs[mbAddrN_B].mb_chroma_4x4_non_zero_count_coeff[0][BlkIdxB];
                }
                else if (mb_residual_level == MB_RESIDUAL_CrIntra16x16DCLevel || mb_residual_level == MB_RESIDUAL_CrIntra16x16ACLevel || mb_residual_level == MB_RESIDUAL_CrLevel4x4)
                {
                    nB = picture.m_mbs[mbAddrN_B].mb_chroma_4x4_non_zero_count_coeff[1][BlkIdxB];
                }
                else if (mb_residual_level == MB_RESIDUAL_ChromaACLevelCb)
                {
                    nB = picture.m_mbs[mbAddrN_B].mb_chroma_4x4_non_zero_count_coeff[0][BlkIdxB];
                }
                else if (mb_residual_level == MB_RESIDUAL_ChromaACLevelCr)
                {
                    nB = picture.m_mbs[mbAddrN_B].mb_chroma_4x4_non_zero_count_coeff[1][BlkIdxB];
                }
            }
        }

        //-----------------------
        if (availableFlagN_A == 1 && availableFlagN_B == 1)
        {
            nC = ( nA + nB + 1 ) >> 1;
        }else if (availableFlagN_A == 1 && availableFlagN_B == 0)
        {
            nC = nA;
        }else if (availableFlagN_A == 0 && availableFlagN_B == 1)
        {
            nC = nB;
        }else // if (availableFlagN_A == 0 && availableFlagN_B == 0)
        {
            nC = 0;
        }
    }

    return 0;
}


// Table 9-5 – coeff_token mapping to TotalCoeff( coeff_token ) and TrailingOnes( coeff_token )
int CH264ResidualBlockCavlc::coeff_token_table(int32_t nC, uint16_t coeff_token, int32_t &coeff_token_bit_length, int32_t &TrailingOnes, int32_t &TotalCoeff)
{
    if (nC >= 0 && nC < 2)
    {
        if ((coeff_token >> 15) == 0x0001) //(1)b
        {
            coeff_token_bit_length = 1;
            TrailingOnes = 0;
            TotalCoeff = 0;
        }else if ((coeff_token >> 10) == 0x0005) //(0001 01)b ==> (00 0101)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 1;
        }else if ((coeff_token >> 14) == 0x0001) //(01)b
        {
            coeff_token_bit_length = 2;
            TrailingOnes = 1;
            TotalCoeff = 1;
        }else if ((coeff_token >> 8) == 0x0007) //(0000 0111)b
        {
            coeff_token_bit_length = 8;
            TrailingOnes = 0;
            TotalCoeff = 2;
        }else if ((coeff_token >> 10) == 0x0004) //(0001 00)b ==> (00 0100)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 1;
            TotalCoeff = 2;
        }else if ((coeff_token >> 13) == 0x0001) //(001)b
        {
            coeff_token_bit_length = 3;
            TrailingOnes = 2;
            TotalCoeff = 2;
        }else if ((coeff_token >> 7) == 0x0007) //(0000 0011 1)b ==> (0 0000 0111)b
        {
            coeff_token_bit_length = 9;
            TrailingOnes = 0;
            TotalCoeff = 3;
        }else if ((coeff_token >> 8) == 0x0006) //(0000 0110)b
        {
            coeff_token_bit_length = 8;
            TrailingOnes = 1;
            TotalCoeff = 3;
        }else if ((coeff_token >> 9) == 0x0005) //(0000 101)b ==> (000 0101)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 2;
            TotalCoeff = 3;
        }else if ((coeff_token >> 11) == 0x0003) //(0001 1)b ==> (0 0011)b
        {
            coeff_token_bit_length = 5;
            TrailingOnes = 3;
            TotalCoeff = 3;
        }else if ((coeff_token >> 6) == 0x0007) //(0000 0001 11)b ==> (00 0000 0111)b
        {
            coeff_token_bit_length = 10;
            TrailingOnes = 0;
            TotalCoeff = 4;
        }else if ((coeff_token >> 7) == 0x0006) //(0000 0011 0)b ==> (0 0000 0110)b
        {
            coeff_token_bit_length = 9;
            TrailingOnes = 1;
            TotalCoeff = 4;
        }else if ((coeff_token >> 8) == 0x0005) //(0000 0101)b ==> (0000 0101)b
        {
            coeff_token_bit_length = 8;
            TrailingOnes = 2;
            TotalCoeff = 4;
        }else if ((coeff_token >> 10) == 0x0003) //(0000 11)b ==> (00 0011)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 3;
            TotalCoeff = 4;
        }else if ((coeff_token >> 5) == 0x0007) //(0000 0000 111)b ==> (000 0000 0111)b
        {
            coeff_token_bit_length = 11;
            TrailingOnes = 0;
            TotalCoeff = 5;
        }else if ((coeff_token >> 6) == 0x0006) //(0000 0001 10)b ==> (00 0000 0110)b
        {
            coeff_token_bit_length = 10;
            TrailingOnes = 1;
            TotalCoeff = 5;
        }else if ((coeff_token >> 7) == 0x0005) //(0000 0010 1)b ==> (0 0000 0101)b
        {
            coeff_token_bit_length = 9;
            TrailingOnes = 2;
            TotalCoeff = 5;
        }else if ((coeff_token >> 9) == 0x0004) //(0000 100)b ==> (000 0100)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 3;
            TotalCoeff = 5;
        }else if ((coeff_token >> 3) == 0x000F) //(0000 0000 0111 1)b ==> (0 0000 0000 1111)b
        {
            coeff_token_bit_length = 13;
            TrailingOnes = 0;
            TotalCoeff = 6;
        }else if ((coeff_token >> 5) == 0x0006) //(0000 0000 110)b ==> (000 0000 0110)b
        {
            coeff_token_bit_length = 11;
            TrailingOnes = 1;
            TotalCoeff = 6;
        }else if ((coeff_token >> 6) == 0x0005) //(0000 0001 01)b ==> (00 0000 0101)b
        {
            coeff_token_bit_length = 10;
            TrailingOnes = 2;
            TotalCoeff = 6;
        }else if ((coeff_token >> 8) == 0x0004) //(0000 0100)b ==> (0000 0100)b
        {
            coeff_token_bit_length = 8;
            TrailingOnes = 3;
            TotalCoeff = 6;
        }else if ((coeff_token >> 3) == 0x000B) //(0000 0000 0101 1)b ==> (0 0000 0000 1011)b
        {
            coeff_token_bit_length = 13;
            TrailingOnes = 0;
            TotalCoeff = 7;
        }else if ((coeff_token >> 3) == 0x000E) //(0000 0000 0111 0)b ==> (0 0000 0000 1110)b
        {
            coeff_token_bit_length = 13;
            TrailingOnes = 1;
            TotalCoeff = 7;
        }else if ((coeff_token >> 5) == 0x0005) //(0000 0000 101)b ==> (000 0000 0101)b
        {
            coeff_token_bit_length = 11;
            TrailingOnes = 2;
            TotalCoeff = 7;
        }else if ((coeff_token >> 7) == 0x0004) //(0000 0010 0)b ==> (0 0000 0100)b
        {
            coeff_token_bit_length = 9;
            TrailingOnes = 3;
            TotalCoeff = 7;
        }else if ((coeff_token >> 3) == 0x0008) //(0000 0000 0100 0)b ==> (0 0000 0000 1000)b
        {
            coeff_token_bit_length = 13;
            TrailingOnes = 0;
            TotalCoeff = 8;
        }else if ((coeff_token >> 3) == 0x000A) //(0000 0000 0101 0)b ==> (0 0000 0000 1010)b
        {
            coeff_token_bit_length = 13;
            TrailingOnes = 1;
            TotalCoeff = 8;
        }else if ((coeff_token >> 3) == 0x000D) //(0000 0000 0110 1)b ==> (0 0000 0000 1101)b
        {
            coeff_token_bit_length = 13;
            TrailingOnes = 2;
            TotalCoeff = 8;
        }else if ((coeff_token >> 6) == 0x0004) //(0000 0001 00)b ==> (00 0000 0100)b
        {
            coeff_token_bit_length = 10;
            TrailingOnes = 3;
            TotalCoeff = 8;
        }else if ((coeff_token >> 2) == 0x000F) //(0000 0000 0011 11)b ==> (00 0000 0000 1111)b
        {
            coeff_token_bit_length = 14;
            TrailingOnes = 0;
            TotalCoeff = 9;
        }else if ((coeff_token >> 2) == 0x000E) //(0000 0000 0011 10)b ==> (00 0000 0000 1110)b
        {
            coeff_token_bit_length = 14;
            TrailingOnes = 1;
            TotalCoeff = 9;
        }else if ((coeff_token >> 3) == 0x0009) //(0000 0000 0100 1)b ==> (0 0000 0000 1001)b
        {
            coeff_token_bit_length = 13;
            TrailingOnes = 2;
            TotalCoeff = 9;
        }else if ((coeff_token >> 5) == 0x0004) //(0000 0000 100)b ==> (000 0000 0100)b
        {
            coeff_token_bit_length = 11;
            TrailingOnes = 3;
            TotalCoeff = 9;
        }else if ((coeff_token >> 2) == 0x000B) //(0000 0000 0010 11)b ==> (00 0000 0000 1011)b
        {
            coeff_token_bit_length = 14;
            TrailingOnes = 0;
            TotalCoeff = 10;
        }else if ((coeff_token >> 2) == 0x000A) //(0000 0000 0010 10)b ==> (00 0000 0000 1010)b
        {
            coeff_token_bit_length = 14;
            TrailingOnes = 1;
            TotalCoeff = 10;
        }else if ((coeff_token >> 2) == 0x000D) //(0000 0000 0011 01)b ==> (00 0000 0000 1101)b
        {
            coeff_token_bit_length = 14;
            TrailingOnes = 2;
            TotalCoeff = 10;
        }else if ((coeff_token >> 3) == 0x000C) //(0000 0000 0110 0)b ==> (0 0000 0000 1100)b
        {
            coeff_token_bit_length = 13;
            TrailingOnes = 3;
            TotalCoeff = 10;
        }else if ((coeff_token >> 1) == 0x000F) //(0000 0000 0001 111)b ==> (000 0000 0000 1111)b
        {
            coeff_token_bit_length = 15;
            TrailingOnes = 0;
            TotalCoeff = 11;
        }else if ((coeff_token >> 1) == 0x000E) //(0000 0000 0001 110)b ==> (000 0000 0000 1110)b
        {
            coeff_token_bit_length = 15;
            TrailingOnes = 1;
            TotalCoeff = 11;
        }else if ((coeff_token >> 2) == 0x0009) //(0000 0000 0010 01)b ==> (00 0000 0000 1001)b
        {
            coeff_token_bit_length = 14;
            TrailingOnes = 2;
            TotalCoeff = 11;
        }else if ((coeff_token >> 2) == 0x000C) //(0000 0000 0011 00)b ==> (00 0000 0000 1100)b
        {
            coeff_token_bit_length = 14;
            TrailingOnes = 3;
            TotalCoeff = 11;
        }else if ((coeff_token >> 1) == 0x000B) //(0000 0000 0001 011)b ==> (000 0000 0000 1011)b
        {
            coeff_token_bit_length = 15;
            TrailingOnes = 0;
            TotalCoeff = 12;
        }else if ((coeff_token >> 1) == 0x000A) //(0000 0000 0001 010)b ==> (000 0000 0000 1010)b
        {
            coeff_token_bit_length = 15;
            TrailingOnes = 1;
            TotalCoeff = 12;
        }else if ((coeff_token >> 1) == 0x000D) //(0000 0000 0001 101)b ==> (000 0000 0000 1101)b
        {
            coeff_token_bit_length = 15;
            TrailingOnes = 2;
            TotalCoeff = 12;
        }else if ((coeff_token >> 2) == 0x0008) //(0000 0000 0010 00)b ==> (00 0000 0000 1000)b
        {
            coeff_token_bit_length = 14;
            TrailingOnes = 3;
            TotalCoeff = 12;
        }else if ((coeff_token >> 0) == 0x000F) //(0000 0000 0000 1111)b ==> (0000 0000 0000 1111)b
        {
            coeff_token_bit_length = 16;
            TrailingOnes = 0;
            TotalCoeff = 13;
        }else if ((coeff_token >> 1) == 0x0001) //(0000 0000 0000 001)b ==> (000 0000 0000 0001)b
        {
            coeff_token_bit_length = 15;
            TrailingOnes = 1;
            TotalCoeff = 13;
        }else if ((coeff_token >> 1) == 0x0009) //(0000 0000 0001 001)b ==> (000 0000 0000 1001)b
        {
            coeff_token_bit_length = 15;
            TrailingOnes = 2;
            TotalCoeff = 13;
        }else if ((coeff_token >> 1) == 0x000C) //(0000 0000 0001 100)b ==> (000 0000 0000 1100)b
        {
            coeff_token_bit_length = 15;
            TrailingOnes = 3;
            TotalCoeff = 13;
        }else if ((coeff_token >> 0) == 0x000B) //(0000 0000 0000 1011)b ==> (0000 0000 0000 1011)b
        {
            coeff_token_bit_length = 16;
            TrailingOnes = 0;
            TotalCoeff = 14;
        }else if ((coeff_token >> 0) == 0x000E) //(0000 0000 0000 1110)b ==> (0000 0000 0000 1110)b
        {
            coeff_token_bit_length = 16;
            TrailingOnes = 1;
            TotalCoeff = 14;
        }else if ((coeff_token >> 0) == 0x000D) //(0000 0000 0000 1101)b ==> (0000 0000 0000 1101)b
        {
            coeff_token_bit_length = 16;
            TrailingOnes = 2;
            TotalCoeff = 14;
        }else if ((coeff_token >> 1) == 0x0008) //(0000 0000 0001 000)b ==> (000 0000 0000 1000)b
        {
            coeff_token_bit_length = 15;
            TrailingOnes = 3;
            TotalCoeff = 14;
        }else if ((coeff_token >> 0) == 0x0007) //(0000 0000 0000 0111)b ==> (0000 0000 0000 0111)b
        {
            coeff_token_bit_length = 16;
            TrailingOnes = 0;
            TotalCoeff = 15;
        }else if ((coeff_token >> 0) == 0x000A) //(0000 0000 0000 1010)b ==> (0000 0000 0000 1010)b
        {
            coeff_token_bit_length = 16;
            TrailingOnes = 1;
            TotalCoeff = 15;
        }else if ((coeff_token >> 0) == 0x0009) //(0000 0000 0000 1001)b ==> (0000 0000 0000 1001)b
        {
            coeff_token_bit_length = 16;
            TrailingOnes = 2;
            TotalCoeff = 15;
        }else if ((coeff_token >> 0) == 0x000C) //(0000 0000 0000 1100)b ==> (0000 0000 0000 1100)b
        {
            coeff_token_bit_length = 16;
            TrailingOnes = 3;
            TotalCoeff = 15;
        }else if ((coeff_token >> 0) == 0x0004) //(0000 0000 0000 0100)b ==> (0000 0000 0000 0100)b
        {
            coeff_token_bit_length = 16;
            TrailingOnes = 0;
            TotalCoeff = 16;
        }else if ((coeff_token >> 0) == 0x0006) //(0000 0000 0000 0110)b ==> (0000 0000 0000 0110)b
        {
            coeff_token_bit_length = 16;
            TrailingOnes = 1;
            TotalCoeff = 16;
        }else if ((coeff_token >> 0) == 0x0005) //(0000 0000 0000 0101)b ==> (0000 0000 0000 0101)b
        {
            coeff_token_bit_length = 16;
            TrailingOnes = 2;
            TotalCoeff = 16;
        }else if ((coeff_token >> 0) == 0x0008) //(0000 0000 0000 1000)b ==> (0000 0000 0000 1000)b
        {
            coeff_token_bit_length = 16;
            TrailingOnes = 3;
            TotalCoeff = 16;
        }else
        {
            RETURN_IF_FAILED(1, -1);
        }
    }else if (nC >= 2 && nC < 4)
    {
        if ((coeff_token >> 14) == 0x0003) // (11)b
        {
            coeff_token_bit_length = 2;
            TrailingOnes = 0;
            TotalCoeff = 0;
        }else if ((coeff_token >> 10) == 0x000B) // (0010 11)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 1;
        }else if ((coeff_token >> 14) == 0x0002) // (10)b
        {
            coeff_token_bit_length = 2;
            TrailingOnes = 1;
            TotalCoeff = 1;
        }else if ((coeff_token >> 10) == 0x0007) // (0001 11)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 2;
        }else if ((coeff_token >> 11) == 0x0007) // (0011 1)b
        {
            coeff_token_bit_length = 5;
            TrailingOnes = 1;
            TotalCoeff = 2;
        }else if ((coeff_token >> 13) == 0x0003) // (011)b
        {
            coeff_token_bit_length = 3;
            TrailingOnes = 2;
            TotalCoeff = 2;
        }else if ((coeff_token >> 9) == 0x0007) // (0000 111)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 0;
            TotalCoeff = 3;
        }else if ((coeff_token >> 10) == 0x000A) // (0010 10)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 1;
            TotalCoeff = 3;
        }else if ((coeff_token >> 10) == 0x0009) // (0010 01)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 2;
            TotalCoeff = 3;
        }else if ((coeff_token >> 12) == 0x0005) // (0101)b
        {
            coeff_token_bit_length = 4;
            TrailingOnes = 3;
            TotalCoeff = 3;
        }else if ((coeff_token >> 8) == 0x0007) // (0000 0111)b
        {
            coeff_token_bit_length = 8;
            TrailingOnes = 0;
            TotalCoeff = 4;
        }else if ((coeff_token >> 10) == 0x0006) // (0001 10)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 1;
            TotalCoeff = 4;
        }else if ((coeff_token >> 10) == 0x0005) // (0001 01)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 2;
            TotalCoeff = 4;
        }else if ((coeff_token >> 12) == 0x0004) // (0100)b
        {
            coeff_token_bit_length = 4;
            TrailingOnes = 3;
            TotalCoeff = 4;
        }else if ((coeff_token >> 8) == 0x0004) // (0000 0100)b
        {
            coeff_token_bit_length = 8;
            TrailingOnes = 0;
            TotalCoeff = 5;
        }else if ((coeff_token >> 9) == 0x0006) // (0000 110)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 1;
            TotalCoeff = 5;
        }else if ((coeff_token >> 9) == 0x0005) // (0000 101)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 2;
            TotalCoeff = 5;
        }else if ((coeff_token >> 11) == 0x0006) // (0011 0)b
        {
            coeff_token_bit_length = 5;
            TrailingOnes = 3;
            TotalCoeff = 5;
        }else if ((coeff_token >> 7) == 0x0007) // (0000 0011 1)b
        {
            coeff_token_bit_length = 9;
            TrailingOnes = 0;
            TotalCoeff = 6;
        }else if ((coeff_token >> 8) == 0x0006) // (0000 0110)b
        {
            coeff_token_bit_length = 8;
            TrailingOnes = 1;
            TotalCoeff = 6;
        }else if ((coeff_token >> 8) == 0x0005) // (0000 0101)b
        {
            coeff_token_bit_length = 8;
            TrailingOnes = 2;
            TotalCoeff = 6;
        }else if ((coeff_token >> 10) == 0x0008) // (0010 00)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 3;
            TotalCoeff = 6;
        }else if ((coeff_token >> 5) == 0x000F) // (0000 0001 111)b
        {
            coeff_token_bit_length = 11;
            TrailingOnes = 0;
            TotalCoeff = 7;
        }else if ((coeff_token >> 7) == 0x0006) // (0000 0011 0)b
        {
            coeff_token_bit_length = 9;
            TrailingOnes = 1;
            TotalCoeff = 7;
        }else if ((coeff_token >> 7) == 0x0005) // (0000 0010 1)b
        {
            coeff_token_bit_length = 9;
            TrailingOnes = 2;
            TotalCoeff = 7;
        }else if ((coeff_token >> 10) == 0x0004) // (0001 00)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 3;
            TotalCoeff = 7;
        }else if ((coeff_token >> 5) == 0x000B) // (0000 0001 011)b
        {
            coeff_token_bit_length = 11;
            TrailingOnes = 0;
            TotalCoeff = 8;
        }else if ((coeff_token >> 5) == 0x000E) // (0000 0001 110)b
        {
            coeff_token_bit_length = 11;
            TrailingOnes = 1;
            TotalCoeff = 8;
        }else if ((coeff_token >> 5) == 0x000D) // (0000 0001 101)b
        {
            coeff_token_bit_length = 11;
            TrailingOnes = 2;
            TotalCoeff = 8;
        }else if ((coeff_token >> 9) == 0x0004) // (0000 100)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 3;
            TotalCoeff = 8;
        }else if ((coeff_token >> 4) == 0x000F) // (0000 0000 1111)b
        {
            coeff_token_bit_length = 12;
            TrailingOnes = 0;
            TotalCoeff = 9;
        }else if ((coeff_token >> 5) == 0x000A) // (0000 0001 010)b
        {
            coeff_token_bit_length = 11;
            TrailingOnes = 1;
            TotalCoeff = 9;
        }else if ((coeff_token >> 5) == 0x0009) // (0000 0001 001)b
        {
            coeff_token_bit_length = 11;
            TrailingOnes = 2;
            TotalCoeff = 9;
        }else if ((coeff_token >> 7) == 0x0004) // (0000 0010 0)b
        {
            coeff_token_bit_length = 9;
            TrailingOnes = 3;
            TotalCoeff = 9;
        }else if ((coeff_token >> 4) == 0x000B) // (0000 0000 1011)b
        {
            coeff_token_bit_length = 12;
            TrailingOnes = 0;
            TotalCoeff = 10;
        }else if ((coeff_token >> 4) == 0x000E) // (0000 0000 1110)b
        {
            coeff_token_bit_length = 12;
            TrailingOnes = 1;
            TotalCoeff = 10;
        }else if ((coeff_token >> 4) == 0x000D) // (0000 0000 1101)b
        {
            coeff_token_bit_length = 12;
            TrailingOnes = 2;
            TotalCoeff = 10;
        }else if ((coeff_token >> 5) == 0x000C) // (0000 0001 100)b
        {
            coeff_token_bit_length = 11;
            TrailingOnes = 3;
            TotalCoeff = 10;
        }else if ((coeff_token >> 4) == 0x0008) // (0000 0000 1000)b
        {
            coeff_token_bit_length = 12;
            TrailingOnes = 0;
            TotalCoeff = 11;
        }else if ((coeff_token >> 4) == 0x000A) // (0000 0000 1010)b
        {
            coeff_token_bit_length = 12;
            TrailingOnes = 1;
            TotalCoeff = 11;
        }else if ((coeff_token >> 4) == 0x0009) // (0000 0000 1001)b
        {
            coeff_token_bit_length = 12;
            TrailingOnes = 2;
            TotalCoeff = 11;
        }else if ((coeff_token >> 5) == 0x0008) // (0000 0001 000)b
        {
            coeff_token_bit_length = 11;
            TrailingOnes = 3;
            TotalCoeff = 11;
        }else if ((coeff_token >> 3) == 0x000F) // (0000 0000 0111 1)b
        {
            coeff_token_bit_length = 13;
            TrailingOnes = 0;
            TotalCoeff = 12;
        }else if ((coeff_token >> 3) == 0x000E) // (0000 0000 0111 0)b
        {
            coeff_token_bit_length = 13;
            TrailingOnes = 1;
            TotalCoeff = 12;
        }else if ((coeff_token >> 3) == 0x000D) // (0000 0000 0110 1)b
        {
            coeff_token_bit_length = 13;
            TrailingOnes = 2;
            TotalCoeff = 12;
        }else if ((coeff_token >> 4) == 0x000C) // (0000 0000 1100)b
        {
            coeff_token_bit_length = 12;
            TrailingOnes = 3;
            TotalCoeff = 12;
        }else if ((coeff_token >> 3) == 0x000B) // (0000 0000 0101 1)b
        {
            coeff_token_bit_length = 13;
            TrailingOnes = 0;
            TotalCoeff = 13;
        }else if ((coeff_token >> 3) == 0x000A) // (0000 0000 0101 0)b
        {
            coeff_token_bit_length = 13;
            TrailingOnes = 1;
            TotalCoeff = 13;
        }else if ((coeff_token >> 3) == 0x0009) // (0000 0000 0100 1)b
        {
            coeff_token_bit_length = 13;
            TrailingOnes = 2;
            TotalCoeff = 13;
        }else if ((coeff_token >> 3) == 0x000C) // (0000 0000 0110 0)b
        {
            coeff_token_bit_length = 13;
            TrailingOnes = 3;
            TotalCoeff = 13;
        }else if ((coeff_token >> 3) == 0x0007) // (0000 0000 0011 1)b
        {
            coeff_token_bit_length = 13;
            TrailingOnes = 0;
            TotalCoeff = 14;
        }else if ((coeff_token >> 2) == 0x000B) // (0000 0000 0010 11)b
        {
            coeff_token_bit_length = 14;
            TrailingOnes = 1;
            TotalCoeff = 14;
        }else if ((coeff_token >> 3) == 0x0006) // (0000 0000 0011 0)b
        {
            coeff_token_bit_length = 13;
            TrailingOnes = 2;
            TotalCoeff = 14;
        }else if ((coeff_token >> 3) == 0x0008) // (0000 0000 0100 0)b
        {
            coeff_token_bit_length = 13;
            TrailingOnes = 3;
            TotalCoeff = 14;
        }else if ((coeff_token >> 2) == 0x0009) // (0000 0000 0010 01)b
        {
            coeff_token_bit_length = 14;
            TrailingOnes = 0;
            TotalCoeff = 15;
        }else if ((coeff_token >> 2) == 0x0008) // (0000 0000 0010 00)b
        {
            coeff_token_bit_length = 14;
            TrailingOnes = 1;
            TotalCoeff = 15;
        }else if ((coeff_token >> 2) == 0x000A) // (0000 0000 0010 10)b
        {
            coeff_token_bit_length = 14;
            TrailingOnes = 2;
            TotalCoeff = 15;
        }else if ((coeff_token >> 3) == 0x0001) // (0000 0000 0000 1)b
        {
            coeff_token_bit_length = 13;
            TrailingOnes = 3;
            TotalCoeff = 15;
        }else if ((coeff_token >> 2) == 0x0007) // (0000 0000 0001 11)b
        {
            coeff_token_bit_length = 14;
            TrailingOnes = 0;
            TotalCoeff = 16;
        }else if ((coeff_token >> 2) == 0x0006) // (0000 0000 0001 10)b
        {
            coeff_token_bit_length = 14;
            TrailingOnes = 1;
            TotalCoeff = 16;
        }else if ((coeff_token >> 2) == 0x0005) // (0000 0000 0001 01)b
        {
            coeff_token_bit_length = 14;
            TrailingOnes = 2;
            TotalCoeff = 16;
        }else if ((coeff_token >> 2) == 0x0004) // (0000 0000 0001 00)b
        {
            coeff_token_bit_length = 14;
            TrailingOnes = 3;
            TotalCoeff = 16;
        }else
        {
            RETURN_IF_FAILED(1, -1);
        }
    }else if (nC >= 4 && nC < 8)
    {
        if ((coeff_token >> 12) == 0x000F) // (1111)b
        {
            coeff_token_bit_length = 4;
            TrailingOnes = 0;
            TotalCoeff = 0;
        }else if ((coeff_token >> 10) == 0x000F) // (0011 11)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 1;
        }else if ((coeff_token >> 12) == 0x000E) // (1110)b
        {
            coeff_token_bit_length = 4;
            TrailingOnes = 1;
            TotalCoeff = 1;
        }else if ((coeff_token >> 10) == 0x000B) // (0010 11)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 2;
        }else if ((coeff_token >> 11) == 0x000F) // (0111 1)b
        {
            coeff_token_bit_length = 5;
            TrailingOnes = 1;
            TotalCoeff = 2;
        }else if ((coeff_token >> 12) == 0x000D) // (1101)b
        {
            coeff_token_bit_length = 4;
            TrailingOnes = 2;
            TotalCoeff = 2;
        }else if ((coeff_token >> 10) == 0x0008) // (0010 00)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 3;
        }else if ((coeff_token >> 11) == 0x000C) // (0110 0)b
        {
            coeff_token_bit_length = 5;
            TrailingOnes = 1;
            TotalCoeff = 3;
        }else if ((coeff_token >> 11) == 0x000E) // (0111 0)b
        {
            coeff_token_bit_length = 5;
            TrailingOnes = 2;
            TotalCoeff = 3;
        }else if ((coeff_token >> 12) == 0x000C) // (1100)b
        {
            coeff_token_bit_length = 4;
            TrailingOnes = 3;
            TotalCoeff = 3;
        }else if ((coeff_token >> 9) == 0x000F) // (0001 111)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 0;
            TotalCoeff = 4;
        }else if ((coeff_token >> 11) == 0x000A) // (0101 0)b
        {
            coeff_token_bit_length = 5;
            TrailingOnes = 1;
            TotalCoeff = 4;
        }else if ((coeff_token >> 11) == 0x000B) // (0101 1)b
        {
            coeff_token_bit_length = 5;
            TrailingOnes = 2;
            TotalCoeff = 4;
        }else if ((coeff_token >> 12) == 0x000B) // (1011)b
        {
            coeff_token_bit_length = 4;
            TrailingOnes = 3;
            TotalCoeff = 4;
        }else if ((coeff_token >> 9) == 0x000B) // (0001 011)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 0;
            TotalCoeff = 5;
        }else if ((coeff_token >> 11) == 0x0008) // (0100 0)b
        {
            coeff_token_bit_length = 5;
            TrailingOnes = 1;
            TotalCoeff = 5;
        }else if ((coeff_token >> 11) == 0x0009) // (0100 1)b
        {
            coeff_token_bit_length = 5;
            TrailingOnes = 2;
            TotalCoeff = 5;
        }else if ((coeff_token >> 12) == 0x000A) // (1010)b
        {
            coeff_token_bit_length = 4;
            TrailingOnes = 3;
            TotalCoeff = 5;
        }else if ((coeff_token >> 9) == 0x0009) // (0001 001)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 0;
            TotalCoeff = 6;
        }else if ((coeff_token >> 10) == 0x000E) // (0011 10)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 1;
            TotalCoeff = 6;
        }else if ((coeff_token >> 10) == 0x000D) // (0011 01)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 2;
            TotalCoeff = 6;
        }else if ((coeff_token >> 12) == 0x0009) // (1001)b
        {
            coeff_token_bit_length = 4;
            TrailingOnes = 3;
            TotalCoeff = 6;
        }else if ((coeff_token >> 9) == 0x0008) // (0001 000)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 0;
            TotalCoeff = 7;
        }else if ((coeff_token >> 10) == 0x000A) // (0010 10)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 1;
            TotalCoeff = 7;
        }else if ((coeff_token >> 10) == 0x0009) // (0010 01)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 2;
            TotalCoeff = 7;
        }else if ((coeff_token >> 12) == 0x0008) // (1000)b
        {
            coeff_token_bit_length = 4;
            TrailingOnes = 3;
            TotalCoeff = 7;
        }else if ((coeff_token >> 8) == 0x000F) // (0000 1111)b
        {
            coeff_token_bit_length = 8;
            TrailingOnes = 0;
            TotalCoeff = 8;
        }else if ((coeff_token >> 9) == 0x000E) // (0001 110)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 1;
            TotalCoeff = 8;
        }else if ((coeff_token >> 9) == 0x000D) // (0001 101)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 2;
            TotalCoeff = 8;
        }else if ((coeff_token >> 11) == 0x000D) // (0110 1)b
        {
            coeff_token_bit_length = 5;
            TrailingOnes = 3;
            TotalCoeff = 8;
        }else if ((coeff_token >> 8) == 0x000B) // (0000 1011)b
        {
            coeff_token_bit_length = 8;
            TrailingOnes = 0;
            TotalCoeff = 9;
        }else if ((coeff_token >> 8) == 0x000E) // (0000 1110)b
        {
            coeff_token_bit_length = 8;
            TrailingOnes = 1;
            TotalCoeff = 9;
        }else if ((coeff_token >> 9) == 0x000A) // (0001 010)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 2;
            TotalCoeff = 9;
        }else if ((coeff_token >> 10) == 0x000C) // (0011 00)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 3;
            TotalCoeff = 9;
        }else if ((coeff_token >> 7) == 0x000F) // (0000 0111 1)b
        {
            coeff_token_bit_length = 9;
            TrailingOnes = 0;
            TotalCoeff = 10;
        }else if ((coeff_token >> 8) == 0x000A) // (0000 1010)b
        {
            coeff_token_bit_length = 8;
            TrailingOnes = 1;
            TotalCoeff = 10;
        }else if ((coeff_token >> 8) == 0x000D) // (0000 1101)b
        {
            coeff_token_bit_length = 8;
            TrailingOnes = 2;
            TotalCoeff = 10;
        }else if ((coeff_token >> 9) == 0x000C) // (0001 100)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 3;
            TotalCoeff = 10;
        }else if ((coeff_token >> 7) == 0x000B) // (0000 0101 1)b
        {
            coeff_token_bit_length = 9;
            TrailingOnes = 0;
            TotalCoeff = 11;
        }else if ((coeff_token >> 7) == 0x000E) // (0000 0111 0)b
        {
            coeff_token_bit_length = 9;
            TrailingOnes = 1;
            TotalCoeff = 11;
        }else if ((coeff_token >> 8) == 0x0009) // (0000 1001)b
        {
            coeff_token_bit_length = 8;
            TrailingOnes = 2;
            TotalCoeff = 11;
        }else if ((coeff_token >> 8) == 0x000C) // (0000 1100)b
        {
            coeff_token_bit_length = 8;
            TrailingOnes = 3;
            TotalCoeff = 11;
        }else if ((coeff_token >> 7) == 0x0008) // (0000 0100 0)b
        {
            coeff_token_bit_length = 9;
            TrailingOnes = 0;
            TotalCoeff = 12;
        }else if ((coeff_token >> 7) == 0x000A) // (0000 0101 0)b
        {
            coeff_token_bit_length = 9;
            TrailingOnes = 1;
            TotalCoeff = 12;
        }else if ((coeff_token >> 7) == 0x000D) // (0000 0110 1)b
        {
            coeff_token_bit_length = 9;
            TrailingOnes = 2;
            TotalCoeff = 12;
        }else if ((coeff_token >> 8) == 0x0008) // (0000 1000)b
        {
            coeff_token_bit_length = 8;
            TrailingOnes = 3;
            TotalCoeff = 12;
        }else if ((coeff_token >> 6) == 0x000D) // (0000 0011 01)b
        {
            coeff_token_bit_length = 10;
            TrailingOnes = 0;
            TotalCoeff = 13;
        }else if ((coeff_token >> 7) == 0x0007) // (0000 0011 1)b
        {
            coeff_token_bit_length = 9;
            TrailingOnes = 1;
            TotalCoeff = 13;
        }else if ((coeff_token >> 7) == 0x0009) // (0000 0100 1)b
        {
            coeff_token_bit_length = 9;
            TrailingOnes = 2;
            TotalCoeff = 13;
        }else if ((coeff_token >> 7) == 0x000C) // (0000 0110 0)b
        {
            coeff_token_bit_length = 9;
            TrailingOnes = 3;
            TotalCoeff = 13;
        }else if ((coeff_token >> 6) == 0x0009) // (0000 0010 01)b
        {
            coeff_token_bit_length = 10;
            TrailingOnes = 0;
            TotalCoeff = 14;
        }else if ((coeff_token >> 6) == 0x000C) // (0000 0011 00)b
        {
            coeff_token_bit_length = 10;
            TrailingOnes = 1;
            TotalCoeff = 14;
        }else if ((coeff_token >> 6) == 0x000B) // (0000 0010 11)b
        {
            coeff_token_bit_length = 10;
            TrailingOnes = 2;
            TotalCoeff = 14;
        }else if ((coeff_token >> 6) == 0x000A) // (0000 0010 10)b
        {
            coeff_token_bit_length = 10;
            TrailingOnes = 3;
            TotalCoeff = 14;
        }else if ((coeff_token >> 6) == 0x0005) // (0000 0001 01)b
        {
            coeff_token_bit_length = 10;
            TrailingOnes = 0;
            TotalCoeff = 15;
        }else if ((coeff_token >> 6) == 0x0008) // (0000 0010 00)b
        {
            coeff_token_bit_length = 10;
            TrailingOnes = 1;
            TotalCoeff = 15;
        }else if ((coeff_token >> 6) == 0x0007) // (0000 0001 11)b
        {
            coeff_token_bit_length = 10;
            TrailingOnes = 2;
            TotalCoeff = 15;
        }else if ((coeff_token >> 6) == 0x0006) // (0000 0001 10)b
        {
            coeff_token_bit_length = 10;
            TrailingOnes = 3;
            TotalCoeff = 15;
        }else if ((coeff_token >> 6) == 0x0001) // (0000 0000 01)b
        {
            coeff_token_bit_length = 10;
            TrailingOnes = 0;
            TotalCoeff = 16;
        }else if ((coeff_token >> 6) == 0x0004) // (0000 0001 00)b
        {
            coeff_token_bit_length = 10;
            TrailingOnes = 1;
            TotalCoeff = 16;
        }else if ((coeff_token >> 6) == 0x0003) // (0000 0000 11)b
        {
            coeff_token_bit_length = 10;
            TrailingOnes = 2;
            TotalCoeff = 16;
        }else if ((coeff_token >> 6) == 0x0002) // (0000 0000 10)b
        {
            coeff_token_bit_length = 10;
            TrailingOnes = 3;
            TotalCoeff = 16;
        }else
        {
            RETURN_IF_FAILED(1, -1);
        }
    }else if (nC >= 8)
    {
        if ((coeff_token >> 10) == 0x0003) // (0000 11)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 0;
        }else if ((coeff_token >> 10) == 0x0000) // (0000 00)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 1;
        }else if ((coeff_token >> 10) == 0x0001) // (0000 01)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 1;
            TotalCoeff = 1;
        }else if ((coeff_token >> 10) == 0x0004) // (0001 00)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 2;
        }else if ((coeff_token >> 10) == 0x0005) // (0001 01)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 1;
            TotalCoeff = 2;
        }else if ((coeff_token >> 10) == 0x0006) // (0001 10)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 2;
            TotalCoeff = 2;
        }else if ((coeff_token >> 10) == 0x0008) // (0010 00)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 3;
        }else if ((coeff_token >> 10) == 0x0009) // (0010 01)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 1;
            TotalCoeff = 3;
        }else if ((coeff_token >> 10) == 0x000A) // (0010 10)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 2;
            TotalCoeff = 3;
        }else if ((coeff_token >> 10) == 0x000B) // (0010 11)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 3;
            TotalCoeff = 3;
        }else if ((coeff_token >> 10) == 0x000C) // (0011 00)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 4;
        }else if ((coeff_token >> 10) == 0x000D) // (0011 01)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 1;
            TotalCoeff = 4;
        }else if ((coeff_token >> 10) == 0x000E) // (0011 10)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 2;
            TotalCoeff = 4;
        }else if ((coeff_token >> 10) == 0x000F) // (0011 11)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 3;
            TotalCoeff = 4;
        }else if ((coeff_token >> 10) == 0x0010) // (0100 00)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 5;
        }else if ((coeff_token >> 10) == 0x0011) // (0100 01)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 1;
            TotalCoeff = 5;
        }else if ((coeff_token >> 10) == 0x0012) // (0100 10)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 2;
            TotalCoeff = 5;
        }else if ((coeff_token >> 10) == 0x0013) // (0100 11)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 3;
            TotalCoeff = 5;
        }else if ((coeff_token >> 10) == 0x0014) // (0101 00)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 6;
        }else if ((coeff_token >> 10) == 0x0015) // (0101 01)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 1;
            TotalCoeff = 6;
        }else if ((coeff_token >> 10) == 0x0016) // (0101 10)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 2;
            TotalCoeff = 6;
        }else if ((coeff_token >> 10) == 0x0017) // (0101 11)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 3;
            TotalCoeff = 6;
        }else if ((coeff_token >> 10) == 0x0018) // (0110 00)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 7;
        }else if ((coeff_token >> 10) == 0x0019) // (0110 01)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 1;
            TotalCoeff = 7;
        }else if ((coeff_token >> 10) == 0x001A) // (0110 10)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 2;
            TotalCoeff = 7;
        }else if ((coeff_token >> 10) == 0x001B) // (0110 11)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 3;
            TotalCoeff = 7;
        }else if ((coeff_token >> 10) == 0x001C) // (0111 00)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 8;
        }else if ((coeff_token >> 10) == 0x001D) // (0111 01)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 1;
            TotalCoeff = 8;
        }else if ((coeff_token >> 10) == 0x001E) // (0111 10)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 2;
            TotalCoeff = 8;
        }else if ((coeff_token >> 10) == 0x001F) // (0111 11)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 3;
            TotalCoeff = 8;
        }else if ((coeff_token >> 10) == 0x0020) // (1000 00)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 9;
        }else if ((coeff_token >> 10) == 0x0021) // (1000 01)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 1;
            TotalCoeff = 9;
        }else if ((coeff_token >> 10) == 0x0022) // (1000 10)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 2;
            TotalCoeff = 9;
        }else if ((coeff_token >> 10) == 0x0023) // (1000 11)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 3;
            TotalCoeff = 9;
        }else if ((coeff_token >> 10) == 0x0024) // (1001 00)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 10;
        }else if ((coeff_token >> 10) == 0x0025) // (1001 01)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 1;
            TotalCoeff = 10;
        }else if ((coeff_token >> 10) == 0x0026) // (1001 10)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 2;
            TotalCoeff = 10;
        }else if ((coeff_token >> 10) == 0x0027) // (1001 11)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 3;
            TotalCoeff = 10;
        }else if ((coeff_token >> 10) == 0x0028) // (1010 00)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 11;
        }else if ((coeff_token >> 10) == 0x0029) // (1010 01)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 1;
            TotalCoeff = 11;
        }else if ((coeff_token >> 10) == 0x002A) // (1010 10)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 2;
            TotalCoeff = 11;
        }else if ((coeff_token >> 10) == 0x002B) // (1010 11)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 3;
            TotalCoeff = 11;
        }else if ((coeff_token >> 10) == 0x002C) // (1011 00)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 12;
        }else if ((coeff_token >> 10) == 0x002D) // (1011 01)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 1;
            TotalCoeff = 12;
        }else if ((coeff_token >> 10) == 0x002E) // (1011 10)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 2;
            TotalCoeff = 12;
        }else if ((coeff_token >> 10) == 0x002F) // (1011 11)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 3;
            TotalCoeff = 12;
        }else if ((coeff_token >> 10) == 0x0030) // (1100 00)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 13;
        }else if ((coeff_token >> 10) == 0x0031) // (1100 01)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 1;
            TotalCoeff = 13;
        }else if ((coeff_token >> 10) == 0x0032) // (1100 10)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 2;
            TotalCoeff = 13;
        }else if ((coeff_token >> 10) == 0x0033) // (1100 11)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 3;
            TotalCoeff = 13;
        }else if ((coeff_token >> 10) == 0x0034) // (1101 00)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 14;
        }else if ((coeff_token >> 10) == 0x0035) // (1101 01)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 1;
            TotalCoeff = 14;
        }else if ((coeff_token >> 10) == 0x0036) // (1101 10)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 2;
            TotalCoeff = 14;
        }else if ((coeff_token >> 10) == 0x0037) // (1101 11)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 3;
            TotalCoeff = 14;
        }else if ((coeff_token >> 10) == 0x0038) // (1110 00)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 15;
        }else if ((coeff_token >> 10) == 0x0039) // (1110 01)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 1;
            TotalCoeff = 15;
        }else if ((coeff_token >> 10) == 0x003A) // (1110 10)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 2;
            TotalCoeff = 15;
        }else if ((coeff_token >> 10) == 0x003B) // (1110 11)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 3;
            TotalCoeff = 15;
        }else if ((coeff_token >> 10) == 0x003C) // (1111 00)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 16;
        }else if ((coeff_token >> 10) == 0x003D) // (1111 01)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 1;
            TotalCoeff = 16;
        }else if ((coeff_token >> 10) == 0x003E) // (1111 10)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 2;
            TotalCoeff = 16;
        }else if ((coeff_token >> 10) == 0x003F) // (1111 11)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 3;
            TotalCoeff = 16;
        }else
        {
            RETURN_IF_FAILED(1, -1);
        }
    }else if (nC == -1)
    {
        if ((coeff_token >> 14) == 0x0001) // (01)b
        {
            coeff_token_bit_length = 2;
            TrailingOnes = 0;
            TotalCoeff = 0;
        }else if ((coeff_token >> 10) == 0x0007) // (0001 11)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 1;
        }else if ((coeff_token >> 15) == 0x0001) // (1)b
        {
            coeff_token_bit_length = 1;
            TrailingOnes = 1;
            TotalCoeff = 1;
        }else if ((coeff_token >> 10) == 0x0004) // (0001 00)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 2;
        }else if ((coeff_token >> 10) == 0x0006) // (0001 10)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 1;
            TotalCoeff = 2;
        }else if ((coeff_token >> 13) == 0x0001) // (001)b
        {
            coeff_token_bit_length = 3;
            TrailingOnes = 2;
            TotalCoeff = 2;
        }else if ((coeff_token >> 10) == 0x0003) // (0000 11)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 3;
        }else if ((coeff_token >> 9) == 0x0003) // (0000 011)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 1;
            TotalCoeff = 3;
        }else if ((coeff_token >> 9) == 0x0002) // (0000 010)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 2;
            TotalCoeff = 3;
        }else if ((coeff_token >> 10) == 0x0005) // (0001 01)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 3;
            TotalCoeff = 3;
        }else if ((coeff_token >> 10) == 0x0002) // (0000 10)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 0;
            TotalCoeff = 4;
        }else if ((coeff_token >> 8) == 0x0003) // (0000 0011)b
        {
            coeff_token_bit_length = 8;
            TrailingOnes = 1;
            TotalCoeff = 4;
        }else if ((coeff_token >> 8) == 0x0002) // (0000 0010)b
        {
            coeff_token_bit_length = 8;
            TrailingOnes = 2;
            TotalCoeff = 4;
        }else if ((coeff_token >> 9) == 0x0000) // (0000 000)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 3;
            TotalCoeff = 4;
        }else
        {
            RETURN_IF_FAILED(1, -1);
        }
    }else if (nC == -2)
    {
        if ((coeff_token >> 15) == 0x0001) // (1)b
        {
            coeff_token_bit_length = 1;
            TrailingOnes = 0;
            TotalCoeff = 0;
        }else if ((coeff_token >> 9) == 0x000F) // (0001 111)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 0;
            TotalCoeff = 1;
        }else if ((coeff_token >> 14) == 0x0001) // (01)b
        {
            coeff_token_bit_length = 2;
            TrailingOnes = 1;
            TotalCoeff = 1;
        }else if ((coeff_token >> 9) == 0x000E) // (0001 110)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 0;
            TotalCoeff = 2;
        }else if ((coeff_token >> 9) == 0x000D) // (0001 101)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 1;
            TotalCoeff = 2;
        }else if ((coeff_token >> 13) == 0x0001) // (001)b
        {
            coeff_token_bit_length = 3;
            TrailingOnes = 2;
            TotalCoeff = 2;
        }else if ((coeff_token >> 7) == 0x0007) // (0000 0011 1)b
        {
            coeff_token_bit_length = 9;
            TrailingOnes = 0;
            TotalCoeff = 3;
        }else if ((coeff_token >> 9) == 0x000C) // (0001 100)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 1;
            TotalCoeff = 3;
        }else if ((coeff_token >> 9) == 0x000B) // (0001 011)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 2;
            TotalCoeff = 3;
        }else if ((coeff_token >> 11) == 0x0001) // (0000 1)b
        {
            coeff_token_bit_length = 5;
            TrailingOnes = 3;
            TotalCoeff = 3;
        }else if ((coeff_token >> 7) == 0x0006) // (0000 0011 0)b
        {
            coeff_token_bit_length = 9;
            TrailingOnes = 0;
            TotalCoeff = 4;
        }else if ((coeff_token >> 7) == 0x0005) // (0000 0010 1)b
        {
            coeff_token_bit_length = 9;
            TrailingOnes = 1;
            TotalCoeff = 4;
        }else if ((coeff_token >> 9) == 0x000A) // (0001 010)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 2;
            TotalCoeff = 4;
        }else if ((coeff_token >> 10) == 0x0001) // (0000 01)b
        {
            coeff_token_bit_length = 6;
            TrailingOnes = 3;
            TotalCoeff = 4;
        }else if ((coeff_token >> 6) == 0x0007) // (0000 0001 11)b
        {
            coeff_token_bit_length = 10;
            TrailingOnes = 0;
            TotalCoeff = 5;
        }else if ((coeff_token >> 6) == 0x0006) // (0000 0001 10)b
        {
            coeff_token_bit_length = 10;
            TrailingOnes = 1;
            TotalCoeff = 5;
        }else if ((coeff_token >> 7) == 0x0004) // (0000 0010 0)b
        {
            coeff_token_bit_length = 9;
            TrailingOnes = 2;
            TotalCoeff = 5;
        }else if ((coeff_token >> 9) == 0x0009) // (0001 001)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 3;
            TotalCoeff = 5;
        }else if ((coeff_token >> 5) == 0x0007) // (0000 0000 111)b
        {
            coeff_token_bit_length = 11;
            TrailingOnes = 0;
            TotalCoeff = 6;
        }else if ((coeff_token >> 5) == 0x0006) // (0000 0000 110)b
        {
            coeff_token_bit_length = 11;
            TrailingOnes = 1;
            TotalCoeff = 6;
        }else if ((coeff_token >> 6) == 0x0005) // (0000 0001 01)b
        {
            coeff_token_bit_length = 10;
            TrailingOnes = 2;
            TotalCoeff = 6;
        }else if ((coeff_token >> 9) == 0x0008) // (0001 000)b
        {
            coeff_token_bit_length = 7;
            TrailingOnes = 3;
            TotalCoeff = 6;
        }else if ((coeff_token >> 4) == 0x0007) // (0000 0000 0111)b
        {
            coeff_token_bit_length = 12;
            TrailingOnes = 0;
            TotalCoeff = 7;
        }else if ((coeff_token >> 4) == 0x0006) // (0000 0000 0110)b
        {
            coeff_token_bit_length = 12;
            TrailingOnes = 1;
            TotalCoeff = 7;
        }else if ((coeff_token >> 5) == 0x0005) // (0000 0000 101)b
        {
            coeff_token_bit_length = 11;
            TrailingOnes = 2;
            TotalCoeff = 7;
        }else if ((coeff_token >> 6) == 0x0004) // (0000 0001 00)b
        {
            coeff_token_bit_length = 10;
            TrailingOnes = 3;
            TotalCoeff = 7;
        }else if ((coeff_token >> 3) == 0x0007) // (0000 0000 0011 1)b
        {
            coeff_token_bit_length = 13;
            TrailingOnes = 0;
            TotalCoeff = 8;
        }else if ((coeff_token >> 4) == 0x0005) // (0000 0000 0101)b
        {
            coeff_token_bit_length = 12;
            TrailingOnes = 1;
            TotalCoeff = 8;
        }else if ((coeff_token >> 4) == 0x0004) // (0000 0000 0100)b
        {
            coeff_token_bit_length = 12;
            TrailingOnes = 2;
            TotalCoeff = 8;
        }else if ((coeff_token >> 5) == 0x0004) // (0000 0000 100)b
        {
            coeff_token_bit_length = 11;
            TrailingOnes = 3;
            TotalCoeff = 8;
        }else
        {
            RETURN_IF_FAILED(1, -1);
        }
    }else
    {
        RETURN_IF_FAILED(1, -1);
    }

    return 0;
}


int CH264ResidualBlockCavlc::get_total_zeros(CBitstream &bs, int32_t maxNumCoeff, int32_t tzVlcIndex, int32_t &total_zeros)
{
    if (maxNumCoeff == 4) //If maxNumCoeff is equal to 4, one of the VLCs specified in Table 9-9 (a) is used.
    {
        int32_t token = 0;
        int32_t token2 = 0;
        int32_t token_length = 0;

        //Table 9-9 – total_zeros tables for chroma DC 2x2 and 2x4 blocks
        //(a) Chroma DC 2x2 block (4:2:0 chroma sampling)
        if (tzVlcIndex == 1)
        {
            token = bs.getBits(3);
            token_length = 0;
            if ((token >> 2) == 0x01) // (1)b
            {
                token_length = 1;
                total_zeros = 0;
            }else if ((token >> 1) == 0x01) // (01)b
            {
                token_length = 2;
                total_zeros = 1;
            }else if ((token >> 0) == 0x01) // (001)b
            {
                token_length = 3;
                total_zeros = 2;
            }else if ((token >> 0) == 0x00) // (000)b
            {
                token_length = 3;
                total_zeros = 3;
            }else
            {
                RETURN_IF_FAILED(1, -1);
            }
            token2 = bs.readBits(token_length);
        }else if (tzVlcIndex == 2)
        {
            token = bs.getBits(2);
            token_length = 0;
            if ((token >> 1) == 0x01) // (1)b
            {
                token_length = 1;
                total_zeros = 0;
            }else if ((token >> 0) == 0x01) // (01)b
            {
                token_length = 2;
                total_zeros = 1;
            }else if ((token >> 0) == 0x00) // (00)b
            {
                token_length = 2;
                total_zeros = 2;
            }else
            {
                RETURN_IF_FAILED(1, -1);
            }
            token2 = bs.readBits(token_length);
        }else if (tzVlcIndex == 3)
        {
            token = bs.getBits(1);
            token_length = 0;
            if ((token >> 0) == 0x01) // (1)b
            {
                token_length = 1;
                total_zeros = 0;
            }else if ((token >> 0) == 0x00) // (0)b
            {
                token_length = 1;
                total_zeros = 1;
            }else
            {
                RETURN_IF_FAILED(1, -1);
            }
            token2 = bs.readBits(token_length);
        }else
        {
            RETURN_IF_FAILED(1, -1);
        }
    }else if (maxNumCoeff == 8) //Otherwise, if maxNumCoeff is equal to 8, one of the VLCs specified in Table 9-9 (b) is used.
    {
        int32_t token = 0;
        int32_t token2 = 0;
        int32_t token_length = 0;

        //Table 9-9 – total_zeros tables for chroma DC 2x2 and 2x4 blocks
        //(b) Chroma DC 2x4 block (4:2:2 chroma sampling)
        if (tzVlcIndex == 1)
        {
            token = bs.getBits(5);
            token_length = 0;
            if ((token >> 4) == 0x01) // (1)b
            {
                token_length = 1;
                total_zeros = 0;
            }else if ((token >> 2) == 0x02) // (010)b
            {
                token_length = 3;
                total_zeros = 1;
            }else if ((token >> 2) == 0x03) // (011)b
            {
                token_length = 3;
                total_zeros = 2;
            }else if ((token >> 1) == 0x02) // (0010)b
            {
                token_length = 4;
                total_zeros = 3;
            }else if ((token >> 1) == 0x03) // (0011)b
            {
                token_length = 4;
                total_zeros = 4;
            }else if ((token >> 1) == 0x01) // (0001)b
            {
                token_length = 4;
                total_zeros = 5;
            }else if ((token >> 0) == 0x01) // (0000 1)b
            {
                token_length = 5;
                total_zeros = 6;
            }else if ((token >> 0) == 0x00) // (0000 0)b
            {
                token_length = 5;
                total_zeros = 7;
            }else
            {
                RETURN_IF_FAILED(1, -1);
            }
            token2 = bs.readBits(token_length);
        }else if (tzVlcIndex == 2)
        {
            token = bs.getBits(3);
            token_length = 0;
            if ((token >> 0) == 0x00) // (000)b
            {
                token_length = 3;
                total_zeros = 0;
            }else if ((token >> 1) == 0x01) // (01)b
            {
                token_length = 2;
                total_zeros = 1;
            }else if ((token >> 0) == 0x01) // (001)b
            {
                token_length = 3;
                total_zeros = 2;
            }else if ((token >> 0) == 0x04) // (100)b
            {
                token_length = 3;
                total_zeros = 3;
            }else if ((token >> 0) == 0x05) // (101)b
            {
                token_length = 3;
                total_zeros = 4;
            }else if ((token >> 0) == 0x06) // (110)b
            {
                token_length = 3;
                total_zeros = 5;
            }else if ((token >> 0) == 0x07) // (111)b
            {
                token_length = 3;
                total_zeros = 6;
            }else
            {
                RETURN_IF_FAILED(1, -1);
            }
            token2 = bs.readBits(token_length);
        }else if (tzVlcIndex == 3)
        {
            token = bs.getBits(3);
            token_length = 0;
            if ((token >> 0) == 0x00) // (000)b
            {
                token_length = 3;
                total_zeros = 0;
            }else if ((token >> 0) == 0x01) // (001)b
            {
                token_length = 3;
                total_zeros = 1;
            }else if ((token >> 1) == 0x01) // (01)b
            {
                token_length = 2;
                total_zeros = 2;
            }else if ((token >> 1) == 0x02) // (10)b
            {
                token_length = 2;
                total_zeros = 3;
            }else if ((token >> 0) == 0x06) // (110)b
            {
                token_length = 3;
                total_zeros = 4;
            }else if ((token >> 0) == 0x07) // (111)b
            {
                token_length = 3;
                total_zeros = 5;
            }else
            {
                RETURN_IF_FAILED(1, -1);
            }
            token2 = bs.readBits(token_length);
        }else if (tzVlcIndex == 4)
        {
            token = bs.getBits(3);
            token_length = 0;
            if ((token >> 0) == 0x06) // (110)b
            {
                token_length = 3;
                total_zeros = 0;
            }else if ((token >> 1) == 0x00) // (00)b
            {
                token_length = 2;
                total_zeros = 1;
            }else if ((token >> 1) == 0x01) // (01)b
            {
                token_length = 2;
                total_zeros = 2;
            }else if ((token >> 1) == 0x02) // (10)b
            {
                token_length = 2;
                total_zeros = 3;
            }else if ((token >> 0) == 0x07) // (111)b
            {
                token_length = 3;
                total_zeros = 4;
            }else
            {
                RETURN_IF_FAILED(1, -1);
            }
            token2 = bs.readBits(token_length);
        }else if (tzVlcIndex == 5)
        {
            token = bs.getBits(2);
            token_length = 0;
            if ((token >> 0) == 0x00) // (00)b
            {
                token_length = 2;
                total_zeros = 0;
            }else if ((token >> 0) == 0x01) // (01)b
            {
                token_length = 2;
                total_zeros = 1;
            }else if ((token >> 0) == 0x02) // (10)b
            {
                token_length = 2;
                total_zeros = 2;
            }else if ((token >> 0) == 0x03) // (11)b
            {
                token_length = 2;
                total_zeros = 3;
            }else
            {
                RETURN_IF_FAILED(1, -1);
            }
            token2 = bs.readBits(token_length);
        }else if (tzVlcIndex == 6)
        {
            token = bs.getBits(2);
            token_length = 0;
            if ((token >> 0) == 0x00) // (00)b
            {
                token_length = 2;
                total_zeros = 0;
            }else if ((token >> 0) == 0x01) // (01)b
            {
                token_length = 2;
                total_zeros = 1;
            }else if ((token >> 1) == 0x01) // (1)b
            {
                token_length = 1;
                total_zeros = 2;
            }else
            {
                RETURN_IF_FAILED(1, -1);
            }
            token2 = bs.readBits(token_length);
        }else if (tzVlcIndex == 7)
        {
            token = bs.getBits(1);
            token_length = 0;
            if ((token >> 0) == 0x00) // (0)b
            {
                token_length = 1;
                total_zeros = 0;
            }else if ((token >> 0) == 0x01) // (1)b
            {
                token_length = 1;
                total_zeros = 1;
            }else
            {
                RETURN_IF_FAILED(1, -1);
            }
            token2 = bs.readBits(token_length);
        }else
        {
            RETURN_IF_FAILED(1, -1);
        }
    }else //Otherwise (maxNumCoeff is not equal to 4 and not equal to 8), VLCs from Tables 9-7 and 9-8 are used.
    {
        int32_t token = 0;
        int32_t token2 = 0;
        int32_t token_length = 0;

        //Table 9-7 – total_zeros tables for 4x4 blocks with tzVlcIndex 1 to 7
        if (tzVlcIndex == 1)
        {
            token = bs.getBits(9);
            token_length = 0;
            if ((token >> 8) == 0x01) // (1)b
            {
                token_length = 1;
                total_zeros = 0;
            }else if ((token >> 6) == 0x03) // (011)b
            {
                token_length = 3;
                total_zeros = 1;
            }else if ((token >> 6) == 0x02) // (010)b
            {
                token_length = 3;
                total_zeros = 2;
            }else if ((token >> 5) == 0x03) // (0011)b
            {
                token_length = 4;
                total_zeros = 3;
            }else if ((token >> 5) == 0x02) // (0010)b
            {
                token_length = 4;
                total_zeros = 4;
            }else if ((token >> 4) == 0x03) // (0001 1)b
            {
                token_length = 5;
                total_zeros = 5;
            }else if ((token >> 4) == 0x02) // (0001 0)b
            {
                token_length = 5;
                total_zeros = 6;
            }else if ((token >> 3) == 0x03) // (0000 11)b
            {
                token_length = 6;
                total_zeros = 7;
            }else if ((token >> 3) == 0x02) // (0000 10)b
            {
                token_length = 6;
                total_zeros = 8;
            }else if ((token >> 2) == 0x03) // (0000 011)b
            {
                token_length = 7;
                total_zeros = 9;
            }else if ((token >> 2) == 0x02) // (0000 010)b
            {
                token_length = 7;
                total_zeros = 10;
            }else if ((token >> 1) == 0x03) // (0000 0011)b
            {
                token_length = 8;
                total_zeros = 11;
            }else if ((token >> 1) == 0x02) // (0000 0010)b
            {
                token_length = 8;
                total_zeros = 12;
            }else if ((token >> 0) == 0x03) // (0000 0001 1)b
            {
                token_length = 9;
                total_zeros = 13;
            }else if ((token >> 0) == 0x02) // (0000 0001 0)b
            {
                token_length = 9;
                total_zeros = 14;
            }else if ((token >> 0) == 0x01) // (0000 0000 1)b
            {
                token_length = 9;
                total_zeros = 15;
            }
            token2 = bs.readBits(token_length);
        }else if (tzVlcIndex == 2)
        {
            token = bs.getBits(6);
            token_length = 0;
            if ((token >> 3) == 0x07) // (111)b
            {
                token_length = 3;
                total_zeros = 0;
            }else if ((token >> 3) == 0x06) // (110)b
            {
                token_length = 3;
                total_zeros = 1;
            }else if ((token >> 3) == 0x05) // (101)b
            {
                token_length = 3;
                total_zeros = 2;
            }else if ((token >> 3) == 0x04) // (100)b
            {
                token_length = 3;
                total_zeros = 3;
            }else if ((token >> 3) == 0x03) // (011)b
            {
                token_length = 3;
                total_zeros = 4;
            }else if ((token >> 2) == 0x05) // (0101)b
            {
                token_length = 4;
                total_zeros = 5;
            }else if ((token >> 2) == 0x04) // (0100)b
            {
                token_length = 4;
                total_zeros = 6;
            }else if ((token >> 2) == 0x03) // (0011)b
            {
                token_length = 4;
                total_zeros = 7;
            }else if ((token >> 2) == 0x02) // (0010)b
            {
                token_length = 4;
                total_zeros = 8;
            }else if ((token >> 1) == 0x03) // (0001 1)b
            {
                token_length = 5;
                total_zeros = 9;
            }else if ((token >> 1) == 0x02) // (0001 0)b
            {
                token_length = 5;
                total_zeros = 10;
            }else if ((token >> 0) == 0x03) // (0000 11)b
            {
                token_length = 6;
                total_zeros = 11;
            }else if ((token >> 0) == 0x02) // (0000 10)b
            {
                token_length = 6;
                total_zeros = 12;
            }else if ((token >> 0) == 0x01) // (0000 01)b
            {
                token_length = 6;
                total_zeros = 13;
            }else if ((token >> 0) == 0x00) // (0000 00)b
            {
                token_length = 6;
                total_zeros = 14;
            }
            token2 = bs.readBits(token_length);
        }else if (tzVlcIndex == 3)
        {
            token = bs.getBits(6);
            token_length = 0;
            if ((token >> 2) == 0x05) // (0101)b
            {
                token_length = 4;
                total_zeros = 0;
            }else if ((token >> 3) == 0x07) // (111)b
            {
                token_length = 3;
                total_zeros = 1;
            }else if ((token >> 3) == 0x06) // (110)b
            {
                token_length = 3;
                total_zeros = 2;
            }else if ((token >> 3) == 0x05) // (101)b
            {
                token_length = 3;
                total_zeros = 3;
            }else if ((token >> 2) == 0x04) // (0100)b
            {
                token_length = 4;
                total_zeros = 4;
            }else if ((token >> 2) == 0x03) // (0011)b
            {
                token_length = 4;
                total_zeros = 5;
            }else if ((token >> 3) == 0x04) // (100)b
            {
                token_length = 3;
                total_zeros = 6;
            }else if ((token >> 3) == 0x03) // (011)b
            {
                token_length = 3;
                total_zeros = 7;
            }else if ((token >> 2) == 0x02) // (0010)b
            {
                token_length = 4;
                total_zeros = 8;
            }else if ((token >> 1) == 0x03) // (0001 1)b
            {
                token_length = 5;
                total_zeros = 9;
            }else if ((token >> 1) == 0x02) // (0001 0)b
            {
                token_length = 5;
                total_zeros = 10;
            }else if ((token >> 0) == 0x01) // (0000 01)b
            {
                token_length = 6;
                total_zeros = 11;
            }else if ((token >> 1) == 0x01) // (0000 1)b
            {
                token_length = 5;
                total_zeros = 12;
            }else if ((token >> 0) == 0x00) // (0000 00)b
            {
                token_length = 6;
                total_zeros = 13;
            }
            token2 = bs.readBits(token_length);
        }else if (tzVlcIndex == 4)
        {
            token = bs.getBits(5);
            token_length = 0;
            if ((token >> 0) == 0x03) // (0001 1)b
            {
                token_length = 5;
                total_zeros = 0;
            }else if ((token >> 2) == 0x07) // (111)b
            {
                token_length = 3;
                total_zeros = 1;
            }else if ((token >> 1) == 0x05) // (0101)b
            {
                token_length = 4;
                total_zeros = 2;
            }else if ((token >> 1) == 0x04) // (0100)b
            {
                token_length = 4;
                total_zeros = 3;
            }else if ((token >> 2) == 0x06) // (110)b
            {
                token_length = 3;
                total_zeros = 4;
            }else if ((token >> 2) == 0x05) // (101)b
            {
                token_length = 3;
                total_zeros = 5;
            }else if ((token >> 2) == 0x04) // (100)b
            {
                token_length = 3;
                total_zeros = 6;
            }else if ((token >> 1) == 0x03) // (0011)b
            {
                token_length = 4;
                total_zeros = 7;
            }else if ((token >> 2) == 0x03) // (011)b
            {
                token_length = 3;
                total_zeros = 8;
            }else if ((token >> 1) == 0x02) // (0010)b
            {
                token_length = 4;
                total_zeros = 9;
            }else if ((token >> 0) == 0x02) // (0001 0)b
            {
                token_length = 5;
                total_zeros = 10;
            }else if ((token >> 0) == 0x01) // (0000 1)b
            {
                token_length = 5;
                total_zeros = 11;
            }else if ((token >> 0) == 0x00) // (0000 0)b
            {
                token_length = 5;
                total_zeros = 12;
            }
            token2 = bs.readBits(token_length);
        }else if (tzVlcIndex == 5)
        {
            token = bs.getBits(5);
            token_length = 0;
            if ((token >> 1) == 0x05) // (0101)b
            {
                token_length = 4;
                total_zeros = 0;
            }else if ((token >> 1) == 0x04) // (0100)b
            {
                token_length = 4;
                total_zeros = 1;
            }else if ((token >> 1) == 0x03) // (0011)b
            {
                token_length = 4;
                total_zeros = 2;
            }else if ((token >> 2) == 0x07) // (111)b
            {
                token_length = 3;
                total_zeros = 3;
            }else if ((token >> 2) == 0x06) // (110)b
            {
                token_length = 3;
                total_zeros = 4;
            }else if ((token >> 2) == 0x05) // (101)b
            {
                token_length = 3;
                total_zeros = 5;
            }else if ((token >> 2) == 0x04) // (100)b
            {
                token_length = 3;
                total_zeros = 6;
            }else if ((token >> 2) == 0x03) // (011)b
            {
                token_length = 3;
                total_zeros = 7;
            }else if ((token >> 1) == 0x02) // (0010)b
            {
                token_length = 4;
                total_zeros = 8;
            }else if ((token >> 0) == 0x01) // (0000 1)b
            {
                token_length = 5;
                total_zeros = 9;
            }else if ((token >> 1) == 0x01) // (0001)b
            {
                token_length = 4;
                total_zeros = 10;
            }else if ((token >> 0) == 0x00) // (0000 0)b
            {
                token_length = 5;
                total_zeros = 11;
            }
            token2 = bs.readBits(token_length);
        }else if (tzVlcIndex == 6)
        {
            token = bs.getBits(6);
            token_length = 0;
            if ((token >> 0) == 0x01) // (0000 01)b
            {
                token_length = 6;
                total_zeros = 0;
            }else if ((token >> 1) == 0x01) // (0000 1)b
            {
                token_length = 5;
                total_zeros = 1;
            }else if ((token >> 3) == 0x07) // (111)b
            {
                token_length = 3;
                total_zeros = 2;
            }else if ((token >> 3) == 0x06) // (110)b
            {
                token_length = 3;
                total_zeros = 3;
            }else if ((token >> 3) == 0x05) // (101)b
            {
                token_length = 3;
                total_zeros = 4;
            }else if ((token >> 3) == 0x04) // (100)b
            {
                token_length = 3;
                total_zeros = 5;
            }else if ((token >> 3) == 0x03) // (011)b
            {
                token_length = 3;
                total_zeros = 6;
            }else if ((token >> 3) == 0x02) // (010)b
            {
                token_length = 3;
                total_zeros = 7;
            }else if ((token >> 2) == 0x01) // (0001)b
            {
                token_length = 4;
                total_zeros = 8;
            }else if ((token >> 3) == 0x01) // (001)b
            {
                token_length = 3;
                total_zeros = 9;
            }else if ((token >> 0) == 0x00) // (0000 00)b
            {
                token_length = 6;
                total_zeros = 10;
            }
            token2 = bs.readBits(token_length);
        }else if (tzVlcIndex == 7)
        {
            token = bs.getBits(6);
            token_length = 0;
            if ((token >> 0) == 0x01) // (0000 01)b
            {
                token_length = 6;
                total_zeros = 0;
            }else if ((token >> 1) == 0x01) // (0000 1)b
            {
                token_length = 5;
                total_zeros = 1;
            }else if ((token >> 3) == 0x05) // (101)b
            {
                token_length = 3;
                total_zeros = 2;
            }else if ((token >> 3) == 0x04) // (100)b
            {
                token_length = 3;
                total_zeros = 3;
            }else if ((token >> 3) == 0x03) // (011)b
            {
                token_length = 3;
                total_zeros = 4;
            }else if ((token >> 4) == 0x03) // (11)b
            {
                token_length = 2;
                total_zeros = 5;
            }else if ((token >> 3) == 0x02) // (010)b
            {
                token_length = 3;
                total_zeros = 6;
            }else if ((token >> 2) == 0x01) // (0001)b
            {
                token_length = 4;
                total_zeros = 7;
            }else if ((token >> 3) == 0x01) // (001)b
            {
                token_length = 3;
                total_zeros = 8;
            }else if ((token >> 0) == 0x00) // (0000 00)b
            {
                token_length = 6;
                total_zeros = 9;
            }
            token2 = bs.readBits(token_length);
        }else if (tzVlcIndex == 8)
        {
            token = bs.getBits(6);
            token_length = 0;
            if ((token >> 0) == 0x01) // (0000 01)b
            {
                token_length = 6;
                total_zeros = 0;
            }else if ((token >> 2) == 0x01) // (0001)b
            {
                token_length = 4;
                total_zeros = 1;
            }else if ((token >> 1) == 0x01) // (0000 1)b
            {
                token_length = 5;
                total_zeros = 2;
            }else if ((token >> 3) == 0x03) // (011)b
            {
                token_length = 3;
                total_zeros = 3;
            }else if ((token >> 4) == 0x03) // (11)b
            {
                token_length = 2;
                total_zeros = 4;
            }else if ((token >> 4) == 0x02) // (10)b
            {
                token_length = 2;
                total_zeros = 5;
            }else if ((token >> 3) == 0x02) // (010)b
            {
                token_length = 3;
                total_zeros = 6;
            }else if ((token >> 3) == 0x01) // (001)b
            {
                token_length = 3;
                total_zeros = 7;
            }else if ((token >> 0) == 0x00) // (0000 00)b
            {
                token_length = 6;
                total_zeros = 8;
            }
            token2 = bs.readBits(token_length);
        }else if (tzVlcIndex == 9)
        {
            token = bs.getBits(6);
            token_length = 0;
            if ((token >> 0) == 0x01) // (0000 01)b
            {
                token_length = 6;
                total_zeros = 0;
            }else if ((token >> 0) == 0x00) // (0000 00)b
            {
                token_length = 6;
                total_zeros = 1;
            }else if ((token >> 2) == 0x01) // (0001)b
            {
                token_length = 4;
                total_zeros = 2;
            }else if ((token >> 4) == 0x03) // (11)b
            {
                token_length = 2;
                total_zeros = 3;
            }else if ((token >> 4) == 0x02) // (10)b
            {
                token_length = 2;
                total_zeros = 4;
            }else if ((token >> 3) == 0x01) // (001)b
            {
                token_length = 3;
                total_zeros = 5;
            }else if ((token >> 4) == 0x01) // (01)b
            {
                token_length = 2;
                total_zeros = 6;
            }else if ((token >> 1) == 0x01) // (0000 1)b
            {
                token_length = 5;
                total_zeros = 7;
            }
            token2 = bs.readBits(token_length);
        }else if (tzVlcIndex == 10)
        {
            token = bs.getBits(5);
            token_length = 0;
            if ((token >> 0) == 0x01) // (0000 1)b
            {
                token_length = 5;
                total_zeros = 0;
            }else if ((token >> 0) == 0x00) // (0000 0)b
            {
                token_length = 5;
                total_zeros = 1;
            }else if ((token >> 2) == 0x01) // (001)b
            {
                token_length = 3;
                total_zeros = 2;
            }else if ((token >> 3) == 0x03) // (11)b
            {
                token_length = 2;
                total_zeros = 3;
            }else if ((token >> 3) == 0x02) // (10)b
            {
                token_length = 2;
                total_zeros = 4;
            }else if ((token >> 3) == 0x01) // (01)b
            {
                token_length = 2;
                total_zeros = 5;
            }else if ((token >> 1) == 0x01) // (0001)b
            {
                token_length = 4;
                total_zeros = 6;
            }
            token2 = bs.readBits(token_length);
        }else if (tzVlcIndex == 11)
        {
            token = bs.getBits(4);
            token_length = 0;
            if ((token >> 0) == 0x00) // (0000)b
            {
                token_length = 4;
                total_zeros = 0;
            }else if ((token >> 0) == 0x01) // (0001)b
            {
                token_length = 4;
                total_zeros = 1;
            }else if ((token >> 1) == 0x01) // (001)b
            {
                token_length = 3;
                total_zeros = 2;
            }else if ((token >> 1) == 0x02) // (010)b
            {
                token_length = 3;
                total_zeros = 3;
            }else if ((token >> 3) == 0x01) // (1)b
            {
                token_length = 1;
                total_zeros = 4;
            }else if ((token >> 1) == 0x03) // (011)b
            {
                token_length = 3;
                total_zeros = 5;
            }
            token2 = bs.readBits(token_length);
        }else if (tzVlcIndex == 12)
        {
            token = bs.getBits(4);
            token_length = 0;
            if ((token >> 0) == 0x00) // (0000)b
            {
                token_length = 4;
                total_zeros = 0;
            }else if ((token >> 0) == 0x01) // (0001)b
            {
                token_length = 4;
                total_zeros = 1;
            }else if ((token >> 2) == 0x01) // (01)b
            {
                token_length = 2;
                total_zeros = 2;
            }else if ((token >> 3) == 0x01) // (1)b
            {
                token_length = 1;
                total_zeros = 3;
            }else if ((token >> 1) == 0x01) // (001)b
            {
                token_length = 3;
                total_zeros = 4;
            }
            token2 = bs.readBits(token_length);
        }else if (tzVlcIndex == 13)
        {
            token = bs.getBits(4);
            token_length = 0;
            if ((token >> 1) == 0x00) // (000)b
            {
                token_length = 3;
                total_zeros = 0;
            }else if ((token >> 1) == 0x01) // (001)b
            {
                token_length = 3;
                total_zeros = 1;
            }else if ((token >> 3) == 0x01) // (1)b
            {
                token_length = 1;
                total_zeros = 2;
            }else if ((token >> 2) == 0x01) // (01)b
            {
                token_length = 2;
                total_zeros = 3;
            }
            token2 = bs.readBits(token_length);
        }else if (tzVlcIndex == 14)
        {
            token = bs.getBits(2);
            token_length = 0;
            if ((token >> 0) == 0x00) // (00)b
            {
                token_length = 2;
                total_zeros = 0;
            }else if ((token >> 0) == 0x01) // (01)b
            {
                token_length = 2;
                total_zeros = 1;
            }else if ((token >> 1) == 0x01) // (1)b
            {
                token_length = 1;
                total_zeros = 2;
            }
            token2 = bs.readBits(token_length);
        }else if (tzVlcIndex == 15)
        {
            token = bs.getBits(1);
            token_length = 0;
            if ((token >> 0) == 0x00) // (0)b
            {
                token_length = 1;
                total_zeros = 0;
            }else if ((token >> 0) == 0x01) // (1)b
            {
                token_length = 1;
                total_zeros = 1;
            }
            token2 = bs.readBits(token_length);
        }
    }

    return 0;
}


int CH264ResidualBlockCavlc::get_run_before(CBitstream &bs, int32_t zerosLeft, int32_t &run_before)
{
    int32_t token = 0;
    int32_t token2 = 0;
    int32_t token_length = 0;

    if (zerosLeft == 1)
    {
        token = bs.getBits(1);
        token_length = 0;
        if ((token >> 0) == 0x01) // (1)b
        {
            token_length = 1;
            run_before = 0;
        }else if ((token >> 0) == 0x00) // (0)b
        {
            token_length = 1;
            run_before = 1;
        }
        token2 = bs.readBits(token_length);
    }else if (zerosLeft == 2)
    {
        token = bs.getBits(2);
        token_length = 0;
        if ((token >> 1) == 0x01) // (1)b
        {
            token_length = 1;
            run_before = 0;
        }else if ((token >> 0) == 0x01) // (01)b
        {
            token_length = 2;
            run_before = 1;
        }else if ((token >> 0) == 0x00) // (00)b
        {
            token_length = 2;
            run_before = 2;
        }
        token2 = bs.readBits(token_length);
    }else if (zerosLeft == 3)
    {
        token = bs.getBits(2);
        token_length = 0;
        if ((token >> 0) == 0x03) // (11)b
        {
            token_length = 2;
            run_before = 0;
        }else if ((token >> 0) == 0x02) // (10)b
        {
            token_length = 2;
            run_before = 1;
        }else if ((token >> 0) == 0x01) // (01)b
        {
            token_length = 2;
            run_before = 2;
        }else if ((token >> 0) == 0x00) // (00)b
        {
            token_length = 2;
            run_before = 3;
        }
        token2 = bs.readBits(token_length);
    }else if (zerosLeft == 4)
    {
        token = bs.getBits(3);
        token_length = 0;
        if ((token >> 1) == 0x03) // (11)b
        {
            token_length = 2;
            run_before = 0;
        }else if ((token >> 1) == 0x02) // (10)b
        {
            token_length = 2;
            run_before = 1;
        }else if ((token >> 1) == 0x01) // (01)b
        {
            token_length = 2;
            run_before = 2;
        }else if ((token >> 0) == 0x01) // (001)b
        {
            token_length = 3;
            run_before = 3;
        }else if ((token >> 0) == 0x00) // (000)b
        {
            token_length = 3;
            run_before = 4;
        }
        token2 = bs.readBits(token_length);
    }else if (zerosLeft == 5)
    {
        token = bs.getBits(3);
        token_length = 0;
        if ((token >> 1) == 0x03) // (11)b
        {
            token_length = 2;
            run_before = 0;
        }else if ((token >> 1) == 0x02) // (10)b
        {
            token_length = 2;
            run_before = 1;
        }else if ((token >> 0) == 0x03) // (011)b
        {
            token_length = 3;
            run_before = 2;
        }else if ((token >> 0) == 0x02) // (010)b
        {
            token_length = 3;
            run_before = 3;
        }else if ((token >> 0) == 0x01) // (001)b
        {
            token_length = 3;
            run_before = 4;
        }else if ((token >> 0) == 0x00) // (000)b
        {
            token_length = 3;
            run_before = 5;
        }
        token2 = bs.readBits(token_length);
    }else if (zerosLeft == 6)
    {
        token = bs.getBits(3);
        token_length = 0;
        if ((token >> 1) == 0x03) // (11)b
        {
            token_length = 2;
            run_before = 0;
        }else if ((token >> 0) == 0x00) // (000)b
        {
            token_length = 3;
            run_before = 1;
        }else if ((token >> 0) == 0x01) // (001)b
        {
            token_length = 3;
            run_before = 2;
        }else if ((token >> 0) == 0x03) // (011)b
        {
            token_length = 3;
            run_before = 3;
        }else if ((token >> 0) == 0x02) // (010)b
        {
            token_length = 3;
            run_before = 4;
        }else if ((token >> 0) == 0x05) // (101)b
        {
            token_length = 3;
            run_before = 5;
        }else if ((token >> 0) == 0x04) // (100)b
        {
            token_length = 3;
            run_before = 6;
        }
        token2 = bs.readBits(token_length);
    }else if (zerosLeft > 6)
    {
        token = bs.getBits(11);
        token_length = 0;
        if ((token >> 8) == 0x07) // (111)b
        {
            token_length = 3;
            run_before = 0;
        }else if ((token >> 8) == 0x06) // (110)b
        {
            token_length = 3;
            run_before = 1;
        }else if ((token >> 8) == 0x05) // (101)b
        {
            token_length = 3;
            run_before = 2;
        }else if ((token >> 8) == 0x04) // (100)b
        {
            token_length = 3;
            run_before = 3;
        }else if ((token >> 8) == 0x03) // (011)b
        {
            token_length = 3;
            run_before = 4;
        }else if ((token >> 8) == 0x02) // (010)b
        {
            token_length = 3;
            run_before = 5;
        }else if ((token >> 8) == 0x01) // (001)b
        {
            token_length = 3;
            run_before = 6;
        }else if ((token >> 7) == 0x01) // (0001)b
        {
            token_length = 4;
            run_before = 7;
        }else if ((token >> 6) == 0x01) // (00001)b
        {
            token_length = 5;
            run_before = 8;
        }else if ((token >> 5) == 0x01) // (000001)b
        {
            token_length = 6;
            run_before = 9;
        }else if ((token >> 4) == 0x01) // (0000001)b
        {
            token_length = 7;
            run_before = 10;
        }else if ((token >> 3) == 0x01) // (00000001)b
        {
            token_length = 8;
            run_before = 11;
        }else if ((token >> 2) == 0x01) // (000000001)b
        {
            token_length = 9;
            run_before = 12;
        }else if ((token >> 1) == 0x01) // (0000000001)b
        {
            token_length = 10;
            run_before = 13;
        }else if ((token >> 0) == 0x01) // (00000000001)b
        {
            token_length = 11;
            run_before = 14;
        }
        token2 = bs.readBits(token_length);
    }

    return 0;
}

