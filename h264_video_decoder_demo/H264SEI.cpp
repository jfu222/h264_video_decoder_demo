//
// H264SEI.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264SEI.h"
#include "H264Golomb.h"
#include "CommonFunction.h"
#include "H264CommonFunc.h"


CH264SEI::CH264SEI()
{
    last_payload_type_byte = 0;
    last_payload_size_byte = 0;
    payloadType = 0;
    payloadSize = 0;
}


CH264SEI::~CH264SEI()
{

}


int CH264SEI::printInfo()
{
    printf("---------SEI info------------\n");
    printf("last_payload_type_byte=%d;\n", last_payload_type_byte);
    printf("last_payload_size_byte=%d;\n", last_payload_size_byte);
    printf("payloadType=%d;\n", payloadType);
    printf("payloadSize=%d;\n", payloadSize);

    return 0;
}


int CH264SEI::sei_rbsp(CBitstream &bs)
{
    int ret = 0;

    do
    {
        ret = sei_message(bs);
        BREAK_IF_FAILED(ret != 0);
    }while(more_rbsp_data(bs));

    ret = rbsp_trailing_bits(bs);

    return 0;
}


/*
 * Page 47/69/812
 * 7.3.2.3.1 Supplemental enhancement information message syntax
 */
int CH264SEI::sei_message(CBitstream &bs)
{
    int32_t payloadType = 0;
    
    while(bs.getBits(8) == 0xFF) //while(next_bits(8) == 0xFF)
    {
        int32_t ff_byte = bs.readBits(8); // /* equal to 0xFF */ 5 f(8)
        payloadType += 255;
    }
    last_payload_type_byte = bs.readBits(8); //5 u(8)
    payloadType += last_payload_type_byte;
    payloadSize = 0;
    
    while(bs.getBits(8) == 0xFF) //while(next_bits(8) == 0xFF)
    {
        int32_t ff_byte = bs.readBits(8); // /* equal to 0xFF */ 5 f(8)
        payloadSize += 255;
    }
    last_payload_size_byte = bs.readBits(8); //5 u(8)
    payloadSize += last_payload_size_byte;
    sei_payload(bs, payloadType, payloadSize);

//    int ret2 = printInfo();

    return 0;
}


