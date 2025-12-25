//
// Created by xinfeng.weng on 2025/12/25.
//

#ifndef ALGORITHMLIBRARY_DD1_H
#define ALGORITHMLIBRARY_DD1_H
#include "public.h"
#include "AlpMPAlgoCTypeInterface.h"

int calcDD1() {
    printf("\n - APS DD1 Start\r\n");
    //导入数据
    std::string F1 =
            "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_25FPS_5000K_Dark_DD1_0Lux_33p0ms_VTX1P5V_60C_G1_F1_W3376H2552P10_T2K-CP1-4S-LY_20251013220522.raw";
    int nWidth = 3376;
    int nHeight = 2552;
    uint32_t nNumber = 1;
    long rawDataBufLen = nWidth * nHeight * 2;
    unsigned long rawDataRealLen = 0;
    uint8_t *rawDataBuf = new uint8_t[rawDataBufLen * nNumber];
    check_ret(__func__, ImageCapture_capture(F1, rawDataBuf, rawDataBufLen, rawDataRealLen, 0));

    // 3. 调用DLL函数
    HANDLE handle = InitHandleAPS(SensorType::ALP_003CA, APSRawType::UNPACK10, "./", PixelFormatType::QuadBayerGBRG,
                                   0);
    //cout << "句柄：" << handle << endl;
    APSAlgorithmThre AlgoThre;
    check_ret(__func__, GetAlgorithmThreAPS(handle, &AlgoThre));
    AlgoThre.nBadPixelLocalRowOffset = 104;
    AlgoThre.nBadPixelLocalColOffset = 52;
    check_ret(__func__, SetAlgorithmThreAPS(handle, &AlgoThre));
    check_ret(__func__, SetRawDataSizeAPS(handle, nHeight, nWidth));
    check_ret(__func__, SetMultiThreadEnableAPS(handle, true));
    //pSetLogEnableAPS(handle, true);

    int nErrCode = 0;
    for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++) {
        check_ret(__func__, ImportRawDataAPS(handle, rawDataBuf + rawDataBufLen * nIndex, nWidth * nHeight * 2, nIndex, 1));
    }

    //开始计算
    ROIArea ROI_OB_R1 = {0, 51, 0, 1683};
    ROIArea ROI_OB_R2 = {52, 1275, 0, 25};
    ROIArea ROI_OB_R3 = {52, 1275, 1658, 1683};
    ROIArea ROI_OB_AA = {52, 1275, 26, 1683};
    APSDataMeanType DataMean_OB1, DataMean_OB2, DataMean_OB3, DataMean_AA;
    if(check_ret(__func__, DataMeanAPS(handle, 0, nNumber, &ROI_OB_R1, &DataMean_OB1))) {
        setResult("DataMean_Total_OB1_DD1", (double) DataMean_OB1.DataMeanFrame);
        setResult("DataMean_Gb_OB1_DD1", (double) DataMean_OB1.SubFrameDataMean[0]);
        setResult("DataMean_B_OB1_DD1", (double) DataMean_OB1.SubFrameDataMean[1]);
        setResult("DataMean_R_OB1_DD1", (double) DataMean_OB1.SubFrameDataMean[2]);
        setResult("DataMean_Gr_OB1_DD1", (double) DataMean_OB1.SubFrameDataMean[3]);
        Set_RegisterData("DD1_OB1_DataMeanDark", 0, DataMean_OB1);
    } else {
        return -1;
    }

    if(check_ret(__func__, DataMeanAPS(handle, 0, nNumber, &ROI_OB_R2, &DataMean_OB2))) {
        setResult("DataMean_Total_OB2_DD1", (double) DataMean_OB2.DataMeanFrame);
        setResult("DataMean_Gb_OB2_DD1", (double) DataMean_OB2.SubFrameDataMean[0]);
        setResult("DataMean_B_OB2_DD1", (double) DataMean_OB2.SubFrameDataMean[1]);
        setResult("DataMean_R_OB2_DD1", (double) DataMean_OB2.SubFrameDataMean[2]);
        setResult("DataMean_Gr_OB2_DD1", (double) DataMean_OB2.SubFrameDataMean[3]);
        Set_RegisterData("DD1_OB2_DataMeanDark", 0, DataMean_OB2);
    } else {
        return -1;
    }

    if(check_ret(__func__, DataMeanAPS(handle, 0, nNumber, &ROI_OB_R3, &DataMean_OB3))) {
        setResult("DataMean_Total_OB3_DD1", (double) DataMean_OB3.DataMeanFrame);
        setResult("DataMean_Gb_OB3_DD1", (double) DataMean_OB3.SubFrameDataMean[0]);
        setResult("DataMean_B_OB3_DD1", (double) DataMean_OB3.SubFrameDataMean[1]);
        setResult("DataMean_R_OB3_DD1", (double) DataMean_OB3.SubFrameDataMean[2]);
        setResult("DataMean_Gr_OB3_DD1", (double) DataMean_OB3.SubFrameDataMean[3]);
        Set_RegisterData("DD1_OB3_DataMeanDark", 0, DataMean_OB3);
    } else {
        return -1;
    }

    if(check_ret(__func__, DataMeanAPS(handle, 0, nNumber, &ROI_OB_AA, &DataMean_AA))) {
        setResult("DataMean_Total_DD1", (double) DataMean_AA.DataMeanFrame);
        setResult("DataMean_Gb_DD1", (double) DataMean_AA.SubFrameDataMean[0]);
        setResult("DataMean_B_DD1", (double) DataMean_AA.SubFrameDataMean[1]);
        setResult("DataMean_R_DD1", (double) DataMean_AA.SubFrameDataMean[2]);
        setResult("DataMean_Gr_DD1", (double) DataMean_AA.SubFrameDataMean[3]);
        Set_RegisterData("DD1_AA_DataMeanDark", 0, DataMean_AA);
    } else {
        return -1;
    }
    DeleteHandleAPS(handle);
    // FreeLibrary(hDll);
    //hDll = NULL;  // 置空句柄，防止野指针
    delete [] rawDataBuf;
    rawDataBuf = NULL;
    return 0;
}
#endif //ALGORITHMLIBRARY_DD1_H
