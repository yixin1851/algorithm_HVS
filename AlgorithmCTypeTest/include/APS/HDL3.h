//
// Created by xinfeng.weng on 2025/12/25.
//

#ifndef ALGORITHMLIBRARY_HDL3_H
#define ALGORITHMLIBRARY_HDL3_H
#include "public.h"
#include "AlpMPAlgoCTypeInterface.h"

int calcHDL3()
{
    printf("\n - APS HDL3 Start\r\n");
	//导入数据
	std::string F1 = "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_25FPS_5000K_Light_HDL3_3080Lux_18p6ms_VTX1P5V_60C_G1_F1_W3280H2464P10_T2K-CP1-4S-LY_20251013220518.raw";
	std::string F2 = "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_25FPS_5000K_Light_HDL3_3080Lux_18p6ms_VTX1P5V_60C_G1_F2_W3280H2464P10_T2K-CP1-4S-LY_20251013220518.raw";
	std::string F3 = "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_25FPS_5000K_Light_HDL3_3080Lux_18p6ms_VTX1P5V_60C_G1_F3_W3280H2464P10_T2K-CP1-4S-LY_20251013220518.raw";
	std::string F4 = "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_25FPS_5000K_Light_HDL3_3080Lux_18p6ms_VTX1P5V_60C_G1_F4_W3280H2464P10_T2K-CP1-4S-LY_20251013220518.raw";
 	std::string F5 = "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_25FPS_5000K_Light_HDL3_3080Lux_18p6ms_VTX1P5V_60C_G1_F5_W3280H2464P10_T2K-CP1-4S-LY_20251013220518.raw";
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
	HANDLE handle = InitHandleAPS(SensorType::ALP_003CA, APSRawType::UNPACK10, "./",PixelFormatType::QuadBayerGBRG, APSCodeType::APS_Code_HVS);
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
		check_ret(__func__, ImportRawDataAPS(handle, rawDataBuf + rawDataBufLen * nIndex, nWidth * nHeight * 2, nIndex, 1));

	uint32_t nIndexStart = 0;
	APSBadpixelTypeC BadPixelRes;
	if (check_ret(__func__, BadPixelTypeCAPS(handle, nIndexStart, nNumber, nullptr, &BadPixelRes)))
	{
		setResult("BadPixelNum_Total_HDL3", (double)BadPixelRes.BadPixelNum);
		setResult("BadPixelNum_Gb_HDL3", (double)BadPixelRes.SubFrameBadpixelData[0].BadPixelNum);
		setResult("BadPixelNum_B_HDL3", (double)BadPixelRes.SubFrameBadpixelData[1].BadPixelNum);
		setResult("BadPixelNum_R_HDL3", (double)BadPixelRes.SubFrameBadpixelData[2].BadPixelNum);
		setResult("BadPixelNum_Gr_HDL3", (double)BadPixelRes.SubFrameBadpixelData[3].BadPixelNum);
		setResult("BadPixel_SingletNum_Total_HDL3", (double)BadPixelRes.SingletNum);
		setResult("BadPixel_SingletNum_Gb_HDL3", (double)BadPixelRes.SubFrameBadpixelData[0].SingletNum);
		setResult("BadPixel_SingletNum_B_HDL3", (double)BadPixelRes.SubFrameBadpixelData[1].SingletNum);
		setResult("BadPixel_SingletNum_R_HDL3", (double)BadPixelRes.SubFrameBadpixelData[2].SingletNum);
		setResult("BadPixel_SingletNum_Gr_HDL3", (double)BadPixelRes.SubFrameBadpixelData[3].SingletNum);
		setResult("BadPixel_CoupletNum_Total_HDL3", (double)BadPixelRes.CoupletNum);
		setResult("BadPixel_CoupletNum_Gb_HDL3", (double)BadPixelRes.SubFrameBadpixelData[0].CoupletNum);
		setResult("BadPixel_CoupletNum_B_HDL3", (double)BadPixelRes.SubFrameBadpixelData[1].CoupletNum);
		setResult("BadPixel_CoupletNum_R_HDL3", (double)BadPixelRes.SubFrameBadpixelData[2].CoupletNum);
		setResult("BadPixel_CoupletNum_Gr_HDL3", (double)BadPixelRes.SubFrameBadpixelData[3].CoupletNum);
		setResult("BadPixel_ClusterNum_Total_HDL3", (double)BadPixelRes.ClusterNum);
		setResult("BadPixel_ClusterNum_Gb_HDL3", (double)BadPixelRes.SubFrameBadpixelData[0].ClusterNum);
		setResult("BadPixel_ClusterNum_B_HDL3", (double)BadPixelRes.SubFrameBadpixelData[1].ClusterNum);
		setResult("BadPixel_ClusterNum_R_HDL3", (double)BadPixelRes.SubFrameBadpixelData[2].ClusterNum);
		setResult("BadPixel_ClusterNum_Gr_HDL3", (double)BadPixelRes.SubFrameBadpixelData[3].ClusterNum);
		setResult("BadPixel_LadderNum_HDL3", (double)BadPixelRes.LadderNum);
		setResult("BadPixel_MaxClusterSize_HDL3", (double)BadPixelRes.MaxClusterSize);
		setResult("BadPixel_DefectRowNum_Gb_HDL3", (double)BadPixelRes.SubFrameBadpixelData[0].DefectRowNum);
		setResult("BadPixel_DefectRowNum_B_HDL3", (double)BadPixelRes.SubFrameBadpixelData[1].DefectRowNum);
		setResult("BadPixel_DefectRowNum_R_HDL3", (double)BadPixelRes.SubFrameBadpixelData[2].DefectRowNum);
		setResult("BadPixel_DefectRowNum_Gr_HDL3", (double)BadPixelRes.SubFrameBadpixelData[3].DefectRowNum);
		setResult("BadPixel_DefectColNum_Gb_HDL3", (double)BadPixelRes.SubFrameBadpixelData[0].DefectColNum);
		setResult("BadPixel_DefectColNum_B_HDL3", (double)BadPixelRes.SubFrameBadpixelData[1].DefectColNum);
		setResult("BadPixel_DefectColNum_R_HDL3", (double)BadPixelRes.SubFrameBadpixelData[2].DefectColNum);
		setResult("BadPixel_DefectColNum_Gr_HDL3", (double)BadPixelRes.SubFrameBadpixelData[3].DefectColNum);
	}
    BadPixelTypeCAPS_Free(&BadPixelRes);

	APSDataMeanType DataMean;
	if (check_ret(__func__, DataMeanAPS(handle, 0, nNumber, nullptr, &DataMean)))
	{
		setResult("DataMean_Total_HDL3", (double)DataMean.DataMeanFrame);
		setResult("DataMean_Gb_HDL3", (double)DataMean.SubFrameDataMean[0]);
		setResult("DataMean_B_HDL3", (double)DataMean.SubFrameDataMean[1]);
		setResult("DataMean_R_HDL3", (double)DataMean.SubFrameDataMean[2]);
		setResult("DataMean_Gr_HDL3", (double)DataMean.SubFrameDataMean[3]);
	}
	else
		return -1;

	APSTNoiseTypeC TNoiseData;
	if (check_ret(__func__, TNoiseAPS(handle, nIndexStart, nNumber, nullptr, &TNoiseData)))
	{
		setResult("TNoise_Total_HDL3", (double)TNoiseData.TNoiseFrame);
		setResult("TNoise_Gb_HDL3", (double)(TNoiseData.SubFrameTNoiseData[0].TempNoise));
		setResult("TNoise_B_HDL3", (double)TNoiseData.SubFrameTNoiseData[1].TempNoise);
		setResult("TNoise_R_HDL3", (double)TNoiseData.SubFrameTNoiseData[2].TempNoise);
		setResult("TNoise_Gr_HDL3", (double)TNoiseData.SubFrameTNoiseData[3].TempNoise);
		setResult("TNoise_RowTemp_Gb_HDL3", (double)TNoiseData.SubFrameTNoiseData[0].RowTemp);
		setResult("TNoise_RowTemp_B_HDL3", (double)TNoiseData.SubFrameTNoiseData[1].RowTemp);
		setResult("TNoise_RowTemp_R_HDL3", (double)TNoiseData.SubFrameTNoiseData[2].RowTemp);
		setResult("TNoise_RowTemp_Gr_HDL3", (double)TNoiseData.SubFrameTNoiseData[3].RowTemp);
		setResult("TNoise_ColTemp_Gb_HDL3", (double)TNoiseData.SubFrameTNoiseData[0].ColTemp);
		setResult("TNoise_ColTemp_B_HDL3", (double)TNoiseData.SubFrameTNoiseData[1].ColTemp);
		setResult("TNoise_ColTemp_R_HDL3", (double)TNoiseData.SubFrameTNoiseData[2].ColTemp);
		setResult("TNoise_ColTemp_Gr_HDL3", (double)TNoiseData.SubFrameTNoiseData[3].ColTemp);
		setResult("TNoise_PixelTemp_Gb_HDL3", (double)TNoiseData.SubFrameTNoiseData[0].PixelTemp);
		setResult("TNoise_PixelTemp_B_HDL3", (double)TNoiseData.SubFrameTNoiseData[1].PixelTemp);
		setResult("TNoise_PixelTemp_R_HDL3", (double)TNoiseData.SubFrameTNoiseData[2].PixelTemp);
		setResult("TNoise_PixelTemp_Gr_HDL3", (double)TNoiseData.SubFrameTNoiseData[3].PixelTemp);
		setResult("TNoise_TempRNRatio_Gb_HDL3", (double)TNoiseData.SubFrameTNoiseData[0].TempRNRatio);
		setResult("TNoise_TempRNRatio_B_HDL3", (double)TNoiseData.SubFrameTNoiseData[1].TempRNRatio);
		setResult("TNoise_TempRNRatio_R_HDL3", (double)TNoiseData.SubFrameTNoiseData[2].TempRNRatio);
		setResult("TNoise_TempRNRatio_Gr_HDL3", (double)TNoiseData.SubFrameTNoiseData[3].TempRNRatio);
		setResult("TNoise_TempCNRatio_Gb_HDL3", (double)TNoiseData.SubFrameTNoiseData[0].TempCNRatio);
		setResult("TNoise_TempCNRatio_B_HDL3", (double)TNoiseData.SubFrameTNoiseData[1].TempCNRatio);
		setResult("TNoise_TempCNRatio_R_HDL3", (double)TNoiseData.SubFrameTNoiseData[2].TempCNRatio);
		setResult("TNoise_TempCNRatio_Gr_HDL3", (double)TNoiseData.SubFrameTNoiseData[3].TempCNRatio);
	}
	else
		return -1;
	TNoiseAPS_Free(&TNoiseData);

	APSSNoiseType SNoiseData;
	if (check_ret(__func__, SNoiseAPS(handle, 0, nNumber, nullptr, &SNoiseData)))
	{
		setResult("SNoise_Total_HDL3", (double)SNoiseData.SNoiseFrame);
		setResult("SNoise_Gb_HDL3", (double)SNoiseData.SubFrameSNoiseData[0].SNoise);
		setResult("SNoise_B_HDL3", (double)SNoiseData.SubFrameSNoiseData[1].SNoise);
		setResult("SNoise_R_HDL3", (double)SNoiseData.SubFrameSNoiseData[2].SNoise);
		setResult("SNoise_Gr_HDL3", (double)SNoiseData.SubFrameSNoiseData[3].SNoise);
		setResult("SNoise_RowSNoise_Gb_HDL3", (double)SNoiseData.SubFrameSNoiseData[0].RowSNoise);
		setResult("SNoise_RowSNoise_B_HDL3", (double)SNoiseData.SubFrameSNoiseData[1].RowSNoise);
		setResult("SNoise_RowSNoise_R_HDL3", (double)SNoiseData.SubFrameSNoiseData[2].RowSNoise);
		setResult("SNoise_RowSNoise_Gr_HDL3", (double)SNoiseData.SubFrameSNoiseData[3].RowSNoise);
		setResult("SNoise_ColSNoise_Gb_HDL3", (double)SNoiseData.SubFrameSNoiseData[0].ColSNoise);
		setResult("SNoise_ColSNoise_B_HDL3", (double)SNoiseData.SubFrameSNoiseData[1].ColSNoise);
		setResult("SNoise_ColSNoise_R_HDL3", (double)SNoiseData.SubFrameSNoiseData[2].ColSNoise);
		setResult("SNoise_ColSNoise_Gr_HDL3", (double)SNoiseData.SubFrameSNoiseData[3].ColSNoise);
	}
	else
		return -1;

	APSYShadingType YShadingRes;
	if (check_ret(__func__, YShadingAPS(handle, 0, nNumber, nullptr, &YShadingRes)))
	{
		setResult("YShading_1_1_HDL3", (double)YShadingRes.YShadingData[0][0]);
		setResult("YShading_1_2_HDL3", (double)YShadingRes.YShadingData[0][1]);
		setResult("YShading_1_3_HDL3", (double)YShadingRes.YShadingData[0][2]);
		setResult("YShading_1_4_HDL3", (double)YShadingRes.YShadingData[0][3]);
		setResult("YShading_1_5_HDL3", (double)YShadingRes.YShadingData[0][4]);
		setResult("YShading_2_1_HDL3", (double)YShadingRes.YShadingData[1][0]);
		setResult("YShading_2_2_HDL3", (double)YShadingRes.YShadingData[1][1]);
		setResult("YShading_2_3_HDL3", (double)YShadingRes.YShadingData[1][2]);
		setResult("YShading_2_4_HDL3", (double)YShadingRes.YShadingData[1][3]);
		setResult("YShading_2_5_HDL3", (double)YShadingRes.YShadingData[1][4]);
		setResult("YShading_3_1_HDL3", (double)YShadingRes.YShadingData[2][0]);
		setResult("YShading_3_2_HDL3", (double)YShadingRes.YShadingData[2][1]);
		setResult("YShading_3_3_HDL3", (double)YShadingRes.YShadingData[2][2]);
		setResult("YShading_3_4_HDL3", (double)YShadingRes.YShadingData[2][3]);
		setResult("YShading_3_5_HDL3", (double)YShadingRes.YShadingData[2][4]);
		setResult("YShading_4_1_HDL3", (double)YShadingRes.YShadingData[3][0]);
		setResult("YShading_4_2_HDL3", (double)YShadingRes.YShadingData[3][1]);
		setResult("YShading_4_3_HDL3", (double)YShadingRes.YShadingData[3][2]);
		setResult("YShading_4_4_HDL3", (double)YShadingRes.YShadingData[3][3]);
		setResult("YShading_4_5_HDL3", (double)YShadingRes.YShadingData[3][4]);
		setResult("YShading_5_1_HDL3", (double)YShadingRes.YShadingData[4][0]);
		setResult("YShading_5_2_HDL3", (double)YShadingRes.YShadingData[4][1]);
		setResult("YShading_5_3_HDL3", (double)YShadingRes.YShadingData[4][2]);
		setResult("YShading_5_4_HDL3", (double)YShadingRes.YShadingData[4][3]);
		setResult("YShading_5_5_HDL3", (double)YShadingRes.YShadingData[4][4]);
		setResult("YShadingLT_HDL3", (double)YShadingRes.YShadingLT);
		setResult("YShadingLB_HDL3", (double)YShadingRes.YShadingLB);
		setResult("YShadingRT_HDL3", (double)YShadingRes.YShadingRT);
		setResult("YShadingRB_HDL3", (double)YShadingRes.YShadingRB);
	}
	else
		return -1;

	APSColorShadingType ColorShadingRes;
	if (check_ret(__func__, ColorShadingAPS(handle, 0, nNumber, nullptr, &ColorShadingRes)))
	{
		for (int i = 0; i < 13; i++)
		{
			for (int j = 0; j < 17; j++)
			{
				setResult("ColorShading_RG_" + std::to_string(i + 1) + "_" + std::to_string(j + 1) + "_HDL3", (double)ColorShadingRes.ColorShadingRGData[i][j]);
			}
		}
		for (int i = 0; i < 13; i++)
		{
			for (int j = 0; j < 17; j++)
			{
				setResult("ColorShading_BG_" + std::to_string(i + 1) + "_" + std::to_string(j + 1) + "_HDL3", (double)ColorShadingRes.ColorShadingBGData[0][0]);
			}
		}
		setResult("ColorShadingRGLT_HDL3", (double)ColorShadingRes.ColorShadingRGLT);
		setResult("ColorShadingRGLB_HDL3", (double)ColorShadingRes.ColorShadingRGLB);
		setResult("ColorShadingRGRT_HDL3", (double)ColorShadingRes.ColorShadingRGRT);
		setResult("ColorShadingRGRB_HDL3", (double)ColorShadingRes.ColorShadingRGRB);
		setResult("ColorShadingBGLT_HDL3", (double)ColorShadingRes.ColorShadingBGLT);
		setResult("ColorShadingBGLB_HDL3", (double)ColorShadingRes.ColorShadingBGLB);
		setResult("ColorShadingBGRT_HDL3", (double)ColorShadingRes.ColorShadingBGRT);
		setResult("ColorShadingBGRB_HDL3", (double)ColorShadingRes.ColorShadingBGRB);
	}
	else
		return -1;

	APSOpticalCenterType OpticalCenterRes;
	if (check_ret(__func__, OpticalCenterAPS(handle, nIndexStart, nNumber, nullptr, &OpticalCenterRes)))
	{
		setResult("OpticalCenter_CenterRow_HDL3", (double)OpticalCenterRes.CenterRow);
		setResult("OpticalCenter_CenterCol_HDL3", (double)OpticalCenterRes.CenterCol);
	}
	else
		return -1;

	DeleteHandleAPS(handle);
	delete [] rawDataBuf;
	rawDataBuf = NULL;
	return 0;
}
#endif //ALGORITHMLIBRARY_HDL3_H
