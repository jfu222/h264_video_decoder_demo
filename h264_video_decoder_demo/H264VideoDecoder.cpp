//
// H264VideoDecoder.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264VideoDecoder.h"
#include "FileReader.h"
#include "Bitstream.h"
#include "CommonFunction.h"
#include "H264Golomb.h"


int32_t g_PicNumCnt = 0;


CH264VideoDecoder::CH264VideoDecoder()
{
    m_filename = "";
    m_output_frame_callback = NULL;
    m_userData = NULL;
}


CH264VideoDecoder::~CH264VideoDecoder()
{

}

int CH264VideoDecoder::init()
{


    return 0;
}


int CH264VideoDecoder::unInit()
{


    return 0;
}


int CH264VideoDecoder::set_output_frame_callback_functuin(output_frame_callback output_frame_callback, void *userData)
{
    m_output_frame_callback = output_frame_callback;
    m_userData = userData;
    return 0;
}


int CH264VideoDecoder::open(const char *url)
{
    int ret = 0;
    unsigned char * data = NULL;
    int size = 0;
    int nal_cnt = 0;
    g_PicNumCnt = 0;

    CFileReader fr;
    ret = fr.init(url);
    RETURN_IF_FAILED(ret != 0, ret);

    //---------------------
    int32_t sps_flag = 0;
    int32_t pps_flag = 0;
    CH264SliceHeader slice_header_last;
    CH264Picture * picture_current = NULL;
    CH264Picture * outPicture = NULL;
    int32_t is_need_flush = 0;
    int32_t isStopLoop = 0;

    CH264PicturesGOP * pictures_gop = new CH264PicturesGOP;
    RETURN_IF_FAILED(pictures_gop == NULL, -1);

    picture_current = pictures_gop->m_DecodedPictureBuffer[0]; //初始化第一个picture_current

    //---------------------
    while(1)
    {
        if (isStopLoop == 1)
        {
            break;
        }

        ret = fr.getNextH264NalUnitByStartCode(data, size);
        BREAK_IF_FAILED(ret != 0);
        
        CH264NalUnit nu;

        ret = nu.getH264RbspFromNalUnit(data, size);
        BREAK_IF_FAILED(ret != 0);
        
        CBitstream bs(nu.rbsp_byte, nu.NumBytesInRBSP);
        
        //--------------------------
        switch(nu.nal_unit_type)
        {
        case H264_NAL_SLICE: //1
            {
                //7.3.2.8 Slice layer without partitioning RBSP syntax
                CH264SliceHeader slice_header;
                ret = slice_header.slice_header(bs, nu, pictures_gop->m_spss, pictures_gop->m_ppss); // 2
                RETURN_IF_FAILED(ret != 0, ret);
                
                bool is_new_pic = slice_header.is_first_VCL_NAL_unit_of_a_picture(slice_header_last);
                if (is_new_pic == true && picture_current != NULL && picture_current->m_current_picture_ptr != NULL)
                {
                    CH264Picture * newEmptyPicture = NULL;
                    picture_current->m_current_picture_ptr->end_decode_the_picture_and_get_a_new_empty_picture(newEmptyPicture);
                    RETURN_IF_FAILED(ret != 0, ret);
                    
                    is_need_flush = picture_current->m_picture_frame.m_h264_slice_header.m_nal_unit.IdrPicFlag; //当前已解码完毕的帧是否是IDR帧
                    ret = do_callback(picture_current, pictures_gop, is_need_flush); //回调操作
                    if (ret != 0)
                    {
                        LOG_ERROR("do_callback() failed! ret=%d;\n", ret);
                        isStopLoop = 1;
                        break;
                    }

                    picture_current = newEmptyPicture;
                }
                
                RETURN_IF_FAILED(picture_current == NULL, -1);
                
                slice_header_last = slice_header;
                
                slice_header.syntax_element_categories = (0x01 | 0x02) | 0x04; //2 | 3 | 4;
                slice_header.slice_id = 0;
                ret = picture_current->decode_one_slice(slice_header, bs, pictures_gop->m_DecodedPictureBuffer); // /* all categories of slice_data() syntax */ 2 | 3 | 4
                CONTINUE_IF_FAILED(ret != 0);

                ret = rbsp_slice_trailing_bits(bs, slice_header.m_pps.entropy_coding_mode_flag); //2
                RETURN_IF_FAILED(ret != 0, ret);

                break;
            }
        case H264_NAL_DPA: //2
            {
                //7.3.2.9.1 Slice data partition A RBSP syntax
                CH264Golomb gb;
                CH264SliceHeader slice_header;

                ret = slice_header.slice_header(bs, nu, pictures_gop->m_spss, pictures_gop->m_ppss); // 2
                RETURN_IF_FAILED(ret != 0, ret);
                
                bool is_new_pic = slice_header.is_first_VCL_NAL_unit_of_a_picture(slice_header_last);
                if (is_new_pic == true && picture_current != NULL && picture_current->m_current_picture_ptr != NULL)
                {
                    CH264Picture * newEmptyPicture = NULL;
                    picture_current->m_current_picture_ptr->end_decode_the_picture_and_get_a_new_empty_picture(newEmptyPicture);
                    RETURN_IF_FAILED(ret != 0, ret);
                    
                    is_need_flush = picture_current->m_picture_frame.m_h264_slice_header.m_nal_unit.IdrPicFlag; //当前已解码完毕的帧是否是IDR帧
                    ret = do_callback(picture_current, pictures_gop, is_need_flush); //回调操作
                    if (ret != 0)
                    {
                        LOG_ERROR("do_callback() failed! ret=%d;\n", ret);
                        isStopLoop = 1;
                        break;
                    }

                    picture_current = newEmptyPicture;
                }
                
                RETURN_IF_FAILED(picture_current == NULL, -1);

                slice_header.syntax_element_categories = 0x01; //2;
                slice_header.slice_id = gb.get_ue_golomb(bs); //All ue(v)

                ret = picture_current->decode_one_slice(slice_header, bs, pictures_gop->m_DecodedPictureBuffer); // /* only category 2 parts of slice_data() syntax */ 2
                CONTINUE_IF_FAILED(ret != 0);

                ret = rbsp_slice_trailing_bits(bs, slice_header.m_pps.entropy_coding_mode_flag); //2
                RETURN_IF_FAILED(ret != 0, ret);

                break;
            }
        case H264_NAL_DPB: //3
            {
                //7.3.2.9.2 Slice data partition B RBSP syntax
                RETURN_IF_FAILED(picture_current->m_current_picture_ptr == NULL, -1);

                CH264SliceHeader & slice_header = picture_current->m_current_picture_ptr->m_h264_slice_header;
                CH264Golomb gb;

                slice_header.slice_id = gb.get_ue_golomb(bs); //All ue(v)

                if (slice_header.m_sps.separate_colour_plane_flag == 1)
                {
                    slice_header.colour_plane_id = bs.readBits(2); //All u(2)
                }

                if (slice_header.m_pps.redundant_pic_cnt_present_flag)
                {
                    slice_header.redundant_pic_cnt = gb.get_ue_golomb(bs); //All ue(v)
                }
                
                slice_header.syntax_element_categories = 0x02; //3;
                ret = picture_current->decode_one_slice(slice_header, bs, pictures_gop->m_DecodedPictureBuffer); // /* only category 3 parts of slice_data() syntax */ 3
                CONTINUE_IF_FAILED(ret != 0);

                ret = rbsp_slice_trailing_bits(bs, slice_header.m_pps.entropy_coding_mode_flag); //3
                RETURN_IF_FAILED(ret != 0, ret);

                break;
            }
        case H264_NAL_DPC: //4
            {
                //7.3.2.9.3 Slice data partition C RBSP syntax
                RETURN_IF_FAILED(picture_current->m_current_picture_ptr == NULL, -1);

                CH264SliceHeader & slice_header = picture_current->m_current_picture_ptr->m_h264_slice_header;
                CH264Golomb gb;

                slice_header.slice_id = gb.get_ue_golomb(bs); //All ue(v)

                if (slice_header.m_sps.separate_colour_plane_flag == 1)
                {
                    slice_header.colour_plane_id = bs.readBits(2); //All u(2)
                }

                if (slice_header.m_pps.redundant_pic_cnt_present_flag)
                {
                    slice_header.redundant_pic_cnt = gb.get_ue_golomb(bs); //All ue(v)
                }
                
                slice_header.syntax_element_categories = 0x04; //4;
                ret = picture_current->decode_one_slice(slice_header, bs, pictures_gop->m_DecodedPictureBuffer); // /* only category 4 parts of slice_data() syntax */ 4
                CONTINUE_IF_FAILED(ret != 0);

                ret = rbsp_slice_trailing_bits(bs, slice_header.m_pps.entropy_coding_mode_flag); //4
                RETURN_IF_FAILED(ret != 0, ret);

                break; 
            }
        case H264_NAL_IDR_SLICE: //5 立即刷新帧（每个GOP的第一帧必须是I帧）
            {
                if (sps_flag == 0 || pps_flag == 0)
                {
                    LOG_ERROR("For IDR slice, sps and pps should be present first. m_sps_index=%d; m_pps_index=%d;\n", sps_flag, pps_flag);
                    break;
                }
                
                //7.3.2.8 Slice layer without partitioning RBSP syntax
                CH264SliceHeader slice_header;
                ret = slice_header.slice_header(bs, nu, pictures_gop->m_spss, pictures_gop->m_ppss); // 2
                RETURN_IF_FAILED(ret != 0, ret);
                
                bool is_new_pic = slice_header.is_first_VCL_NAL_unit_of_a_picture(slice_header_last);
                if (is_new_pic == true && picture_current != NULL && picture_current->m_current_picture_ptr != NULL)
                {
                    CH264Picture * newEmptyPicture = NULL;
                    picture_current->m_current_picture_ptr->end_decode_the_picture_and_get_a_new_empty_picture(newEmptyPicture);
                    RETURN_IF_FAILED(ret != 0, ret);
                    
                    is_need_flush = picture_current->m_picture_frame.m_h264_slice_header.m_nal_unit.IdrPicFlag; //当前已解码完毕的帧是否是IDR帧
                    ret = do_callback(picture_current, pictures_gop, is_need_flush); //回调操作
                    if (ret != 0)
                    {
                        LOG_ERROR("do_callback() failed! ret=%d;\n", ret);
                        isStopLoop = 1;
                        break;
                    }

                    picture_current = newEmptyPicture;
                }
                
                RETURN_IF_FAILED(picture_current == NULL, -1);
                
                slice_header_last = slice_header;
                
                slice_header.syntax_element_categories = 0x01 | 0x02; //2 | 3;
                slice_header.slice_id = 0;
                ret = picture_current->decode_one_slice(slice_header, bs, pictures_gop->m_DecodedPictureBuffer); // /* all categories of slice_data() syntax */ 2 | 3 | 4
                CONTINUE_IF_FAILED(ret != 0);

                ret = rbsp_slice_trailing_bits(bs, slice_header.m_pps.entropy_coding_mode_flag); //2
                RETURN_IF_FAILED(ret != 0, ret);
                
                break;
            }
        case H264_NAL_SEI: //6
            {
                ret = pictures_gop->m_sei.sei_rbsp(bs);
                if (ret != 0)
                {
                    LOG_ERROR("pictures_gop->m_sei.sei_rbsp(bs) failed! ret=%d;\n", ret);
                }
                break;
            }
        case H264_NAL_SPS: //7
            {
                CH264SPS sps;
                ret = sps.seq_parameter_set_data(bs);
                if (ret != 0)
                {
                    LOG_ERROR("this->m_sps[m_sps_index].seq_parameter_set_data(bs) failed! ret=%d;\n", ret);
                }

                if (sps.seq_parameter_set_id < 0 || sps.seq_parameter_set_id >= H264_MAX_SPS_COUNT)
                {
                    LOG_ERROR("seq_parameter_set_id shall be in the range of 0 to 31, inclusive. sps.seq_parameter_set_id=%d;\n", sps.seq_parameter_set_id);
                }

                pictures_gop->m_spss[sps.seq_parameter_set_id] = sps;
                pictures_gop->max_num_reorder_frames = sps.m_vui.max_num_reorder_frames; //用于按显示顺序输出B帧
                sps_flag = 1;
                break;
            }
        case H264_NAL_PPS: //8
            {
                CH264PPS pps;
                ret = pps.pic_parameter_set_rbsp(bs, pictures_gop->m_spss);
                if (ret != 0)
                {
                    LOG_ERROR("this->m_pps[m_pps_index].pic_parameter_set_rbsp(bs) failed! ret=%d;\n", ret);
                }

                if (pps.pic_parameter_set_id < 0 || pps.pic_parameter_set_id >= H264_MAX_PPS_COUNT)
                {
                    LOG_ERROR("pic_parameter_set_id shall be in the range of 0 to 255, inclusive. pps.pic_parameter_set_id=%d;\n", pps.pic_parameter_set_id);
                }

                pictures_gop->m_ppss[pps.pic_parameter_set_id] = pps;
                pps_flag = 1;
                break;
            }
        case H264_NAL_SPS_EXT: //13
            {
                ret = pictures_gop->m_sps_ext.seq_parameter_set_extension_rbsp(bs);
                if (ret != 0)
                {
                    LOG_ERROR("pictures_gop->m_sps_ext.seq_parameter_set_extension_rbsp(bs) failed! ret=%d;\n", ret);
                }
                break;
            }
        default:
            {
                LOG_ERROR("Unsupported nu.m_nal_unit_type=%d; nal_cnt=%d;\n", nu.nal_unit_type, nal_cnt);
                break;
            }
        }

        nal_cnt++;
    }

    //--------flush操作--------------
    if (isStopLoop == 0)
    {
        if (picture_current)
        {
            is_need_flush = picture_current->m_picture_frame.m_h264_slice_header.m_nal_unit.IdrPicFlag; //当前已解码完毕的帧是否是IDR帧
            ret = do_callback(picture_current, pictures_gop, is_need_flush); //回调操作
            //RETURN_IF_FAILED(ret != 0, ret);
        }

        if (ret == 0)
        {
            is_need_flush = 1;
            ret = do_callback(NULL, pictures_gop, is_need_flush); //最后再回调一次
            //RETURN_IF_FAILED(ret != 0, ret);
        }
    }

    //----------再回调一次空帧，表示解码结束---------------
    int errorCode = H264_DECODE_ERROR_CODE_FILE_END;

    ret = m_output_frame_callback(NULL, m_userData, errorCode);

    //--------------------
    SAFE_DELETE(pictures_gop);

    return ret;
}


