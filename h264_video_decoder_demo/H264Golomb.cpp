//
// H264Golomb.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264Golomb.h"
#include "Bitstream.h"
#include "CommonFunction.h"
#include "H264CommonFunc.h"


CH264Golomb::CH264Golomb()
{

}


CH264Golomb::~CH264Golomb()
{

}


/**
 * 9.1 Parsing process for Exp-Golomb codes
 * Rec. ITU-T H.264 (04/2017) P208
 *
 *     leadingZeroBits = −1
 *     for (b = 0; !b; leadingZeroBits++)
 *           b = read_bits(1)
 *
 *     codeNum = 2^leadingZeroBits − 1 + read_bits(leadingZeroBits)
 *
 *        Bit string              codeNum
 *            1                      0
 *           010                     1
 *           011                     2
 *          00100                    3
 *          00101                    4
 *          ......                ......
 *         0001010                   9
 *          ......                ......
 *
 * Golomb解码
 */
int CH264Golomb::get_ue_golomb(CBitstream &bs)
{
    int leadingZeroBits = -1;
    for (int b = 0; !b; leadingZeroBits++)
    {
        b = bs.readBits(1); // must be zero
    }

    RETURN_IF_FAILED(leadingZeroBits < 0 || leadingZeroBits > 32, -1);

    int codeNum = (1 << leadingZeroBits) - 1 + bs.readBits(leadingZeroBits);

    return codeNum;
}


/**
 * signed integer Exp-Golomb-coded syntax element with the left bit first.
 * Table 9-3 – Assignment of syntax element to codeNum for signed Exp-Golomb 
 * coded syntax elements se(v)
 *
 * Rec. ITU-T H.264 (04/2017) P210
 *
 * codeNum            syntax element value
 *    0                       0
 *    1                       1
 *    2                      −1
 *    3                       2
 *    4                      −2
 *    5                       3
 *    6                      −3
 *    k                     (−1)^(k+1) * Ceil(k÷2)
 *
 * 有符号指数Golomb熵编码
 *    k = ue(v);
 *    value = ((-1)^(k+1)) * (Ceil(k/2));
 */
int CH264Golomb::get_se_golomb(CBitstream &bs)
{
    int k = get_ue_golomb(bs);
    
    //int sign = -(k & 1);
    //int value = ((k >> 1) ^ sign) + 1;

    int sign = (k + 1) & 1;
    int value = (k + 1) >> 1; // Ceil(k/2)
    if (sign == 1) //奇数
    {
        value *= -1;
    }

    return value;
}


/**
 * 9.1.2    Mapping process for coded block pattern
 *     Input to this process is codeNum as specified in clause 9.1.
 *     Output of this process is a value of the syntax element 
 *     coded_block_pattern coded as me(v).
 *
 *     Table 9-4 shows the assignment of coded_block_pattern to codeNum 
 *     depending on whether the macroblock prediction mode is equal to 
 *     Intra_4x4, Intra_8x8 or Inter.
 *
 * Rec. ITU-T H.264 (04/2017) P210
 * Table 9-4 – Assignment of codeNum to values of coded_block_pattern for macroblock prediction modes
 *   (a) ChromaArrayType is equal to 1 or 2
 * codeNum                coded_block_pattern
 *             Intra_4x4, Intra_8x8     Inter
 *    0            47                     0
 *    1            31                     16
 *    2            15                     1
 *    3            0                      2
 *    4            23                     4
 *    5            27                     8
 *    6            29                     32
 *    7            30                     3
 *    8            7                      5
 *    9            11                     10
 *    10           13                     12
 *    11           14                     15
 *    12           39                     47
 *    13           43                     7
 *    14           45                     11
 *    15           46                     13
 *    16           16                     14
 *    17           3                      6
 *    18           5                      9
 *    19           10                     31
 *    20           12                     35
 *    21           19                     37
 *    22           21                     42
 *    23           26                     44
 *    24           28                     33
 *    25           35                     34
 *    26           37                     36
 *    27           42                     40
 *    28           44                     39
 *    29           1                      43
 *    30           2                      45
 *    31           4                      46
 *    32           8                      17
 *    33           17                     18
 *    34           18                     20
 *    35           20                     24
 *    36           24                     19
 *    37           6                      21
 *    38           9                      26
 *    39           22                     28
 *    40           25                     23
 *    41           32                     27
 *    42           33                     29
 *    43           34                     30
 *    44           36                     22
 *    45           40                     25
 *    46           38                     38
 *    47           41                     41
 *
 *   (b) ChromaArrayType is equal to 0 or 3
 * codeNum                coded_block_pattern
 *             Intra_4x4, Intra_8x8     Inter
 *    0            15                     0
 *    1            0                      1
 *    2            7                      2
 *    3            11                     4
 *    4            13                     8
 *    5            14                     3
 *    6            3                      5
 *    7            5                      10
 *    8            10                     12
 *    9            12                     15
 *    10           1                      7
 *    11           2                      11
 *    12           4                      13
 *    13           8                      14
 *    14           6                      6
 *    15           9                      9
 *
 * 映射指数Golomb熵编码
 */

struct maping_exp_golomb_t
{
    int32_t code_num;
    int32_t coded_block_pattern_of_Intra_4x4_or_Intra_8x8;
    int32_t coded_block_pattern_of_Inter;
};


