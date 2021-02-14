//
// H264PictureDeblockingFilterProcess.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264PictureBase.h"


//6.4.11.1 Derivation process for neighbouring macroblocks
int CH264PictureBase::Derivation_process_for_neighbouring_macroblocks(int32_t MbaffFrameFlag, int32_t _CurrMbAddr, int32_t &mbAddrA, int32_t &mbAddrB, int32_t isChroma)
{
    int ret = 0;
    
    CH264SliceHeader & slice_header = m_h264_slice_header;
    
    int32_t xW = 0;
    int32_t yW = 0;
    
    //---------------mbAddrA---------------------
    MB_ADDR_TYPE mbAddrA_type = MB_ADDR_TYPE_UNKOWN;
    int32_t luma4x4BlkIdxA = 0;
    int32_t luma8x8BlkIdxA = 0;
    int32_t xA = -1;
    int32_t yA = 0;
    
    //6.4.12 Derivation process for neighbouring locations
    ret = Derivation_process_for_neighbouring_locations(MbaffFrameFlag, xA, yA, _CurrMbAddr, mbAddrA_type, mbAddrA, luma4x4BlkIdxA, luma8x8BlkIdxA, xW, yW, isChroma);
    RETURN_IF_FAILED(ret != 0, ret);
    
    //---------------mbAddrB---------------------
    MB_ADDR_TYPE mbAddrB_type = MB_ADDR_TYPE_UNKOWN;
    int32_t luma4x4BlkIdxB = 0;
    int32_t luma8x8BlkIdxB = 0;
    int32_t xB = 0;
    int32_t yB = -1;
    
    //6.4.12 Derivation process for neighbouring locations
    ret = Derivation_process_for_neighbouring_locations(MbaffFrameFlag, xB, yB, _CurrMbAddr, mbAddrB_type, mbAddrB, luma4x4BlkIdxB, luma8x8BlkIdxB, xW, yW, isChroma);
    RETURN_IF_FAILED(ret != 0, ret);

    return ret;
}


//6.4.13.1 Derivation process for 4x4 luma block indices
int CH264PictureBase::Derivation_process_for_4x4_luma_block_indices(uint8_t xP, uint8_t yP, uint8_t &luma4x4BlkIdx)
{
    luma4x4BlkIdx = 8 * ( yP / 8 ) + 4 * ( xP / 8 ) + 2 * ( ( yP % 8 ) / 4 ) + ( ( xP % 8 ) / 4 );
    return 0;
}


//6.4.13.2 Derivation process for 4x4 chroma block indices
int CH264PictureBase::Derivation_process_for_4x4_chroma_block_indices(uint8_t xP, uint8_t yP, uint8_t &chroma4x4BlkIdx)
{
    chroma4x4BlkIdx = 2 * ( yP / 4 ) + ( xP / 4 );
    return 0;
}


//6.4.13.3 Derivation process for 8x8 luma block indices
int CH264PictureBase::Derivation_process_for_8x8_luma_block_indices(uint8_t xP, uint8_t yP, uint8_t &luma8x8BlkIdx)
{
    luma8x8BlkIdx = 2 * ( yP / 8 ) + ( xP / 8 );
    return 0;
}


