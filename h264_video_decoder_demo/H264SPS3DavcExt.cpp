//
// H264SPS3DavcExt.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264SPS3DavcExt.h"
#include "H264Golomb.h"
#include "Bitstream.h"
#include "CommonFunction.h"


CH264SPS3DavcExt::CH264SPS3DavcExt()
{
    memset(this, 0, sizeof(CH264SPS3DavcExt));
}


CH264SPS3DavcExt::~CH264SPS3DavcExt()
{

}


int CH264SPS3DavcExt::printInfo()
{
    printf("---------SPS-3D-AVC-Ext------------\n");

    return 0;
}


int CH264SPS3DavcExt::seq_parameter_set_3davc_extension(CBitstream &bs)
{
/*    int ret = 0;
    CH264Golomb gb;
    int i = 0;
    int j = 0;
    int k = 0;

    if ( NumDepthViews > 0 )
    {
        3dv_acquisition_idc 0 ue(v)
        for ( i = 0; i < NumDepthViews; i++ )
        {
            view_id_3dv[ i ] 0 ue(v)
        }
        if ( 3dv_acquisition_idc )
        {
            depth_ranges( NumDepthViews, 2, 0 )
            vsp_param( NumDepthViews, 2, 0 )
        }
        reduced_resolution_flag 0 u(1)
        if ( reduced_resolution_flag )
        {
            depth_pic_width_in_mbs_minus1 0 ue(v)
            depth_pic_height_in_map_units_minus1 0 ue(v)
            depth_hor_mult_minus1 0 ue(v)
            depth_ver_mult_minus1 0 ue(v)
            depth_hor_rsh 0 ue(v)
            depth_ver_rsh 0 ue(v)
        }
        depth_frame_cropping_flag 0 u(1)
        if ( depth_frame_cropping_flag )
        {
            depth_frame_crop_left_offset 0 ue(v)
            depth_frame_crop_right_offset 0 ue(v)
            depth_frame_crop_top_offset 0 ue(v)
            depth_frame_crop_bottom_offset 0 ue(v)
        }
        grid_pos_num_views 0 ue(v)
        for ( i = 0; i < grid_pos_num_views; i++ )
        {
            grid_pos_view_id[ i ] 0 ue(v)
            grid_pos_x[ grid_pos_view_id[ i ] ] 0 se(v)
            grid_pos_y[ grid_pos_view_id[ i ] ] 0 se(v)
        }
        slice_header_prediction_flag 0 u(1)
        seq_view_synthesis_flag 0 u(1)
    }
    alc_sps_enable_flag 0 u(1)
    enable_rle_skip_flag 0 u(1)
    if ( !AllViewsPairedFlag )
    {
        for ( i = 1; i <= num_views_minus1; i++ )
        {
            if ( texture_view_present_flag[ i ] )
            {
                num_anchor_refs_l0[ i ] 0 ue(v)
                for ( j = 0; j < num_anchor_refs_l0[ i ]; j++ )
                {
                    anchor_ref_l0[ i ][ j ] 0 ue(v)
                }
                num_anchor_refs_l1[ i ] 0 ue(v)
                for ( j = 0; j < num_anchor_refs_l1[ i ]; j++ )
                {
                    anchor_ref_l1[ i ][ j ] 0 ue(v)
                }
            }
        }
        for ( i = 1; i <= num_views_minus1; i++ )
        {
            if ( texture_view_present_flag[ i ] )
            {
                num_non_anchor_refs_l0[ i ] 0 ue(v)
                for ( j = 0; j < num_non_anchor_refs_l0[ i ]; j++ )
                {
                    non_anchor_ref_l0[ i ][ j ] 0 ue(v)
                }
                num_non_anchor_refs_l1[ i ] 0 ue(v)
                for ( j = 0; j < num_non_anchor_refs_l1[ i ]; j++ )
                {
                    non_anchor_ref_l1[ i ][ j ] 0 ue(v)
                }
            }
        }
    }*/
    return 0;
}
