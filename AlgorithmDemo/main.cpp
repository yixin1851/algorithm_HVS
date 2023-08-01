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

	std::string path = "K:\\share_all\\to_zhangtaiyong\\APX003CA\\APX003CA_RGB芯片打平行光图像异常现象_20230728\\参考数据\\349685742_Unpack10_20230606T084117_QSCcali.8192X6144.unpack10_rggb.vcmpos_0.raw";
	Get16SubframeRaw(path);
	system("pause");
	return 0;
}