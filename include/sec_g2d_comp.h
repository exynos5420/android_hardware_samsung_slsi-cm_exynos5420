/*
 * Copyright Samsung Electronics Co.,LTD.
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdlib.h>
#include <stdio.h>

/* int comp_value[src][dst][scale][filter_mode][blending_mode]
 * [src]
 *  0 : kRGB_565_Config
 *  1 : kARGB_4444_Config
 *  2 : kARGB_8888_Config
 *  3 : kNo_Config
 *  [dst]
 *  0 : kRGB_565_Config
 *  1 : kARGB_4444_Config
 *  2 : kARGB_8888_Config
 *  [scale]
 *  0 : No scaling
 *  1 : Scaling_up
 *  2 : Scaling_down
 *  [filter_mode]
 *  0 : nearest
 *  1 : bilinear
 *  [blending_mode]
 *  0 : SRC
 *  1 : SRC_OVER
 */

int comp_value[3][2][3][2][2] = {
//rgb565 to rgb565
//{      nearest      }, {      bilinear      }
//{   SRC  , SRC_OVER }, {   SRC   , SRC_OVER }
{{
{{ 135*  76,1920*1080 }, { 125*  70, 922* 519 }},  //No scaling
{{ 121*  68, 438* 246 }, { 111*  62, 145*  81 }},  //Scaling up
{{ 119*  67, 374* 211 }, {  96*  54, 144*  81 }}}, //Scaling down
//rgb565 to argb8888
{
{{ 194* 109, 221* 124 }, { 197* 111, 203* 114 }},  //No Scaling
{{ 201* 113, 201* 113 }, { 155*  87, 159*  89 }},  //Scaling up
{{ 202* 114, 200* 112 }, { 125*  70, 116*  66 }}}  //Scaling down
},
//argb8888 to rgb565
{{
{{ 136*  77, 343* 193 }, { 139*  78, 357* 201 }},  //No Scaling
{{ 148*  83, 282* 159 }, { 107*  60, 143*  81 }},  //Scaling up
{{ 144*  81, 243* 137 }, {  95*  53, 122*  69 }}}, //Scaling down
//argb8888 to argb8888
{
{{ 744* 418, 712* 401 }, { 736* 414, 478* 269 }},  //No Scaling
{{ 406* 228, 305* 172 }, { 187* 105, 162*  91 }},  //Scaling up
{{ 307* 173, 257* 144 }, { 118*  66, 115*  65 }}}  //Scaling down
},
//No Src to rgb565 (Not measured yet)
{{
{{8000*8000,8000*8000 }, {8000*8000,8000*8000 }},  //No Scaling
{{8000*8000,8000*8000 }, {8000*8000,8000*8000 }},  //Scaling up
{{8000*8000,8000*8000 }, {8000*8000,8000*8000 }}}, //Scaling down
//No src to argb8888 (Fill to argb8888)
{
{{1920*1080,1279* 719 }, {1266* 712,1276* 718 }},  //No Scaling
{{1272* 715,1272* 715 }, {1258* 707,1278* 719 }},  //Scaling up
{{1269* 714,1269* 714 }, {1258* 708,1271* 715 }}}  //Scaling down
}
};
