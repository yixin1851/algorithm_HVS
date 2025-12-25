//
// Created by xinfeng.weng on 2025/12/25.
//

#ifndef ALGORITHMLIBRARY_DL2_H
#define ALGORITHMLIBRARY_DL2_H
#include "public.h"
#include "AlpMPAlgoCTypeInterface.h"

int calcDL2()
{
    printf("\n - APS DL2 Start\r\n");
	//导入数据
	std::string F1 = "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_25FPS_5000K_Light_DL2_3080Lux_13p0ms_VTX1P5V_60C_G1_F1_W3264H2448P10_T2K-CP1-4S-LY_20251013220516.raw";
	std::string F2 = "D:/Work/Tmp/APX003CA/TestData_CP1/APX003CB_APS_513331333139382D05033279802E85020324_X121Y128_S2_25FPS_5000K_Light_DL2_3080Lux_13p0ms_VTX1P5V_60C_G1_F2_W3264H2448P10_T2K-CP1-4S-LY_20251013220516.raw";
	int nWidth = 3264;
	int nHeight = 2448;
	uint32_t nNumber = 2;
	long rawDataBufLen = nWidth * nHeight * 2;
	unsigned long rawDataRealLen = 0;
	uint8_t *rawDataBuf = new uint8_t[rawDataBufLen * nNumber];
	ImageCapture_capture(F1, rawDataBuf, rawDataBufLen, rawDataRealLen, 0);
	ImageCapture_capture(F2, rawDataBuf + rawDataBufLen * 1, rawDataBufLen, rawDataRealLen, 0);

	// 3. 调用DLL函数
	HANDLE handle = InitHandleAPS(SensorType::ALP_003CA, APSRawType::UNPACK10, "./",PixelFormatType::QuadBayerGBRG, 0);
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

	ROIArea* ROI = nullptr;
	ROIArea ROI_temp;
	ROI_temp.Up = int((nHeight / 2.0) / 20.0 * 9.0);
	ROI_temp.Down = int((nHeight / 2.0) / 20.0 * 11.0) - 1;
	ROI_temp.Left = int((nWidth / 2.0) / 20.0 * 9.0);
	ROI_temp.Right = int((nWidth / 2.0) / 20.0 * 11.0) - 1;
	APSDataMeanType DataMean;
	if (check_ret(__func__, DataMeanAPS(handle, 0, nNumber, ROI, &DataMean)))
	{
		setResult("DataMean_Total_DL2", (double)DataMean.DataMeanFrame);
		setResult("DataMean_Gb_DL2", (double)DataMean.SubFrameDataMean[0]);
		setResult("DataMean_B_DL2", (double)DataMean.SubFrameDataMean[1]);
		setResult("DataMean_R_DL2", (double)DataMean.SubFrameDataMean[2]);
		setResult("DataMean_Gr_DL2", (double)DataMean.SubFrameDataMean[3]);
	}
	else{
		return -1;
	}

	APSDataMeanType DataMean_Center;
	if (check_ret(__func__, DataMeanAPS(handle, 0, nNumber, &ROI_temp, &DataMean_Center)))
	{
		setResult("DataMean_Center_Total_DL2", (double)DataMean_Center.DataMeanFrame);
		setResult("DataMean_Center_Gb_DL2", (double)DataMean_Center.SubFrameDataMean[0]);
		setResult("DataMean_Center_B_DL2", (double)DataMean_Center.SubFrameDataMean[1]);
		setResult("DataMean_Center_R_DL2", (double)DataMean_Center.SubFrameDataMean[2]);
		setResult("DataMean_Center_Gr_DL2", (double)DataMean_Center.SubFrameDataMean[3]);
	}
	else{
		return -1;
	}

	DeleteHandleAPS(handle);
	delete [] rawDataBuf;
	rawDataBuf = NULL;
	return 0;
}
#endif //ALGORITHMLIBRARY_DL2_H