int CH264VideoDecoder::do_callback(CH264Picture *picture_current, CH264PicturesGOP *pictures_gop, int32_t is_need_flush)
{
    int ret = 0;
    CH264Picture * outPicture = NULL;
    int errorCode = 0;

    if (is_need_flush) //说明当前已解码完毕的帧是IDR帧
    {
        while(1)
        {
            ret = pictures_gop->getOneOutPicture(NULL, outPicture); //flush操作
            RETURN_IF_FAILED(ret != 0, ret);

            if (outPicture != NULL)
            {
                if (m_output_frame_callback != NULL)
                {
                    ret = m_output_frame_callback(outPicture, m_userData, errorCode); //当找到可输出的帧时，主动通知外部用户
                    if (ret != 0)
                    {
                        return -1; //直接退出
                    }
                }

                outPicture->m_is_in_use = 0; //标记为闲置状态，以便后续回收重复利用
            }
            else //if (outPicture == NULL) //说明已经flush完毕，DPB缓存中已经没有可输出的帧了
            {
                break;
            }
        }
    }

    //-----------------------------------------------------------------------
    ret = pictures_gop->getOneOutPicture(picture_current, outPicture);
    RETURN_IF_FAILED(ret != 0, ret);

    if (outPicture != NULL)
    {
        if (m_output_frame_callback != NULL)
        {
            ret = m_output_frame_callback(outPicture, m_userData, errorCode); //当找到可输出的帧时，主动通知外部用户
            if (ret != 0)
            {
                return -1; //直接退出
            }
        }

        outPicture->m_is_in_use = 0; //标记为闲置状态，以便后续回收重复利用
    }
    
    return 0;
}

