//
// H264SubsetSPS.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264SubsetSPS.h"
#include "H264Golomb.h"
#include "Bitstream.h"
#include "CommonFunction.h"


CH264SubsetSPS::CH264SubsetSPS()
{
    svc_vui_parameters_present_flag = 0;
    bit_equal_to_one = 0;
    mvc_vui_parameters_present_flag = 0;
    additional_extension2_flag = 0;
    additional_extension2_data_flag = 0;
}


CH264SubsetSPS::~CH264SubsetSPS()
{

}


int CH264SubsetSPS::printInfo()
{
    printf("---------Subset SPS info------------\n");
    printf("svc_vui_parameters_present_flag=%d;\n", svc_vui_parameters_present_flag);
    printf("bit_equal_to_one=%d;\n", bit_equal_to_one);
    printf("mvc_vui_parameters_present_flag=%d;\n", mvc_vui_parameters_present_flag);
    printf("additional_extension2_flag=%d;\n", additional_extension2_flag);
    printf("additional_extension2_data_flag=%d;\n", additional_extension2_data_flag);

    return 0;
}


int CH264SubsetSPS::subset_seq_parameter_set_rbsp(CBitstream &bs)
{
    int ret = 0;
    int ChromaArrayType = 0;

    ret = m_sps.seq_parameter_set_data(bs);
    RETURN_IF_FAILED(ret != 0, ret);

    if (m_sps.separate_colour_plane_flag == 0)
    {
        ChromaArrayType = m_sps.chroma_format_idc;
    }
    else //if (m_sps.separate_colour_plane_flag == 1)
    {
        ChromaArrayType = 0;
    }

    if (m_sps.profile_idc == 83 || m_sps.profile_idc == 86)
    {
        ret = m_sps_svc_ext.seq_parameter_set_svc_extension(bs, ChromaArrayType); // /* specified in Annex G */ 0
        RETURN_IF_FAILED(ret != 0, ret);
        
        this->svc_vui_parameters_present_flag = bs.readBits(1); //0 u(1)
        if (svc_vui_parameters_present_flag == 1)
        {
            ret = m_svc_vui_ext.svc_vui_parameters_extension(bs); // /* specified in Annex G */ 0
            RETURN_IF_FAILED(ret != 0, ret);
        }
    }
    else if (m_sps.profile_idc == 118 || m_sps.profile_idc == 128 || m_sps.profile_idc == 134)
    {
        this->bit_equal_to_one = bs.readBits(1); // /* equal to 1 */ 0 f(1)
        this->m_sps_mvc_ext.seq_parameter_set_mvc_extension(bs, m_sps.profile_idc, m_sps.frame_mbs_only_flag); // /* specified in Annex H */ 0
        this->mvc_vui_parameters_present_flag = bs.readBits(1); //0 u(1)
        if (this->mvc_vui_parameters_present_flag == 1)
        {
            ret = m_mvc_vui_ext.mvc_vui_parameters_extension(bs); // /* specified in Annex H */ 0
            RETURN_IF_FAILED(ret != 0, ret);
        }
    }
    else if (m_sps.profile_idc == 138 || m_sps.profile_idc == 135)
    {
        this->bit_equal_to_one = bs.readBits(1); // /* equal to 1 */ 0 f(1)
        ret = m_sps_mvcd_ext.seq_parameter_set_mvcd_extension(bs); // /* specified in Annex I */
        RETURN_IF_FAILED(ret != 0, ret);
    }
    else if (m_sps.profile_idc == 139)
    {
        this->bit_equal_to_one = bs.readBits(1); // /* equal to 1 */ 0 f(1)
        ret = m_sps_mvcd_ext.seq_parameter_set_mvcd_extension(bs); // /* specified in Annex I */ 0
        RETURN_IF_FAILED(ret != 0, ret);

        ret = m_sps_3d_avc_ext.seq_parameter_set_3davc_extension(bs); // /* specified in Annex J */ 0
        RETURN_IF_FAILED(ret != 0, ret);
    }
    this->additional_extension2_flag = bs.readBits(1); //0 u(1)
    if (this->additional_extension2_flag == 1)
    {
//        while (more_rbsp_data())
//        {
//            this->additional_extension2_data_flag = bs.readBits(1); //0 u(1)
//        }
    }
    //rbsp_trailing_bits();

    return 0;
}