//8.7 Deblocking filter process
//This filtering process is performed on a macroblock basis after the completion of the picture construction process prior 
//to deblocking filter process (as specified in clauses 8.5 and 8.6) for the entire decoded picture, with all macroblocks 
//in a picture processed in order of increasing macroblock addresses.
int CH264PictureBase::Deblocking_filter_process()
{
    int ret = 0;
    
    CH264SliceHeader & slice_header = m_h264_slice_header;
    
    int32_t i = 0;
    int32_t k = 0;
    int32_t isChroma = 0;
    int32_t fieldMbInFrameFlag = 0;
    int32_t filterInternalEdgesFlag = 0;
    int32_t filterLeftMbEdgeFlag = 0;
    int32_t filterTopMbEdgeFlag = 0;

    for (i = 0; i <= PicSizeInMbs - 1; i++)
    {
        int32_t CurrMbAddrTemp = i;
        
        int32_t mbAddrA = 0;
        int32_t mbAddrB = 0;

        isChroma = 0;

        int32_t MbaffFrameFlag = m_mbs[CurrMbAddrTemp].MbaffFrameFlag; //slice_header.MbaffFrameFlag;
        int32_t leftMbEdgeFlag = 0;

        //1. The derivation process for neighbouring macroblocks specified in clause 6.4.11.1 is invoked and the output is
        //assigned to mbAddrA and mbAddrB.
        ret = Derivation_process_for_neighbouring_macroblocks(MbaffFrameFlag, CurrMbAddrTemp, mbAddrA, mbAddrB, isChroma);
        RETURN_IF_FAILED(ret != 0, ret);

        //2. The variables fieldMbInFrameFlag, filterInternalEdgesFlag, filterLeftMbEdgeFlag and filterTopMbEdgeFlag are
        //derived as specified by the following ordered steps:
        if (MbaffFrameFlag == 1
            && m_mbs[CurrMbAddrTemp].mb_field_decoding_flag == 1
            )
        {
            fieldMbInFrameFlag = 1;
        }
        else
        {
            fieldMbInFrameFlag = 0;
        }
        
        if (m_mbs[CurrMbAddrTemp].disable_deblocking_filter_idc == 1) //If disable_deblocking_filter_idc for the slice that contains the macroblock CurrMbAddr is equal to 1
        {
            filterInternalEdgesFlag = 0;
        }
        else
        {
            filterInternalEdgesFlag = 1;
        }

        if ((MbaffFrameFlag == 0 && CurrMbAddrTemp % PicWidthInMbs == 0)
            || (MbaffFrameFlag == 1 && ( CurrMbAddrTemp >> 1 ) % PicWidthInMbs == 0)
            || (m_mbs[CurrMbAddrTemp].disable_deblocking_filter_idc == 1)
            || (m_mbs[CurrMbAddrTemp].disable_deblocking_filter_idc == 2 && mbAddrA < 0) //mbAddrA is not available
            )
        {
            filterLeftMbEdgeFlag = 0;
        }
        else
        {
            filterLeftMbEdgeFlag = 1;
        }
        
        if ((MbaffFrameFlag == 0 && CurrMbAddrTemp < PicWidthInMbs)
            || (MbaffFrameFlag == 1 && ( CurrMbAddrTemp >> 1 ) < PicWidthInMbs && m_mbs[CurrMbAddrTemp].mb_field_decoding_flag == 1) //the macroblock CurrMbAddr is a field macroblock
            || (MbaffFrameFlag == 1 && ( CurrMbAddrTemp >> 1 ) < PicWidthInMbs && m_mbs[CurrMbAddrTemp].mb_field_decoding_flag == 0 && (CurrMbAddrTemp % 2) == 0)
            || (m_mbs[CurrMbAddrTemp].disable_deblocking_filter_idc == 1)
            || (m_mbs[CurrMbAddrTemp].disable_deblocking_filter_idc == 2 && mbAddrB < 0) //mbAddrB is not available
            )
        {
            filterTopMbEdgeFlag = 0;
        }
        else
        {
            filterTopMbEdgeFlag = 1;
        }

        //3. Given the variables fieldMbInFrameFlag, filterInternalEdgesFlag, filterLeftMbEdgeFlag 
        //and filterTopMbEdgeFlag the deblocking filtering is controlled as follows:
        int32_t chromaEdgeFlag = 0;
        int32_t verticalEdgeFlag = 0;
        int32_t fieldModeInFrameFilteringFlag = 0;
        int32_t iCbCr = 0;
        int32_t E[16][2] = {0};

        //a. When filterLeftMbEdgeFlag is equal to 1, the left vertical luma edge is filtered
        if (filterLeftMbEdgeFlag == 1)
        {
            leftMbEdgeFlag = 0;

            if (MbaffFrameFlag == 1
                && CurrMbAddrTemp >= 2
                && m_mbs[CurrMbAddrTemp].mb_field_decoding_flag == 0 //the macroblock CurrMbAddr is a frame macroblock
                && m_mbs[(CurrMbAddrTemp - 2)].mb_field_decoding_flag == 1 //is a field macroblock
                )
            {
                leftMbEdgeFlag = 1;
            }

            chromaEdgeFlag = 0;
            verticalEdgeFlag = 1;
            fieldModeInFrameFilteringFlag = fieldMbInFrameFlag;

            for (k = 0; k <= 15; k++)
            {
                E[k][0] = 0; //(xEk, yEk) = (0, k)
                E[k][1] = k;
            }

            //8.7.1 Filtering process for block edges
            ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrA,
                verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
            RETURN_IF_FAILED(ret != 0, ret);
        }

        //b. When filterInternalEdgesFlag is equal to 1, the filtering of the internal vertical luma edges 
        //is specified by the following ordered steps:
        if (filterInternalEdgesFlag == 1)
        {
            chromaEdgeFlag = 0;
            verticalEdgeFlag = 1;
            fieldModeInFrameFilteringFlag = fieldMbInFrameFlag;
            leftMbEdgeFlag = 0;

            if (m_mbs[CurrMbAddrTemp].transform_size_8x8_flag == 0)
            {
                for (k = 0; k <= 15; k++)
                {
                    E[k][0] = 4; //(xEk, yEk) = (4, k)
                    E[k][1] = k;
                }

                ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrA,
                    verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                RETURN_IF_FAILED(ret != 0, ret);
            }

            //------------------------------
            for (k = 0; k <= 15; k++)
            {
                E[k][0] = 8; //(xEk, yEk) = (8, k)
                E[k][1] = k;
            }

            ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrA,
                verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
            RETURN_IF_FAILED(ret != 0, ret);

            //------------------------------
            if (m_mbs[CurrMbAddrTemp].transform_size_8x8_flag == 0)
            {
                for (k = 0; k <= 15; k++)
                {
                    E[k][0] = 12; //(xEk, yEk) = (12, k)
                    E[k][1] = k;
                }

                ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrA,
                    verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                RETURN_IF_FAILED(ret != 0, ret);
            }
        }

        //c. When filterTopMbEdgeFlag is equal to 1, the filtering of the top horizontal luma edge 
        //is specified as follows:
        if (filterTopMbEdgeFlag == 1)
        {
            if (MbaffFrameFlag == 1
                && (CurrMbAddrTemp % 2) == 0
                && CurrMbAddrTemp >= 2 * PicWidthInMbs
                && m_mbs[CurrMbAddrTemp].mb_field_decoding_flag == 0 //the macroblock CurrMbAddr is a frame macroblock
                && m_mbs[(CurrMbAddrTemp - 2 * PicWidthInMbs + 1)].mb_field_decoding_flag == 1 //is a field macroblock
                )
            {
                chromaEdgeFlag = 0;
                verticalEdgeFlag = 0;
                fieldModeInFrameFilteringFlag = 1;
                leftMbEdgeFlag = 0;

                for (k = 0; k <= 15; k++)
                {
                    E[k][0] = k; //(xEk, yEk) = (k, 0)
                    E[k][1] = 0;
                }

                ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrB - 1,
                    verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                RETURN_IF_FAILED(ret != 0, ret);

                //---------------------------
                for (k = 0; k <= 15; k++)
                {
                    E[k][0] = k; //(xEk, yEk) = (k, 1)
                    E[k][1] = 1;
                }

                ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrB,
                    verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                RETURN_IF_FAILED(ret != 0, ret);
            }
            else
            {
                chromaEdgeFlag = 0;
                verticalEdgeFlag = 0;
                fieldModeInFrameFilteringFlag = fieldMbInFrameFlag;
                leftMbEdgeFlag = 0;

                for (k = 0; k <= 15; k++)
                {
                    E[k][0] = k; //(xEk, yEk) = (k, 0)
                    E[k][1] = 0;
                }

                ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrB,
                    verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                RETURN_IF_FAILED(ret != 0, ret);
            }
        }

        //d. When filterInternalEdgesFlag is equal to 1, the filtering of the internal horizontal luma edges 
        //is specified by the following ordered steps:
        if (filterInternalEdgesFlag == 1)
        {
            chromaEdgeFlag = 0;
            verticalEdgeFlag = 0;
            fieldModeInFrameFilteringFlag = fieldMbInFrameFlag;
            leftMbEdgeFlag = 0;

            if (m_mbs[CurrMbAddrTemp].transform_size_8x8_flag == 0)
            {
                for (k = 0; k <= 15; k++)
                {
                    E[k][0] = k; //(xEk, yEk) = (k, 4)
                    E[k][1] = 4;
                }

                ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrB,
                    verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                RETURN_IF_FAILED(ret != 0, ret);
            }

            //------------------------------
            for (k = 0; k <= 15; k++)
            {
                E[k][0] = k; //(xEk, yEk) = (k, 8)
                E[k][1] = 8;
            }

            ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrB,
                verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
            RETURN_IF_FAILED(ret != 0, ret);

            //------------------------------
            if (m_mbs[CurrMbAddrTemp].transform_size_8x8_flag == 0)
            {
                for (k = 0; k <= 15; k++)
                {
                    E[k][0] = k; //(xEk, yEk) = (k, 12)
                    E[k][1] = 12;
                }

                ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrB,
                    verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                RETURN_IF_FAILED(ret != 0, ret);
            }
        }

        //e. When ChromaArrayType is not equal to 0, for the filtering of both chroma components, 
        //with iCbCr = 0 for Cb and iCbCr = 1 for Cr, the following ordered steps are specified:
        if (slice_header.m_sps.ChromaArrayType != 0)
        {
            //i. When filterLeftMbEdgeFlag is equal to 1, the left vertical chroma edge is filtered
            if (filterLeftMbEdgeFlag == 1)
            {
                leftMbEdgeFlag = 0;

                if (MbaffFrameFlag == 1
                    && CurrMbAddrTemp >= 2
                    && m_mbs[CurrMbAddrTemp].mb_field_decoding_flag == 0 //the macroblock CurrMbAddr is a frame macroblock
                    && m_mbs[(CurrMbAddrTemp - 2)].mb_field_decoding_flag == 1 //is a field macroblock
                    )
                {
                    leftMbEdgeFlag = 1;
                }

                chromaEdgeFlag = 1;
                iCbCr = 0; //Cb
                verticalEdgeFlag = 1;
                fieldModeInFrameFilteringFlag = fieldMbInFrameFlag;

                for (k = 0; k <= MbHeightC - 1; k++)
                {
                    E[k][0] = 0; //(xEk, yEk) = (0, k)
                    E[k][1] = k;
                }

                ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrA,
                    verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                RETURN_IF_FAILED(ret != 0, ret);

                iCbCr = 1; //Cr
                ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrA,
                    verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                RETURN_IF_FAILED(ret != 0, ret);
            }

            //ii. When filterInternalEdgesFlag is equal to 1, the filtering of the internal vertical chroma edge 
            //is specified by the following ordered steps:
            if (filterInternalEdgesFlag == 1)
            {
                chromaEdgeFlag = 1;
                verticalEdgeFlag = 1;
                fieldModeInFrameFilteringFlag = fieldMbInFrameFlag;
                leftMbEdgeFlag = 0;

                if (slice_header.m_sps.ChromaArrayType != 3 || m_mbs[CurrMbAddrTemp].transform_size_8x8_flag == 0)
                {
                    for (k = 0; k <= MbHeightC - 1; k++)
                    {
                        E[k][0] = 4; //(xEk, yEk) = (4, k)
                        E[k][1] = k;
                    }
                    
                    iCbCr = 0; //Cb
                    ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrA,
                        verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                    RETURN_IF_FAILED(ret != 0, ret);

                    iCbCr = 1; //Cr
                    ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrA,
                        verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                    RETURN_IF_FAILED(ret != 0, ret);
                }

                if (slice_header.m_sps.ChromaArrayType == 3)
                {
                    for (k = 0; k <= MbHeightC - 1; k++)
                    {
                        E[k][0] = 8; //(xEk, yEk) = (8, k)
                        E[k][1] = k;
                    }
                    
                    iCbCr = 0; //Cb
                    ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrA,
                        verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                    RETURN_IF_FAILED(ret != 0, ret);

                    iCbCr = 1; //Cr
                    ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrA,
                        verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                    RETURN_IF_FAILED(ret != 0, ret);
                }

                if (slice_header.m_sps.ChromaArrayType == 3 && m_mbs[CurrMbAddrTemp].transform_size_8x8_flag == 0)
                {
                    for (k = 0; k <= MbHeightC - 1; k++)
                    {
                        E[k][0] = 12; //(xEk, yEk) = (12, k)
                        E[k][1] = k;
                    }
                    
                    iCbCr = 0; //Cb
                    ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrA,
                        verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                    RETURN_IF_FAILED(ret != 0, ret);

                    iCbCr = 1; //Cr
                    ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrA,
                        verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                    RETURN_IF_FAILED(ret != 0, ret);
                }
            }

            //iii. When filterTopMbEdgeFlag is equal to 1, the filtering of the top horizontal chroma edge is specified as follows:
            if (filterTopMbEdgeFlag == 1)
            {
                if (MbaffFrameFlag == 1
                    && (CurrMbAddrTemp % 2) == 0
                    && CurrMbAddrTemp >= 2 * PicWidthInMbs
                    && m_mbs[CurrMbAddrTemp].mb_field_decoding_flag == 0 //the macroblock CurrMbAddr is a frame macroblock
                    && m_mbs[(CurrMbAddrTemp - 2 * PicWidthInMbs + 1)].mb_field_decoding_flag == 1 //is a field macroblock
                    )
                {
                    chromaEdgeFlag = 1;
                    verticalEdgeFlag = 0;
                    fieldModeInFrameFilteringFlag = 1;
                    leftMbEdgeFlag = 0;

                    for (k = 0; k <= MbWidthC - 1; k++)
                    {
                        E[k][0] = k; //(xEk, yEk) = (k, 0)
                        E[k][1] = 0;
                    }

                    iCbCr = 0; //Cb
                    ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrB,
                        verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                    RETURN_IF_FAILED(ret != 0, ret);

                    iCbCr = 1; //Cr
                    ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrB,
                        verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                    RETURN_IF_FAILED(ret != 0, ret);

                    //---------------------------
                    for (k = 0; k <= MbWidthC - 1; k++)
                    {
                        E[k][0] = k; //(xEk, yEk) = (k, 1)
                        E[k][1] = 1;
                    }
                    
                    iCbCr = 0; //Cb
                    ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrB,
                        verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                    RETURN_IF_FAILED(ret != 0, ret);

                    iCbCr = 1; //Cr
                    ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrB,
                        verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                    RETURN_IF_FAILED(ret != 0, ret);
                }
                else
                {
                    chromaEdgeFlag = 1;
                    verticalEdgeFlag = 0;
                    fieldModeInFrameFilteringFlag = fieldMbInFrameFlag;
                    leftMbEdgeFlag = 0;

                    for (k = 0; k <= MbWidthC - 1; k++)
                    {
                        E[k][0] = k; //(xEk, yEk) = (k, 0)
                        E[k][1] = 0;
                    }
                    
                    iCbCr = 0; //Cb
                    ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrB,
                        verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                    RETURN_IF_FAILED(ret != 0, ret);

                    iCbCr = 1; //Cr
                    ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrB,
                        verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                    RETURN_IF_FAILED(ret != 0, ret);
                }
            }

            //iv. When filterInternalEdgesFlag is equal to 1, the filtering of the internal horizontal chroma edge 
            //is specified by the following ordered steps:
            if (filterInternalEdgesFlag == 1)
            {
                chromaEdgeFlag = 1;
                verticalEdgeFlag = 0;
                fieldModeInFrameFilteringFlag = fieldMbInFrameFlag;
                leftMbEdgeFlag = 0;
                
                if (slice_header.m_sps.ChromaArrayType != 3 || m_mbs[CurrMbAddrTemp].transform_size_8x8_flag == 0)
                {
                    for (k = 0; k <= MbWidthC - 1; k++)
                    {
                        E[k][0] = k; //(xEk, yEk) = (k, 4)
                        E[k][1] = 4;
                    }
                    
                    iCbCr = 0; //Cb
                    ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrB,
                        verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                    RETURN_IF_FAILED(ret != 0, ret);

                    iCbCr = 1; //Cr
                    ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrB,
                        verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                    RETURN_IF_FAILED(ret != 0, ret);
                }

                if (slice_header.m_sps.ChromaArrayType != 1)
                {
                    for (k = 0; k <= MbWidthC - 1; k++)
                    {
                        E[k][0] = k; //(xEk, yEk) = (k, 8)
                        E[k][1] = 8;
                    }
                    
                    iCbCr = 0; //Cb
                    ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrB,
                        verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                    RETURN_IF_FAILED(ret != 0, ret);

                    iCbCr = 1; //Cr
                    ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrB,
                        verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                    RETURN_IF_FAILED(ret != 0, ret);
                }
                
                if (slice_header.m_sps.ChromaArrayType == 2)
                {
                    for (k = 0; k <= MbWidthC - 1; k++)
                    {
                        E[k][0] = k; //(xEk, yEk) = (k, 12)
                        E[k][1] = 12;
                    }
                    
                    iCbCr = 0; //Cb
                    ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrB,
                        verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                    RETURN_IF_FAILED(ret != 0, ret);

                    iCbCr = 1; //Cr
                    ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrB,
                        verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                    RETURN_IF_FAILED(ret != 0, ret);
                }

                if (slice_header.m_sps.ChromaArrayType == 3 && m_mbs[CurrMbAddrTemp].transform_size_8x8_flag == 0)
                {
                    for (k = 0; k <= MbWidthC - 1; k++)
                    {
                        E[k][0] = k; //(xEk, yEk) = (k, 12)
                        E[k][1] = 12;
                    }
                    
                    iCbCr = 0; //Cb
                    ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrB,
                        verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                    RETURN_IF_FAILED(ret != 0, ret);

                    iCbCr = 1; //Cr
                    ret = Filtering_process_for_block_edges(MbaffFrameFlag, CurrMbAddrTemp, m_mbs[CurrMbAddrTemp].mb_field_decoding_flag, chromaEdgeFlag, iCbCr, mbAddrB,
                        verticalEdgeFlag, fieldModeInFrameFilteringFlag, leftMbEdgeFlag, E);
                    RETURN_IF_FAILED(ret != 0, ret);
                }
            }
        }
    }

    //-----------------------
    if (slice_header.m_sps.separate_colour_plane_flag == 0)
    {
        //the arrays S′L, S′Cb, S′Cr are assigned to the arrays SL, SCb, SCr (which represent the decoded picture), respectively.
    }
    else //if (slice_header.m_sps.separate_colour_plane_flag == 1) //CHROMA_FORMAT_IDC_444
    {
        if (slice_header.colour_plane_id == 0)
        {
            //the arrays S′L is assigned to the array SL (which represent the luma component of the decoded picture).
        }
        else if (slice_header.colour_plane_id == 1)
        {
            //the arrays S′L is assigned to the array SCb (which represents the Cb component of the decoded picture).
        }
        else //if (slice_header.colour_plane_id == 2)
        {
            //the arrays S′L is assigned to the array SCr (which represents the Cr component of the decoded picture).
        }
    }

    return ret;
}


