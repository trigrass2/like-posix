/*
 * Copyright (c) 2015 Michael Stuart.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the Appleseed project, <https://github.com/drmetal/app-l-seed>
 *
 * based heavily on the code by Zizzle at: https://github.com/Zizzle/stm32_freertos_example/tree/master/drivers
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

/**
 * this is a nasty mess.
 * todo - fix all this up?
 * registers common to ili932x and lgdp4532 are prefixed with "ILI932x_"
 * registers or settings that are not common are prefixed with "ILI9320_", "ILI9325_" or "LGDP4532_",
 */

#ifndef ILI932x_H_
#define ILI932x_H_

#define LGDP4532_DEVICE_ID          0x4532
#define ILI9320_DEVICE_ID           0x9320
#define ILI9325_DEVICE_ID           0x9325


#define ILI932x_CMD_WRITE_GRAM      0x22


// R00
#define ILI932x_DEVICE_ID               0x0000
#define ILI932x_START_OSCILLATOR        0x0000
#define ILI932x_START_OSC               0x0001
// R01
#define ILI932x_DRIVER_OUTPUT_CONTROL   0x0001
#define ILI932x_DRIVER_OUTPUT_CTRL_SS   0x0100      // source driver shift direction (horizontal mirror)
#define ILI932x_DRIVER_OUTPUT_CTRL_SM   0x0400
// R02
#define ILI932x_DRIVIG_WAVE_CONTROL     0x0002
#define ILI932x_DRIVIG_WAVE_CTRL_EOR    0x0100
#define ILI932x_DRIVIG_WAVE_CTRL_BC     0x0200
// R03
#define ILI932x_ENTRY_MODE          0x0003
#define ILI932x_EM_AM               0x0008  // Address update in vertical direction (AM)
#define ILI932x_EM_HORIZONTAL_INC               0x0010  // Horizontal increment (ID0)
#define ILI932x_EM_VERTICAL_INC               0x0020  // Vertical increment (ID1)
#define ILI932x_EM_ORG              0x0080  // Moves origin address according to the ID
                                    // setting when a window address area is made
#define ILI932x_EM_HWM              0x0200  // High-speed write mode
#define ILI932x_EM_BGR              0x1000  // Swap R and B order of written data
#define ILI932x_EM_65K              0x0000  // 16-bit MCU interface data format (transferring mode)
                                    // 80-system 16-bit interface (1 transfers/pixel) 65,536 colors