maping_exp_golomb_t maping_exp_golomb_arrays1[] = 
{
    {0,  47,   0},
    {1,  31,   16},
    {2,  15,   1},
    {3,  0,   2},
    {4,  23,   4},
    {5,  27,   8},
    {6,  29,   32},
    {7,  30,   3},
    {8,  7,   5},
    {9,  11,   10},
    {10,  13,   12},
    {11,  14,   15},
    {12,  39,   47},
    {13,  43,   7},
    {14,  45,   11},
    {15,  46,   13},
    {16,  16,   14},
    {17,  3,   6},
    {18,  5,   9},
    {19,  10,   31},
    {20,  12,   35},
    {21,  19,   37},
    {22,  21,   42},
    {23,  26,   44},
    {24,  28,   33},
    {25,  35,   34},
    {26,  37,   36},
    {27,  42,   40},
    {28,  44,   39},
    {29,  1,   43},
    {30,  2,   45},
    {31,  4,   46},
    {32,  8,   17},
    {33,  17,   18},
    {34,  18,   20},
    {35,  20,   24},
    {36,  24,   19},
    {37,  6,   21},
    {38,  9,   26},
    {39,  22,   28},
    {40,  25,   23},
    {41,  32,   27},
    {42,  33,   29},
    {43,  34,   30},
    {44,  36,   22},
    {45,  40,   25},
    {46,  38,   38},
    {47,  41,   41},
};


maping_exp_golomb_t maping_exp_golomb_arrays2[] = 
{
    {0,  15,   0},
    {1,  0,   1},
    {2,  7,   2},
    {3,  11,   4},
    {4,  13,   8},
    {5,  14,   3},
    {6,  3,   5},
    {7,  5,   10},
    {8,  10,   12},
    {9,  12,   15},
    {10,  1,   7},
    {11,  2,   11},
    {12,  4,   13},
    {13,  8,   14},
    {14,  6,   6},
    {15,  9,   9},
};


int CH264Golomb::get_me_golomb(CBitstream &bs, int32_t ChromaArrayType, H264_MB_PART_PRED_MODE MbPartPredMode)
{
    int32_t coded_block_pattern = 0;
    int32_t codeNum = get_ue_golomb(bs);

    //----------------
    switch(ChromaArrayType)
    {
    case 1:
    case 2:
        {
            RETURN_IF_FAILED(codeNum < 0 || codeNum > 47, -1);

            if (MbPartPredMode == Intra_4x4 //#define    Intra_4x4        0
                || MbPartPredMode == Intra_8x8 //#define    Intra_8x8        1
                )
            {
                coded_block_pattern = maping_exp_golomb_arrays1[codeNum].coded_block_pattern_of_Intra_4x4_or_Intra_8x8;
            }
            else //if (IS_INTER_Prediction_Mode(MbPartPredMode))//#define    Inter            3
            {
                coded_block_pattern = maping_exp_golomb_arrays1[codeNum].coded_block_pattern_of_Inter;
            }
//            else
//            {
//                LOG_ERROR("MbPartPredMode=%d, must be 0,1 or 3;\n", MbPartPredMode);
//                return -1;
//            }
            break;
        }
    case 0:
    case 3:
        {
            RETURN_IF_FAILED(codeNum < 0 || codeNum > 15, -1);

            if (MbPartPredMode == Intra_4x4 //#define    Intra_4x4        0
                || MbPartPredMode == Intra_8x8 //#define    Intra_8x8        1
                )
            {
                coded_block_pattern = maping_exp_golomb_arrays2[codeNum].coded_block_pattern_of_Intra_4x4_or_Intra_8x8;
            }
            else //if (IS_INTER_Prediction_Mode(MbPartPredMode))//#define    Inter            3
            {
                coded_block_pattern = maping_exp_golomb_arrays2[codeNum].coded_block_pattern_of_Inter;
            }
//            else
//            {
//                LOG_ERROR("MbPartPredMode=%d, must be 0,1 or 3;\n", MbPartPredMode);
//                return -1;
//            }
            break;
        }
    default:
        {
            LOG_ERROR("ChromaArrayType=%d, must be 0,1,2 or 3;\n", ChromaArrayType);
            return -1;
        }
    }

    return coded_block_pattern;
}


/**
 * truncated Exp-Golomb-coded syntax element with left bit first.
 * Otherwise (the syntax element is coded as te(v)), the range of possible 
 * values for the syntax element is determined first. The range of this syntax 
 * element may be between 0 and x, with x being greater than or equal to 1 and 
 * the range is used in the derivation of the value of the syntax element value 
 * as follows:
 * – If x is greater than 1, codeNum and the value of the syntax element is 
 *    derived in the same way as for syntax elements coded as ue(v).
 *
 * – Otherwise (x is equal to 1), the parsing process for codeNum which is 
 *    equal to the value of the syntax element is given by a process equivalent to:
 *           b = read_bits( 1 )
 *           codeNum = !b
 *
 * Rec. ITU-T H.264 (04/2017) P209
 *
 * 截断指数Golomb熵编码
 * 参数 range 的取值是根据相应的h264语法元素的取值范围来确定的，比如语法元素ref_idx_l0[]
 * 1. 当RefPicList0Length=1时，ref_idx_l0只能是0，此时输入参数range应该设为0
 * 2. 当RefPicList0Length=2时，ref_idx_l0只能是0或者1，此时输入参数range应该设为1
 * 3. 当RefPicList0Length>=3时，ref_idx_l0有更多的取值情况，此时输入参数range应该设为>2
 */
int CH264Golomb::get_te_golomb(CBitstream &bs, int range)
{
    int32_t codeNum = 0;

    if (range <= 0)
    {
        return 0;
    }
    else if (range == 1)
    {
        int b = bs.readBits(1);
        codeNum = !b;
    }
    else //if (range >= 2)
    {
        codeNum = get_ue_golomb(bs);
    }

    return codeNum;
}