//8.7.1 Filtering process for block edges
int CH264PictureBase::Filtering_process_for_block_edges(int32_t MbaffFrameFlag, int32_t _CurrMbAddr, int32_t mb_field_decoding_flag, int32_t chromaEdgeFlag, int32_t iCbCr, int32_t mbAddrN,
        int32_t verticalEdgeFlag, int32_t fieldModeInFrameFilteringFlag, int32_t leftMbEdgeFlag, int32_t (&E)[16][2])
{
    int ret = 0;

    CH264SliceHeader & slice_header = m_h264_slice_header;
    
    //-----------------------
    uint8_t * pic_buff = NULL;
    int32_t PicWidthInSamples = 0;
    int32_t i = 0;
    int32_t k = 0;

    if (chromaEdgeFlag == 0)
    {
        pic_buff = m_pic_buff_luma;
        PicWidthInSamples = PicWidthInSamplesL;
    }
    else if (chromaEdgeFlag == 1 && iCbCr == 0)
    {
        pic_buff = m_pic_buff_cb;
        PicWidthInSamples = PicWidthInSamplesC;
    }
    else if (chromaEdgeFlag == 1 && iCbCr == 1)
    {
        pic_buff = m_pic_buff_cr;
        PicWidthInSamples = PicWidthInSamplesC;
    }

    //-----------------------
    int32_t nE = 0;
    
    if (chromaEdgeFlag == 0)
    {
        nE = 16;
    }
    else
    {
        nE = ( verticalEdgeFlag == 1 ) ? MbHeightC : MbWidthC;
    }
    
    int32_t dy = (1 + fieldModeInFrameFilteringFlag);

    int32_t xI = 0;
    int32_t yI = 0;

    //6.4.1 Inverse macroblock scanning process
    ret = Inverse_macroblock_scanning_process(MbaffFrameFlag, _CurrMbAddr, mb_field_decoding_flag, xI, yI);
    RETURN_IF_FAILED(ret != 0, ret);
    
    int32_t xP = 0;
    int32_t yP = 0;

    if (chromaEdgeFlag == 0)
    {
        xP = xI;
        yP = yI;
    }
    else
    {
        xP = xI / slice_header.m_sps.SubWidthC;
        yP = (yI + slice_header.m_sps.SubHeightC - 1) / slice_header.m_sps.SubHeightC;
    }

    //---------------------------------
    // p3 p2 p1 p0 | q0 q1 q2 q3
    for (k = 0; k <= nE - 1; k++)
    {
        uint8_t p[4] = {0};
        uint8_t q[4] = {0};

        //1. The filtering process is applied to a set of eight samples across a 4x4 block horizontal or vertical edge 
        //denoted as pi and qi with i = 0..3 as shown in Figure 8-11 with the edge lying between p0 and q0. pi and qi 
        //with i = 0..3 are specified as follows:
        for (i = 0; i <= 3; i++)
        {
            if (verticalEdgeFlag == 1)
            {
                //qi = s′[ xP + xEk + i, yP + dy * yEk ]
                q[i] = pic_buff[(yP + dy * E[k][1]) * PicWidthInSamples + (xP + E[k][0] + i)];

                //pi = s′[ xP + xEk − i − 1, yP + dy * yEk ]
                p[i] = pic_buff[(yP + dy * E[k][1]) * PicWidthInSamples + (xP + E[k][0] - i - 1)];
            }
            else //if (verticalEdgeFlag == 0)
            {
                //qi = s′[ xP + xEk, yP + dy * ( yEk + i ) − (yEk % 2 ) ]
                q[i] = pic_buff[(yP + dy * (E[k][1] + i) - (E[k][1] % 2)) * PicWidthInSamples + (xP + E[k][0])];

                //pi = s′[ xP + xEk, yP + dy * ( yEk − i − 1 ) − (yEk % 2 ) ]
                p[i] = pic_buff[(yP + dy * (E[k][1] - i - 1) - (E[k][1] % 2)) * PicWidthInSamples + (xP + E[k][0])];
            }
        }
        
        //-----------用于查找4x4或8x8非零系数数目------------------
        int32_t mbAddr_p0 = _CurrMbAddr;
        int32_t mbAddr_q0 = _CurrMbAddr;

        int8_t mb_x_p0 = 0;
        int8_t mb_y_p0 = 0;
        int8_t mb_x_q0 = 0;
        int8_t mb_y_q0 = 0;
        
        if (verticalEdgeFlag == 1)
        {
            mb_x_p0 = E[k][0] - 0 - 1;
            if (mb_x_p0 < 0)
            {
                if (mbAddrN >= 0)
                {
                    if (leftMbEdgeFlag == 0)
                    {
                        mbAddr_p0 = mbAddrN;
                    }
                    else //if (leftMbEdgeFlag == 1)
                    {
                        mbAddr_p0 = mbAddrN + (E[k][1] % 2);
                    }
                }
                else //if (mbAddrN < 0)
                {
//                    LOG_ERROR("mb_x_p0(%d) < 0 and mbAddrN(%d) < 0\n", mb_x_p0, mbAddrN);
                }
                
                if (chromaEdgeFlag == 0)
                {
                    mb_x_p0 += 16;
                }
                else
                {
                    mb_x_p0 += 8;
                }
            }

            mb_y_p0 = E[k][1];
            mb_x_q0 = E[k][0] + 0;
            mb_y_q0 = E[k][1];
        }
        else //if (verticalEdgeFlag == 0)
        {
            mb_x_p0 = E[k][0];

            mb_y_p0 = (E[k][1] - 0 - 1) - (E[k][1] % 2);
            if (mb_y_p0 < 0)
            {
                if (mbAddrN >= 0)
                {
                    mbAddr_p0 = mbAddrN;
                }
                else //if (mbAddrN < 0)
                {
//                    LOG_ERROR("mb_x_p0(%d) < 0 and mbAddrN(%d) < 0\n", mb_x_p0, mbAddrN);
                }
                
                if (chromaEdgeFlag == 0)
                {
                    mb_y_p0 += 16;
                }
                else
                {
                    mb_y_p0 += 8;
                }
            }
            
            mb_x_q0 = E[k][0];
            mb_y_q0 = (E[k][1] + 0) - (E[k][1] % 2);
        }

        //-----------------------------
        uint8_t pp[3] = {0};
        uint8_t qq[3] = {0};

        //2. The process specified in clause 8.7.2 is invoked with the sample values pi and qi (i = 0..3), chromaEdgeFlag,
        // and verticalEdgeFlag as the inputs, and the output is assigned to the filtered result sample values p′i and q′i with i = 0..2.
        ret = Filtering_process_for_a_set_of_samples_across_a_horizontal_or_vertical_block_edge(MbaffFrameFlag, _CurrMbAddr, chromaEdgeFlag,
                iCbCr, mb_x_p0, mb_y_p0, mb_x_q0, mb_y_q0,
                verticalEdgeFlag, mbAddr_p0, p, q, pp, qq);
        RETURN_IF_FAILED(ret != 0, ret);

        //3. The input sample values pi and qi with i = 0..2 are replaced by the corresponding filtered result sample 
        //values p′i and q′i with i = 0..2 inside the sample array s′ as follows:
        for (i = 0; i <= 2; i++)
        {
            if (verticalEdgeFlag == 1)
            {
                //s′[ xP + xEk + i, yP + dy * yEk ] = q′i
                //s′[ xP + xEk − i − 1, yP + dy * yEk ] = p′i
                pic_buff[(yP + dy * E[k][1]) * PicWidthInSamples + (xP + E[k][0] + i)] = qq[i];
                pic_buff[(yP + dy * E[k][1]) * PicWidthInSamples + (xP + E[k][0] - i - 1)] = pp[i];
            }
            else //if (verticalEdgeFlag == 0)
            {
                //s′[ xP + xEk, yP + dy * ( yEk + i ) − ( yEk % 2 ) ] = q′i
                //s′[ xP + xEk, yP + dy * ( yEk − i − 1 ) − ( yEk % 2 ) ] = p′i
                pic_buff[(yP + dy * (E[k][1] + i) - (E[k][1] % 2)) * PicWidthInSamples + (xP + E[k][0])] = qq[i];
                pic_buff[(yP + dy * (E[k][1] - i - 1) - (E[k][1] % 2)) * PicWidthInSamples + (xP + E[k][0])] = pp[i];
            }
        }
    }

    return ret;
}


