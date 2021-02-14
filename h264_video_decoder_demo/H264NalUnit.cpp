//
// H264NalUnit.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264NalUnit.h"
#include "Bitstream.h"
#include "CommonFunction.h"


CH264NalUnit::CH264NalUnit()
{
    int ret = init(0);
}


CH264NalUnit::~CH264NalUnit()
{
    int ret = unInit();
}


int CH264NalUnit::printInfo()
{
    printf("---------NAL unit info------------\n");
    printf("forbidden_zero_bit=%d;\n", forbidden_zero_bit);
    printf("nal_ref_idc=%d;\n", nal_ref_idc);
    printf("nal_unit_type=%d;\n", nal_unit_type);
    printf("svc_extension_flag=%d;\n", svc_extension_flag);
    printf("avc_3d_extension_flag=%d;\n", avc_3d_extension_flag);
    printf("emulation_prevention_three_byte=%d;\n", emulation_prevention_three_byte);
    printf("rbsp_byte=0x%p;\n", rbsp_byte);
    printf("NumBytesInNALunit=%d;\n", NumBytesInNALunit);
    printf("NumBytesInRBSP=%d;\n", NumBytesInRBSP);
    printf("IdrPicFlag=%d;\n", IdrPicFlag);
    printf("m_is_malloc_mem_self=%d;\n", m_is_malloc_mem_self);

    return 0;
}


int CH264NalUnit::init(int numBytesInNALunit)
{
    int ret = 0;

    forbidden_zero_bit = 0;
    nal_ref_idc = 0;
    nal_unit_type = 0;
    svc_extension_flag = 0;
    avc_3d_extension_flag = 0;
    emulation_prevention_three_byte = 0;
    NumBytesInNALunit = numBytesInNALunit;
    NumBytesInRBSP = 0;
    rbsp_byte = NULL;
    IdrPicFlag = 0;
    m_is_malloc_mem_self = 0;
    
    if (NumBytesInNALunit > 0)
    {
        rbsp_byte = (uint8_t *)my_malloc(sizeof(uint8_t) * NumBytesInNALunit);
        RETURN_IF_FAILED(rbsp_byte == NULL, -1);
        memset(rbsp_byte, 0, sizeof(uint8_t) * NumBytesInNALunit);
        m_is_malloc_mem_self = 1;
    }

    return ret;
}


int CH264NalUnit::unInit()
{
    int ret = 0;
    
    if (m_is_malloc_mem_self == 1)
    {
        SAFE_FREE(rbsp_byte);
    }

    return ret;
}


CH264NalUnit & CH264NalUnit::operator = (const CH264NalUnit &src)
{
    int ret = 0;
    bool isMallocAndCopyData = false;

    ret = copyData(src, isMallocAndCopyData); //重载的等号运算符

    return *this;
}


int CH264NalUnit::copyData(const CH264NalUnit &src, bool isMallocAndCopyData)
{
    int ret = 0;
    
    ret = unInit();
    RETURN_IF_FAILED(ret != 0, ret);
    
    memcpy(this, &src, sizeof(CH264NalUnit));

    m_is_malloc_mem_self = 0;

    if (isMallocAndCopyData)
    {
        rbsp_byte = (uint8_t *)my_malloc(sizeof(uint8_t) * NumBytesInNALunit);
        RETURN_IF_FAILED(rbsp_byte == NULL, -1);

        memcpy(rbsp_byte, src.rbsp_byte, sizeof(uint8_t) * NumBytesInNALunit);
        m_is_malloc_mem_self = 1;
    }

    return ret;
}


