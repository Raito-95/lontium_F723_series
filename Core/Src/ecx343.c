/*
 * ecx343.c
 *
 *  Created on: Dec 21, 2022
 *      Author: User
 */
#include "spi.h"
#include "ecx343.h"
#include "main.h"
//#include "usbd_cdc_if.h"

//PowContStatus status;
uint8_t pow_on_seq = 1;
uint8_t pow_off_seq = 1;
uint8_t panel_cont_seq = 0;

SPI_HandleTypeDef *spi_handle_L;
SPI_HandleTypeDef *spi_handle_R;
uint8_t PanelCurrentMap;
uint8_t select_mode;
uint8_t addr_valueL, addr_valueR;

const setPanelSeqTable PanelContPanelReg60HzSettingTable[];
const setPanelSeqTable PanelContPanelReg120HzSettingTable[];
const setPanelSeqTable PanelContLuminance60HzSettingTable[];
const setPanelSeqTable PanelContLuminance120HzSettingTable[];
const setPanelSeqTable PanelContPSReleaseTable[];
const setPanelSeqTable PanelContPSTransitionTable[];

static void panel_reg_rw_init(SPI_HandleTypeDef *hspi_L, SPI_HandleTypeDef *hspi_R);
static PowContStatus pow_on_sequence(void);
static PowContStatus panel_reg_setting(void);
static PowContStatus panel_luminance_setting(void);
static PowContStatus panel_ps_release(void);

static PowContStatus pow_off_sequence(void);
static PowContStatus panel_ps_transition(void);

static PanelRegStatus panel_reg_read_value(uint8_t addr, uint8_t *value);
static PanelRegStatus panel_reg_write_value(uint8_t addr, uint8_t value);

void ECX343EN_Init(void)
{
    panel_reg_rw_init(&hspi2, &hspi4);

}

void ECX343EN_Run(void)
{
    ECX343EN_PowerOn();

}

void ECX343EN_PowerOn(void)
{
    while(pow_on_sequence() == PANEL_CONT_CONTINUE)
    {
        HAL_Delay(10);
    }
    pow_on_seq = 1;
}

void ECX343EN_PowerOff(void)
{
    while(pow_off_sequence() == PANEL_CONT_CONTINUE)
    {
        HAL_Delay(10);
    }
    pow_off_seq = 1;
}

static PowContStatus pow_on_sequence(void)
{
    PowContStatus pnl_status;

    //power on sequence
    switch(pow_on_seq)
    {
        case POW_ON_SEQ_NONE:
            return POW_CONT_OK;

        case POW_ON_SEQ_PNL_1V8:
            ECX343EN_1V8_ENABLE();
            pow_on_seq = POW_ON_SEQ_PNL_3V3;
            break;

        case POW_ON_SEQ_PNL_3V3:
            ECX343EN_3V3_ENABLE();
            pow_on_seq = POW_ON_SEQ_PNL_6V6_N;
            break;

        case POW_ON_SEQ_PNL_6V6_N:
            ECX343EN_6V6_N_ENABLE();
            pow_on_seq = POW_ON_SEQ_P_XCLR;
            break;

        case POW_ON_SEQ_P_XCLR:
            ECX343EN_L_XCLR_ENABLE();
            ECX343EN_R_XCLR_ENABLE();
            pow_on_seq = POW_ON_SEQ_PANEL_REG_SETTING;
            break;

        case POW_ON_SEQ_PANEL_REG_SETTING:
            pnl_status = panel_reg_setting();
            if(pnl_status == PANEL_CONT_OK)
            {
                pow_on_seq = POW_ON_SEQ_PANEL_LUMINANCE_SETTING;
            }
            else if(pnl_status == PANEL_CONT_ERR)
            {
                pow_on_seq = POW_ON_SEQ_NONE;
                return POW_CONT_ERR;
            }
            break;

        case POW_ON_SEQ_PANEL_LUMINANCE_SETTING:
            pnl_status = panel_luminance_setting();
            if(pnl_status == PANEL_CONT_OK)
            {
                pow_on_seq = POW_ON_SEQ_LVDS_EN;
            }
            else if(pnl_status == PANEL_CONT_ERR)
            {
                pow_on_seq = POW_ON_SEQ_NONE;
                return POW_CONT_ERR;
            }
            break;

        case POW_ON_SEQ_LVDS_EN:
//            ECX343EN_LVDS_ENABLE();
            pow_on_seq = POW_ON_SEQ_PS_OFF_SETTING;
            break;

        case POW_ON_SEQ_PS_OFF_SETTING:
            pnl_status = panel_ps_release();
            if(pnl_status == PANEL_CONT_OK)
            {
                pow_on_seq = POW_ON_SEQ_NONE;
                return POW_CONT_OK;
            }
            else if(pnl_status == PANEL_CONT_ERR)
            {
                pow_on_seq = POW_ON_SEQ_NONE;
                return POW_CONT_ERR;
            }
            break;

        default:
            pow_on_seq = POW_ON_SEQ_NONE;
            return POW_CONT_OK;
    }
    return POW_CONT_CONTINUE;
}