//8.7.2 Filtering process for a set of samples across a horizontal or vertical block edge
int CH264PictureBase::Filtering_process_for_a_set_of_samples_across_a_horizontal_or_vertical_block_edge(int32_t MbaffFrameFlag, int32_t _CurrMbAddr, int32_t chromaEdgeFlag,
        int32_t isChromaCb, uint8_t mb_x_p0, uint8_t mb_y_p0, uint8_t mb_x_q0, uint8_t mb_y_q0,
        int32_t verticalEdgeFlag, int32_t mbAddrN, const uint8_t (&p)[4], const uint8_t (&q)[4], uint8_t (&pp)[3], uint8_t (&qq)[3])
{
    int ret = 0;
    
    CH264SliceHeader & slice_header = m_h264_slice_header;

    int32_t bS = 0;
    int32_t mbAddr_p0 = mbAddrN;
    int32_t mbAddr_q0 = _CurrMbAddr;

    if (chromaEdgeFlag == 0)
    {
        //8.7.2.1 Derivation process for the luma content dependent boundary filtering strength
        ret = Derivation_process_for_the_luma_content_dependent_boundary_filtering_strength(MbaffFrameFlag, p[0], q[0], 
                mb_x_p0, mb_y_p0, mb_x_q0, mb_y_q0,
                mbAddr_p0, mbAddr_q0, verticalEdgeFlag, bS);
        RETURN_IF_FAILED(ret != 0, ret);
    }
    else //if (chromaEdgeFlag == 1)
    {
        //the bS used for filtering a set of samples of a horizontal or vertical chroma edge is set equal to the value 
        //of bS for filtering the set of samples of a horizontal or vertical luma edge, respectively, that contains the 
        //luma sample at location ( SubWidthC * x, SubHeightC * y ) inside the luma array of the same field, where ( x, y ) 
        //is the location of the chroma sample q0 inside the chroma array for that field.

        uint8_t mb_x_p0_chroma = m_h264_slice_header.m_sps.SubWidthC * mb_x_p0;
        uint8_t mb_y_p0_chroma = m_h264_slice_header.m_sps.SubHeightC * mb_y_p0;
        uint8_t mb_x_q0_chroma = m_h264_slice_header.m_sps.SubWidthC * mb_x_q0;
        uint8_t mb_y_q0_chroma = m_h264_slice_header.m_sps.SubHeightC * mb_y_q0;

        //8.7.2.1 Derivation process for the luma content dependent boundary filtering strength
        ret = Derivation_process_for_the_luma_content_dependent_boundary_filtering_strength(MbaffFrameFlag, p[0], q[0], 
                mb_x_p0_chroma, mb_y_p0_chroma, mb_x_q0_chroma, mb_y_q0_chroma,
                mbAddr_p0, mbAddr_q0, verticalEdgeFlag, bS);
        RETURN_IF_FAILED(ret != 0, ret);
    }

    //--------------------------------
    int32_t filterOffsetA = m_mbs[mbAddr_q0].FilterOffsetA;
    int32_t filterOffsetB = m_mbs[mbAddr_q0].FilterOffsetB;

    int32_t qPp = 0;
    int32_t qPq = 0;

    if (chromaEdgeFlag == 0)
    {
        if (m_mbs[mbAddr_p0].m_name_of_mb_type == I_PCM)
        {
            qPp = 0;
        }
        else
        {
            qPp = m_mbs[mbAddr_p0].QPY;
        }

        if (m_mbs[mbAddr_q0].m_name_of_mb_type == I_PCM)
        {
            qPq = 0;
        }
        else
        {
            qPq = m_mbs[mbAddr_q0].QPY;
        }
    }
    else //if (chromaEdgeFlag == 1)
    {
        int32_t QPY = 0;
        int32_t QPC = 0;

        if (m_mbs[mbAddr_p0].m_name_of_mb_type == I_PCM)
        {
            QPY = 0;
            ret = get_chroma_quantisation_parameters2(QPY, isChromaCb, QPC);
            RETURN_IF_FAILED(ret != 0, ret);
            qPp = QPC;
        }
        else
        {
            QPY = m_mbs[mbAddr_p0].QPY;
            ret = get_chroma_quantisation_parameters2(QPY, isChromaCb, QPC);
            RETURN_IF_FAILED(ret != 0, ret);
            qPp = QPC;
        }
        
        if (m_mbs[mbAddr_q0].m_name_of_mb_type == I_PCM)
        {
            QPY = 0;
            ret = get_chroma_quantisation_parameters2(QPY, isChromaCb, QPC);
            RETURN_IF_FAILED(ret != 0, ret);
            qPq = QPC;
        }
        else
        {
            QPY = m_mbs[mbAddr_q0].QPY;
            ret = get_chroma_quantisation_parameters2(QPY, isChromaCb, QPC);
            RETURN_IF_FAILED(ret != 0, ret);
            qPq = QPC;
        }
    }

    //---------------------------
    //The process specified in clause 8.7.2.2 is invoked with p0, q0, p1, q1, chromaEdgeFlag, bS, filterOffsetA, 
    //filterOffsetB, qPp, and qPq as inputs, and the outputs are assigned to filterSamplesFlag, indexA, α, and β.
    int32_t filterSamplesFlag = 0;
    int32_t indexA = 0;
    int32_t alpha = 0;
    int32_t beta = 0;

    ret = Derivation_process_for_the_thresholds_for_each_block_edge(p[0], q[0], p[1], q[1],
         chromaEdgeFlag, bS, filterOffsetA, filterOffsetB, qPp, qPq,
         filterSamplesFlag, indexA, alpha, beta);
    RETURN_IF_FAILED(ret != 0, ret);

    int32_t chromaStyleFilteringFlag = chromaEdgeFlag && ( slice_header.m_sps.ChromaArrayType != 3 );

    if (filterSamplesFlag == 1)
    {
        if (bS < 4)
        {
            //8.7.2.3 Filtering process for edges with bS less than 4
            ret = Filtering_process_for_edges_with_bS_less_than_4(p, q, chromaEdgeFlag, chromaStyleFilteringFlag, 
                    bS, beta, indexA, pp, qq);
            RETURN_IF_FAILED(ret != 0, ret);
        }
        else //if (bS == 4)
        {
            //8.7.2.4 Filtering process for edges for bS equal to 4
            ret = Filtering_process_for_edges_for_bS_equal_to_4(p, q, chromaEdgeFlag, chromaStyleFilteringFlag, 
                    alpha, beta, pp, qq);
            RETURN_IF_FAILED(ret != 0, ret);
        }
    }
    else //if (filterSamplesFlag == 0)
    {
        pp[0] = p[0];
        pp[1] = p[1];
        pp[2] = p[2];
        
        qq[0] = q[0];
        qq[1] = q[1];
        qq[2] = q[2];
    }

    return ret;
}


