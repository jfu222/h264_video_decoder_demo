//
// H264SPSMVCDExt.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264SPSMVCDExt.h"
#include "H264Golomb.h"
#include "Bitstream.h"
#include "CommonFunction.h"


CH264SPSMVCDExt::CH264SPSMVCDExt()
{
    memset(this, 0, sizeof(CH264SPSMVCDExt));
}


CH264SPSMVCDExt::~CH264SPSMVCDExt()
{

}


int CH264SPSMVCDExt::printInfo()
{
    printf("---------SPS-MVCD-Ext------------\n");

    return 0;
}


int CH264SPSMVCDExt::seq_parameter_set_mvcd_extension(CBitstream &bs)
{
    int ret = 0;
    CH264Golomb gb;
    int32_t i = 0;
    int32_t j = 0;
    int32_t k = 0;
    int32_t NumDepthViews = 0;

    num_views_minus1 = gb.get_ue_golomb(bs); //0 ue(v)
    for ( i = 0, NumDepthViews = 0; i <= num_views_minus1; i++ )
    {
        view_id[ i ]= gb.get_ue_golomb(bs); // 0 ue(v)
        depth_view_present_flag[ i ] = bs.readBits(1); //0 u(1)
        DepthViewId[ NumDepthViews ] = view_id[ i ];
        NumDepthViews += depth_view_present_flag[ i ];
        texture_view_present_flag[ i ] = bs.readBits(1); //0 u(1)
    }
    for ( i = 1; i <= num_views_minus1; i++ )
    {
        if ( depth_view_present_flag[ i ] )
        {
            num_anchor_refs_l0[ i ] = gb.get_ue_golomb(bs); //0 ue(v)
            for ( j = 0; j < num_anchor_refs_l0[ i ]; j++ )
            {
                anchor_ref_l0[ i ][ j ] = gb.get_ue_golomb(bs); //0 ue(v)
            }
            num_anchor_refs_l1[ i ] = gb.get_ue_golomb(bs); //0 ue(v)
            for ( j = 0; j < num_anchor_refs_l1[ i ]; j++ )
            {
                anchor_ref_l1[ i ][ j ] = gb.get_ue_golomb(bs); //0 ue(v)
            }
        }
    }
    for ( i = 1; i <= num_views_minus1; i++ )
    {
        if ( depth_view_present_flag[ i ] )
        {
            num_non_anchor_refs_l0[ i ] = gb.get_ue_golomb(bs); //0 ue(v)
            for ( j = 0; j < num_non_anchor_refs_l0[ i ]; j++ )
            {
                non_anchor_ref_l0[ i ][ j ] = gb.get_ue_golomb(bs); //0 ue(v)
            }
            num_non_anchor_refs_l1[ i ] = gb.get_ue_golomb(bs); //0 ue(v)
            for ( j = 0; j < num_non_anchor_refs_l1[ i ]; j++ )
            {
                non_anchor_ref_l1[ i ][ j ]= gb.get_ue_golomb(bs); //0 ue(v)
            }
        }
    }
    num_level_values_signalled_minus1 = gb.get_ue_golomb(bs); //0 ue(v)
    for ( i = 0; i <= num_level_values_signalled_minus1; i++ )
    {
        level_idc[ i ] = bs.readBits(8); //0 u(8)
        num_applicable_ops_minus1[ i ] = gb.get_ue_golomb(bs); //0 ue(v)
        for ( j = 0; j <= num_applicable_ops_minus1[ i ]; j++ )
        {
            applicable_op_temporal_id[ i ][ j ] = bs.readBits(3); //0 u(3)
            applicable_op_num_target_views_minus1[ i ][ j ] = gb.get_ue_golomb(bs); //0 ue(v)
            for ( k = 0; k <= applicable_op_num_target_views_minus1[ i ][ j ]; k++ )
            {
                applicable_op_target_view_id[ i ][ j ][ k ] = gb.get_ue_golomb(bs); //0 ue(v)
                applicable_op_depth_flag[ i ][ j ][ k ] = bs.readBits(1); //0 u(1)
                applicable_op_texture_flag[ i ][ j ][ k ] = bs.readBits(1); //0 u(1)
            }
            applicable_op_num_texture_views_minus1[ i ][ j ]= gb.get_ue_golomb(bs); // 0 ue(v)
            applicable_op_num_depth_views[ i ][ j ] = gb.get_ue_golomb(bs); //0 ue(v)
        }
    }
    mvcd_vui_parameters_present_flag = bs.readBits(1); //0 u(1)
    if ( mvcd_vui_parameters_present_flag == 1 )
    {
        ret = m_mvcd_vui.mvcd_vui_parameters_extension(bs);
        RETURN_IF_FAILED(ret != 0, ret);
    }
    texture_vui_parameters_present_flag = bs.readBits(1); //0 u(1)
    if ( texture_vui_parameters_present_flag == 1 )
    {
        ret = m_texture_vui.mvcd_vui_parameters_extension(bs);
        RETURN_IF_FAILED(ret != 0, ret);
    }
    return 0;
}