static PowContStatus panel_reg_setting(void)
{
    //Panel register setting
    if (select_mode)
    {
        if(PanelContPanelReg120HzSettingTable[panel_cont_seq].map != PANEL_TABLE_END)
        {
            if(panel_reg_write(PanelContPanelReg120HzSettingTable[panel_cont_seq].map,
                               PanelContPanelReg120HzSettingTable[panel_cont_seq].addr,
                               PanelContPanelReg120HzSettingTable[panel_cont_seq].value) == PANEL_REG_ERR)
            {
                panel_cont_seq = PANEL_CONT_SEQ_NONE;
                return PANEL_REG_ERR;
            }
            panel_cont_seq++;
        }
        else
        {
            panel_cont_seq = PANEL_CONT_SEQ_NONE;
            return PANEL_REG_OK;
        }
    }
    else
    {
        if(PanelContPanelReg60HzSettingTable[panel_cont_seq].map != PANEL_TABLE_END)
        {
            if(panel_reg_write(PanelContPanelReg60HzSettingTable[panel_cont_seq].map,
                               PanelContPanelReg60HzSettingTable[panel_cont_seq].addr,
                               PanelContPanelReg60HzSettingTable[panel_cont_seq].value) == PANEL_REG_ERR)
            {
                panel_cont_seq = PANEL_CONT_SEQ_NONE;
                return PANEL_REG_ERR;
            }
            panel_cont_seq++;
        }
        else
        {
            panel_cont_seq = PANEL_CONT_SEQ_NONE;
            return PANEL_REG_OK;
        }
    }
    return PANEL_REG_CONTINUE;
}

static PowContStatus panel_luminance_setting(void)
{
    //Panel register setting
    if (select_mode)
    {
        if(PanelContLuminance120HzSettingTable[panel_cont_seq].map != PANEL_TABLE_END)
        {
            if(panel_reg_write(PanelContLuminance120HzSettingTable[panel_cont_seq].map,
                               PanelContLuminance120HzSettingTable[panel_cont_seq].addr,
                               PanelContLuminance120HzSettingTable[panel_cont_seq].value) == PANEL_REG_ERR)
            {
                panel_cont_seq = PANEL_CONT_SEQ_NONE;
                return PANEL_REG_ERR;
            }
            panel_cont_seq++;
        }
        else
        {
            panel_cont_seq = PANEL_CONT_SEQ_NONE;
            return PANEL_REG_OK;
        }
    }
    else
    {
        if(PanelContLuminance60HzSettingTable[panel_cont_seq].map != PANEL_TABLE_END)
        {
            if(panel_reg_write(PanelContLuminance60HzSettingTable[panel_cont_seq].map,
                               PanelContLuminance60HzSettingTable[panel_cont_seq].addr,
                               PanelContLuminance60HzSettingTable[panel_cont_seq].value) == PANEL_REG_ERR)
            {
                panel_cont_seq = PANEL_CONT_SEQ_NONE;
                return PANEL_REG_ERR;
            }
            panel_cont_seq++;
        }
        else
        {
            panel_cont_seq = PANEL_CONT_SEQ_NONE;
            return PANEL_REG_OK;
        }
    }
    return PANEL_REG_CONTINUE;
}

static PowContStatus panel_ps_release(void)
{
    //Panel register setting
    if(PanelContPSReleaseTable[panel_cont_seq].map != PANEL_TABLE_END)
    {
        if(panel_reg_write(PanelContPSReleaseTable[panel_cont_seq].map,
                           PanelContPSReleaseTable[panel_cont_seq].addr,
                           PanelContPSReleaseTable[panel_cont_seq].value) == PANEL_REG_ERR)
        {
            panel_cont_seq = PANEL_CONT_SEQ_NONE;
            return PANEL_REG_ERR;
        }
        panel_cont_seq++;
    }
    else
    {
        panel_cont_seq = PANEL_CONT_SEQ_NONE;
        return PANEL_REG_OK;
    }
    return PANEL_REG_CONTINUE;
}

static PowContStatus pow_off_sequence(void)
{
    PowContStatus pnl_status;

        //power on sequence
        switch(pow_off_seq)
        {
            case POW_OFF_SEQ_NONE:
                break;

            case POW_OFF_SEQ_PS_ON_SETTING:
                pnl_status = panel_ps_transition();
                if(pnl_status == PANEL_CONT_OK)
                {
                    HAL_Delay(24);
                    pow_off_seq = POW_OFF_SEQ_LVDS_DIS;
                }
                else if(pnl_status == PANEL_CONT_ERR)
                {
                    pow_off_seq = POW_OFF_SEQ_NONE;
                    return POW_CONT_ERR;
                }
                break;

            case POW_OFF_SEQ_LVDS_DIS:
//                ECX343EN_LVDS_DISABLE();
                pow_off_seq = POW_OFF_SEQ_P_XCLR;
                break;

            case POW_OFF_SEQ_P_XCLR:
                ECX343EN_L_XCLR_DISABLE();
                ECX343EN_R_XCLR_DISABLE();
                pow_off_seq = POW_OFF_SEQ_PNL_6V6_N;
                break;

            case POW_OFF_SEQ_PNL_6V6_N:
                ECX343EN_6V6_N_DISABLE();
                pow_off_seq = POW_OFF_SEQ_PNL_3V3;
                break;

            case POW_OFF_SEQ_PNL_3V3:
                ECX343EN_3V3_DISABLE();
                pow_off_seq = POW_OFF_SEQ_PNL_1V8;
                break;

            case POW_OFF_SEQ_PNL_1V8:
                ECX343EN_1V8_DISABLE();
                pow_off_seq = POW_OFF_SEQ_NONE;
                return POW_CONT_OK;

            default:
                pow_off_seq = POW_OFF_SEQ_NONE;
                break;
        }
        return POW_CONT_CONTINUE;
}