//8.7.2.1 Derivation process for the luma content dependent boundary filtering strength
int CH264PictureBase::Derivation_process_for_the_luma_content_dependent_boundary_filtering_strength(int32_t MbaffFrameFlag, int32_t p0, int32_t q0,
        uint8_t mb_x_p0, uint8_t mb_y_p0, uint8_t mb_x_q0, uint8_t mb_y_q0,
        int32_t mbAddr_p0, int32_t mbAddr_q0, int32_t verticalEdgeFlag, int32_t &bS)
{
    int ret = 0;
    
    int32_t mixedModeEdgeFlag = 0;

    //If MbaffFrameFlag is equal to 1 and the samples p0 and q0 are in different macroblock pairs, 
    //one of which is a field macroblock pair and the other is a frame macroblock pair,
    if (MbaffFrameFlag == 1
        && mbAddr_p0 != mbAddr_q0
        && m_mbs[mbAddr_p0].mb_field_decoding_flag != m_mbs[mbAddr_q0].mb_field_decoding_flag
        )
    {
        mixedModeEdgeFlag = 1;
    }
    else
    {
        mixedModeEdgeFlag = 0;
    }

    //----------------------------
    //If the block edge is also a macroblock edge
    if (mbAddr_p0 != mbAddr_q0)
    {
        //the samples p0 and q0 are both in frame macroblocks and either or both of 
        //the samples p0 or q0 is in a macroblock coded using an Intra macroblock prediction mode,
        if (m_mbs[mbAddr_p0].mb_field_decoding_flag == 0 && m_mbs[mbAddr_q0].mb_field_decoding_flag == 0
           && (IS_INTRA_Prediction_Mode(m_mbs[mbAddr_p0].m_mb_pred_mode) == true
               || IS_INTRA_Prediction_Mode(m_mbs[mbAddr_q0].m_mb_pred_mode) == true)

        //the samples p0 and q0 are both in frame macroblocks and either or both of 
        //the samples p0 or q0 is in a macroblock that is in a slice with slice_type equal to SP or SI,
        || (m_mbs[mbAddr_p0].mb_field_decoding_flag == 0 && m_mbs[mbAddr_q0].mb_field_decoding_flag == 0
           && (    m_mbs[mbAddr_p0].m_slice_type == H264_SLIECE_TYPE_SP
                 || m_mbs[mbAddr_p0].m_slice_type == H264_SLIECE_TYPE_SI
                 || m_mbs[mbAddr_q0].m_slice_type == H264_SLIECE_TYPE_SP
                 || m_mbs[mbAddr_q0].m_slice_type == H264_SLIECE_TYPE_SI)
          )

        //MbaffFrameFlag is equal to 1 or field_pic_flag is equal to 1, and verticalEdgeFlag is equal to 1, 
        //and either or both of the samples p0 or q0 is in a macroblock coded using an Intra macroblock prediction mode,
        || ( (MbaffFrameFlag == 1 || m_mbs[mbAddr_q0].field_pic_flag == 1)
            && verticalEdgeFlag == 1
            && (    IS_INTRA_Prediction_Mode(m_mbs[mbAddr_p0].m_mb_pred_mode) == true
                 || IS_INTRA_Prediction_Mode(m_mbs[mbAddr_q0].m_mb_pred_mode) == true
               )
          )
        
        //MbaffFrameFlag is equal to 1 or field_pic_flag is equal to 1, and verticalEdgeFlag is equal to 1, 
        //and either or both of the samples p0 or q0 is in a macroblock that is in a slice with slice_type equal to SP or SI.
        || ( (MbaffFrameFlag == 1 || m_mbs[mbAddr_q0].field_pic_flag == 1)
            && verticalEdgeFlag == 1
            && (    m_mbs[mbAddr_p0].m_slice_type == H264_SLIECE_TYPE_SP
                 || m_mbs[mbAddr_p0].m_slice_type == H264_SLIECE_TYPE_SI
                 || m_mbs[mbAddr_q0].m_slice_type == H264_SLIECE_TYPE_SP
                 || m_mbs[mbAddr_q0].m_slice_type == H264_SLIECE_TYPE_SI
               )
           )
         )
        {
            bS = 4;
            return 0;
        }
    }

    //mixedModeEdgeFlag is equal to 0 and either or both of the samples p0 or q0 
    //is in a macroblock coded using an Intra macroblock prediction mode,
    if ( (mixedModeEdgeFlag == 0
          && (IS_INTRA_Prediction_Mode(m_mbs[mbAddr_p0].m_mb_pred_mode) == true
              || IS_INTRA_Prediction_Mode(m_mbs[mbAddr_q0].m_mb_pred_mode) == true
             )
         )

        //mixedModeEdgeFlag is equal to 0 and either or both of the samples p0 or q0 
        //is in a macroblock that is in a slice with slice_type equal to SP or SI,
        || (mixedModeEdgeFlag == 0
        && (m_mbs[mbAddr_p0].m_slice_type == H264_SLIECE_TYPE_SP
            || m_mbs[mbAddr_p0].m_slice_type == H264_SLIECE_TYPE_SI
            || m_mbs[mbAddr_q0].m_slice_type == H264_SLIECE_TYPE_SP
            || m_mbs[mbAddr_q0].m_slice_type == H264_SLIECE_TYPE_SI
            )
        )

        //mixedModeEdgeFlag is equal to 1, verticalEdgeFlag is equal to 0, 
        //and either or both of the samples p0 or q0 is in a macroblock coded using an Intra macroblock prediction mode,
        || (mixedModeEdgeFlag == 1
            && verticalEdgeFlag == 0
            && (IS_INTRA_Prediction_Mode(m_mbs[mbAddr_p0].m_mb_pred_mode) == true
                || IS_INTRA_Prediction_Mode(m_mbs[mbAddr_q0].m_mb_pred_mode) == true
                )
            )

        //mixedModeEdgeFlag is equal to 1, verticalEdgeFlag is equal to 0, 
        //and either or both of the samples p0 or q0 is in a macroblock that is in a slice with slice_type equal to SP or SI.
        || (mixedModeEdgeFlag == 1
            && verticalEdgeFlag == 0
            && (m_mbs[mbAddr_p0].m_slice_type == H264_SLIECE_TYPE_SP
                || m_mbs[mbAddr_p0].m_slice_type == H264_SLIECE_TYPE_SI
                || m_mbs[mbAddr_q0].m_slice_type == H264_SLIECE_TYPE_SP
                || m_mbs[mbAddr_q0].m_slice_type == H264_SLIECE_TYPE_SI
                )
            )
        )
    {
        bS = 3;
        return 0;
    }

    //------------------------------------------
    uint8_t luma4x4BlkIdx_p0 = 0;
    uint8_t luma4x4BlkIdx_q0 = 0;
    uint8_t luma8x8BlkIdx_p0 = 0;
    uint8_t luma8x8BlkIdx_q0 = 0;

    ret = Derivation_process_for_4x4_luma_block_indices(mb_x_p0, mb_y_p0, luma4x4BlkIdx_p0);
    RETURN_IF_FAILED(ret != 0, ret);

    ret = Derivation_process_for_4x4_luma_block_indices(mb_x_q0, mb_y_q0, luma4x4BlkIdx_q0);
    RETURN_IF_FAILED(ret != 0, ret);

    ret = Derivation_process_for_8x8_luma_block_indices(mb_x_p0, mb_y_p0, luma8x8BlkIdx_p0);
    RETURN_IF_FAILED(ret != 0, ret);

    ret = Derivation_process_for_8x8_luma_block_indices(mb_x_q0, mb_y_q0, luma8x8BlkIdx_q0);
    RETURN_IF_FAILED(ret != 0, ret);

    //transform_size_8x8_flag is equal to 1 for the macroblock containing the sample p0 and the 8x8 luma transform block 
    //associated with the 8x8 luma block containing the sample p0 contains non-zero transform coefficient levels,
    if ( (m_mbs[mbAddr_p0].transform_size_8x8_flag == 1
          && m_mbs[mbAddr_p0].mb_luma_8x8_non_zero_count_coeff[luma8x8BlkIdx_p0] > 0)

        //transform_size_8x8_flag is equal to 0 for the macroblock containing the sample p0 and the 4x4 luma transform block 
        //associated with the 4x4 luma block containing the sample p0 contains non-zero transform coefficient levels,
        || (m_mbs[mbAddr_p0].transform_size_8x8_flag == 0
            && m_mbs[mbAddr_p0].mb_luma_4x4_non_zero_count_coeff[luma4x4BlkIdx_p0] > 0
            )

        //transform_size_8x8_flag is equal to 1 for the macroblock containing the sample q0 and the 8x8 luma transform block 
        //associated with the 8x8 luma block containing the sample q0 contains non-zero transform coefficient levels,
        || (m_mbs[mbAddr_q0].transform_size_8x8_flag == 1
            && m_mbs[mbAddr_q0].mb_luma_8x8_non_zero_count_coeff[luma8x8BlkIdx_q0] > 0
            )

        //transform_size_8x8_flag is equal to 0 for the macroblock containing the sample q0 and the 4x4 luma transform block 
        //associated with the 4x4 luma block containing the sample q0 contains non-zero transform coefficient levels.
        || (m_mbs[mbAddr_q0].transform_size_8x8_flag == 0
            && m_mbs[mbAddr_q0].mb_luma_4x4_non_zero_count_coeff[luma4x4BlkIdx_q0] > 0
            )
        )
    {
        bS = 2;
        return 0;
    }

    //--------------------------------------
    int32_t mbPartIdx_p0 = 0;
    int32_t subMbPartIdx_p0 = 0;
    int32_t mbPartIdx_q0 = 0;
    int32_t subMbPartIdx_q0 = 0;

    //NOTE 3 – A vertical difference of 4 in units of quarter luma frame samples is a difference of 2 in units of quarter luma field samples.
    int32_t mv_y_diff = ((m_picture_coded_type == H264_PICTURE_CODED_TYPE_TOP_FIELD || m_picture_coded_type == H264_PICTURE_CODED_TYPE_BOTTOM_FIELD) 
                            || (MbaffFrameFlag == 1 && m_mbs[mbAddr_q0].mb_field_decoding_flag == 1)) ? 2 : 4;
    
    //6.4.13.4 Derivation process for macroblock and sub-macroblock partition indices
    ret = Derivation_process_for_macroblock_and_sub_macroblock_partition_indices(m_mbs[mbAddr_p0].m_name_of_mb_type, m_mbs[mbAddr_p0].m_name_of_sub_mb_type, mb_x_p0, mb_y_p0, mbPartIdx_p0, subMbPartIdx_p0);
    RETURN_IF_FAILED(ret != 0, ret);
    
    //6.4.13.4 Derivation process for macroblock and sub-macroblock partition indices
    ret = Derivation_process_for_macroblock_and_sub_macroblock_partition_indices(m_mbs[mbAddr_q0].m_name_of_mb_type, m_mbs[mbAddr_q0].m_name_of_sub_mb_type, mb_x_q0, mb_y_q0, mbPartIdx_q0, subMbPartIdx_q0);
    RETURN_IF_FAILED(ret != 0, ret);

    if (mixedModeEdgeFlag == 1)
    {
        bS = 1;
        return 0;
    }
    else if (mixedModeEdgeFlag == 0)
    {
        CH264Picture * RefPicList0_p0 = (m_mbs[mbAddr_p0].m_RefIdxL0[mbPartIdx_p0] >= 0) ? this->m_RefPicList0[m_mbs[mbAddr_p0].m_RefIdxL0[mbPartIdx_p0]] : NULL;
        CH264Picture * RefPicList1_p0 = (m_mbs[mbAddr_p0].m_RefIdxL1[mbPartIdx_p0] >= 0) ? this->m_RefPicList1[m_mbs[mbAddr_p0].m_RefIdxL1[mbPartIdx_p0]] : NULL;
        CH264Picture * RefPicList0_q0 = (m_mbs[mbAddr_q0].m_RefIdxL0[mbPartIdx_q0] >= 0) ? this->m_RefPicList0[m_mbs[mbAddr_q0].m_RefIdxL0[mbPartIdx_q0]] : NULL;
        CH264Picture * RefPicList1_q0 = (m_mbs[mbAddr_q0].m_RefIdxL1[mbPartIdx_q0] >= 0) ? this->m_RefPicList1[m_mbs[mbAddr_q0].m_RefIdxL1[mbPartIdx_q0]] : NULL;

        int32_t PredFlagL0_p0 = m_mbs[mbAddr_p0].m_PredFlagL0[mbPartIdx_p0];
        int32_t PredFlagL1_p0 = m_mbs[mbAddr_p0].m_PredFlagL1[mbPartIdx_p0];
        int32_t PredFlagL0_q0 = m_mbs[mbAddr_q0].m_PredFlagL0[mbPartIdx_q0];
        int32_t PredFlagL1_q0 = m_mbs[mbAddr_q0].m_PredFlagL1[mbPartIdx_q0];
        
        int32_t MvL0_p0_x = m_mbs[mbAddr_p0].m_MvL0[mbPartIdx_p0][subMbPartIdx_p0][0];
        int32_t MvL0_p0_y = m_mbs[mbAddr_p0].m_MvL0[mbPartIdx_p0][subMbPartIdx_p0][1];
        int32_t MvL0_q0_x = m_mbs[mbAddr_q0].m_MvL0[mbPartIdx_q0][subMbPartIdx_q0][0];
        int32_t MvL0_q0_y = m_mbs[mbAddr_q0].m_MvL0[mbPartIdx_q0][subMbPartIdx_q0][1];
        
        int32_t MvL1_p0_x = m_mbs[mbAddr_p0].m_MvL1[mbPartIdx_p0][subMbPartIdx_p0][0];
        int32_t MvL1_p0_y = m_mbs[mbAddr_p0].m_MvL1[mbPartIdx_p0][subMbPartIdx_p0][1];
        int32_t MvL1_q0_x = m_mbs[mbAddr_q0].m_MvL1[mbPartIdx_q0][subMbPartIdx_q0][0];
        int32_t MvL1_q0_y = m_mbs[mbAddr_q0].m_MvL1[mbPartIdx_q0][subMbPartIdx_q0][1];
        
        //mixedModeEdgeFlag is equal to 0 and for the prediction of the macroblock/sub-macroblock partition containing the sample p0 different reference pictures or 
        //a different number of motion vectors are used than for the prediction of the macroblock/sub-macroblock partition containing the sample q0,
        //The number of motion vectors that are used for the prediction of a macroblock partition with macroblock partition index mbPartIdx, or a sub-macroblock 
        //partition contained in this macroblock partition, is equal to PredFlagL0[ mbPartIdx ] + PredFlagL1[ mbPartIdx ].
        //p0和q0有不同的参考图片，或者p0和q0有不同数量的运动向量
        if ( ((RefPicList0_p0 == RefPicList0_q0 && RefPicList1_p0 == RefPicList1_q0)
             || (RefPicList0_p0 == RefPicList1_q0 && RefPicList1_p0 == RefPicList0_q0)) //相同的参考图片
            && (PredFlagL0_p0 + PredFlagL1_p0) == (PredFlagL0_q0 + PredFlagL1_q0) //相同数量的运动向量
          )
        {
            //do nothing
        }
        else
        {
            bS = 1;
            return 0;
        }

        //mixedModeEdgeFlag is equal to 0 and one motion vector is used to predict the macroblock/sub-macroblock partition containing 
        //the sample p0 and one motion vector is used to predict the macroblock/sub-macroblock partition containing the sample q0 and 
        //the absolute difference between the horizontal or vertical components of the motion vectors used is greater than or equal 
        //to 4 in units of quarter luma frame samples,
        //一个运动矢量用于预测p0,一个运动矢量用于预测q0
        if ( (PredFlagL0_p0 == 1 && PredFlagL1_p0 == 0) && (PredFlagL0_q0 == 1 && PredFlagL1_q0 == 0)
            && (ABS(MvL0_p0_x - MvL0_q0_x) >= 4 || ABS(MvL0_p0_y - MvL0_q0_y) >= mv_y_diff)
          )
        {
            bS = 1;
            return 0;
        }
        else if ( (PredFlagL0_p0 == 1 && PredFlagL1_p0 == 0) && (PredFlagL0_q0 == 0 && PredFlagL1_q0 == 1)
            && (ABS(MvL0_p0_x - MvL1_q0_x) >= 4 || ABS(MvL0_p0_y - MvL1_q0_y) >= mv_y_diff)
          )
        {
            bS = 1;
            return 0;
        }
        else if ( (PredFlagL0_p0 == 0 && PredFlagL1_p0 == 1) && (PredFlagL0_q0 == 1 && PredFlagL1_q0 == 0)
            && (ABS(MvL1_p0_x - MvL0_q0_x) >= 4 || ABS(MvL1_p0_y - MvL0_q0_y) >= mv_y_diff)
          )
        {
            bS = 1;
            return 0;
        }
        else if ( (PredFlagL0_p0 == 0 && PredFlagL1_p0 == 1) && (PredFlagL0_q0 == 0 && PredFlagL1_q0 == 1)
            && (ABS(MvL1_p0_x - MvL1_q0_x) >= 4 || ABS(MvL1_p0_y - MvL1_q0_y) >= mv_y_diff)
          )
        {
            bS = 1;
            return 0;
        }
        
        //mixedModeEdgeFlag is equal to 0 and two motion vectors and two different reference pictures are used to predict the macroblock/sub-macroblock 
        //partition containing the sample p0 and two motion vectors for the same two reference pictures are used to predict the macroblock/sub-macroblock 
        //partition containing the sample q0 and, for either or both of the two used reference pictures, the absolute difference between the horizontal 
        //or vertical components of the two motion vectors used in the prediction of the two macroblock/sub-macroblock partitions for the particular 
        //reference picture is greater than or equal to 4 in units of quarter luma frame samples,
        //p0有两个不同的参考图片，q0也有两个不同的参考图片，并且q0的这两个参考图片和p0的两个参考图片是一样的
        if ( (PredFlagL0_p0 == 1 && PredFlagL1_p0 == 1) && (RefPicList0_p0 != RefPicList1_p0) //two motion vectors and two different reference pictures for p0
            && (PredFlagL0_q0 == 1 && PredFlagL1_q0 == 1)
            && ((RefPicList0_q0 == RefPicList0_p0 && RefPicList1_q0 == RefPicList1_p0)
                 || (RefPicList0_q0 == RefPicList1_p0 && RefPicList1_q0 == RefPicList0_p0))//two motion vectors for the same two reference pictures for q0
          )
        {
            if (RefPicList0_q0 == RefPicList0_p0
               && ((ABS(MvL0_p0_x - MvL0_q0_x) >= 4 || ABS(MvL0_p0_y - MvL0_q0_y) >= mv_y_diff)
                    || (ABS(MvL1_p0_x - MvL1_q0_x) >= 4 || ABS(MvL1_p0_y - MvL1_q0_y) >= mv_y_diff))
              )
            {
                bS = 1;
                return 0;
            }
            else if (RefPicList0_q0 == RefPicList1_p0
               && ((ABS(MvL1_p0_x - MvL0_q0_x) >= 4 || ABS(MvL1_p0_y - MvL0_q0_y) >= mv_y_diff)
                    || (ABS(MvL0_p0_x - MvL1_q0_x) >= 4 || ABS(MvL0_p0_y - MvL1_q0_y) >= mv_y_diff))
              )
            {
                bS = 1;
                return 0;
            }
        }

        //mixedModeEdgeFlag is equal to 0 and two motion vectors for the same reference picture are used to predict the macroblock/sub-macroblock partition 
        //containing the sample p0 and two motion vectors for the same reference picture are used to predict the macroblock/sub-macroblock partition containing 
        //the sample q0 and both of the following conditions are true:
        //p0的两个运动矢量都来自同一张参考图片，q0的两个运动矢量也都来自同一张参考图片，并且q0的这张参考图片和p0的那张参考图片是同一张参考图片
        if ( (PredFlagL0_p0 == 1 && PredFlagL1_p0 == 1) && (RefPicList0_p0 == RefPicList1_p0) //two motion vectors for the same reference picture for p0
            && (PredFlagL0_q0 == 1 && PredFlagL1_q0 == 1) && (RefPicList0_q0 == RefPicList1_q0) //two motion vectors for the same reference picture for q0
            && RefPicList0_q0 == RefPicList0_p0 //q0的这张参考图片和p0的那张参考图片是同一张参考图片
          )
        {
            //The absolute difference between the horizontal or vertical components of list 0 motion vectors used in the prediction of the two macroblock/sub-macroblock 
            //partitions is greater than or equal to 4 in quarter luma frame samples or the absolute difference between the horizontal or vertical components of the 
            //list 1 motion vectors used in the prediction of the two macroblock/sub-macroblock partitions is greater than or equal to 4 in units of quarter luma frame samples,
            if ( (ABS(MvL0_p0_x - MvL0_q0_x) >= 4 || ABS(MvL0_p0_y - MvL0_q0_y) >= mv_y_diff)
                 || (ABS(MvL1_p0_x - MvL1_q0_x) >= 4 || ABS(MvL1_p0_y - MvL1_q0_y) >= mv_y_diff)
            
            //The absolute difference between the horizontal or vertical components of list 0 motion vector used in the prediction of the macroblock/sub-macroblock 
            //partition containing the sample p0 and the list 1 motion vector used in the prediction of the macroblock/sub-macroblock partition containing the 
            //sample q0 is greater than or equal to 4 in units of quarter luma frame samples or the absolute difference between the horizontal or vertical components 
            //of the list 1 motion vector used in the prediction of the macroblock/sub-macroblock partition containing the sample p0 and list 0 motion vector used in 
            //the prediction of the macroblock/sub-macroblock partition containing the sample q0 is greater than or equal to 4 in units of quarter luma frame samples.
                 && (ABS(MvL0_p0_x - MvL1_q0_x) >= 4 || ABS(MvL0_p0_y - MvL1_q0_y) >= mv_y_diff)
                     || (ABS(MvL1_p0_x - MvL0_q0_x) >= 4 || ABS(MvL1_p0_y - MvL0_q0_y) >= mv_y_diff)
              )
            {
                bS = 1;
                return 0;
            }
        }
    }
    
    bS = 0;

    return ret;
}


