//
// Created by xinfeng.weng on 2025/12/24.
//

#ifndef ALGORITHMLIBRARY_PL16_H
#define ALGORITHMLIBRARY_PL16_H
#include "../public.h"
#include "AlpMPAlgoCTypeInterface.h"
int calcPL16()
{
	//导入数据
	std::string F1  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL16_4000-12000Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F1_W4080H352P10_T2K-CP2-4S-LY_20251205172628.raw";
	std::string F2  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL16_4000-12000Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F2_W4080H352P10_T2K-CP2-4S-LY_20251205172628.raw";
	std::string F3  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL16_4000-12000Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F3_W4080H352P10_T2K-CP2-4S-LY_20251205172628.raw";
	std::string F4  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL16_4000-12000Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F4_W4080H352P10_T2K-CP2-4S-LY_20251205172628.raw";
 	std::string F5  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL16_4000-12000Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F5_W4080H352P10_T2K-CP2-4S-LY_20251205172628.raw";
 	std::string F6  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL16_4000-12000Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F6_W4080H352P10_T2K-CP2-4S-LY_20251205172628.raw";
 	std::string F7  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL16_4000-12000Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F7_W4080H352P10_T2K-CP2-4S-LY_20251205172628.raw";
 	std::string F8  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL16_4000-12000Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F8_W4080H352P10_T2K-CP2-4S-LY_20251205172628.raw";
 	std::string F9  = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL16_4000-12000Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F9_W4080H352P10_T2K-CP2-4S-LY_20251205172628.raw";
 	std::string F10 = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL16_4000-12000Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F10_W4080H352P10_T2K-CP2-4S-LY_20251205172628.raw";
 	std::string F11 = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL16_4000-12000Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F11_W4080H352P10_T2K-CP2-4S-LY_20251205172628.raw";
 	std::string F12 = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL16_4000-12000Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F12_W4080H352P10_T2K-CP2-4S-LY_20251205172628.raw";
 	std::string F13 = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL16_4000-12000Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F13_W4080H352P10_T2K-CP2-4S-LY_20251205172628.raw";
 	std::string F14 = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL16_4000-12000Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F14_W4080H352P10_T2K-CP2-4S-LY_20251205172628.raw";
 	std::string F15 = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL16_4000-12000Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F15_W4080H352P10_T2K-CP2-4S-LY_20251205172628.raw";
 	std::string F16 = "D:/Work/Tmp/APX003CA/TestData_CP2/APX003CB_EVS_513331333139382D0503327980852E020324_X121Y128_S2_300FPS_5000K_EVS_PL16_4000-12000Lux_TD0p2-XVS37p332ms-1p5mV_VTX1P5V_30C_G2_F16_W4080H352P10_T2K-CP2-4S-LY_20251205172628.raw";

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

    // 3. 调用DLL函数
    HANDLE handle = InitHandleDVS(SensorType::ALP_003CA, "./", BayerGBRG, 1);
    //cout << "句柄：" << handle << endl;
    DVSAlgorithmThre DVSAlgoThre_new;
    check_ret(__func__, GetAlgorithmThreDVS(handle, &DVSAlgoThre_new));
    DVSAlgoThre_new.nPeakCycle = 10;
    check_ret(__func__, SetAlgorithmThreDVS(handle, &DVSAlgoThre_new));
    check_ret(__func__, SetMultiThreadEnableDVS(handle, true));
    //pSetLogEnableDVS(handle, true);

    int framelength = nWidth * nHeight / EVS_cnt;
    int nErrCode = 0;
    if (ImportRawDataDVS(handle, pbRawData + nWidth * 8 + framelength * DropFrameN, framelength * UseFrameN, 0,
                         UseFrameN)) {
        std::cout << "Error: fail tp ImportRawdata.\r\n";
        return 1;
    }

    int ImageCalRes = 0;
    uint32_t nEvsSaveNum = UseFrameN; //100
    DVSBadpixelType BadpixelRes;
    uint32_t nPeakNum = 3;
    DVSPeakInfo PeakInfoRes;
    check_ret(__func__, FindPeakDVS(handle, 0, nEvsSaveNum, 6, &PeakInfoRes, On_OffEvents));
    int onNum = PeakInfoRes.nOnEventsPeakNumber;
    int offNum = PeakInfoRes.nOffEventsPeakNumber;

    if (check_ret(__func__, BadPixelDVS(handle, 0, nEvsSaveNum, &PeakInfoRes, nPeakNum, On_OffEvents, &BadpixelRes))) {
        setResult("BadpixelRes_nOffEventsDeadPixelNum_PL16", (double) BadpixelRes.nOffEventsDeadPixelNum);
        setResult("BadpixelRes_nOffEventsClusterNum_PL16", (double) BadpixelRes.nOffEventsClusterNum);
        setResult("BadpixelRes_nOnEventsDeadPixelNum_PL16", (double) BadpixelRes.nOnEventsDeadPixelNum);
        setResult("BadpixelRes_nOnEventsClusterNum_PL16", (double) BadpixelRes.nOnEventsClusterNum);
    } else
        return -1;


    DeleteHandleDVS(handle);
    delete [] rawDataBuf;
    rawDataBuf = NULL;
    delete [] pbRawData;
    pbRawData = NULL;
    return 0;
}
#endif //ALGORITHMLIBRARY_PL16_H