int CH264SEI::sei_payload(CBitstream &bs, int32_t payloadType,int32_t payloadSize)
{
    int ret = 0;
/*
    if ( payloadType == 0 )
        buffering_period( payloadSize );
    else if ( payloadType == 1 )
        pic_timing( payloadSize );
    else if ( payloadType == 2 )
        pan_scan_rect( payloadSize );
    else if ( payloadType == 3 )
        filler_payload( payloadSize );
    else if ( payloadType == 4 )
        user_data_registered_itu_t_t35( payloadSize );
    else if ( payloadType == 5 )
        user_data_unregistered( payloadSize );
    else if ( payloadType == 6 )
        recovery_point( payloadSize );
    else if ( payloadType == 7 )
        dec_ref_pic_marking_repetition( payloadSize );
    else if ( payloadType == 8 )
        spare_pic( payloadSize );
    else if ( payloadType == 9 )
        scene_info( payloadSize );
    else if ( payloadType == 10 )
        sub_seq_info( payloadSize );
    else if ( payloadType == 11 )
        sub_seq_layer_characteristics( payloadSize );
    else if ( payloadType == 12 )
        sub_seq_characteristics( payloadSize );
    else if ( payloadType == 13 )
        full_frame_freeze( payloadSize );
    else if ( payloadType == 14 )
        full_frame_freeze_release( payloadSize );
    else if ( payloadType == 15 )
        full_frame_snapshot( payloadSize );
    else if ( payloadType == 16 )
        progressive_refinement_segment_start( payloadSize );
    else if ( payloadType == 17 )
        progressive_refinement_segment_end( payloadSize );
    else if ( payloadType == 18 )
        motion_constrained_slice_group_set( payloadSize );
    else if ( payloadType == 19 )
        film_grain_characteristics( payloadSize );
    else if ( payloadType == 20 )
        deblocking_filter_display_preference( payloadSize );
    else if ( payloadType == 21 )
        stereo_video_info( payloadSize );
    else if ( payloadType == 22 )
        post_filter_hint( payloadSize );
    else if ( payloadType == 23 )
        tone_mapping_info( payloadSize );
    else if ( payloadType == 24 )
        scalability_info( payloadSize ); // specified in Annex G
    else if ( payloadType == 25 )
        sub_pic_scalable_layer( payloadSize ); // specified in Annex G
    else if ( payloadType == 26 )
        non_required_layer_rep( payloadSize ); // specified in Annex G
    else if ( payloadType == 27 )
        priority_layer_info( payloadSize ); // specified in Annex G
    else if ( payloadType == 28 )
        layers_not_present( payloadSize ); // specified in Annex G
    else if ( payloadType == 29 )
        layer_dependency_change( payloadSize ); // specified in Annex G
    else if ( payloadType == 30 )
        scalable_nesting( payloadSize ); // specified in Annex G
    else if ( payloadType == 31 )
        base_layer_temporal_hrd( payloadSize ); // specified in Annex G
    else if ( payloadType == 32 )
        quality_layer_integrity_check( payloadSize ); // specified in Annex G
    else if ( payloadType == 33 )
        redundant_pic_property( payloadSize ); // specified in Annex G
    else if ( payloadType == 34 )
        tl0_dep_rep_index( payloadSize ); // specified in Annex G
    else if ( payloadType == 35 )
        tl_switching_point( payloadSize ); // specified in Annex G
    else if ( payloadType == 36 )
        parallel_decoding_info( payloadSize ); // specified in Annex H
    else if ( payloadType == 37 )
        mvc_scalable_nesting( payloadSize ); // specified in Annex H
    else if ( payloadType == 38 )
        view_scalability_info( payloadSize ); // specified in Annex H
    else if ( payloadType == 39 )
        multiview_scene_info( payloadSize ); // specified in Annex H
    else if ( payloadType == 40 )
        multiview_acquisition_info( payloadSize ); // specified in Annex H
    else if ( payloadType == 41 )
        non_required_view_component( payloadSize ); // specified in Annex H
    else if ( payloadType == 42 )
        view_dependency_change( payloadSize ); // specified in Annex H
    else if ( payloadType == 43 )
        operation_points_not_present( payloadSize ); // specified in Annex H
    else if ( payloadType == 44 )
        base_view_temporal_hrd( payloadSize ); // specified in Annex H
    else if ( payloadType == 45 )
        frame_packing_arrangement( payloadSize );
    else if ( payloadType == 46 )
        multiview_view_position( payloadSize ); // specified in Annex H
    else if ( payloadType == 47 )
        display_orientation( payloadSize );
    else if ( payloadType == 48 )
        mvcd_scalable_nesting( payloadSize ); // specified in Annex I
    else if ( payloadType == 49 )
        mvcd_view_scalability_info( payloadSize ); // specified in Annex I
    else if ( payloadType == 50 )
        depth_representation_info( payloadSize ); // specified in Annex I
    else if ( payloadType == 51 )
        three_dimensional_reference_displays_info( payloadSize ); // specified in Annex I
    else if ( payloadType == 52 )
        depth_timing( payloadSize ); // specified in Annex I
    else if ( payloadType == 53 )
        depth_sampling_info( payloadSize ); // specified in Annex I
    else if ( payloadType == 54 )
        constrained_depth_parameter_set_identifier( payloadSize ); // specified in Annex J
    else if ( payloadType == 56 )
        green_metadata( payloadSize ); // specified in ISO/IEC 23001-11
    else if ( payloadType == 137 )
        mastering_display_colour_volume( payloadSize );
    else if ( payloadType == 142 )
        colour_remapping_info( payloadSize );
    else if ( payloadType == 147 )
        alternative_transfer_characteristics( payloadSize );
    else if ( payloadType == 181 )
        alternative_depth_info( payloadSize ); // specified in Annex I
    else
        reserved_sei_message( payloadSize );
*/
    if ( !byte_aligned(bs) )
    {
        int32_t bit_equal_to_one = bs.readBits(1); // /* equal to 1 */ f(1)
        while( !byte_aligned(bs) )
            int32_t bit_equal_to_zero = bs.readBits(1); // /* equal to 0 */ f(1)
    }
    return 0;
}