//8.7.2.2 Derivation process for the thresholds for each block edge
int CH264PictureBase::Derivation_process_for_the_thresholds_for_each_block_edge(int32_t p0, int32_t q0, int32_t p1, int32_t q1,
     int32_t chromaEdgeFlag, int32_t bS, int32_t filterOffsetA, int32_t filterOffsetB, int32_t qPp, int32_t qPq,
     int32_t &filterSamplesFlag, int32_t &indexA, int32_t &alpha, int32_t &beta)
{
    int ret = 0;
    
    CH264SliceHeader & slice_header = m_h264_slice_header;

    int32_t qPav = ( qPp + qPq + 1 ) >> 1;

    indexA = CLIP3( 0, 51, qPav + filterOffsetA );
    int32_t indexB = CLIP3( 0, 51, qPav + filterOffsetB );

    //-----------------
    //Table 8-16 – Derivation of offset dependent threshold variables α´ and β´ from indexA and indexB
    //indexA (for α′) or indexB (for β′)
    int32_t alpha2[52] =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 4, 4, 5, 6, 
        7, 8, 9, 10, 12, 13, 15, 17, 20, 22, 
        25, 28, 32, 36, 40, 45, 50, 56, 63, 71, 
        80, 90, 101, 113, 127, 144, 162, 182, 203, 226, 
        255, 255
    };

    int32_t beta2[52] =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 2, 2, 2, 3, 
        3, 3, 3, 4, 4, 4, 6, 6, 7, 7, 
        8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 
        13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 
        18, 18
    };

    //----------------------
    if (chromaEdgeFlag == 0)
    {
        //α = α′ * (1 << ( BitDepthY − 8 ) )
        //β = β′ * (1 << ( BitDepthY − 8 ) )
        alpha = alpha2[indexA] * (1 << ( slice_header.m_sps.BitDepthY - 8 ));
        beta = beta2[indexB] * (1 << ( slice_header.m_sps.BitDepthY - 8 ));
    }
    else //if (chromaEdgeFlag == 1)
    {
        //α = α′ * (1 << ( BitDepthC − 8 ) )
        //β = β′ * (1 << ( BitDepthC − 8 ) )
        alpha = alpha2[indexA] * (1 << ( slice_header.m_sps.BitDepthC - 8 ));
        beta = beta2[indexB] * (1 << ( slice_header.m_sps.BitDepthC - 8 ));
    }

    filterSamplesFlag = ( bS != 0 && ABS( p0 - q0 ) < alpha && ABS( p1 - p0 ) < beta && ABS( q1 - q0 ) < beta );

    return ret;
}