static PowContStatus panel_ps_transition(void)
{
    //Panel register setting
    if(PanelContPSTransitionTable[panel_cont_seq].map != PANEL_TABLE_END)
    {
        if(panel_reg_write(PanelContPSTransitionTable[panel_cont_seq].map,
                           PanelContPSTransitionTable[panel_cont_seq].addr,
                           PanelContPSTransitionTable[panel_cont_seq].value) == PANEL_REG_ERR)
        {
            panel_cont_seq = PANEL_CONT_SEQ_NONE;
            return PANEL_REG_ERR;
        }
        panel_cont_seq++;
    }
    else
    {
        panel_cont_seq = PANEL_CONT_SEQ_NONE;
        return PANEL_REG_OK;
    }
    return PANEL_REG_CONTINUE;
}

static void panel_reg_rw_init(SPI_HandleTypeDef *hspi_L, SPI_HandleTypeDef *hspi_R)
{
    PanelCurrentMap = PANEL_MAP_0;
    spi_handle_L = hspi_L;
    spi_handle_R = hspi_R;

    select_mode = 0;

    if(select_mode)
    {
        ECX343EN_120HZ_ENABLE();
    }
    else
    {
        ECX343EN_120HZ_DISABLE();
    }

}

static PanelRegStatus panel_reg_read_value(uint8_t addr, uint8_t *value)
{
    uint8_t PanelWriteBuf[10];
    uint8_t PanelReadBuf[10];

    PanelWriteBuf[0] = PANEL_ADDR_81;
    PanelWriteBuf[1] = addr;

    ECX343EN_L_SPI_ENABLE();
    delay_us(30);
    if(HAL_SPI_Transmit(spi_handle_L, &PanelWriteBuf[0], PANEL_WRITE_LENGTH, PANEL_REG_RW_TIMEOUT) != HAL_OK)
    {
        return PANEL_REG_ERR;
    }
    ECX343EN_L_SPI_DISABLE();
    delay_us(30);

    PanelWriteBuf[0] = PANEL_ADDR_81;
    PanelWriteBuf[1] = 0x00;

    ECX343EN_L_SPI_ENABLE();
    delay_us(30);
    if(HAL_SPI_TransmitReceive(spi_handle_L, &PanelWriteBuf[0], &PanelReadBuf[0], PANEL_WRITE_LENGTH, PANEL_REG_RW_TIMEOUT) != HAL_OK)
    {
        return PANEL_REG_ERR;
    }
    ECX343EN_L_SPI_DISABLE();
    delay_us(30);

    addr_valueL = PanelReadBuf[1];


    PanelWriteBuf[0] = PANEL_ADDR_81;
    PanelWriteBuf[1] = addr;

    ECX343EN_L_SPI_ENABLE();
    delay_us(30);
    if(HAL_SPI_Transmit(spi_handle_R, &PanelWriteBuf[0], PANEL_WRITE_LENGTH, PANEL_REG_RW_TIMEOUT) != HAL_OK)
    {
        return PANEL_REG_ERR;
    }
    ECX343EN_L_SPI_DISABLE();
    delay_us(30);

    PanelWriteBuf[0] = PANEL_ADDR_81;
    PanelWriteBuf[1] = 0x00;

    ECX343EN_L_SPI_ENABLE();
    delay_us(30);
    if(HAL_SPI_TransmitReceive(spi_handle_R, &PanelWriteBuf[0], &PanelReadBuf[0], PANEL_WRITE_LENGTH, PANEL_REG_RW_TIMEOUT) != HAL_OK)
    {
        return PANEL_REG_ERR;
    }
    ECX343EN_L_SPI_DISABLE();
    delay_us(30);

    addr_valueR = PanelReadBuf[1];

    return PANEL_REG_OK;
}

static PanelRegStatus panel_reg_write_value(uint8_t addr, uint8_t value)
{
    uint8_t PanelWriteBuf[10];

    PanelWriteBuf[0] = addr;
    PanelWriteBuf[1] = value;

    ECX343EN_L_SPI_ENABLE();
    delay_us(30);
    if(HAL_SPI_Transmit(spi_handle_L, &PanelWriteBuf[0], PANEL_WRITE_LENGTH, PANEL_REG_RW_TIMEOUT) != HAL_OK)
    {
        return PANEL_REG_ERR;
    }
    ECX343EN_L_SPI_DISABLE();
    delay_us(30);

    ECX343EN_R_SPI_ENABLE();
    delay_us(30);
    if(HAL_SPI_Transmit(spi_handle_R, &PanelWriteBuf[0], PANEL_WRITE_LENGTH, PANEL_REG_RW_TIMEOUT) != HAL_OK)
    {
        return PANEL_REG_ERR;
    }
    ECX343EN_R_SPI_DISABLE();
    delay_us(30);

    return PANEL_REG_OK;
}

PanelRegStatus panel_reg_read(uint8_t map, uint8_t addr, uint8_t *value)
{
    //map check
    if(PanelCurrentMap != map)
    {
        //change reg map
        if(panel_reg_write_value(PANEL_ADDR_82, map) == PANEL_REG_ERR)
        {
            return PANEL_REG_ERR;
        }
        PanelCurrentMap = map;

    }

    //read panel register
    if(panel_reg_read_value(addr, value) == PANEL_REG_ERR)
    {
        return PANEL_REG_ERR;
    }

    return PANEL_REG_OK;
}