#define ILI932x_EM_65KA             0x4000
#define ILI932x_EM_246K_SL          0x8000  // 80-system 16-bit interface (2 transfers/pixel) 262,144 colors
#define ILI932x_EM_246K_SF          0xc000
// R04
#define ILI932x_RESIZE_CONTROL      0x0004
// R07
#define ILI932x_DISP_CONTROL1       0x0007
#define ILI932x_DISP_CONT_D0        0x0001
#define ILI932x_DISP_CONT_D1        0x0002
#define ILI932x_DISP_CONT_CL        0x0008
#define ILI932x_DISP_CONT_DTE       0x0010
#define ILI932x_DISP_CONT_GON       0x0020
#define ILI932x_DISP_CONT_BASEE     0x0100
// R08
#define ILI932x_DISP_CONTROL2       0x0008
#define ILI932x_DISP_CONT_BP0       0x0001
#define ILI932x_DISP_CONT_BP1       0x0002
#define ILI932x_DISP_CONT_BP2       0x0004
#define ILI932x_DISP_CONT_BP3       0x0008
#define ILI932x_DISP_CONT_FP0       0x0100
#define ILI932x_DISP_CONT_FP1       0x0200
#define ILI932x_DISP_CONT_FP2       0x0400
#define ILI932x_DISP_CONT_FP3       0x0800
// R09
#define ILI932x_DISP_CONTROL3       0x0009
// R0A
#define ILI932x_DISP_CONTROL4       0x000A
// R0C
#define ILI932x_RGB_DISP_IF_CTRL    0x000C
#define ILI932x_RGB_IF_CTRL_RIM0    0x0001
#define ILI932x_RGB_IF_CTRL_RIM1    0x0002
#define ILI932x_RGB_IF_CTRL_ENC0    0x1000
// R0D
#define ILI932x_FRAME_MARKER_POS    0x000D
// R0F
#define ILI932x_RGB_DISP_IF_CTRL2   0x000F
// R10
#define ILI932x_POWER_CONTROL1      0x0010
#define LGDP4532_POWER_CTRL1_DK     0x0008
#define ILI932x_POWER_CTRL1_AP0     0x0010
#define ILI932x_POWER_CTRL1_AP1     0x0020
#define ILI932x_POWER_CTRL1_AP2     0x0040
#define ILI932x_POWER_CTRL1_APE     0x0080
#define ILI932x_POWER_CTRL1_BT0     0x0100
#define ILI932x_POWER_CTRL1_BT1     0x0200
#define ILI932x_POWER_CTRL1_BT2     0x0400
#define ILI932x_POWER_CTRL1_BT3     0x0800
#define ILI932x_POWER_CTRL1_SAP     0x1000
#define LGDP4532_POWER_CTRL1_SAP0   0x1000
#define LGDP4532_POWER_CTRL1_SAP1   0x2000
#define LGDP4532_POWER_CTRL1_SAP2   0x4000
// R11
#define ILI932x_POWER_CONTROL2      0x0011
#define ILI932x_POWER_CTRL2_VC0     0x0001
#define ILI932x_POWER_CTRL2_VC1     0x0002
#define ILI932x_POWER_CTRL2_VC2     0x0004
#define ILI932x_POWER_CTRL2_DC00    0x0010
#define ILI932x_POWER_CTRL2_DC01    0x0020
#define ILI932x_POWER_CTRL2_DC02    0x0040
#define ILI932x_POWER_CTRL2_DC10    0x0100
#define ILI932x_POWER_CTRL2_DC11    0x0200
#define ILI932x_POWER_CTRL2_DC12    0x0400
// R12
#define ILI932x_POWER_CONTROL3      0x0012
#define ILI932x_POWER_CTRL3_VRH0    0x0001
#define ILI932x_POWER_CTRL3_VRH1    0x0002
#define ILI932x_POWER_CTRL3_VRH2    0x0004
#define ILI932x_POWER_CTRL3_VRH3    0x0008
#define ILI932x_POWER_CTRL3_PON     0x0010
#define ILI932x_POWER_CTRL3_VCIRE   0x0080
// R13
#define ILI932x_POWER_CONTROL4      0x0013
#define LGDP4532_POWER_CTRL4_VCM0   0x0001
#define LGDP4532_POWER_CTRL4_VCM1   0x0002
#define LGDP4532_POWER_CTRL4_VCM2   0x0004
#define LGDP4532_POWER_CTRL4_VCM3   0x0008
#define LGDP4532_POWER_CTRL4_VCM4   0x0010
#define LGDP4532_POWER_CTRL4_VCM5   0x0020
#define LGDP4532_POWER_CTRL4_VCM6   0x0040
#define ILI932x_POWER_CTRL4_VDV0    0x0100
#define ILI932x_POWER_CTRL4_VDV1    0x0200
#define ILI932x_POWER_CTRL4_VDV2    0x0400
#define ILI932x_POWER_CTRL4_VDV3    0x0800
#define ILI932x_POWER_CTRL4_VDV4    0x1000
#define LGDP4532_POWER_CTRL4_VCOMG  0x2000
// R15
#define LGDP4532_REGULATOR_CONTROL  0x0015
#define LGDP4532_REGULATOR_CTRL_RV0 0x0010
#define LGDP4532_REGULATOR_CTRL_RV1 0x0020
#define LGDP4532_REGULATOR_CTRL_RV2 0x0040
// R20
#define ILI932x_H_GRAM_ADDR_SET     0x0020
// R21
#define ILI932x_V_GRAM_ADDR_SET     0x0021
// R29
#define ILI932x_POWER_CONTROL7      0x0029
#define ILI932x_POWER_CTRL7_VCM0    0x0001
#define ILI932x_POWER_CTRL7_VCM1    0x0002
#define ILI932x_POWER_CTRL7_VCM2    0x0004
#define ILI932x_POWER_CTRL7_VCM3    0x0008
#define ILI932x_POWER_CTRL7_VCM4    0x0010
#define ILI932x_POWER_CTRL7_VCM5    0x0020
// R2B
#define ILI932x_FR_AND_CLR_CTRL         0x002B
#define ILI932x_FR_AND_CLR_CTRL_FRS0    0x0001
#define ILI932x_FR_AND_CLR_CTRL_FRS1    0x0002
#define ILI932x_FR_AND_CLR_CTRL_FRS2    0x0004
#define ILI932x_FR_AND_CLR_CTRL_FRS3    0x0008
// R30
// R31
// R32
// R33
// R34
// R35
// R36
// R37
// R38
// R39
// R3C
// R3D
// R41
#define LGDP4532_EPROM_CONTROL2         0x0041
#define LGDP4532_EPROM_CTRL2_VCMSEL0    0x0001
#define LGDP4532_EPROM_CTRL2_VCMSEL1    0x0002
// R50 - R53
#define ILI932x_HSA                     0x0050
#define ILI932x_HEA                     0x0051
#define ILI932x_VSA                     0x0052
#define ILI932x_VEA                     0x0053
// R60
#define ILI932x_GATE_SCAN_CONTROL1      0x0060
#define ILI932x_GATE_SCAN_CTRL1_NL0     0x0100
#define ILI932x_GATE_SCAN_CTRL1_NL1     0x0200
#define ILI932x_GATE_SCAN_CTRL1_NL2     0x0400
#define ILI932x_GATE_SCAN_CTRL1_NL3     0x0800
#define ILI932x_GATE_SCAN_CTRL1_NL4     0x1000
#define ILI932x_GATE_SCAN_CTRL1_NL5     0x2000
#define ILI932x_GATE_SCAN_CTRL1_GS      0x8000
// R61
#define ILI932x_GATE_SCAN_CONTROL2      0x0061
#define ILI932x_GATE_SCAN_CTRL2_REV     0x0001
// R6A
#define ILI932x_GATE_SCAN_CONTROL3      0x006A
// R80-R85
#define ILI932x_PARTIAL_IMG1_POS        0x0080
#define ILI932x_PARTIAL_IMG1_START_ADDR 0x0081
#define ILI932x_PARTIAL_IMG1_END_ADDR   0x0082
#define ILI932x_PARTIAL_IMG2_POS        0x0083
#define ILI932x_PARTIAL_IMG2_START_ADDR 0x0084
#define ILI932x_PARTIAL_IMG2_END_ADDR   0x0085
// R90
#define ILI932x_PANEL_IF_CONTROL1       0x0090
#define ILI932x_PANEL_IF_CTRL1_RTNI1    0x0002
#define ILI932x_PANEL_IF_CTRL1_RTNI4    0x0010
#define ILI932x_PANEL_IF_CTRL1_RTNI7    0x0080
#define ILI932x_PANEL_IF_CTRL1_DIVI0    0x0100
// R92
#define ILI932x_PANEL_IF_CONTROL2       0x0092
#define ILI932x_PANEL_IF_CTRL2_NOWI1    0x0200
#define ILI932x_PANEL_IF_CTRL2_NOWI2    0x0400
// R93
#define LGDP4532_PANEL_IF_CONTROL3      0x0093
#define LGDP4532_PANEL_IF_CTRL3_MCPI0   0x0001


#define R48 0x30
#define R49 0x31
#define R50 0x32

#define R53 0x35
#define R54 0x36
#define R55 0x37
#define R56 0x38
#define R57 0x39


#define R60 0x3C
#define R61 0x3D



#endif // ILI932x_H_
