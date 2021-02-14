//
// H264SliceData.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264SliceData.h"
#include "H264Golomb.h"
#include "Bitstream.h"
#include "CommonFunction.h"
#include "H264CommonFunc.h"
#include "H264MacroBlock.h"
#include "H264Picture.h"
#include "H264Cabac.h"


CH264SliceData::CH264SliceData()
{
    int ret = init();
}


CH264SliceData::~CH264SliceData()
{

}


int CH264SliceData::printInfo()
{
    printf("---------Slice Data info------------\n");
    printf("cabac_alignment_one_bit=%d;\n", cabac_alignment_one_bit);
    printf("mb_skip_run=%d;\n", mb_skip_run);
    printf("mb_skip_flag=%d;\n", mb_skip_flag);
    printf("end_of_slice_flag=%d;\n", end_of_slice_flag);
    printf("mb_field_decoding_flag=%d;\n", mb_field_decoding_flag);

    return 0;
}


int CH264SliceData::init()
{
    cabac_alignment_one_bit = 0;
    mb_skip_run = 0;
    mb_skip_flag = 0;
    end_of_slice_flag = 0;
    mb_field_decoding_flag = 0;
    slice_id = 0;
    slice_number = -1;
    CurrMbAddr = 0;
    syntax_element_categories = 0;

    return 0;
}


/*
 * Page 55/77/812
 * 7.3.4 Slice data syntax
 */
