//
// Created by xinfeng.weng on 2025/12/25.
//

#ifndef ALGORITHMLIBRARY_DD3_H
#define ALGORITHMLIBRARY_DD3_H
#include "public.h"
#include "AlpMPAlgoCTypeInterface.h"

int calcDD3()
{
    printf("\n - APS DD3 Start\r\n");
	//导入数据
	std::string F1 = "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_15FPS_5000K_Dark_DD3_0Lux_66p0ms_VTX1P5V_60C_G16_F1_W1632H1224P10_T2K-CP1-4S-LY_20251013220534.raw";
	std::string F2 = "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_15FPS_5000K_Dark_DD3_0Lux_66p0ms_VTX1P5V_60C_G16_F2_W1632H1224P10_T2K-CP1-4S-LY_20251013220534.raw";
	std::string F3 = "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_15FPS_5000K_Dark_DD3_0Lux_66p0ms_VTX1P5V_60C_G16_F3_W1632H1224P10_T2K-CP1-4S-LY_20251013220534.raw";
	std::string F4 = "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_15FPS_5000K_Dark_DD3_0Lux_66p0ms_VTX1P5V_60C_G16_F4_W1632H1224P10_T2K-CP1-4S-LY_20251013220534.raw";
 	std::string F5 = "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_15FPS_5000K_Dark_DD3_0Lux_66p0ms_VTX1P5V_60C_G16_F5_W1632H1224P10_T2K-CP1-4S-LY_20251013220534.raw";
    // std::string F1 = "D:/Work/APX003_KLT/Test_Debug/260423/test_data/dark_1/Q123456-7_CP1/DD3_CFGV3p2p2_TMP60_Default/APX003CE_APS_513132333435362D0703310000457A010104_X0Y0_S0_15FPS_5000K_Dark_DD3_0Lux_66p0ms_VT1P5V_60C_G16_F1_W1632H1224P10_I7KP-CP1-8S-KLT_20260423183009.raw";
    // std::string F2 = "D:/Work/APX003_KLT/Test_Debug/260423/test_data/dark_1/Q123456-7_CP1/DD3_CFGV3p2p2_TMP60_Default/APX003CE_APS_513132333435362D0703310000457A010104_X0Y0_S0_15FPS_5000K_Dark_DD3_0Lux_66p0ms_VT1P5V_60C_G16_F2_W1632H1224P10_I7KP-CP1-8S-KLT_20260423183009.raw";
    // std::string F3 = "D:/Work/APX003_KLT/Test_Debug/260423/test_data/dark_1/Q123456-7_CP1/DD3_CFGV3p2p2_TMP60_Default/APX003CE_APS_513132333435362D0703310000457A010104_X0Y0_S0_15FPS_5000K_Dark_DD3_0Lux_66p0ms_VT1P5V_60C_G16_F3_W1632H1224P10_I7KP-CP1-8S-KLT_20260423183009.raw";
    // std::string F4 = "D:/Work/APX003_KLT/Test_Debug/260423/test_data/dark_1/Q123456-7_CP1/DD3_CFGV3p2p2_TMP60_Default/APX003CE_APS_513132333435362D0703310000457A010104_X0Y0_S0_15FPS_5000K_Dark_DD3_0Lux_66p0ms_VT1P5V_60C_G16_F4_W1632H1224P10_I7KP-CP1-8S-KLT_20260423183009.raw";
    // std::string F5 = "D:/Work/APX003_KLT/Test_Debug/260423/test_data/dark_1/Q123456-7_CP1/DD3_CFGV3p2p2_TMP60_Default/APX003CE_APS_513132333435362D0703310000457A010104_X0Y0_S0_15FPS_5000K_Dark_DD3_0Lux_66p0ms_VT1P5V_60C_G16_F5_W1632H1224P10_I7KP-CP1-8S-KLT_20260423183009.raw";

	int nWidth = 1632;
	int nHeight = 1224;
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
	HANDLE handle = InitHandleAPS(SensorType::ALP_003CA, APSRawType::UNPACK10, "./",PixelFormatType::BayerGBRG, 0);
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
	area.Down = nHeight/2-1;
	area.Up = 0;
	area.Left = 0;
	area.Right = nWidth/2-1;
	check_ret(__func__, SetActiveAreaAPS(handle, area));
	//pGetActiveAreaAPS(handle, area);
	//std::cout << "ROIArea.Down: "<<area.Down << endl;
	//std::cout << "ROIArea.UP: "<<area.Up << endl;
	//std::cout << "ROIArea.Left: "<<area.Left << endl;
	//std::cout << "ROIArea.Right: "<<area.Right << endl;

	int nErrCode = 0;
	for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
	{
		check_ret(__func__, ImportRawDataAPS(handle, rawDataBuf + rawDataBufLen * nIndex, nWidth * nHeight * 2, nIndex, 1));
	}

	int nIndexStart = 0;
	APSBadpixelTypeC HotPixelRes;
	if(check_ret(__func__, HotPixelTypeCAPS(handle, nIndexStart, nNumber, nullptr, &HotPixelRes)))
	{
		setResult("HotPixelNum_Total_DD3", (double)HotPixelRes.BadPixelNum);
		setResult("HotPixelNum_Gb_DD3", (double)HotPixelRes.SubFrameBadpixelData[0].BadPixelNum);
		setResult("HotPixelNum_B_DD3", (double)HotPixelRes.SubFrameBadpixelData[1].BadPixelNum);
		setResult("HotPixelNum_R_DD3", (double)HotPixelRes.SubFrameBadpixelData[2].BadPixelNum);
		setResult("HotPixelNum_Gr_DD3", (double)HotPixelRes.SubFrameBadpixelData[3].BadPixelNum);
		setResult("HotPixel_SingletNum_Total_DD3", (double)HotPixelRes.SingletNum);
		setResult("HotPixel_SingletNum_Gb_DD3", (double)HotPixelRes.SubFrameBadpixelData[0].SingletNum);
		setResult("HotPixel_SingletNum_B_DD3", (double)HotPixelRes.SubFrameBadpixelData[1].SingletNum);
		setResult("HotPixel_SingletNum_R_DD3", (double)HotPixelRes.SubFrameBadpixelData[2].SingletNum);
		setResult("HotPixel_SingletNum_Gr_DD3", (double)HotPixelRes.SubFrameBadpixelData[3].SingletNum);
		setResult("HotPixel_CoupletNum_Total_DD3", (double)HotPixelRes.CoupletNum);
		setResult("HotPixel_CoupletNum_Gb_DD3", (double)HotPixelRes.SubFrameBadpixelData[0].CoupletNum);
		setResult("HotPixel_CoupletNum_B_DD3", (double)HotPixelRes.SubFrameBadpixelData[1].CoupletNum);
		setResult("HotPixel_CoupletNum_R_DD3", (double)HotPixelRes.SubFrameBadpixelData[2].CoupletNum);
		setResult("HotPixel_CoupletNum_Gr_DD3", (double)HotPixelRes.SubFrameBadpixelData[3].CoupletNum);
		setResult("HotPixel_ClusterNum_Total_DD3", (double)HotPixelRes.ClusterNum);
		setResult("HotPixel_ClusterNum_Gb_DD3", (double)HotPixelRes.SubFrameBadpixelData[0].ClusterNum);
		setResult("HotPixel_ClusterNum_B_DD3", (double)HotPixelRes.SubFrameBadpixelData[1].ClusterNum);
		setResult("HotPixel_ClusterNum_R_DD3", (double)HotPixelRes.SubFrameBadpixelData[2].ClusterNum);
		setResult("HotPixel_ClusterNum_Gr_DD3", (double)HotPixelRes.SubFrameBadpixelData[3].ClusterNum);
		setResult("HotPixel_LadderNum_DD3", (double)HotPixelRes.LadderNum);
		setResult("HotPixel_MaxClusterSize_DD3", (double)HotPixelRes.MaxClusterSize);
		setResult("HotPixel_DefectRowNum_Gb_DD3", (double)HotPixelRes.SubFrameBadpixelData[0].DefectRowNum);
		setResult("HotPixel_DefectRowNum_B_DD3", (double)HotPixelRes.SubFrameBadpixelData[1].DefectRowNum);
		setResult("HotPixel_DefectRowNum_R_DD3", (double)HotPixelRes.SubFrameBadpixelData[2].DefectRowNum);
		setResult("HotPixel_DefectRowNum_Gr_DD3", (double)HotPixelRes.SubFrameBadpixelData[3].DefectRowNum);
		setResult("HotPixel_DefectColNum_Gb_DD3", (double)HotPixelRes.SubFrameBadpixelData[0].DefectColNum);
		setResult("HotPixel_DefectColNum_B_DD3", (double)HotPixelRes.SubFrameBadpixelData[1].DefectColNum);
		setResult("HotPixel_DefectColNum_R_DD3", (double)HotPixelRes.SubFrameBadpixelData[2].DefectColNum);
		setResult("HotPixel_DefectColNum_Gr_DD3", (double)HotPixelRes.SubFrameBadpixelData[3].DefectColNum);
	}
	HotPixelTypeCAPS_Free(&HotPixelRes);


	//ROIArea area_mean;
	//pGetActiveAreaAPS(handle, area_mean);
	//std::cout << "ROIArea.Down: "<<area_mean.Down << endl;
	//std::cout << "ROIArea.UP: "<<area_mean.Up << endl;
	//std::cout << "ROIArea.Left: "<<area_mean.Left << endl;
	//std::cout << "ROIArea.Right: "<<area_mean.Right << endl;
	APSDataMeanType DataMean;
	if(check_ret(__func__, DataMeanAPS(handle, nIndexStart, nNumber, &area, &DataMean)))
	{
		setResult("DataMean_Total_DD3", (double)DataMean.DataMeanFrame);
		setResult("DataMean_Gb_DD3", (double)DataMean.SubFrameDataMean[0]);
		setResult("DataMean_B_DD3", (double)DataMean.SubFrameDataMean[1]);
		setResult("DataMean_R_DD3", (double)DataMean.SubFrameDataMean[2]);
		setResult("DataMean_Gr_DD3", (double)DataMean.SubFrameDataMean[3]);
	}
	else
		return -1;

	APSTNoiseTypeC TNoiseData;
	if(check_ret(__func__, TNoiseAPS(handle, nIndexStart, nNumber, nullptr, &TNoiseData)))
	{
		setResult("TNoise_Total_DD3", (double)TNoiseData.TNoiseFrame);
		setResult("TNoise_Gb_DD3", (double)TNoiseData.SubFrameTNoiseData[0].TempNoise);
		setResult("TNoise_B_DD3", (double)TNoiseData.SubFrameTNoiseData[1].TempNoise);
		setResult("TNoise_R_DD3", (double)TNoiseData.SubFrameTNoiseData[2].TempNoise);
		setResult("TNoise_Gr_DD3", (double)TNoiseData.SubFrameTNoiseData[3].TempNoise);
		setResult("TNoise_RowTemp_Gb_DD3", (double)TNoiseData.SubFrameTNoiseData[0].RowTemp);
		setResult("TNoise_RowTemp_B_DD3", (double)TNoiseData.SubFrameTNoiseData[1].RowTemp);
		setResult("TNoise_RowTemp_R_DD3", (double)TNoiseData.SubFrameTNoiseData[2].RowTemp);
		setResult("TNoise_RowTemp_Gr_DD3", (double)TNoiseData.SubFrameTNoiseData[3].RowTemp);
		setResult("TNoise_ColTemp_Gb_DD3", (double)TNoiseData.SubFrameTNoiseData[0].ColTemp);
		setResult("TNoise_ColTemp_B_DD3", (double)TNoiseData.SubFrameTNoiseData[1].ColTemp);
		setResult("TNoise_ColTemp_R_DD3", (double)TNoiseData.SubFrameTNoiseData[2].ColTemp);
		setResult("TNoise_ColTemp_Gr_DD3", (double)TNoiseData.SubFrameTNoiseData[3].ColTemp);
		setResult("TNoise_PixelTemp_Gb_DD3", (double)TNoiseData.SubFrameTNoiseData[0].PixelTemp);
		setResult("TNoise_PixelTemp_B_DD3", (double)TNoiseData.SubFrameTNoiseData[1].PixelTemp);
		setResult("TNoise_PixelTemp_R_DD3", (double)TNoiseData.SubFrameTNoiseData[2].PixelTemp);
		setResult("TNoise_PixelTemp_Gr_DD3", (double)TNoiseData.SubFrameTNoiseData[3].PixelTemp);
		setResult("TNoise_TempRNRatio_Gb_DD3", (double)TNoiseData.SubFrameTNoiseData[0].TempRNRatio);
		setResult("TNoise_TempRNRatio_B_DD3", (double)TNoiseData.SubFrameTNoiseData[1].TempRNRatio);
		setResult("TNoise_TempRNRatio_R_DD3", (double)TNoiseData.SubFrameTNoiseData[2].TempRNRatio);
		setResult("TNoise_TempRNRatio_Gr_DD3", (double)TNoiseData.SubFrameTNoiseData[3].TempRNRatio);
		setResult("TNoise_TempCNRatio_Gb_DD3", (double)TNoiseData.SubFrameTNoiseData[0].TempCNRatio);
		setResult("TNoise_TempCNRatio_B_DD3", (double)TNoiseData.SubFrameTNoiseData[1].TempCNRatio);
		setResult("TNoise_TempCNRatio_R_DD3", (double)TNoiseData.SubFrameTNoiseData[2].TempCNRatio);
		setResult("TNoise_TempCNRatio_Gr_DD3", (double)TNoiseData.SubFrameTNoiseData[3].TempCNRatio);
	}
	else
		return -1;
	TNoiseAPS_Free(&TNoiseData);

	APSSNoiseType SNoiseData;
	if(check_ret(__func__, SNoiseAPS(handle, nIndexStart, nNumber, nullptr, &SNoiseData)))
	{
		setResult("SNoise_Total_DD3", (double)SNoiseData.SNoiseFrame);
		setResult("SNoise_Gb_DD3", (double)SNoiseData.SubFrameSNoiseData[0].SNoise);
		setResult("SNoise_B_DD3", (double)SNoiseData.SubFrameSNoiseData[1].SNoise);
		setResult("SNoise_R_DD3", (double)SNoiseData.SubFrameSNoiseData[2].SNoise);
		setResult("SNoise_Gr_DD3", (double)SNoiseData.SubFrameSNoiseData[3].SNoise);
		setResult("SNoise_RowSNoise_Gb_DD3", (double)SNoiseData.SubFrameSNoiseData[0].RowSNoise);
		setResult("SNoise_RowSNoise_B_DD3", (double)SNoiseData.SubFrameSNoiseData[1].RowSNoise);
		setResult("SNoise_RowSNoise_R_DD3", (double)SNoiseData.SubFrameSNoiseData[2].RowSNoise);
		setResult("SNoise_RowSNoise_Gr_DD3", (double)SNoiseData.SubFrameSNoiseData[3].RowSNoise);
		setResult("SNoise_ColSNoise_Gb_DD3", (double)SNoiseData.SubFrameSNoiseData[0].ColSNoise);
		setResult("SNoise_ColSNoise_B_DD3", (double)SNoiseData.SubFrameSNoiseData[1].ColSNoise);
		setResult("SNoise_ColSNoise_R_DD3", (double)SNoiseData.SubFrameSNoiseData[2].ColSNoise);
		setResult("SNoise_ColSNoise_Gr_DD3", (double)SNoiseData.SubFrameSNoiseData[3].ColSNoise);
	}
	else
		return -1;

	DeleteHandleAPS(handle);
	delete [] rawDataBuf;
	rawDataBuf = NULL;
	return 0;
}
#endif //ALGORITHMLIBRARY_DD1_H