//7.3.1 NAL unit syntax
int CH264NalUnit::getH264RbspFromNalUnit(unsigned char *srcData, int srcSize)
{
    int ret = 0;
    
    //------------------------
    this->init(srcSize);

    //----------------------------
    uint8_t b8 = 0;
    int32_t NumBytesInNALunit = srcSize;
    int32_t temp = 0;
    CBitstream bs(srcData, srcSize);

    forbidden_zero_bit = bs.readBits(1); //forbidden_zero_bit All f(1)
    nal_ref_idc = bs.readBits(2); //nal_ref_idc All u(2)
    nal_unit_type = bs.readBits(5); //nal_unit_type All u(5)

    //-------------------------------------------------------------
    int32_t nalUnitHeaderBytes = 1;

    if (nal_unit_type == 14 //H264_NAL_PREFIX
        || nal_unit_type == 20 //H264_NAL_EXTEN_SLICE
        || nal_unit_type == 21 //H264_NAL_DEPTH_EXTEN_SLICE
        )
    {
        if (nal_unit_type != 21) //H264_NAL_DEPTH_EXTEN_SLICE
        {
            svc_extension_flag = bs.readBits(1); //svc_extension_flag All u(1)
        }else
        {
            avc_3d_extension_flag = bs.readBits(1); //avc_3d_extension_flag All u(1)
        }

        if (svc_extension_flag)
        {
            //nal_unit_header_svc_extension() /* specified in Annex G */ All
            nalUnitHeaderBytes += 3;

            //G.7.3.1.1 NAL unit header SVC extension syntax
            int32_t idr_flag = bs.readBits(1); //All u(1)
            int32_t priority_id = bs.readBits(6); //All u(6)
            int32_t no_inter_layer_pred_flag = bs.readBits(1); //All u(1)
            int32_t dependency_id = bs.readBits(3); //All u(3)
            int32_t quality_id = bs.readBits(4); //All u(4)
            int32_t temporal_id = bs.readBits(3); //All u(3)
            int32_t use_ref_base_pic_flag = bs.readBits(1); //All u(1)
            int32_t discardable_flag = bs.readBits(1); //All u(1)
            int32_t output_flag = bs.readBits(1); //All u(1)
            int32_t reserved_three_2bits = bs.readBits(2); //All u(2)
        }
        else if (avc_3d_extension_flag)
        {
            //nal_unit_header_3davc_extension() /* specified in Annex J */
            nalUnitHeaderBytes += 2;

            //J.7.3.1.1 NAL unit header 3D-AVC extension syntax
            int32_t view_idx = bs.readBits(8); //All u(8)
            int32_t depth_flag = bs.readBits(1); //All u(1)
            int32_t non_idr_flag = bs.readBits(1); //All u(1)
            int32_t temporal_id = bs.readBits(3); //All u(3)
            int32_t anchor_pic_flag = bs.readBits(1); //All u(1)
            int32_t inter_view_flag = bs.readBits(1); //All u(1)
        }else
        {
            //nal_unit_header_mvc_extension() /* specified in Annex H */ All 
            nalUnitHeaderBytes += 3;

            //H.7.3.1.1 NAL unit header MVC extension syntax
            int32_t non_idr_flag = bs.readBits(1); //All u(1)
            int32_t priority_id = bs.readBits(6); //All u(6)
            int32_t view_id = bs.readBits(10); //All u(10)
            int32_t temporal_id = bs.readBits(3); //All u(3)
            int32_t anchor_pic_flag = bs.readBits(1); //All u(1)
            int32_t inter_view_flag = bs.readBits(1); //All u(1)
            int32_t reserved_one_bit = bs.readBits(1); //All u(1)
        }
    }

    //-------------------------------------------------------------
    for (int32_t i = nalUnitHeaderBytes; i < NumBytesInNALunit; ++i)
    {
        temp = bs.getBits(24);

        if (i + 2 < NumBytesInNALunit && temp == 0x000003)
        {
            rbsp_byte[NumBytesInRBSP++] = bs.readBits(8); //rbsp_byte[ NumBytesInRBSP++ ] All b(8)
            rbsp_byte[NumBytesInRBSP++] = bs.readBits(8); //rbsp_byte[ NumBytesInRBSP++ ] All b(8)
            i += 2;
            emulation_prevention_three_byte = bs.readBits(8); //emulation_prevention_three_byte /* equal to 0x03 */ All f(8)
        }else
        {
            rbsp_byte[NumBytesInRBSP++] = bs.readBits(8); //rbsp_byte[ NumBytesInRBSP++ ] All b(8)
        }
    }

    IdrPicFlag = ( ( nal_unit_type == 5 ) ? 1 : 0 ); //H264_NAL_IDR_SLICE = 5

//    int ret2 = printInfo();

    return ret;
}
