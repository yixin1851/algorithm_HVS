//
// Created by xinfeng.weng on 2025/12/25.
//

#ifndef ALGORITHMLIBRARY_HDD3_H
#define ALGORITHMLIBRARY_HDD3_H
#include "public.h"
#include "AlpMPAlgoCTypeInterface.h"

int calcHDD3()
{
    printf("\n - APS HDD3 Start\r\n");
	//导入数据
	std::string F1 = "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_15FPS_5000K_Dark_HDD3_0Lux_66p0ms_VTX1P5V_60C_G16_F1_W3280H2464P10_T2K-CP1-4S-LY_20251013220514.raw";
	std::string F2 = "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_15FPS_5000K_Dark_HDD3_0Lux_66p0ms_VTX1P5V_60C_G16_F2_W3280H2464P10_T2K-CP1-4S-LY_20251013220514.raw";
	std::string F3 = "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_15FPS_5000K_Dark_HDD3_0Lux_66p0ms_VTX1P5V_60C_G16_F3_W3280H2464P10_T2K-CP1-4S-LY_20251013220514.raw";
	std::string F4 = "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_15FPS_5000K_Dark_HDD3_0Lux_66p0ms_VTX1P5V_60C_G16_F4_W3280H2464P10_T2K-CP1-4S-LY_20251013220514.raw";
 	std::string F5 = "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_15FPS_5000K_Dark_HDD3_0Lux_66p0ms_VTX1P5V_60C_G16_F5_W3280H2464P10_T2K-CP1-4S-LY_20251013220514.raw";
	int nWidth = 3280;
	int nHeight = 2464;
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
	HANDLE handle = InitHandleAPS(SensorType::ALP_003CA, APSRawType::UNPACK10, "./", PixelFormatType::QuadBayerGBRG, APSCodeType::APS_Code_HVS);
	//cout << "句柄：" << handle << endl;
	APSAlgorithmThre AlgoThre;
	check_ret(__func__, GetAlgorithmThreAPS(handle, &AlgoThre));
	AlgoThre.nBadPixelLocalRowOffset = 112;
	AlgoThre.nBadPixelLocalColOffset = 60;
	check_ret(__func__, SetAlgorithmThreAPS(handle, &AlgoThre));
	uint32_t get_nHeight, get_nWidth;
	//pGetRawDataSizeAPS(handle, get_nHeight, get_nWidth);
	check_ret(__func__, SetRawDataSizeAPS(handle, nHeight, nWidth));
	check_ret(__func__, SetMultiThreadEnableAPS(handle, true));
	//pSetLogEnableAPS(handle, true);

	ROIArea area;
	area.Up = 0;
	area.Down = nHeight/2-1;
	area.Left = 0;
	area.Right = nWidth/2-1;
	check_ret(__func__, SetActiveAreaAPS(handle, area));
	//pGetActiveAreaAPS(handle, area);

	int nErrCode = 0;
	for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
		check_ret(__func__, ImportRawDataAPS(handle, rawDataBuf + rawDataBufLen * nIndex, nWidth * nHeight * 2, nIndex, 1));

	//调用接口
	uint32_t nIndexStart = 0;
	APSBadpixelTypeC HotpixelRes = { 0 };
	if (check_ret(__func__, HotPixelTypeCAPS(handle, nIndexStart, nNumber, nullptr, &HotpixelRes)))
	{
		setResult("HotPixelNum_Total_HDD3_DPC", (double)HotpixelRes.BadPixelNum);
		setResult("HotPixelNum_Gb_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[0].BadPixelNum);
		setResult("HotPixelNum_B_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[1].BadPixelNum);
		setResult("HotPixelNum_R_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[2].BadPixelNum);
		setResult("HotPixelNum_Gr_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[3].BadPixelNum);
		setResult("HotPixel_SingletNum_Total_HDD3_DPC", (double)HotpixelRes.SingletNum);
		setResult("HotPixel_SingletNum_Gb_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[0].SingletNum);
		setResult("HotPixel_SingletNum_B_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[1].SingletNum);
		setResult("HotPixel_SingletNum_R_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[2].SingletNum);
		setResult("HotPixel_SingletNum_Gr_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[3].SingletNum);
		setResult("HotPixel_CoupletNum_Total_HDD3_DPC", (double)HotpixelRes.CoupletNum);
		setResult("HotPixel_CoupletNum_Gb_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[0].CoupletNum);
		setResult("HotPixel_CoupletNum_B_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[1].CoupletNum);
		setResult("HotPixel_CoupletNum_R_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[2].CoupletNum);
		setResult("HotPixel_CoupletNum_Gr_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[3].CoupletNum);
		setResult("HotPixel_ClusterNum_Total_HDD3_DPC", (double)HotpixelRes.ClusterNum);
		setResult("HotPixel_ClusterNum_Gb_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[0].ClusterNum);
		setResult("HotPixel_ClusterNum_B_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[1].ClusterNum);
		setResult("HotPixel_ClusterNum_R_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[2].ClusterNum);
		setResult("HotPixel_ClusterNum_Gr_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[3].ClusterNum);
		setResult("HotPixel_LadderNum_HDD3_DPC", (double)HotpixelRes.LadderNum);
		setResult("HotPixel_MaxClusterSize_HDD3_DPC", (double)HotpixelRes.MaxClusterSize);
		setResult("HotPixel_DefectRowNum_Gb_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[0].DefectRowNum);
		setResult("HotPixel_DefectRowNum_B_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[1].DefectRowNum);
		setResult("HotPixel_DefectRowNum_R_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[2].DefectRowNum);
		setResult("HotPixel_DefectRowNum_Gr_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[3].DefectRowNum);
		setResult("HotPixel_DefectColNum_Gb_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[0].DefectColNum);
		setResult("HotPixel_DefectColNum_B_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[1].DefectColNum);
		setResult("HotPixel_DefectColNum_R_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[2].DefectColNum);
		setResult("HotPixel_DefectColNum_Gr_HDD3_DPC", (double)HotpixelRes.SubFrameBadpixelData[3].DefectColNum);
	}
    HotPixelTypeCAPS_Free(&HotpixelRes);

	APSDataMeanType DataMean;
	if (check_ret(__func__, DataMeanAPS(handle, 0, nNumber, nullptr, &DataMean)))
	{
		setResult("DataMean_Total_HDD3", (double)DataMean.DataMeanFrame);
		setResult("DataMean_Gb_HDD3", (double)DataMean.SubFrameDataMean[0]);
		setResult("DataMean_B_HDD3", (double)DataMean.SubFrameDataMean[1]);
		setResult("DataMean_R_HDD3", (double)DataMean.SubFrameDataMean[2]);
		setResult("DataMean_Gr_HDD3", (double)DataMean.SubFrameDataMean[3]);
	}
	else
		return -1;

	APSTNoiseTypeC TNoiseData = { 0 };
	if (check_ret(__func__, TNoiseAPS(handle, 0, nNumber, nullptr, &TNoiseData)))
	{
		setResult("TNoise_Total_HDD3", (double)TNoiseData.TNoiseFrame);
		setResult("TNoise_Gb_HDD3", (double)TNoiseData.SubFrameTNoiseData[0].TempNoise);
		setResult("TNoise_B_HDD3", (double)TNoiseData.SubFrameTNoiseData[1].TempNoise);
		setResult("TNoise_R_HDD3", (double)TNoiseData.SubFrameTNoiseData[2].TempNoise);
		setResult("TNoise_Gr_HDD3", (double)TNoiseData.SubFrameTNoiseData[3].TempNoise);
		setResult("TNoise_RowTemp_Gb_HDD3", (double)TNoiseData.SubFrameTNoiseData[0].RowTemp);
		setResult("TNoise_RowTemp_B_HDD3", (double)TNoiseData.SubFrameTNoiseData[1].RowTemp);
		setResult("TNoise_RowTemp_R_HDD3", (double)TNoiseData.SubFrameTNoiseData[2].RowTemp);
		setResult("TNoise_RowTemp_Gr_HDD3", (double)TNoiseData.SubFrameTNoiseData[3].RowTemp);
		setResult("TNoise_ColTemp_Gb_HDD3", (double)TNoiseData.SubFrameTNoiseData[0].ColTemp);
		setResult("TNoise_ColTemp_B_HDD3", (double)TNoiseData.SubFrameTNoiseData[1].ColTemp);
		setResult("TNoise_ColTemp_R_HDD3", (double)TNoiseData.SubFrameTNoiseData[2].ColTemp);
		setResult("TNoise_ColTemp_Gr_HDD3", (double)TNoiseData.SubFrameTNoiseData[3].ColTemp);
		setResult("TNoise_PixelTemp_Gb_HDD3", (double)TNoiseData.SubFrameTNoiseData[0].PixelTemp);
		setResult("TNoise_PixelTemp_B_HDD3", (double)TNoiseData.SubFrameTNoiseData[1].PixelTemp);
		setResult("TNoise_PixelTemp_R_HDD3", (double)TNoiseData.SubFrameTNoiseData[2].PixelTemp);
		setResult("TNoise_PixelTemp_Gr_HDD3", (double)TNoiseData.SubFrameTNoiseData[3].PixelTemp);
		setResult("TNoise_TempRNRatio_Gb_HDD3", (double)TNoiseData.SubFrameTNoiseData[0].TempRNRatio);
		setResult("TNoise_TempRNRatio_B_HDD3", (double)TNoiseData.SubFrameTNoiseData[1].TempRNRatio);
		setResult("TNoise_TempRNRatio_R_HDD3", (double)TNoiseData.SubFrameTNoiseData[2].TempRNRatio);
		setResult("TNoise_TempRNRatio_Gr_HDD3", (double)TNoiseData.SubFrameTNoiseData[3].TempRNRatio);
		setResult("TNoise_TempCNRatio_Gb_HDD3", (double)TNoiseData.SubFrameTNoiseData[0].TempCNRatio);
		setResult("TNoise_TempCNRatio_B_HDD3", (double)TNoiseData.SubFrameTNoiseData[1].TempCNRatio);
		setResult("TNoise_TempCNRatio_R_HDD3", (double)TNoiseData.SubFrameTNoiseData[2].TempCNRatio);
		setResult("TNoise_TempCNRatio_Gr_HDD3", (double)TNoiseData.SubFrameTNoiseData[3].TempCNRatio);
	}
	else
		return -1;
	TNoiseAPS_Free(&TNoiseData);

	APSSNoiseType SNoiseData;
	if (check_ret(__func__, SNoiseAPS(handle, 0, nNumber, nullptr, &SNoiseData)))
	{
		setResult("SNoise_Total_HDD3", (double)SNoiseData.SNoiseFrame);
		setResult("SNoise_Gb_HDD3", (double)SNoiseData.SubFrameSNoiseData[0].SNoise);
		setResult("SNoise_B_HDD3", (double)SNoiseData.SubFrameSNoiseData[1].SNoise);
		setResult("SNoise_R_HDD3", (double)SNoiseData.SubFrameSNoiseData[2].SNoise);
		setResult("SNoise_Gr_HDD3", (double)SNoiseData.SubFrameSNoiseData[3].SNoise);
		setResult("SNoise_RowSNoise_Gb_HDD3", (double)SNoiseData.SubFrameSNoiseData[0].RowSNoise);
		setResult("SNoise_RowSNoise_B_HDD3", (double)SNoiseData.SubFrameSNoiseData[1].RowSNoise);
		setResult("SNoise_RowSNoise_R_HDD3", (double)SNoiseData.SubFrameSNoiseData[2].RowSNoise);
		setResult("SNoise_RowSNoise_Gr_HDD3", (double)SNoiseData.SubFrameSNoiseData[3].RowSNoise);
		setResult("SNoise_ColSNoise_Gb_HDD3", (double)SNoiseData.SubFrameSNoiseData[0].ColSNoise);
		setResult("SNoise_ColSNoise_B_HDD3", (double)SNoiseData.SubFrameSNoiseData[1].ColSNoise);
		setResult("SNoise_ColSNoise_R_HDD3", (double)SNoiseData.SubFrameSNoiseData[2].ColSNoise);
		setResult("SNoise_ColSNoise_Gr_HDD3", (double)SNoiseData.SubFrameSNoiseData[3].ColSNoise);
	}
	else
		return -1;

	DeleteHandleAPS(handle);
	delete [] rawDataBuf;
	rawDataBuf = NULL;
	return 0;
}
#endif //ALGORITHMLIBRARY_HDD3_H
