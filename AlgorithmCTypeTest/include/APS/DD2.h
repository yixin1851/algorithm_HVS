//
// Created by xinfeng.weng on 2025/12/25.
//

#ifndef ALGORITHMLIBRARY_DD2_H
#define ALGORITHMLIBRARY_DD2_H
#include "public.h"
#include "AlpMPAlgoCTypeInterface.h"

int calcDD2() {
    printf("\n - APS DD2 Start\r\n");
    //导入数据
    std::string F1 = "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_4FPS_5000K_Dark_DD2_0Lux_200p0ms_VTX1P5V_60C_G1_F1_W3376H2552P10_T2K-CP1-4S-LY_20251013220523.raw";
    int nWidth = 3376;
    int nHeight = 2552;
    uint32_t nNumber = 1;
    long rawDataBufLen = nWidth * nHeight * 2;
    unsigned long rawDataRealLen = 0;
    uint8_t *rawDataBuf = new uint8_t[rawDataBufLen * nNumber];
    ImageCapture_capture(F1, rawDataBuf, rawDataBufLen, rawDataRealLen, 0);

    // 3. 调用DLL函数
    HANDLE handle = InitHandleAPS(SensorType::ALP_003CA, APSRawType::UNPACK10, "./", PixelFormatType::QuadBayerGBRG,
                                   0);
    //cout << "句柄：" << handle << endl;

    uint32_t get_nHeight, get_nWidth;
    //pGetRawDataSizeAPS(handle, get_nHeight, get_nWidth);
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
    double expTimeData[2] = {33.0, 200.0};
    APSDataMeanType DataMeanDark1_OB1, DataMeanDark2_OB1;
    APSDarkCurrentType DarkCurrent_OB1;
    if (check_ret(__func__, DataMeanAPS(handle, 0, nNumber, &ROI_OB_R1, &DataMeanDark2_OB1))) {
        setResult("DataMean_Total_OB1_DD2", (double) DataMeanDark2_OB1.DataMeanFrame);
        setResult("DataMean_Gb_OB1_DD2", (double) DataMeanDark2_OB1.SubFrameDataMean[0]);
        setResult("DataMean_B_OB1_DD2", (double) DataMeanDark2_OB1.SubFrameDataMean[1]);
        setResult("DataMean_R_OB1_DD2", (double) DataMeanDark2_OB1.SubFrameDataMean[2]);
        setResult("DataMean_Gr_OB1_DD2", (double) DataMeanDark2_OB1.SubFrameDataMean[3]);
        if (!Get_RegisterData("DD1_OB1_DataMeanDark", 0, DataMeanDark1_OB1)) return -1;
        printf("DD2 LINE:%d\r\n", __LINE__);
        APSDataMeanType DataMean_OB1[2] = {DataMeanDark1_OB1, DataMeanDark2_OB1};
        if (check_ret(__func__, DarkCurrentDataMeanAPS(handle, DataMean_OB1, 2, expTimeData, 2, &DarkCurrent_OB1))) {
            printf("DD2 LINE:%d\r\n", __LINE__);
            setResult("DarkCurrent_Total_OB1_DD2",
                      (double) ((DataMeanDark2_OB1.DataMeanFrame - DataMeanDark1_OB1.DataMeanFrame) * 1000.0 / (
                                    200 - 33)));
            setResult("DarkCurrent_Gb_OB1_DD2", (double) DarkCurrent_OB1.SubFrameKValue[0]);
            setResult("DarkCurrent_B_OB1_DD2", (double) DarkCurrent_OB1.SubFrameKValue[1]);
            setResult("DarkCurrent_R_OB1_DD2", (double) DarkCurrent_OB1.SubFrameKValue[2]);
            setResult("DarkCurrent_Gr_OB1_DD2", (double) DarkCurrent_OB1.SubFrameKValue[3]);
        } else
            return -1;
    } else {
        return -1;
    }

    APSDataMeanType DataMeanDark1_OB2, DataMeanDark2_OB2;
    APSDarkCurrentType DarkCurrent_OB2;
    if (check_ret(__func__, DataMeanAPS(handle, 0, nNumber, &ROI_OB_R2, &DataMeanDark2_OB2))){
        setResult("DataMean_Total_OB2_DD2", (double) DataMeanDark2_OB2.DataMeanFrame);
        setResult("DataMean_Gb_OB2_DD2", (double) DataMeanDark2_OB2.SubFrameDataMean[0]);
        setResult("DataMean_B_OB2_DD2", (double) DataMeanDark2_OB2.SubFrameDataMean[1]);
        setResult("DataMean_R_OB2_DD2", (double) DataMeanDark2_OB2.SubFrameDataMean[2]);
        setResult("DataMean_Gr_OB2_DD2", (double) DataMeanDark2_OB2.SubFrameDataMean[3]);
        if (!Get_RegisterData("DD1_OB2_DataMeanDark", 0, DataMeanDark1_OB2)) return -1;
        APSDataMeanType DataMean_OB2[2] = {DataMeanDark1_OB2, DataMeanDark2_OB2};
        if (check_ret(__func__, DarkCurrentDataMeanAPS(handle, DataMean_OB2, 2, expTimeData, 2, &DarkCurrent_OB2))){
            setResult("DarkCurrent_Total_OB2_DD2",
                      (double) ((DataMeanDark2_OB2.DataMeanFrame - DataMeanDark1_OB2.DataMeanFrame) * 1000.0 / (
                                    200 - 33)));
            setResult("DarkCurrent_Gb_OB2_DD2", (double) DarkCurrent_OB2.SubFrameKValue[0]);
            setResult("DarkCurrent_B_OB2_DD2", (double) DarkCurrent_OB2.SubFrameKValue[1]);
            setResult("DarkCurrent_R_OB2_DD2", (double) DarkCurrent_OB2.SubFrameKValue[2]);
            setResult("DarkCurrent_Gr_OB2_DD2", (double) DarkCurrent_OB2.SubFrameKValue[3]);
        } else
            return -1;
    } else {
        return -1;
    }

    APSDataMeanType DataMeanDark1_OB3, DataMeanDark2_OB3;
    APSDarkCurrentType DarkCurrent_OB3;
    if (check_ret(__func__, DataMeanAPS(handle, 0, nNumber, &ROI_OB_R3, &DataMeanDark2_OB3))) {
        setResult("DataMean_Total_OB3_DD2", (double) DataMeanDark2_OB3.DataMeanFrame);
        setResult("DataMean_Gb_OB3_DD2", (double) DataMeanDark2_OB3.SubFrameDataMean[0]);
        setResult("DataMean_B_OB3_DD2", (double) DataMeanDark2_OB3.SubFrameDataMean[1]);
        setResult("DataMean_R_OB3_DD2", (double) DataMeanDark2_OB3.SubFrameDataMean[2]);
        setResult("DataMean_Gr_OB3_DD2", (double) DataMeanDark2_OB3.SubFrameDataMean[3]);
        if (!Get_RegisterData("DD1_OB3_DataMeanDark", 0, DataMeanDark1_OB3)) return -1;
        APSDataMeanType DataMean_OB3[2] = {DataMeanDark1_OB3, DataMeanDark2_OB3};
        if (check_ret(__func__, DarkCurrentDataMeanAPS(handle, DataMean_OB3, 2, expTimeData, 2, &DarkCurrent_OB3))) {
            setResult("DarkCurrent_Total_OB3_DD2",
                      (double) ((DataMeanDark2_OB3.DataMeanFrame - DataMeanDark1_OB3.DataMeanFrame) * 1000.0 / (
                                    200 - 33)));
            setResult("DarkCurrent_Gb_OB3_DD2", (double) DarkCurrent_OB3.SubFrameKValue[0]);
            setResult("DarkCurrent_B_OB3_DD2", (double) DarkCurrent_OB3.SubFrameKValue[1]);
            setResult("DarkCurrent_R_OB3_DD2", (double) DarkCurrent_OB3.SubFrameKValue[2]);
            setResult("DarkCurrent_Gr_OB3_DD2", (double) DarkCurrent_OB3.SubFrameKValue[3]);
        } else
            return -1;
    } else {
        return -1;
    }

    APSDataMeanType DataMeanDark1_AA, DataMeanDark2_AA;
    APSDarkCurrentType DarkCurrent_AA;
    if (check_ret(__func__, DataMeanAPS(handle, 0, nNumber, &ROI_OB_AA, &DataMeanDark2_AA))) {
        setResult("DataMean_Total_DD2", (double) DataMeanDark2_AA.DataMeanFrame);
        setResult("DataMean_Gb_DD2", (double) DataMeanDark2_AA.SubFrameDataMean[0]);
        setResult("DataMean_B_DD2", (double) DataMeanDark2_AA.SubFrameDataMean[1]);
        setResult("DataMean_R_DD2", (double) DataMeanDark2_AA.SubFrameDataMean[2]);
        setResult("DataMean_Gr_DD2", (double) DataMeanDark2_AA.SubFrameDataMean[3]);
        if (!Get_RegisterData("DD1_AA_DataMeanDark", 0, DataMeanDark1_AA)) return -1;
        APSDataMeanType DataMean_AA[2] = {DataMeanDark1_AA, DataMeanDark2_AA};
        if (check_ret(__func__, DarkCurrentDataMeanAPS(handle, DataMean_AA, 2, expTimeData, 2, &DarkCurrent_AA))) {
            setResult("DarkCurrent_Total_DD2",
                      (double) ((DataMeanDark2_AA.DataMeanFrame - DataMeanDark1_AA.DataMeanFrame) * 1000.0 / (
                                    200 - 33)));
            setResult("DarkCurrent_Gb_DD2", (double) DarkCurrent_AA.SubFrameKValue[0]);
            setResult("DarkCurrent_B_DD2", (double) DarkCurrent_AA.SubFrameKValue[1]);
            setResult("DarkCurrent_R_DD2", (double) DarkCurrent_AA.SubFrameKValue[2]);
            setResult("DarkCurrent_Gr_DD2", (double) DarkCurrent_AA.SubFrameKValue[3]);
        } else
            return -1;
    } else {
        return -1;
    }

    Reset_RegisterData();
    DeleteHandleAPS(handle);
    delete [] rawDataBuf;
    rawDataBuf = NULL;
    return 0;
}
#endif //ALGORITHMLIBRARY_DD2_H