PanelRegStatus panel_reg_write(uint8_t map, uint8_t addr, uint8_t value)
{
    //map check

    if(PanelCurrentMap != map)
    {
        //change reg map
        if(panel_reg_write_value(PANEL_ADDR_82, map) == PANEL_REG_ERR)
        {
            return PANEL_REG_ERR;
        }
        PanelCurrentMap = map;

    }

    //write panel register
    if(panel_reg_write_value(addr, value) == PANEL_REG_ERR)
    {
        return PANEL_REG_ERR;
    }

    return PANEL_REG_OK;
}

void ECX343EN_Inversion(uint8_t value)
{
    /*
    default: 1000 1100
    [2]
    Selection of rightward / leftward scan
    0: Leftward scan
    1: Rightward scan (Default)
    [3]
    Selection of upward / downward scan
    0: Upward scan
    1: Downward scan (Default)
    */
    while(panel_ps_transition() == PANEL_CONT_CONTINUE)
    {
        HAL_Delay(10);
    }

    panel_reg_write(PANEL_MAP_0, SCAN_MODE, value);
    HAL_Delay(10);

    while(panel_ps_release() == PANEL_CONT_CONTINUE)
    {
        HAL_Delay(10);
    }

}

void ECX343EN_LuminanceModeSetting(uint8_t value)
{
    /*
    default: 0000 1111
    [1]
    Luminance setting
    0: Preset Mode
    1: REQUEST_LV (Default)
    [3]
    Auto adjustment of white balance in case this function is valid.
    0: Preset Mode
    1: REQUEST_LV (Default)
    */
    panel_reg_write(PANEL_MAP_0, LUMINANCE_MODE_SETTING, value);
}

void ECX343EN_ArbitraryLuminanceH(uint8_t value)
{
    /*
    0x12 [1:0] default: 0000 0001
    0x13 [7:0] default: 0101 1110
    Luminance setting register
    0001100100: 1000cd/m^2
    |
    0111110100: 5000cd/m^2
    *Be sure to enter a value between 1000 and 5000cd/ m^2.
    (Default 0101011110: 3500cd/m^2)
    */
    panel_reg_write(PANEL_MAP_0, ARBITRARY_LUMINANCE_VALUE_HIGH, value);
}

void ECX343EN_ArbitraryLuminanceL(uint8_t value)
{
    /*
    0x12 [1:0] default: 0000 0001
    0x13 [7:0] default: 0101 1110
    Luminance setting register
    0001100100: 1000cd/m^2
    |
    0111110100: 5000cd/m^2
    *Be sure to enter a value between 1000 and 5000cd/ m^2.
    (Default 0101011110: 3500cd/m^2)
    */
    panel_reg_write(PANEL_MAP_0, ARBITRARY_LUMINANCE_VALUE_LOW, value);
}

void ECX343EN_PresetLuminanceValue(uint8_t value)
{
    /*
    default: 0010 0000
    [0]
    Luminance preset mode selection
    0: 500cd/m2 (Default)
    1: 1000cd/m2
    */
    panel_reg_write(PANEL_MAP_0, PRESET_LUMINANCE_VALUE, value);
}

void ECX343EN_OrbitH(uint8_t H_value)
{
    /*
    0x05 [5:0] default: 0000 0000
    Horizontal orbit adjustment
    110000: -16 pixels
    |
    000000: center(Default)
    |
    010000: +16 pixels
    */
    panel_reg_write(PANEL_MAP_0, ORBIT_HORIZONTAL, H_value);
}

void ECX343EN_OrbitV(uint8_t V_value)
{
    /*
    0x06 [5:0] default: 0000 0000
    Vertical orbit adjustment
    110000: -16 pixels
    |
    000000: center(Default)
    |
    010000: +16 pixels
    */
    panel_reg_write(PANEL_MAP_0, ORBIT_VERTICAL, V_value);
}

