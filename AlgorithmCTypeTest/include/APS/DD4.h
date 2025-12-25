//
// Created by xinfeng.weng on 2025/12/25.
//

#ifndef ALGORITHMLIBRARY_DD4_H
#define ALGORITHMLIBRARY_DD4_H
#include "public.h"
#include "AlpMPAlgoCTypeInterface.h"

int calcDD4()
{
    printf("\n - APS DD4 Start\r\n");
	//导入数据
	std::string F1 = "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_4FPS_5000K_Dark_DD4_0Lux_233p0ms_VTX1P5V_60C_G16_F1_W3264H2448P10_T2K-CP1-4S-LY_20251013220527.raw";
	std::string F2 = "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_4FPS_5000K_Dark_DD4_0Lux_233p0ms_VTX1P5V_60C_G16_F2_W3264H2448P10_T2K-CP1-4S-LY_20251013220527.raw";
	std::string F3 = "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_4FPS_5000K_Dark_DD4_0Lux_233p0ms_VTX1P5V_60C_G16_F3_W3264H2448P10_T2K-CP1-4S-LY_20251013220527.raw";
	std::string F4 = "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_4FPS_5000K_Dark_DD4_0Lux_233p0ms_VTX1P5V_60C_G16_F4_W3264H2448P10_T2K-CP1-4S-LY_20251013220527.raw";
 	std::string F5 = "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_4FPS_5000K_Dark_DD4_0Lux_233p0ms_VTX1P5V_60C_G16_F5_W3264H2448P10_T2K-CP1-4S-LY_20251013220527.raw";
    int nWidth = 3264;
    int nHeight = 2448;
    uint32_t nNumber = 5;
    long rawDataBufLen = nWidth * nHeight * 2;
    unsigned long rawDataRealLen = 0;
    uint8_t *rawDataBuf = new uint8_t[rawDataBufLen * nNumber];
    ImageCapture_capture(F1, rawDataBuf, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F2, rawDataBuf + rawDataBufLen * 1, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F3, rawDataBuf + rawDataBufLen * 2, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F4, rawDataBuf + rawDataBufLen * 3, rawDataBufLen, rawDataRealLen, 0);
    ImageCapture_capture(F5, rawDataBuf + rawDataBufLen * 4, rawDataBufLen, rawDataRealLen, 0);

    // 3. 调用DLL函数
    HANDLE handle = InitHandleAPS(SensorType::ALP_003CA, APSRawType::UNPACK10, "./", PixelFormatType::QuadBayerGBRG, 0);
    //cout << "句柄：" << handle << endl;
    APSAlgorithmThre AlgoThre;
    check_ret(__func__, GetAlgorithmThreAPS(handle, &AlgoThre));
    AlgoThre.nBadPixelLocalRowOffset = 104;
    AlgoThre.nBadPixelLocalColOffset = 52;
    check_ret(__func__, SetAlgorithmThreAPS(handle, &AlgoThre));
    uint32_t get_nHeight, get_nWidth;
    check_ret(__func__, GetRawDataSizeAPS(handle, &get_nHeight, &get_nWidth));
    check_ret(__func__, SetRawDataSizeAPS(handle, nHeight, nWidth));
    check_ret(__func__, SetMultiThreadEnableAPS(handle, true));
    //pSetLogEnableAPS(handle, true);

    ROIArea area;
    area.Down = nHeight / 2 - 1;
    area.Up = 0;
    area.Left = 0;
    area.Right = nWidth / 2 - 1;
    check_ret(__func__, SetActiveAreaAPS(handle, area));
    //pGetActiveAreaAPS(handle, area);

    int nErrCode = 0;
    for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
        check_ret(__func__, ImportRawDataAPS(handle, rawDataBuf + rawDataBufLen * nIndex, nWidth * nHeight * 2, nIndex,
                                             1));

    APSDSNUType DSNURes;
    if (check_ret(__func__, DSNUAPS(handle, 0, nNumber, nullptr, DSNURes))) {
        setResult("DSNU_RangeR_DD4", (double) DSNURes.RangeR);
        setResult("DSNU_RangeG_DD4", (double) DSNURes.RangeG);
        setResult("DSNU_RangeB_DD4", (double) DSNURes.RangeB);
        setResult("DSNU_SignalMax_DD4", (double) DSNURes.SignalMax);
        setResult("DSNU_DeltaSignalMax_DD4", (double) DSNURes.DeltaSignalMax);
        setResult("DSNU_DeltaSignalCentreMax_DD4", (double) DSNURes.DeltaSignalCentreMax);
        setResult("DSNU_DeltaSignalEdgeMax_DD4", (double) DSNURes.DeltaSignalEdgeMax);
        setResult("DSNU_DeltaSignalCornerMax_DD4", (double) DSNURes.DeltaSignalCornerMax);
        setResult("DSNU_Rmax_DD4", DSNURes.RMax);
        setResult("DSNU_Rmin_DD4", DSNURes.RMin);
        setResult("DSNU_Gmax_DD4", DSNURes.GMax);
        setResult("DSNU_Gmin_DD4", DSNURes.GMin);
        setResult("DSNU_Bmax_DD4", DSNURes.BMax);
        setResult("DSNU_Bmin_DD4", DSNURes.BMin);
    } else
        return -1;

    DeleteHandleAPS(handle);
    delete [] rawDataBuf;
    rawDataBuf = NULL;
    return 0;
}
#endif //ALGORITHMLIBRARY_DD4_H