//8.7.2.3 Filtering process for edges with bS less than 4
int CH264PictureBase::Filtering_process_for_edges_with_bS_less_than_4(const uint8_t (&p)[4], const uint8_t (&q)[4], int32_t chromaEdgeFlag, int32_t chromaStyleFilteringFlag, 
        int32_t bS, int32_t beta, int32_t indexA, uint8_t (&pp)[3], uint8_t (&qq)[3])
{
    int ret = 0;
    
    CH264SliceHeader & slice_header = m_h264_slice_header;

    //-----------------
    //Table 8-17 – Value of variable t´C0 as a function of indexA and bS
    //indexA
    int32_t ttC0[3][52] =
    {
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 
            1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 
            4, 4, 4, 5, 6, 6, 7, 8, 9, 10, 
            11, 13
        },
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
            0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
            1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 
            5, 5, 6, 7, 8, 8, 10, 11, 12, 13, 
            15, 17
        },
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 
            1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 
            2, 3, 3, 3, 4, 4, 4, 5, 6, 6, 
            7, 8, 9, 10, 11, 13, 14, 16, 18, 20, 
            23, 25
        }
    };
    
    //----------------------
    int32_t tC0 = 0;

    if (chromaEdgeFlag == 0)
    {
        //tC0 = t′C0 * (1 << ( BitDepthY − 8 ) )
        tC0 = ttC0[bS - 1][indexA] * (1 << ( slice_header.m_sps.BitDepthY - 8 ) );
    }
    else //if (chromaEdgeFlag == 1)
    {
        //tC0 = t′C0 * (1 << ( BitDepthC − 8 ) )
        tC0 = ttC0[bS - 1][indexA] * (1 << ( slice_header.m_sps.BitDepthC - 8 ) );
    }

    int32_t ap = ABS( p[2] - p[0] );
    int32_t aq = ABS( q[2] - q[0] );
    
    int32_t tC = 0;

    if (chromaStyleFilteringFlag == 0)
    {
        tC = tC0 + ( ( ap < beta ) ? 1 : 0 ) + ( ( aq < beta ) ? 1 : 0 );
    }
    else //if (chromaStyleFilteringFlag == 1)
    {
        tC = tC0 + 1;
    }

    //--------------------
    int32_t delta = CLIP3( -tC, tC, ( ( ( ( q[0] - p[0] ) << 2 ) + ( p[1] - q[1] ) + 4 ) >> 3 ) );

    if (chromaEdgeFlag == 0)
    {
        pp[0] = CLIP3(0, ( 1 << slice_header.m_sps.BitDepthY ) - 1, p[0] +  delta);
        qq[0] = CLIP3(0, ( 1 << slice_header.m_sps.BitDepthY ) - 1, q[0] -  delta);
    }
    else //if (chromaEdgeFlag == 1)
    {
        pp[0] = CLIP3(0, ( 1 << slice_header.m_sps.BitDepthC ) - 1, p[0] +  delta);
        qq[0] = CLIP3(0, ( 1 << slice_header.m_sps.BitDepthC ) - 1, q[0] -  delta);
    }

    //-------------------
    if (chromaStyleFilteringFlag == 0 && ap < beta)
    {
        pp[1] = p[1] + CLIP3( -tC0, tC0, ( p[2] + ( ( p[0] + q[0] + 1 ) >> 1 ) - ( p[1] << 1 ) ) >> 1 );
    }
    else //if (chromaEdgeFlag == 1)
    {
        pp[1] = p[1];
    }
    
    if (chromaStyleFilteringFlag == 0 && aq < beta)
    {
        qq[1] = q[1] + CLIP3( -tC0, tC0, ( q[2] + ( ( p[0] + q[0] + 1 ) >> 1 ) - ( q[1] << 1 ) ) >> 1 );
    }
    else //if (chromaEdgeFlag == 1)
    {
        qq[1] = q[1];
    }

    //-------------------
    pp[2] = p[2];
    qq[2] = q[2];

    return ret;
}