const setPanelSeqTable PanelContPanelReg60HzSettingTable[] = {
//    {PANEL_MAP_0, PANEL_ADDR_01, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_02, 0x8C, 0},
//    {PANEL_MAP_0, PANEL_ADDR_03, 0x58, 0},
//    {PANEL_MAP_0, PANEL_ADDR_04, 0x03, 0},
//    {PANEL_MAP_0, PANEL_ADDR_05, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_06, 0x00, 0},
    {PANEL_MAP_0, PANEL_ADDR_07, 0x20, 0},
//    {PANEL_MAP_0, PANEL_ADDR_08, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_09, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_0A, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_0B, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_0C, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_0D, 0x10, 0},
//    {PANEL_MAP_0, PANEL_ADDR_0E, 0x44, 0},
//    {PANEL_MAP_0, PANEL_ADDR_0F, 0x00, 0},

//    {PANEL_MAP_0, PANEL_ADDR_10, 0x0F, 0},
//    {PANEL_MAP_0, PANEL_ADDR_11, 0x04, 0},
    {PANEL_MAP_0, PANEL_ADDR_12, 0x01, 0},
    {PANEL_MAP_0, PANEL_ADDR_13, 0x5E, 0},
//    {PANEL_MAP_0, PANEL_ADDR_14, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_15, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_16, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_17, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_18, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_19, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_1A, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_1B, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_1C, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_1D, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_1E, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_1F, 0x, 0},
//
//    {PANEL_MAP_0, PANEL_ADDR_20, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_21, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_22, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_23, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_24, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_25, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_26, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_27, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_28, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_29, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_2A, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_2B, 0xE0, 0},
//    {PANEL_MAP_0, PANEL_ADDR_2C, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_2D, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_2E, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_2F, 0x, 0},
//
//    {PANEL_MAP_0, PANEL_ADDR_30, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_31, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_32, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_33, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_34, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_35, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_36, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_37, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_38, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_39, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_3A, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_3B, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_3C, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_3D, 0x, 0},
    {PANEL_MAP_0, PANEL_ADDR_3E, 0x21, 0},
//    {PANEL_MAP_0, PANEL_ADDR_3F, 0x, 0},
//
//    {PANEL_MAP_0, PANEL_ADDR_40, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_41, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_42, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_43, 0x40, 0},
//    {PANEL_MAP_0, PANEL_ADDR_44, 0x29, 0},
//    {PANEL_MAP_0, PANEL_ADDR_45, 0xD9, 0},
//    {PANEL_MAP_0, PANEL_ADDR_46, 0x02, 0},
//    {PANEL_MAP_0, PANEL_ADDR_47, 0x71, 0},
//    {PANEL_MAP_0, PANEL_ADDR_48, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_49, 0x18, 0},
//    {PANEL_MAP_0, PANEL_ADDR_4A, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_4B, 0x22, 0},
//    {PANEL_MAP_0, PANEL_ADDR_4C, 0x72, 0},
//    {PANEL_MAP_0, PANEL_ADDR_4D, 0x71, 0},
//    {PANEL_MAP_0, PANEL_ADDR_4E, 0x04, 0},
//    {PANEL_MAP_0, PANEL_ADDR_4F, 0x7D, 0},

//    {PANEL_MAP_0, PANEL_ADDR_50, 0x18, 0},
//    {PANEL_MAP_0, PANEL_ADDR_51, 0x22, 0},
//    {PANEL_MAP_0, PANEL_ADDR_52, 0x73, 0},
//    {PANEL_MAP_0, PANEL_ADDR_53, 0x72, 0},
//    {PANEL_MAP_0, PANEL_ADDR_54, 0x04, 0},
//    {PANEL_MAP_0, PANEL_ADDR_55, 0x7D, 0},
//    {PANEL_MAP_0, PANEL_ADDR_56, 0x18, 0},
//    {PANEL_MAP_0, PANEL_ADDR_57, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_58, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_59, 0x, 0},
    {PANEL_MAP_0, PANEL_ADDR_5A, 0x10, 0},
//    {PANEL_MAP_0, PANEL_ADDR_5B, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_5C, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_5D, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_5E, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_5F, 0x, 0},
//
//    {PANEL_MAP_0, PANEL_ADDR_60, 0x, 0},
    {PANEL_MAP_0, PANEL_ADDR_61, 0x11, 0},
    {PANEL_MAP_0, PANEL_ADDR_62, 0x03, 0},
    {PANEL_MAP_0, PANEL_ADDR_63, 0x00, 0},
    {PANEL_MAP_0, PANEL_ADDR_64, 0xED, 0},
    {PANEL_MAP_0, PANEL_ADDR_65, 0x06, 0},
    {PANEL_MAP_0, PANEL_ADDR_66, 0x00, 0},
    {PANEL_MAP_0, PANEL_ADDR_67, 0x17, 0},
    {PANEL_MAP_0, PANEL_ADDR_68, 0xF9, 0},
    {PANEL_MAP_0, PANEL_ADDR_69, 0x00, 0},
    {PANEL_MAP_0, PANEL_ADDR_6A, 0x09, 0},
    {PANEL_MAP_0, PANEL_ADDR_6B, 0xE7, 0},
    {PANEL_MAP_0, PANEL_ADDR_6C, 0x00, 0},
    {PANEL_MAP_0, PANEL_ADDR_6D, 0xEE, 0},
    {PANEL_MAP_0, PANEL_ADDR_6E, 0xF5, 0},
//    {PANEL_MAP_0, PANEL_ADDR_6F, 0x10, 0},

    {PANEL_MAP_0, PANEL_ADDR_70, 0x19, 0},
    {PANEL_MAP_0, PANEL_ADDR_71, 0x11, 0},
    {PANEL_MAP_0, PANEL_ADDR_72, 0x06, 0},
    {PANEL_MAP_0, PANEL_ADDR_73, 0x16, 0},
//    {PANEL_MAP_0, PANEL_ADDR_74, 0x, 0},
    {PANEL_MAP_0, PANEL_ADDR_75, 0x0E, 0},
    {PANEL_MAP_0, PANEL_ADDR_76, 0x00, 0},
    {PANEL_MAP_0, PANEL_ADDR_77, 0x00, 0},
    {PANEL_MAP_0, PANEL_ADDR_78, 0x00, 0},
    {PANEL_MAP_0, PANEL_ADDR_79, 0x00, 0},
    {PANEL_MAP_0, PANEL_ADDR_7A, 0x00, 0},
    {PANEL_MAP_0, PANEL_ADDR_7B, 0x00, 0},
    {PANEL_MAP_0, PANEL_ADDR_7C, 0x00, 0},
    {PANEL_MAP_0, PANEL_ADDR_7D, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_7E, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_7F, 0x, 0},

//    {PANEL_MAP_0, PANEL_ADDR_80, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_81, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_82, 0x00, 0},
    {PANEL_MAP_0, PANEL_ADDR_83, 0x03, 0},
    {PANEL_MAP_0, PANEL_ADDR_84, 0x84, 0},
    {PANEL_MAP_0, PANEL_ADDR_85, 0x14, 0},
//    {PANEL_MAP_0, PANEL_ADDR_86, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_87, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_88, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_89, 0x04, 0},
//    {PANEL_MAP_0, PANEL_ADDR_8A, 0xE2, 0},
    {PANEL_MAP_0, PANEL_ADDR_8B, 0xDA, 0},
//    {PANEL_MAP_0, PANEL_ADDR_8C, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_8D, 0x, 0},
    {PANEL_MAP_0, PANEL_ADDR_8E, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_8F, 0x, 0},
//
//    {PANEL_MAP_0, PANEL_ADDR_90, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_91, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_92, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_93, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_94, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_95, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_96, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_97, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_98, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_99, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_9A, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_9B, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_9C, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_9D, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_9E, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_9F, 0x, 0},
//
//    {PANEL_MAP_0, PANEL_ADDR_A0, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_A1, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_A2, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_A3, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_A4, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_A5, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_A6, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_A7, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_A8, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_A9, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_AA, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_AB, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_AC, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_AD, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_AE, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_AF, 0x, 0},
//
//    {PANEL_MAP_0, PANEL_ADDR_B0, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_B1, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_B2, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_B3, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_B4, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_B5, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_B6, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_B7, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_B8, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_B9, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_BA, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_BB, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_BC, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_BD, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_BE, 0x, 0},
    {PANEL_MAP_0, PANEL_ADDR_BF, 0x08, 0},

//    {PANEL_MAP_0, PANEL_ADDR_C0, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_C1, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_C2, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_C3, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_C4, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_C5, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_C6, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_C7, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_C8, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_C9, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_CA, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_CB, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_CC, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_CD, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_CE, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_CF, 0x, 0},
//
//    {PANEL_MAP_0, PANEL_ADDR_D0, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_D1, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_D2, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_D3, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_D4, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_D5, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_D6, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_D7, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_D8, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_D9, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_DA, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_DB, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_DC, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_DD, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_DE, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_DF, 0x, 0},
//
//    {PANEL_MAP_0, PANEL_ADDR_E0, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_E1, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_E2, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_E3, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_E4, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_E5, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_E6, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_E7, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_E8, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_E9, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_EA, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_EB, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_EC, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_ED, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_EE, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_EF, 0x, 0},
//
//    {PANEL_MAP_0, PANEL_ADDR_F0, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_F1, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_F2, 0x01, 0},
//    {PANEL_MAP_0, PANEL_ADDR_F3, 0x38, 0},
//    {PANEL_MAP_0, PANEL_ADDR_F4, 0x40, 0},
    {PANEL_MAP_0, PANEL_ADDR_F5, 0xE1, 0},
//    {PANEL_MAP_0, PANEL_ADDR_F6, 0x42, 0},
//    {PANEL_MAP_0, PANEL_ADDR_F7, 0x71, 0},
    {PANEL_MAP_0, PANEL_ADDR_F8, 0xE1, 0},
//    {PANEL_MAP_0, PANEL_ADDR_F9, 0x42, 0},
//    {PANEL_MAP_0, PANEL_ADDR_FA, 0x71, 0},
    {PANEL_MAP_0, PANEL_ADDR_FB, 0xE1, 0},
//    {PANEL_MAP_0, PANEL_ADDR_FC, 0x70, 0},
    {PANEL_MAP_0, PANEL_ADDR_FD, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_FE, 0xFF, 0},
    {PANEL_TABLE_END, 0, 0, 0},

};

