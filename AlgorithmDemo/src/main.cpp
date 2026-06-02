#include "AlpMPAlgoInterface.h"
#include <fstream>
#include <iostream>
#include "ImageTest.h"
#include <thread>
#include <windows.h>


#define LOOP_TEST

int main()
{
#ifdef LOOP_TEST
    while (true) {
#endif
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

	//std::string data1 = "D:\\Data\\EVSģ����������_20230512\\��ӡ������Դ\\003BA-1-2#\\1.5mv";
	//std::string data2 = "D:\\Data\\EVSģ����������_20230512\\��ӡ������Դ\\003BA-1-2#\\3mv";

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

	// std::string path;



	// path = "D:\\Data\\003CA-MONO-1#1\\Pedestal_variation\\x1_33ms_30fps";
	//GetPedestalVariation(path);

	// path = "D:\\014aa\\014AA__2024_11_08\\APS_Optical_Automation_Test_Result_2024_12_09_15_50_40\\Dark";
	// path = "D:\\Data\\014AA\\60_16_66";
	// GetDSNU(path);
	// GetPedestalVariation(path);

	//path = "D:\\Data\\003CA-MONO-1#1\\FPN&Temporal_noise&Defect_Pixels_dark\\x16";
	// GetFPN(path);

	//path = "D:\\Data\\003CA-MONO-1#1\\FPN&Temporal_noise&Defect_Pixels_dark\\x16";
	// GetTNoise(path);

	//path = "D:\\Data\\003CA-MONO-1#1\\Defect_Pixels_light&light_noise\\x1";
	//GetTNoise(path);

	//path = "D:\\Data\\003CA-MONO-1#1\\Read_Noise\\x16\\";
	// GetReadNoise(path);

	//path = "D:\\Data\\003CA-MONO-1#1\\FPN&Temporal_noise&Defect_Pixels_dark\\x16";
	// GetDefectPixelsDark(path);

	//path = "D:\\Data\\003CA-MONO-1#1\\Defect_Pixels_light&light_noise\\x1";
	//GetDefectPixelsLight(path);

	// path = "D:\\Data\\003CA-MONO-1#1\\Linearity\\x1\\";
	//GetLinearity(path);

	// path = "D:\\Data\\003CA-MONO-1#1\\OETC\\x1\\";
	//GetOETC(path);

	//DPC_On_ChipTest();
    const int thread_count = 16;
    std::thread threads[thread_count];

    for (int i = 0; i < thread_count; ++i) {
        threads[i] = std::thread([i]() {
            CalcHotPixelEVS();
            // CalcBadpixelEVS();
            // CalcBadpixelAPS();
        });
    }

    for (auto &t : threads) t.join();
#ifdef LOOP_TEST
        Sleep(50);
    }
#endif
	system("pause");
	return 0;
}