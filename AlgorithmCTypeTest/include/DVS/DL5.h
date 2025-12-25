//
// Created by xinfeng.weng on 2025/12/24.
//
#ifndef ALGORITHMLIBRARY_CALCDL5_CPP_H
#define ALGORITHMLIBRARY_CALCDL5_CPP_H

#include "public.h"
#include "AlpMPAlgoCTypeInterface.h"

int calcDL5() {
    //导入数据
    std::string F1  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_DL5_0-0Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F1_W4080H352P10_T2K-CP2-4S-LY_20251205172620.raw";
    std::string F2  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_DL5_0-0Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F2_W4080H352P10_T2K-CP2-4S-LY_20251205172620.raw";
    std::string F3  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_DL5_0-0Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F3_W4080H352P10_T2K-CP2-4S-LY_20251205172620.raw";
    std::string F4  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_DL5_0-0Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F4_W4080H352P10_T2K-CP2-4S-LY_20251205172620.raw";
    std::string F5  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_DL5_0-0Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F5_W4080H352P10_T2K-CP2-4S-LY_20251205172620.raw";
    std::string F6  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_DL5_0-0Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F6_W4080H352P10_T2K-CP2-4S-LY_20251205172620.raw";
    std::string F7  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_DL5_0-0Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F7_W4080H352P10_T2K-CP2-4S-LY_20251205172620.raw";
    std::string F8  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_DL5_0-0Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F8_W4080H352P10_T2K-CP2-4S-LY_20251205172620.raw";
    std::string F9  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_DL5_0-0Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F9_W4080H352P10_T2K-CP2-4S-LY_20251205172620.raw";
    std::string F10 = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_DL5_0-0Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F10_W4080H352P10_T2K-CP2-4S-LY_20251205172620.raw";
    std::string F11 = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_DL5_0-0Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F11_W4080H352P10_T2K-CP2-4S-LY_20251205172620.raw";
    std::string F12 = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_DL5_0-0Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F12_W4080H352P10_T2K-CP2-4S-LY_20251205172620.raw";
    std::string F13 = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_DL5_0-0Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F13_W4080H352P10_T2K-CP2-4S-LY_20251205172620.raw";
    std::string F14 = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_DL5_0-0Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F14_W4080H352P10_T2K-CP2-4S-LY_20251205172620.raw";
    std::string F15 = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_DL5_0-0Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F15_W4080H352P10_T2K-CP2-4S-LY_20251205172620.raw";
    std::string F16 = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_DL5_0-0Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F16_W4080H352P10_T2K-CP2-4S-LY_20251205172620.raw";

    int nWidth = 4080;
    int nHeight = 352;
    uint32_t nNumber = 16;
    uint8_t DropFrameN = 0;
    uint8_t UseFrameN = 165;
    uint8_t EVS_cnt = 11;

    long rawDataBufLen = nWidth * nHeight * 2;
    unsigned long rawDataRealLen = 0;
    uint8_t *rawDataBuf = new uint8_t[rawDataBufLen * nNumber];
    ImageCapture_capture(F1, rawDataBuf, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F2, rawDataBuf + rawDataBufLen * 1, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F3, rawDataBuf + rawDataBufLen * 2, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F4, rawDataBuf + rawDataBufLen * 3, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F5, rawDataBuf + rawDataBufLen * 4, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F6, rawDataBuf + rawDataBufLen * 5, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F7, rawDataBuf + rawDataBufLen * 6, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F8, rawDataBuf + rawDataBufLen * 7, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F9, rawDataBuf + rawDataBufLen * 8, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F10, rawDataBuf + rawDataBufLen * 9, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F11, rawDataBuf + rawDataBufLen * 10, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F12, rawDataBuf + rawDataBufLen * 11, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F13, rawDataBuf + rawDataBufLen * 12, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F14, rawDataBuf + rawDataBufLen * 13, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F15, rawDataBuf + rawDataBufLen * 14, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F16, rawDataBuf + rawDataBufLen * 15, rawDataBufLen, rawDataRealLen, 0);
    uint8_t *pbRawData = new uint8_t[nWidth * nHeight * nNumber];
    for (int i = 0; i < nWidth * nHeight * nNumber; i++) {
        *(pbRawData + i) = *(rawDataBuf + i * 2);
    }

    int ret{0};
    // auto check_ret = [&](std::string func, int func_ret) -> bool  {
    //     if (func_ret!=0) {
    //         std::cout<<func<<" error ret:"<<func_ret<<std::endl;
    //         return false;
    //     }
    //     return true;
    // };
    HANDLE handle = InitHandleDVS(SensorType::ALP_003CA, "./", BayerGBRG, 1);
    DVSAlgorithmThre DVSAlgoThre_new;
    check_ret(__func__, GetAlgorithmThreDVS(handle, &DVSAlgoThre_new));
    DVSAlgoThre_new.nPeakCycle = 10;
    check_ret(__func__, SetAlgorithmThreDVS(handle, &DVSAlgoThre_new));
    int framelength = nWidth * nHeight / EVS_cnt;
    check_ret(__func__, ImportRawDataDVS(handle, pbRawData + nWidth * 8 + framelength * DropFrameN,
                                         framelength * UseFrameN, 0, UseFrameN));

    uint32_t nIndexStart = 0;
    DVSEventsNumberCountType EventsNumberCountRes;
    check_ret(__func__, EventsNumberCountDVS(handle, nIndexStart, UseFrameN, &EventsNumberCountRes));

    DVSPeakInfo PeakInfoRes;
    check_ret(__func__, FindPeakDVS(handle, 0, UseFrameN, 6, &PeakInfoRes, On_OffEvents));
    if (PeakInfoRes.nOnEventsPeakNumber >= 3 && PeakInfoRes.nOffEventsPeakNumber >= 3) {
        DVSImageContrastSensitivityType ImageContrastSensitivityRes;
        uint32_t nPeakNum = 3;
        if (check_ret(__func__, ImageContrastSensitivityDVS(handle, 0, UseFrameN, &PeakInfoRes, nPeakNum, On_OffEvents,
                                                            &ImageContrastSensitivityRes))) {
            setResult("OnEvents_EventsRatio_All_DL5", (double) ImageContrastSensitivityRes.OnEventsRatio[All]);
            setResult("OnEvents_EventsRatio_Gb_DL5", (double) ImageContrastSensitivityRes.OnEventsRatio[Gb]);
            setResult("OnEvents_EventsRatio_B_DL5", (double) ImageContrastSensitivityRes.OnEventsRatio[B]);
            setResult("OnEvents_EventsRatio_R_DL5", (double) ImageContrastSensitivityRes.OnEventsRatio[R]);
            setResult("OnEvents_EventsRatio_Gr_DL5", (double) ImageContrastSensitivityRes.OnEventsRatio[Gr]);
            setResult("OnEvents_EventsRatio_R_Gb_DL5", (double) ImageContrastSensitivityRes.R_Gb_OnEventsRatio);
            setResult("OnEvents_EventsRatio_B_Gb_DL5", (double) ImageContrastSensitivityRes.B_Gb_OnEventsRatio);
            setResult("OnEvents_EventsRatio_Gr_Gb_DL5", (double) ImageContrastSensitivityRes.Gr_Gb_OnEventsRatio);
            setResult("OffEvents_EventsRatio_All_DL5", (double) ImageContrastSensitivityRes.OffEventsRatio[All]);
            setResult("OffEvents_EventsRatio_Gb_DL5", (double) ImageContrastSensitivityRes.OffEventsRatio[Gb]);
            setResult("OffEvents_EventsRatio_B_DL5", (double) ImageContrastSensitivityRes.OffEventsRatio[B]);
            setResult("OffEvents_EventsRatio_R_DL5", (double) ImageContrastSensitivityRes.OffEventsRatio[R]);
            setResult("OffEvents_EventsRatio_Gr_DL5", (double) ImageContrastSensitivityRes.OffEventsRatio[Gr]);
            setResult("OffEvents_EventsRatio_R_Gb_DL5", (double) ImageContrastSensitivityRes.R_Gb_OffEventsRatio);
            setResult("OffEvents_EventsRatio_B_Gb_DL5", (double) ImageContrastSensitivityRes.B_Gb_OffEventsRatio);
            setResult("OffEvents_EventsRatio_Gr_Gb_DL5", (double) ImageContrastSensitivityRes.Gr_Gb_OffEventsRatio);
        } else
            return -1;
    }
    DVSStationaryNoiseType StationaryNoiseRes;
    if (check_ret(__func__, StationaryNoiseDVS(handle, 0, UseFrameN, &StationaryNoiseRes))) {
        setResult("StationaryNoise_DL5_Mean_All", (double) StationaryNoiseRes.dStationaryNoiseMeanAll);
        setResult("StationaryNoise_DL5_Mean_On", (double) StationaryNoiseRes.dStationaryNoiseMeanOn);
        setResult("StationaryNoise_DL5_Mean_Off", (double) StationaryNoiseRes.dStationaryNoiseMeanOff);
        setResult("StationaryNoise_DL5_Std_All", (double) StationaryNoiseRes.dStationaryNoiseStdAll);
        setResult("StationaryNoise_DL5_Std_On", (double) StationaryNoiseRes.dStationaryNoiseStdOn);
        setResult("StationaryNoise_DL5_Std_Off", (double) StationaryNoiseRes.dStationaryNoiseStdOff);
        setResult("StationaryNoise_DL5_RowNoise", (double) StationaryNoiseRes.dStationaryRowTNoise);
        setResult("StationaryNoise_DL5_ColNoise", (double) StationaryNoiseRes.dStationaryColTNoise);
        setResult("StationaryNoise_DL5_nFlashFrameNum", (double) StationaryNoiseRes.nFlashFrameNumber);
        setResult("StationaryNoise_DL5_dMaxNoise", (double) StationaryNoiseRes.dMaxStationaryNoise);
    } else
        return -1;

    DVSStationaryUniformityType UniformityRes;
    if (check_ret(__func__, StationaryUniformityDVS(handle, 0, UseFrameN, &UniformityRes))) {
        setResult("UniformityBlockData_DL5_0_0", (double) UniformityRes.UniformityBlockData[0][0]);
        setResult("UniformityBlockData_DL5_0_1", (double) UniformityRes.UniformityBlockData[0][1]);
        setResult("UniformityBlockData_DL5_0_2", (double) UniformityRes.UniformityBlockData[0][2]);
        setResult("UniformityBlockData_DL5_0_3", (double) UniformityRes.UniformityBlockData[0][3]);
        setResult("UniformityBlockData_DL5_0_4", (double) UniformityRes.UniformityBlockData[0][4]);
        setResult("UniformityBlockData_DL5_1_0", (double) UniformityRes.UniformityBlockData[1][0]);
        setResult("UniformityBlockData_DL5_1_1", (double) UniformityRes.UniformityBlockData[1][1]);
        setResult("UniformityBlockData_DL5_1_2", (double) UniformityRes.UniformityBlockData[1][2]);
        setResult("UniformityBlockData_DL5_1_3", (double) UniformityRes.UniformityBlockData[1][3]);
        setResult("UniformityBlockData_DL5_1_4", (double) UniformityRes.UniformityBlockData[1][4]);
        setResult("UniformityBlockData_DL5_2_0", (double) UniformityRes.UniformityBlockData[2][0]);
        setResult("UniformityBlockData_DL5_2_1", (double) UniformityRes.UniformityBlockData[2][1]);
        setResult("UniformityBlockData_DL5_2_2", (double) UniformityRes.UniformityBlockData[2][2]);
        setResult("UniformityBlockData_DL5_2_3", (double) UniformityRes.UniformityBlockData[2][3]);
        setResult("UniformityBlockData_DL5_2_4", (double) UniformityRes.UniformityBlockData[2][4]);
        setResult("UniformityBlockData_DL5_3_0", (double) UniformityRes.UniformityBlockData[3][0]);
        setResult("UniformityBlockData_DL5_3_1", (double) UniformityRes.UniformityBlockData[3][1]);
        setResult("UniformityBlockData_DL5_3_2", (double) UniformityRes.UniformityBlockData[3][2]);
        setResult("UniformityBlockData_DL5_3_3", (double) UniformityRes.UniformityBlockData[3][3]);
        setResult("UniformityBlockData_DL5_3_4", (double) UniformityRes.UniformityBlockData[3][4]);
        setResult("UniformityBlockData_DL5_4_0", (double) UniformityRes.UniformityBlockData[4][0]);
        setResult("UniformityBlockData_DL5_4_1", (double) UniformityRes.UniformityBlockData[4][1]);
        setResult("UniformityBlockData_DL5_4_2", (double) UniformityRes.UniformityBlockData[4][2]);
        setResult("UniformityBlockData_DL5_4_3", (double) UniformityRes.UniformityBlockData[4][3]);
        setResult("UniformityBlockData_DL5_4_4", (double) UniformityRes.UniformityBlockData[4][4]);
        setResult("UniformityRatio_DL5", (double) UniformityRes.UniformityRatio);
    } else
        return -1;

    DVSHotpixelType HotpixelRes;
    if (check_ret(__func__, HotPixelDVS(handle, 0, UseFrameN, &HotpixelRes))) {
        setResult("HotPixel_HotPixelNum_DL5", (double) HotpixelRes.HotPixelNum);
        setResult("HotPixel_HotLineNum_DL5", (double) HotpixelRes.HotLineNum);
        setResult("HotPixel_SingletNum_DL5", (double) HotpixelRes.SingletNum);
        setResult("HotPixel_CoupletNum_DL5", (double) HotpixelRes.CoupletNum);
        setResult("HotPixel_TripletNum_DL5", (double) HotpixelRes.TripletNum);
        setResult("HotPixel_FourConnectedNum_DL5", (double) HotpixelRes.FourConnectedNum);
        setResult("HotPixel_ClusterNum_DL5", (double) HotpixelRes.ClusterNum);
    } else
        return -1;

    delete [] rawDataBuf;
    rawDataBuf = NULL;
    delete [] pbRawData;
    pbRawData = NULL;

    DeleteHandleDVS(handle);
    return 0;
}

#endif //ALGORITHMLIBRARY_CALCDL5_CPP_H
