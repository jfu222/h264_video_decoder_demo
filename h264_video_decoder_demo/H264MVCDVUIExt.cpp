//
// H264MVCDVUIExt.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264MVCDVUIExt.h"
#include "H264Golomb.h"
#include "Bitstream.h"
#include "CommonFunction.h"


CH264MVCDVUIExt::CH264MVCDVUIExt()
{
    memset(this, 0, sizeof(CH264MVCDVUIExt));
}


CH264MVCDVUIExt::~CH264MVCDVUIExt()
{

}


int CH264MVCDVUIExt::printInfo()
{
    printf("---------MVCD-VUI-Ext------------\n");

    return 0;
}


int CH264MVCDVUIExt::mvcd_vui_parameters_extension(CBitstream &bs)
{
    int ret = 0;
    CH264Golomb gb;
    int32_t i = 0;
    int32_t j = 0;

    vui_mvcd_num_ops_minus1 = gb.get_ue_golomb(bs); //0 ue(v)
    for ( i = 0; i <= vui_mvcd_num_ops_minus1; i++ )
    {
        vui_mvcd_temporal_id[ i ] = bs.readBits(3); //0 u(3)
        vui_mvcd_num_target_output_views_minus1[ i ] = gb.get_ue_golomb(bs); //0 ue(v)
        for ( j = 0; j <= vui_mvcd_num_target_output_views_minus1[ i ]; j++ )
        {
            vui_mvcd_view_id[ i ][ j ] = gb.get_ue_golomb(bs); //0 ue(v)
            vui_mvcd_depth_flag[ i ][ j ] = bs.readBits(1); //0 u(1)
            vui_mvcd_texture_flag[ i ][ j ] = bs.readBits(1); //0 u(1)
        }
        vui_mvcd_timing_info_present_flag[ i ] = bs.readBits(1); //0 u(1)
        if ( vui_mvcd_timing_info_present_flag[ i ] )
        {
            vui_mvcd_num_units_in_tick[ i ] = bs.readBits(32); //0 u(32)
            vui_mvcd_time_scale[ i ] = bs.readBits(32); //0 u(32)
            vui_mvcd_fixed_frame_rate_flag[ i ] = bs.readBits(1); //0 u(1)
        }
        vui_mvcd_nal_hrd_parameters_present_flag[ i ] = bs.readBits(1); //0 u(1)
        if ( vui_mvcd_nal_hrd_parameters_present_flag[ i ] )
        {
            ret = m_vui_mvcd_nal_hrd_parameters.hrd_parameters(bs);
            RETURN_IF_FAILED(ret != 0, ret);
        }
        vui_mvcd_vcl_hrd_parameters_present_flag[ i ] = bs.readBits(1); //0 u(1)
        if ( vui_mvcd_vcl_hrd_parameters_present_flag[ i ] )
        {
            ret = m_vui_mvcd_vcl_hrd_parameters.hrd_parameters(bs);
            RETURN_IF_FAILED(ret != 0, ret);
        }
        if ( vui_mvcd_nal_hrd_parameters_present_flag[ i ] || vui_mvcd_vcl_hrd_parameters_present_flag[ i ] )
        {
            vui_mvcd_low_delay_hrd_flag[ i ] = bs.readBits(1); //0 u(1)
            vui_mvcd_pic_struct_present_flag[ i ] = bs.readBits(1); //0 u(1)
        }
    }
    return 0;
}