const setPanelSeqTable PanelContPanelReg120HzSettingTable[] = {
//    {PANEL_MAP_0, PANEL_ADDR_01, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_02, 0x8C, 0},
//    {PANEL_MAP_0, PANEL_ADDR_03, 0xD9, 0},
//    {PANEL_MAP_0, PANEL_ADDR_04, 0x05, 0},
//    {PANEL_MAP_0, PANEL_ADDR_05, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_06, 0x00, 0},
    {PANEL_MAP_0, PANEL_ADDR_07, 0x20, 0},
//    {PANEL_MAP_0, PANEL_ADDR_08, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_09, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_0A, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_0B, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_0C, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_0D, 0x10, 0},
//    {PANEL_MAP_0, PANEL_ADDR_0E, 0x44, 0},
//    {PANEL_MAP_0, PANEL_ADDR_0F, 0x00, 0},

//    {PANEL_MAP_0, PANEL_ADDR_10, 0x0F, 0},
//    {PANEL_MAP_0, PANEL_ADDR_11, 0x04, 0},
    {PANEL_MAP_0, PANEL_ADDR_12, 0x01, 0},
    {PANEL_MAP_0, PANEL_ADDR_13, 0x5E, 0},
//    {PANEL_MAP_0, PANEL_ADDR_14, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_15, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_16, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_17, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_18, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_19, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_1A, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_1B, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_1C, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_1D, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_1E, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_1F, 0x, 0},
//
//    {PANEL_MAP_0, PANEL_ADDR_20, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_21, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_22, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_23, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_24, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_25, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_26, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_27, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_28, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_29, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_2A, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_2B, 0xE0, 0},
//    {PANEL_MAP_0, PANEL_ADDR_2C, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_2D, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_2E, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_2F, 0x, 0},
//
//    {PANEL_MAP_0, PANEL_ADDR_30, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_31, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_32, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_33, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_34, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_35, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_36, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_37, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_38, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_39, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_3A, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_3B, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_3C, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_3D, 0x, 0},
    {PANEL_MAP_0, PANEL_ADDR_3E, 0x21, 0},
//    {PANEL_MAP_0, PANEL_ADDR_3F, 0x, 0},
//
//    {PANEL_MAP_0, PANEL_ADDR_40, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_41, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_42, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_43, 0x40, 0},
//    {PANEL_MAP_0, PANEL_ADDR_44, 0x29, 0},
//    {PANEL_MAP_0, PANEL_ADDR_45, 0xD9, 0},
//    {PANEL_MAP_0, PANEL_ADDR_46, 0x02, 0},
//    {PANEL_MAP_0, PANEL_ADDR_47, 0x71, 0},
//    {PANEL_MAP_0, PANEL_ADDR_48, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_49, 0x18, 0},
//    {PANEL_MAP_0, PANEL_ADDR_4A, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_4B, 0x22, 0},
//    {PANEL_MAP_0, PANEL_ADDR_4C, 0x72, 0},
//    {PANEL_MAP_0, PANEL_ADDR_4D, 0x71, 0},
//    {PANEL_MAP_0, PANEL_ADDR_4E, 0x04, 0},
//    {PANEL_MAP_0, PANEL_ADDR_4F, 0x7D, 0},

//    {PANEL_MAP_0, PANEL_ADDR_50, 0x18, 0},
//    {PANEL_MAP_0, PANEL_ADDR_51, 0x22, 0},
//    {PANEL_MAP_0, PANEL_ADDR_52, 0x73, 0},
//    {PANEL_MAP_0, PANEL_ADDR_53, 0x72, 0},
//    {PANEL_MAP_0, PANEL_ADDR_54, 0x04, 0},
//    {PANEL_MAP_0, PANEL_ADDR_55, 0x7D, 0},
//    {PANEL_MAP_0, PANEL_ADDR_56, 0x18, 0},
//    {PANEL_MAP_0, PANEL_ADDR_57, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_58, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_59, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_5A, 0x19, 0},
//    {PANEL_MAP_0, PANEL_ADDR_5B, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_5C, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_5D, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_5E, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_5F, 0x, 0},
//
//    {PANEL_MAP_0, PANEL_ADDR_60, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_61, 0x1F, 0},
//    {PANEL_MAP_0, PANEL_ADDR_62, 0x06, 0},
//    {PANEL_MAP_0, PANEL_ADDR_63, 0x01, 0},
//    {PANEL_MAP_0, PANEL_ADDR_64, 0xD7, 0},
//    {PANEL_MAP_0, PANEL_ADDR_65, 0x0B, 0},
//    {PANEL_MAP_0, PANEL_ADDR_66, 0x10, 0},
//    {PANEL_MAP_0, PANEL_ADDR_67, 0x2A, 0},
//    {PANEL_MAP_0, PANEL_ADDR_68, 0xEF, 0},
//    {PANEL_MAP_0, PANEL_ADDR_69, 0x10, 0},
//    {PANEL_MAP_0, PANEL_ADDR_6A, 0x0E, 0},
//    {PANEL_MAP_0, PANEL_ADDR_6B, 0xCB, 0},
//    {PANEL_MAP_0, PANEL_ADDR_6C, 0x11, 0},
//    {PANEL_MAP_0, PANEL_ADDR_6D, 0xDA, 0},
//    {PANEL_MAP_0, PANEL_ADDR_6E, 0xE7, 0},
//    {PANEL_MAP_0, PANEL_ADDR_6F, 0x10, 0},

//    {PANEL_MAP_0, PANEL_ADDR_70, 0x32, 0},
//    {PANEL_MAP_0, PANEL_ADDR_71, 0x22, 0},
//    {PANEL_MAP_0, PANEL_ADDR_72, 0x0B, 0},
//    {PANEL_MAP_0, PANEL_ADDR_73, 0x2C, 0},
//    {PANEL_MAP_0, PANEL_ADDR_74, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_75, 0x1C, 0},
    {PANEL_MAP_0, PANEL_ADDR_76, 0x00, 0},
    {PANEL_MAP_0, PANEL_ADDR_77, 0x00, 0},
    {PANEL_MAP_0, PANEL_ADDR_78, 0x00, 0},
    {PANEL_MAP_0, PANEL_ADDR_79, 0x00, 0},
    {PANEL_MAP_0, PANEL_ADDR_7A, 0x00, 0},
    {PANEL_MAP_0, PANEL_ADDR_7B, 0x00, 0},
    {PANEL_MAP_0, PANEL_ADDR_7C, 0x00, 0},
    {PANEL_MAP_0, PANEL_ADDR_7D, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_7E, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_7F, 0x, 0},

//    {PANEL_MAP_0, PANEL_ADDR_80, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_81, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_82, 0x00, 0},
    {PANEL_MAP_0, PANEL_ADDR_83, 0x03, 0},
    {PANEL_MAP_0, PANEL_ADDR_84, 0x84, 0},
    {PANEL_MAP_0, PANEL_ADDR_85, 0x14, 0},
//    {PANEL_MAP_0, PANEL_ADDR_86, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_87, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_88, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_89, 0x04, 0},
//    {PANEL_MAP_0, PANEL_ADDR_8A, 0xE2, 0},
    {PANEL_MAP_0, PANEL_ADDR_8B, 0xDA, 0},
//    {PANEL_MAP_0, PANEL_ADDR_8C, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_8D, 0x, 0},
    {PANEL_MAP_0, PANEL_ADDR_8E, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_8F, 0x, 0},
//
//    {PANEL_MAP_0, PANEL_ADDR_90, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_91, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_92, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_93, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_94, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_95, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_96, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_97, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_98, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_99, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_9A, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_9B, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_9C, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_9D, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_9E, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_9F, 0x, 0},
//
//    {PANEL_MAP_0, PANEL_ADDR_A0, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_A1, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_A2, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_A3, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_A4, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_A5, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_A6, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_A7, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_A8, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_A9, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_AA, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_AB, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_AC, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_AD, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_AE, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_AF, 0x, 0},
//
//    {PANEL_MAP_0, PANEL_ADDR_B0, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_B1, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_B2, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_B3, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_B4, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_B5, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_B6, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_B7, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_B8, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_B9, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_BA, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_BB, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_BC, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_BD, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_BE, 0x, 0},
    {PANEL_MAP_0, PANEL_ADDR_BF, 0x08, 0},

//    {PANEL_MAP_0, PANEL_ADDR_C0, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_C1, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_C2, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_C3, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_C4, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_C5, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_C6, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_C7, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_C8, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_C9, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_CA, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_CB, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_CC, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_CD, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_CE, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_CF, 0x, 0},
//
//    {PANEL_MAP_0, PANEL_ADDR_D0, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_D1, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_D2, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_D3, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_D4, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_D5, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_D6, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_D7, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_D8, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_D9, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_DA, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_DB, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_DC, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_DD, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_DE, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_DF, 0x, 0},
//
//    {PANEL_MAP_0, PANEL_ADDR_E0, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_E1, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_E2, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_E3, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_E4, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_E5, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_E6, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_E7, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_E8, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_E9, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_EA, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_EB, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_EC, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_ED, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_EE, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_EF, 0x, 0},
//
//    {PANEL_MAP_0, PANEL_ADDR_F0, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_F1, 0x, 0},
//    {PANEL_MAP_0, PANEL_ADDR_F2, 0x01, 0},
//    {PANEL_MAP_0, PANEL_ADDR_F3, 0x38, 0},
//    {PANEL_MAP_0, PANEL_ADDR_F4, 0x40, 0},
    {PANEL_MAP_0, PANEL_ADDR_F5, 0xE1, 0},
//    {PANEL_MAP_0, PANEL_ADDR_F6, 0x42, 0},
//    {PANEL_MAP_0, PANEL_ADDR_F7, 0x71, 0},
    {PANEL_MAP_0, PANEL_ADDR_F8, 0xE1, 0},
//    {PANEL_MAP_0, PANEL_ADDR_F9, 0x42, 0},
//    {PANEL_MAP_0, PANEL_ADDR_FA, 0x71, 0},
    {PANEL_MAP_0, PANEL_ADDR_FB, 0xE1, 0},
//    {PANEL_MAP_0, PANEL_ADDR_FC, 0x70, 0},
    {PANEL_MAP_0, PANEL_ADDR_FD, 0x00, 0},
//    {PANEL_MAP_0, PANEL_ADDR_FE, 0xFF, 0},
    {PANEL_TABLE_END, 0, 0, 0},

};

