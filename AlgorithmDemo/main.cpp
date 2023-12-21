#include "AlpMPAlgoInterface.h"
#include <fstream>
#include <iostream>
#include "ImageTest.h"


//ATE采图接口

int main()
{
	//GetImageSensitivityData();
	//std::string datapath;
	//std::cin >> datapath;
	//uint32_t nStartIndex;
	//std::cin >> nStartIndex;
	//uint32_t nDataLen;
	//std::cin >> nDataLen;

	//GetCenterImageSensitivityData(datapath, datapath + "/CenterImageData.csv");

	//datapath = "Z:/share_all/to_fuenqi/003BA_rawdata/Rigi_0606_cfg/300fps_3mV/Stationary_Noise";
	//nStartIndex = 200;
	//nDataLen = 200;
	//GetStationaryNoise(datapath, datapath + "/StationaryNoise.csv", nStartIndex, nDataLen);

	//datapath = "Z:/share_all/to_fuenqi/003BA_rawdata/Rigi_0606_cfg/500fps_3mV/Stationary_Noise";
	//GetStationaryNoise(datapath, datapath + "/StationaryNoise.csv", nStartIndex, nDataLen);

	//datapath = "Z:/share_all/to_fuenqi/003BA_rawdata/Rigi_0606_cfg/750fps_3mV/Stationary_Noise";
	//GetStationaryNoise(datapath, datapath + "/StationaryNoise.csv", nStartIndex, nDataLen);

	//GetSensitivityUniformityData2(datapath, "./SensitivityUniformity.csv");

	//std::string data1 = "D:\\Data\\EVS模组性能评测_20230512\\正印量产光源\\003BA-1-2#\\1.5mv";
	//std::string data2 = "D:\\Data\\EVS模组性能评测_20230512\\正印量产光源\\003BA-1-2#\\3mv";

	//GetImageSensitivityData(data1, data1 + "/ImageSensitivity.csv");
	//GetImageSensitivityData(data2, data2 + "/ImageSensitivity.csv");
	//GetSensitivityUniformityData(data1, data1 + "/SensitivityUniformity.csv");
	//GetSensitivityUniformityData(data2, data2 + "/SensitivityUniformity.csv");
	//GetSensitivityUniformityData(datapath, datapath + "/SensitivityUniformity.csv");
	//Init();
	//OneTestItem();
	//UnInit();

	//std::string path = "D:\\Data\\taiyong\\RGB12BIT-2.raw";
	//Get16SubframeRaw(path);

	std::string path;



	//path = "D:\\Data\\oppo客户测试需求_小demo#5_v01_360M_v3_oppo文件命名格式\\Pedestal_variation\\Dark_33ms_1.74x_01.raw";
	//GetPedestalVariation(path);

	//path = "D:\\Data\\oppo客户测试需求_小demo#5_v01_360M_v3_oppo文件命名格式\\DSNU\\1.74\\";
	//GetDSNU(path);

	//path = "D:\\Data\\oppo客户测试需求_小demo#5_v01_360M_v3_oppo文件命名格式\\FPN&Temporal_noise&Defect_Pixels_dark\\1.74";
	//GetFPN(path);

	//path = "D:\\Data\\oppo客户测试需求_小demo#5_v01_360M_v3_oppo文件命名格式\\FPN&Temporal_noise&Defect_Pixels_dark\\1.74";
	//GetTNoise(path);

	//path = "D:\\Data\\oppo客户测试需求_小demo#5_v01_360M_v3_oppo文件命名格式\\Defect_Pixels_light&light_noise";
	//GetTNoise(path);

	//path = "D:\\Data\\oppo客户测试需求_小demo#5_v01_360M_v3_oppo文件命名格式\\Read_Noise\\1.74_0.5ms\\";
	//GetReadNoise(path);

	//path = "D:\\Data\\oppo客户测试需求_小demo#5_v01_360M_v3_oppo文件命名格式\\FPN&Temporal_noise&Defect_Pixels_dark\\1.74";
	//GetDefectPixelsDark(path);

	//path = "D:\\Data\\oppo客户测试需求_小demo#5_v01_360M_v3_oppo文件命名格式\\Defect_Pixels_light&light_noise";
	//GetDefectPixelsLight(path);

	//path = "D:\\Data\\oppo客户测试需求_小demo#5_v01_360M_v3_oppo文件命名格式\\Linearity\\1.74x\\";
	//GetLinearity(path);

	//path = "D:\\Data\\oppo客户测试需求_小demo#5_v01_360M_v3_oppo文件命名格式\\OETC\\";
	//GetOETC(path);

	path = "D:\\Data\\Linearity\\x1\\";
	GetLinearity(path);

	//path = "D:\\Data\\OETC\\x1\\";
	//GetOETC(path);

	system("pause");
	return 0;
}