int CH264SliceData::slice_data(CBitstream &bs, CH264PictureBase &picture, int32_t _slice_id)
{
    int ret = 0;
    int32_t i = 0;
    CH264Golomb gb;
    CH264Cabac cabac;

    CH264SliceHeader & slice_header = picture.m_h264_slice_header;

    slice_id = _slice_id;
    slice_number++;

    printf("slice_id=%d; slice_number=%d; syntax_element_categories=%d;\n", slice_id, slice_number, slice_header.syntax_element_categories);

    int is_ae = slice_header.m_pps.entropy_coding_mode_flag; //ae(v)表示CABAC编码

    if (is_ae)
    {
        while(!byte_aligned(bs))
        {
            cabac_alignment_one_bit = bs.readBits(1); //2 f(1)
        }

        ret = cabac.Initialisation_process_for_context_variables((H264_SLIECE_TYPE)slice_header.slice_type, slice_header.cabac_init_idc, slice_header.SliceQPY); //cabac初始化环境变量
        RETURN_IF_FAILED(ret != 0, -1);
        
        ret = cabac.Initialisation_process_for_the_arithmetic_decoding_engine(bs); //cabac初始化解码引擎
        RETURN_IF_FAILED(ret != 0, -1);
    }

    //When MbaffFrameFlag is equal to 0 (mb_field_decoding_flag is not present), 
    //mb_field_decoding_flag is inferred to be equal to field_pic_flag.
    if (slice_header.MbaffFrameFlag == 0)
    {
        mb_field_decoding_flag = slice_header.field_pic_flag;
    }

    CurrMbAddr = slice_header.first_mb_in_slice * (1 + slice_header.MbaffFrameFlag);
    int32_t moreDataFlag = 1;
    int32_t prevMbSkipped = 0;
    int32_t mb_skip_flag_next_mb = 0;
    picture.CurrMbAddr = CurrMbAddr;

    //picNumLXPred is the prediction value for the variable picNumLXNoWrap. When the process specified 
    //in this clause is invoked the first time for a slice (that is, for the first occurrence of 
    //modification_of_pic_nums_idc equal to 0 or 1 in the ref_pic_list_modification( ) syntax), 
    //picNumL0Pred and picNumL1Pred are initially set equal to CurrPicNum.

    slice_header.picNumL0Pred = slice_header.CurrPicNum;
    slice_header.picNumL1Pred = slice_header.CurrPicNum;

    //8.2.1 (only needed to be invoked for one slice of a picture)
    if (picture.m_slice_cnt == 0)
    {
        ret = picture.Decoding_process_for_picture_order_count(); //解码POC
        RETURN_IF_FAILED(ret != 0, ret);

        if (slice_header.m_sps.frame_mbs_only_flag == 0) //有可能出现场帧或场宏块
        {
            picture.m_parent->m_picture_top_filed.copyDataPicOrderCnt(picture); //顶（底）场帧有可能被选为参考帧，在解码P/B帧时，会用到PicOrderCnt字段，所以需要在此处复制一份
            picture.m_parent->m_picture_bottom_filed.copyDataPicOrderCnt(picture);
        }

        //--------参考帧重排序------------
        //只有当前帧为P帧，B帧时，才会对参考图像数列表组进行重排序
        if (slice_header.slice_type == H264_SLIECE_TYPE_P
            || slice_header.slice_type == H264_SLIECE_TYPE_SP
            || slice_header.slice_type == H264_SLIECE_TYPE_B
            )
        {
            //8.2.4 Decoding process for reference picture lists construction
            //This process is invoked at the beginning of the decoding process for each P, SP, or B slice.
            ret = picture.Decoding_process_for_reference_picture_lists_construction(picture.m_dpb, picture.m_RefPicList0, picture.m_RefPicList1);
            RETURN_IF_FAILED(ret != 0, ret);

            //--------------
            for (int i = 0; i < picture.m_RefPicList0Length; ++i)
            {
                printf("m_PicNumCnt=%d(%s); PicOrderCnt=%d; m_RefPicList0[%d]: %s; PicOrderCnt=%d; PicNum=%d; PicNumCnt=%d;\n",
                    picture.m_PicNumCnt, H264_SLIECE_TYPE_TO_STR(slice_header.slice_type), picture.PicOrderCnt, i,
                    (picture.m_RefPicList0[i]) ? H264_SLIECE_TYPE_TO_STR(picture.m_RefPicList0[i]->m_picture_frame.m_h264_slice_header.slice_type) : "UNKNOWN",
                    (picture.m_RefPicList0[i]) ? picture.m_RefPicList0[i]->m_picture_frame.PicOrderCnt : -1,
                    (picture.m_RefPicList0[i]) ? picture.m_RefPicList0[i]->m_picture_frame.PicNum : -1,
                    (picture.m_RefPicList0[i]) ? picture.m_RefPicList0[i]->m_picture_frame.m_PicNumCnt : -1);
            }
            for (int i = 0; i < picture.m_RefPicList1Length; ++i)
            {
                printf("m_PicNumCnt=%d(%s); PicOrderCnt=%d; m_RefPicList1[%d]: %s; PicOrderCnt=%d; PicNum=%d; PicNumCnt=%d;\n",
                    picture.m_PicNumCnt, H264_SLIECE_TYPE_TO_STR(slice_header.slice_type), picture.PicOrderCnt, i,
                    (picture.m_RefPicList1[i]) ? H264_SLIECE_TYPE_TO_STR(picture.m_RefPicList1[i]->m_picture_frame.m_h264_slice_header.slice_type) : "UNKNOWN",
                    (picture.m_RefPicList1[i]) ? picture.m_RefPicList1[i]->m_picture_frame.PicOrderCnt : -1,
                    (picture.m_RefPicList1[i]) ? picture.m_RefPicList1[i]->m_picture_frame.PicNum : -1,
                    (picture.m_RefPicList1[i]) ? picture.m_RefPicList1[i]->m_picture_frame.m_PicNumCnt : -1);
            }
        }
    }

    picture.m_slice_cnt++;

    //-------------------------------
    bool is_need_skip_read_mb_field_decoding_flag = false;

    do
    {
        if (slice_header.slice_type != H264_SLIECE_TYPE_I && slice_header.slice_type != H264_SLIECE_TYPE_SI)
        {
            if (!slice_header.m_pps.entropy_coding_mode_flag)
            {
                mb_skip_run = gb.get_ue_golomb(bs); //2 ue(v)
                
                prevMbSkipped = (mb_skip_run > 0);
                for (i = 0; i < mb_skip_run; i++)
                {
                    picture.mb_x =  (CurrMbAddr % (picture.PicWidthInMbs * (1 + slice_header.MbaffFrameFlag))) / (1 + slice_header.MbaffFrameFlag);
                    picture.mb_y =  (CurrMbAddr / (picture.PicWidthInMbs * (1 + slice_header.MbaffFrameFlag)) * (1 + slice_header.MbaffFrameFlag)) 
                                     + ((CurrMbAddr % (picture.PicWidthInMbs * (1 + slice_header.MbaffFrameFlag))) % (1 + slice_header.MbaffFrameFlag));
                    picture.CurrMbAddr = CurrMbAddr;
                    
                    picture.mb_cnt++;
                    
                    if (slice_header.MbaffFrameFlag)
                    {
                        if (CurrMbAddr % 2 == 0) //只需要处理top field macroblock
                        {
                            if (i == mb_skip_run - 1)
                            {
                                if (is_ae) //ae(v)表示CABAC编码
                                {
                                    //mb_field_decoding_flag; //2 u(1) | ae(v)
                                }
                                else //ue(v) 表示CAVLC编码
                                {
                                    mb_field_decoding_flag = bs.readBits(1); //2 u(1) | ae(v)
                                }

                                is_need_skip_read_mb_field_decoding_flag = true;
                            }
                            else
                            {
                                //When MbaffFrameFlag is equal to 1 and mb_field_decoding_flag is not present 
                                //for both the top and the bottom macroblock of a macroblock pair
                                if (picture.mb_x > 0 && picture.m_mbs[CurrMbAddr - 2].slice_number == slice_number) //the left of the current macroblock pair in the same slice
                                {
                                    mb_field_decoding_flag = picture.m_mbs[CurrMbAddr - 2].mb_field_decoding_flag;
                                }
                                else if (picture.mb_y > 0 && picture.m_mbs[CurrMbAddr - 2 * picture.PicWidthInMbs].slice_number == slice_number) //above the current macroblock pair in the same slice
                                {
                                    mb_field_decoding_flag = picture.m_mbs[CurrMbAddr - 2 * picture.PicWidthInMbs].mb_field_decoding_flag;
                                }
                                else
                                {
                                    mb_field_decoding_flag = 0; //is inferred to be equal to 0
                                }
                            }
                        }
                    }
                    
                    //-----------------------------------------------------------------
                    ret = picture.m_mbs[picture.CurrMbAddr].macroblock_layer_mb_skip(picture, *this, cabac); //2 | 3 | 4
                    RETURN_IF_FAILED(ret != 0, ret);

                    //The inter prediction process for P and B macroblocks is specified in clause 8.4 with inter prediction samples being the output.
                    ret = picture.Inter_prediction_process(); //帧间预测
                    RETURN_IF_FAILED(ret != 0, ret);

                    CurrMbAddr = NextMbAddress(slice_header, CurrMbAddr);
                    if (CurrMbAddr < 0)
                    {
                        LOG_ERROR("CurrMbAddr(%d) < 0\n", CurrMbAddr);
                        break;
                    }
                }

                if (mb_skip_run > 0)
                {
                    moreDataFlag = more_rbsp_data(bs);
                }
            }
            else //ae(v)表示CABAC编码
            {
                picture.mb_x = (CurrMbAddr % (picture.PicWidthInMbs * (1 + slice_header.MbaffFrameFlag))) / (1 + slice_header.MbaffFrameFlag);
                picture.mb_y = (CurrMbAddr / (picture.PicWidthInMbs * (1 + slice_header.MbaffFrameFlag)) * (1 + slice_header.MbaffFrameFlag)) 
                                + ((CurrMbAddr % (picture.PicWidthInMbs * (1 + slice_header.MbaffFrameFlag))) % (1 + slice_header.MbaffFrameFlag));
                picture.CurrMbAddr = CurrMbAddr;

                //picture.m_mbs[picture.CurrMbAddr].MbaffFrameFlag = slice_header.MbaffFrameFlag; //因为解码mb_skip_flag需要事先知道MbaffFrameFlag的值
                picture.m_mbs[picture.CurrMbAddr].slice_number = slice_number; //因为解码mb_skip_flag需要事先知道slice_id的值
                
                if (slice_header.MbaffFrameFlag)
                {
                    if (CurrMbAddr % 2 == 0) //顶场宏块
                    {
                        if (picture.mb_x == 0 && picture.mb_y >= 2) //注意：此处在T-REC-H.264-201704-S!!PDF-E.pdf文档中，并没有明确写出来，所以这是一个坑
                        {
                            //When MbaffFrameFlag is equal to 1 and mb_field_decoding_flag is not present 
                            //for both the top and the bottom macroblock of a macroblock pair
                            if (picture.mb_x > 0 && picture.m_mbs[CurrMbAddr - 2].slice_number == slice_number) //the left of the current macroblock pair in the same slice
                            {
                                mb_field_decoding_flag = picture.m_mbs[CurrMbAddr - 2].mb_field_decoding_flag;
                            }
                            else if (picture.mb_y > 0 && picture.m_mbs[CurrMbAddr - 2 * picture.PicWidthInMbs].slice_number == slice_number) //above the current macroblock pair in the same slice
                            {
                                mb_field_decoding_flag = picture.m_mbs[CurrMbAddr - 2 * picture.PicWidthInMbs].mb_field_decoding_flag;
                            }
                            else
                            {
                                mb_field_decoding_flag = 0; //is inferred to be equal to 0
                            }
                        }
                    }

                    picture.m_mbs[picture.CurrMbAddr].mb_field_decoding_flag = mb_field_decoding_flag; //因为解码mb_skip_flag需要事先知道mb_field_decoding_flag的值
                }
                
                //-------------解码mb_skip_flag-----------------------
                if (slice_header.MbaffFrameFlag && CurrMbAddr % 2 == 1 && prevMbSkipped) //如果是bottom field macroblock
                {
                    mb_skip_flag = mb_skip_flag_next_mb;
                }
                else
                {
                    ret = cabac.CABAC_decode_mb_skip_flag(picture, bs, CurrMbAddr, mb_skip_flag);  //2 ae(v)
                    RETURN_IF_FAILED(ret != 0, ret);
                }

                //------------------------------------
                if (mb_skip_flag == 1) //表示本宏块没有残差数据，相应的像素值只需要利用之前已经解码的I/P帧来预测获得
                {
                    picture.mb_cnt++;
                    
                    if (slice_header.MbaffFrameFlag)
                    {
                        if (CurrMbAddr % 2 == 0) //只需要处理top field macroblock
                        {
                            picture.m_mbs[picture.CurrMbAddr].mb_skip_flag = mb_skip_flag; //因为解码mb_skip_flag_next_mb需要事先知道前面顶场宏块的mb_skip_flag值
                            picture.m_mbs[picture.CurrMbAddr + 1].slice_number = slice_number; //因为解码mb_skip_flag需要事先知道slice_id的值
                            picture.m_mbs[picture.CurrMbAddr + 1].mb_field_decoding_flag = mb_field_decoding_flag; //特别注意：底场宏块和顶场宏块的mb_field_decoding_flag值是相同的
                            
                            ret = cabac.CABAC_decode_mb_skip_flag(picture, bs, CurrMbAddr + 1, mb_skip_flag_next_mb);  //2 ae(v) 先读取底场宏块的mb_skip_flag
                            RETURN_IF_FAILED(ret != 0, ret);

                            if (mb_skip_flag_next_mb == 0) //如果底场宏块mb_skip_flag=0
                            {
                                ret = cabac.CABAC_decode_mb_field_decoding_flag(picture, bs, mb_field_decoding_flag); //2 u(1) | ae(v) 再读取底场宏块的mb_field_decoding_flag
                                RETURN_IF_FAILED(ret != 0, ret);

                                is_need_skip_read_mb_field_decoding_flag = true;
                            }
                            else //if (mb_skip_flag_next_mb == 1)
                            {
                                //When MbaffFrameFlag is equal to 1 and mb_field_decoding_flag is not present 
                                //for both the top and the bottom macroblock of a macroblock pair
                                if (picture.mb_x > 0 && picture.m_mbs[CurrMbAddr - 2].slice_number == slice_number) //the left of the current macroblock pair in the same slice
                                {
                                    mb_field_decoding_flag = picture.m_mbs[CurrMbAddr - 2].mb_field_decoding_flag;
                                }
                                else if (picture.mb_y > 0 && picture.m_mbs[CurrMbAddr - 2 * picture.PicWidthInMbs].slice_number == slice_number) //above the current macroblock pair in the same slice
                                {
                                    mb_field_decoding_flag = picture.m_mbs[CurrMbAddr - 2 * picture.PicWidthInMbs].mb_field_decoding_flag;
                                }
                                else
                                {
                                    mb_field_decoding_flag = 0; //is inferred to be equal to 0
                                }
                            }
                        }
                    }
                    
                    //-----------------------------------------------------------------
                    ret = picture.m_mbs[picture.CurrMbAddr].macroblock_layer_mb_skip(picture, *this, cabac); //2 | 3 | 4
                    RETURN_IF_FAILED(ret != 0, ret);

                    //The inter prediction process for P and B macroblocks is specified in clause 8.4 with inter prediction samples being the output.
                    ret = picture.Inter_prediction_process(); //帧间预测
                    RETURN_IF_FAILED(ret != 0, ret);
                }

                moreDataFlag = !mb_skip_flag;
            }
        }

        //----------------------
        if (moreDataFlag)
        {
            //NOTE – When MbaffFrameFlag is equal to 1 and mb_field_decoding_flag is not present for the top macroblock of a macroblock pair 
            //(because the top macroblock is skipped), a decoder must wait until mb_field_decoding_flag for the bottom macroblock is read 
            //(when the bottom macroblock is not skipped) or the value of mb_field_decoding_flag is inferred as specified above (when the bottom 
            //macroblock is also skipped) before it starts the decoding process for the top macroblock.
            if (slice_header.MbaffFrameFlag && (CurrMbAddr % 2 == 0 || (CurrMbAddr % 2 == 1 && prevMbSkipped))) //表示本宏块是属于一个宏块对中的一个
            {
                if (is_need_skip_read_mb_field_decoding_flag == false)
                {
                    if (is_ae) //ae(v)表示CABAC编码
                    {
                        ret = cabac.CABAC_decode_mb_field_decoding_flag(picture, bs, mb_field_decoding_flag); //2 u(1) | ae(v) 表示本宏块对是帧宏块对，还是场宏块对
                        RETURN_IF_FAILED(ret != 0, ret);
                    }
                    else //ue(v) 表示CAVLC编码
                    {
                        mb_field_decoding_flag = bs.readBits(1); //2 u(1) | ae(v) 表示本宏块对是帧宏块对，还是场宏块对
                    }
                }
                else
                {
                    is_need_skip_read_mb_field_decoding_flag = false;
                }
            }
            
            //----------------------------
            picture.mb_x =  (CurrMbAddr % (picture.PicWidthInMbs * (1 + slice_header.MbaffFrameFlag))) / (1 + slice_header.MbaffFrameFlag);
            picture.mb_y =  (CurrMbAddr / (picture.PicWidthInMbs * (1 + slice_header.MbaffFrameFlag)) * (1 + slice_header.MbaffFrameFlag)) + ((CurrMbAddr % (picture.PicWidthInMbs * (1 + slice_header.MbaffFrameFlag))) % (1 + slice_header.MbaffFrameFlag));
            picture.CurrMbAddr = CurrMbAddr; //picture.mb_x + picture.mb_y * picture.PicWidthInMbs;

            picture.mb_cnt++;
            
            //--------熵解码------------
            ret = picture.m_mbs[picture.CurrMbAddr].macroblock_layer(bs, picture, *this, cabac); //2 | 3 | 4
            if (ret != 0)
            {
                LOG_ERROR("picture.m_mbs[%d].macroblock_layer(...) failed!\n", picture.CurrMbAddr);
            }
            
            //--------帧内/间预测------------
            //--------反量化------------
            //--------反变换------------
            int32_t isChroma = 0;
            int32_t isChromaCb = 0;
            int32_t BitDepth = 0;
            
            int32_t picWidthInSamplesL = picture.PicWidthInSamplesL;
            int32_t picWidthInSamplesC = picture.PicWidthInSamplesC;

            uint8_t * pic_buff_luma = picture.m_pic_buff_luma;
            uint8_t * pic_buff_cb = picture.m_pic_buff_cb;
            uint8_t * pic_buff_cr = picture.m_pic_buff_cr;

            if (picture.m_mbs[picture.CurrMbAddr].m_mb_pred_mode == Intra_4x4) //帧内预测
            {
                isChroma = 0;
                isChromaCb = 0;
                BitDepth = picture.m_h264_slice_header.m_sps.BitDepthY;
                
                ret = picture.transform_decoding_process_for_4x4_luma_residual_blocks(isChroma, isChromaCb, BitDepth, picWidthInSamplesL, pic_buff_luma);
                RETURN_IF_FAILED(ret != 0, ret);
                
                isChromaCb = 1;
                ret = picture.transform_decoding_process_for_chroma_samples(isChromaCb, picWidthInSamplesC, pic_buff_cb);
                RETURN_IF_FAILED(ret != 0, ret);
                
                isChromaCb = 0;
                ret = picture.transform_decoding_process_for_chroma_samples(isChromaCb, picWidthInSamplesC, pic_buff_cr);
                RETURN_IF_FAILED(ret != 0, ret);
            }
            else if (picture.m_mbs[picture.CurrMbAddr].m_mb_pred_mode == Intra_8x8) //帧内预测
            {
                isChroma = 0;
                isChromaCb = 0;
                BitDepth = picture.m_h264_slice_header.m_sps.BitDepthY;
                
                ret = picture.transform_decoding_process_for_8x8_luma_residual_blocks(isChroma, isChromaCb, BitDepth, picWidthInSamplesL, 
                            picture.m_mbs[picture.CurrMbAddr].LumaLevel8x8, pic_buff_luma);
                RETURN_IF_FAILED(ret != 0, ret);
                
                isChromaCb = 1;
                ret = picture.transform_decoding_process_for_chroma_samples(isChromaCb, picWidthInSamplesC, pic_buff_cb);
                RETURN_IF_FAILED(ret != 0, ret);
                
                isChromaCb = 0;
                ret = picture.transform_decoding_process_for_chroma_samples(isChromaCb, picWidthInSamplesC, pic_buff_cr);
                RETURN_IF_FAILED(ret != 0, ret);
            }
            else if (picture.m_mbs[picture.CurrMbAddr].m_mb_pred_mode == Intra_16x16) //帧内预测
            {
                isChroma = 0;
                isChromaCb = 0;
                BitDepth = picture.m_h264_slice_header.m_sps.BitDepthY;
                int32_t QP1 = picture.m_mbs[picture.CurrMbAddr].QP1Y;
                
                ret = picture.transform_decoding_process_for_luma_samples_of_Intra_16x16_macroblock_prediction_mode(isChroma, BitDepth, QP1, 
                            picWidthInSamplesL, picture.m_mbs[picture.CurrMbAddr].Intra16x16DCLevel, picture.m_mbs[picture.CurrMbAddr].Intra16x16ACLevel, pic_buff_luma);
                RETURN_IF_FAILED(ret != 0, ret);
                
                isChromaCb = 1;
                ret = picture.transform_decoding_process_for_chroma_samples(isChromaCb, picWidthInSamplesC, pic_buff_cb);
                RETURN_IF_FAILED(ret != 0, ret);
                
                isChromaCb = 0;
                ret = picture.transform_decoding_process_for_chroma_samples(isChromaCb, picWidthInSamplesC, pic_buff_cr);
                RETURN_IF_FAILED(ret != 0, ret);
            }
            else if (picture.m_mbs[picture.CurrMbAddr].m_name_of_mb_type == I_PCM) //说明该宏块没有残差，也没有预测值，码流中的数据直接为原始像素值
            {
                ret = picture.Sample_construction_process_for_I_PCM_macroblocks();
                RETURN_IF_FAILED(ret != 0, ret);
            }
            else //if (IS_INTER_Prediction_Mode(picture.m_mbs[picture.CurrMbAddr].m_mb_pred_mode)) //帧间预测
            {
                //The inter prediction process for P and B macroblocks is specified in clause 8.4 with inter prediction samples being the output.
                ret = picture.Inter_prediction_process(); //帧间预测
                RETURN_IF_FAILED(ret != 0, ret);
                
                BitDepth = picture.m_h264_slice_header.m_sps.BitDepthY;

                //-------残差-----------
                if (picture.m_mbs[picture.CurrMbAddr].transform_size_8x8_flag == 0)
                {
                    ret = picture.transform_decoding_process_for_4x4_luma_residual_blocks_inter(isChroma, isChromaCb, BitDepth, picWidthInSamplesL, pic_buff_luma);
                    RETURN_IF_FAILED(ret != 0, ret);
                }
                else //if (picture.m_mbs[picture.CurrMbAddr].transform_size_8x8_flag == 1)
                {
                    ret = picture.transform_decoding_process_for_8x8_luma_residual_blocks_inter(isChroma, isChromaCb, BitDepth, picWidthInSamplesL, 
                            picture.m_mbs[picture.CurrMbAddr].LumaLevel8x8, pic_buff_luma);
                    RETURN_IF_FAILED(ret != 0, ret);
                }
                
                isChromaCb = 1;
                ret = picture.transform_decoding_process_for_chroma_samples_inter(isChromaCb, picWidthInSamplesC, pic_buff_cb);
                RETURN_IF_FAILED(ret != 0, ret);
                
                isChromaCb = 0;
                ret = picture.transform_decoding_process_for_chroma_samples_inter(isChromaCb, picWidthInSamplesC, pic_buff_cr);
                RETURN_IF_FAILED(ret != 0, ret);
            }
        }

        if (!slice_header.m_pps.entropy_coding_mode_flag)
        {
            moreDataFlag = more_rbsp_data(bs);
        }
        else
        {
            if (slice_header.slice_type != H264_SLIECE_TYPE_I && slice_header.slice_type != H264_SLIECE_TYPE_SI)
            {
                prevMbSkipped = mb_skip_flag;
            }

            if (slice_header.MbaffFrameFlag && CurrMbAddr % 2 == 0)
            {
                moreDataFlag = 1;
            }
            else
            {
                ret = cabac.CABAC_decode_end_of_slice_flag(picture, bs, end_of_slice_flag); //2 ae(v)
                RETURN_IF_FAILED(ret != 0, ret);

                moreDataFlag = !end_of_slice_flag;
            }
        }

        CurrMbAddr = NextMbAddress(slice_header, CurrMbAddr);

        if (CurrMbAddr < 0)
        {
            LOG_ERROR("CurrMbAddr(%d) < 0\n", CurrMbAddr);
            break;
        }
    } while(moreDataFlag);

    //----------------------
    if (picture.mb_cnt == picture.PicSizeInMbs)
    {
        picture.m_is_decode_finished = 1;
    }
    else
    {
        LOG_ERROR("picture.mb_cnt(%d) != picture.PicSizeInMbs(%d)\n", picture.mb_cnt, picture.PicSizeInMbs);
    }

    return ret;
}


//8.2.2 Decoding process for macroblock to slice group map
int CH264SliceData::NextMbAddress(const CH264SliceHeader &slice_header, int32_t n)
{
    int32_t i = n + 1;

    if (slice_header.MbToSliceGroupMap == NULL)
    {
        LOG_ERROR("slice_header.MbToSliceGroupMap == NULL\n");
        return -1;
    }

    if (i >= slice_header.PicSizeInMbs)
    {
        LOG_ERROR("i(%d) >= slice_header.PicSizeInMbs(%d);\n", i, slice_header.PicSizeInMbs);
        return -2;
    }

    while( i < slice_header.PicSizeInMbs && slice_header.MbToSliceGroupMap[ i ] != slice_header.MbToSliceGroupMap[ n ] )
    {
        i++;
        if (i >= slice_header.PicSizeInMbs)
        {
            LOG_ERROR("i(%d) >= slice_header.PicSizeInMbs(%d);\n", i, slice_header.PicSizeInMbs);
            return -3;
        }
    }

    return i;
}