const setPanelSeqTable PanelContLuminance60HzSettingTable[] = {
    {PANEL_MAP_0, PANEL_ADDR_10, 0x0F, 0},
    {PANEL_MAP_0, PANEL_ADDR_02, 0x8C, 0},
    {PANEL_MAP_0, PANEL_ADDR_03, 0x58, 0},
    {PANEL_MAP_0, PANEL_ADDR_04, 0x03, 0},
    {PANEL_TABLE_END, 0, 0, 0},
};

const setPanelSeqTable PanelContLuminance120HzSettingTable[] = {
    {PANEL_MAP_0, PANEL_ADDR_10, 0x0F, 0},
    {PANEL_MAP_0, PANEL_ADDR_02, 0x8C, 0},
    {PANEL_MAP_0, PANEL_ADDR_03, 0xD9, 0},
    {PANEL_MAP_0, PANEL_ADDR_04, 0x05, 0},
    {PANEL_TABLE_END, 0, 0, 0},
};

const setPanelSeqTable PanelContPSReleaseTable[] = {
    {PANEL_MAP_0, PANEL_ADDR_00, 0x01, 0},
    {PANEL_MAP_0, PANEL_ADDR_00, 0x03, 0},
    {PANEL_MAP_0, PANEL_ADDR_00, 0x07, 0},
    {PANEL_TABLE_END, 0, 0, 0},
};

const setPanelSeqTable PanelContPSTransitionTable[] = {
    {PANEL_MAP_0, PANEL_ADDR_00, 0x03, 0},
    {PANEL_MAP_0, PANEL_ADDR_00, 0x01, 0},
    {PANEL_MAP_0, PANEL_ADDR_00, 0x00, 0},
    {PANEL_TABLE_END, 0, 0, 0},
};