//8.7.2.4 Filtering process for edges for bS equal to 4
int CH264PictureBase::Filtering_process_for_edges_for_bS_equal_to_4(const uint8_t (&p)[4], const uint8_t (&q)[4], int32_t chromaEdgeFlag, int32_t chromaStyleFilteringFlag, 
        int32_t alpha, int32_t beta, uint8_t (&pp)[3], uint8_t (&qq)[3])
{
    int ret = 0;
    
    int32_t ap = ABS( p[2] - p[0] );
    int32_t aq = ABS( q[2] - q[0] );
    
    //-----------------------------------
    if (chromaStyleFilteringFlag == 0
        && ( ap < beta && ABS( p[0] - q[0] ) < ( ( alpha >> 2 ) + 2 ) )
        )
    {
        pp[0] = ( p[2] + 2 * p[1] + 2 * p[0] + 2 * q[0] + q[1] + 4 ) >> 3; //抽头滤波器
        pp[1] = ( p[2] + p[1] + p[0] + q[0] + 2 ) >> 2;
        pp[2] = ( 2 * p[3] + 3 * p[2] + p[1] + p[0] + q[0] + 4 ) >> 3;
    }
    else
    {
        pp[0] = ( 2 * p[1] + p[0] + q[1] + 2 ) >> 2;
        pp[1] = p[1];
        pp[2] = p[2];
    }
    
    //-----------------------------------
    if (chromaStyleFilteringFlag == 0
        && ( aq < beta && ABS( p[0] - q[0] ) < ( ( alpha >> 2 ) + 2 ) )
        )
    {
        qq[0] = ( p[1] + 2 * p[0] + 2 * q[0] + 2 * q[1] + q[2] + 4 ) >> 3; //抽头滤波器
        qq[1] = ( p[0] + q[0] + q[1] + q[2] + 2 ) >> 2;
        qq[2] = ( 2 * q[3] + 3 * q[2] + q[1] + q[0] + p[0] + 4 ) >> 3;
    }
    else
    {
        qq[0] = ( 2 * q[1] + q[0] + p[1] + 2 ) >> 2;
        qq[1] = q[1];
        qq[2] = q[2];
    }
    
    return ret;
}

