#include "ImageTest.h"
#include <string>
#include <vector>
#include <io.h>
#include <fstream>
#include <iostream>
#include "AlpMPAlgoInterface.h"

static CAlpDVSMPAlgoInterface* gDVSInterface = nullptr;

void FindFiles(std::string strPath, std::vector<std::string>& FileQuene)
{
	_finddata_t file_info;
	std::string current_path = strPath + "\\*";
	long long handle = _findfirst(current_path.c_str(), &file_info);
	uint32_t file_num = 0;
	//FileQuene.clear();
	if (-1 == handle)
		return;
	do
	{
		std::string attribute;
		if ((file_info.attrib & _A_SUBDIR) == 0)
		{
			std::string name = file_info.name;
			if (name.find(".bin") != std::string::npos)
			{
				FileQuene.push_back(strPath + "/" + file_info.name);
			}
		}
		else
		{
			if (file_info.name[0] != '.')
			{
				FindFiles(strPath + "/" + file_info.name, FileQuene);
			}
		}

		file_num++;

	} while (!_findnext(handle, &file_info));
	_findclose(handle);
	return;
}

void GetCenterImageSensitivityData(std::string datapath, std::string outpath)
{
	std::vector<std::string> FileQuene;

	gDVSInterface = CreateDVSAlgoInterface(ALP_003BA, "D:/");
	gDVSInterface->SetMultiThreadEnable(true);
	gDVSInterface->SetLogEnable(true);

	bool bRet = false;
	clock_t time = 0;
	FindFiles(datapath, FileQuene);

	std::ofstream imageData;
	imageData.open(outpath, std::ios::out | std::ios::trunc);

	for (uint32_t nIndex = 0; nIndex < FileQuene.size(); nIndex++)
	{
		std::ifstream infile;
		infile.open(FileQuene[nIndex], std::ios::binary | std::ios::in);
		if (!infile.fail())
		{
			infile.seekg(0, std::ios::end);
			uint64_t length = infile.tellg();
			infile.seekg(0, std::ios::beg);
			uint8_t* pRawData = new uint8_t[length];
			infile.read((char*)pRawData, length);
			infile.close();

			auto start = clock();
			bRet = gDVSInterface->ImportRawData(pRawData, length, 0, 160);
			auto end = clock();
			time = end - start;
			delete[] pRawData;
		}
		else
		{
			bRet = false;
		}
		if (bRet)
		{
			std::cout << "ImportData pass " << time << "," << nIndex + 1 << "/" << FileQuene.size() << std::endl;

			SpatialResponseUniformityData data;
			gDVSInterface->SpatialResponseUniformity(20, 60, nullptr, 3, On_OffEvents, data);

			//std::string strSub1 = FileQuene[nIndex].substr(0, FileQuene[nIndex].find_last_of('/'));
			//std::string strSub2 = strSub1.substr(0, strSub1.find_last_of('/'));
			//std::string strSub3 = strSub2.substr(0, strSub2.find_last_of('/'));
			//std::string Lux = strSub2.substr(strSub3.size() + 1);
			//std::string Ratio = strSub1.substr(strSub2.size() + 1);

			if (!imageData.fail())
			{
				imageData << FileQuene[nIndex] << ",";

				double CenterOn = data.OnEventsUniformityBlockData[4][1][1] + data.OnEventsUniformityBlockData[4][1][2] + data.OnEventsUniformityBlockData[4][1][3];
				CenterOn += data.OnEventsUniformityBlockData[4][2][1] + data.OnEventsUniformityBlockData[4][2][2] + data.OnEventsUniformityBlockData[4][2][3];
				CenterOn += data.OnEventsUniformityBlockData[4][3][1] + data.OnEventsUniformityBlockData[4][3][2] + data.OnEventsUniformityBlockData[4][3][3];
				CenterOn /= 9;

				double CenterOff = data.OffEventsUniformityBlockData[4][1][1] + data.OffEventsUniformityBlockData[4][1][2] + data.OffEventsUniformityBlockData[4][1][3];
				CenterOff += data.OffEventsUniformityBlockData[4][2][1] + data.OffEventsUniformityBlockData[4][2][2] + data.OffEventsUniformityBlockData[4][2][3];
				CenterOff += data.OffEventsUniformityBlockData[4][3][1] + data.OffEventsUniformityBlockData[4][3][2] + data.OffEventsUniformityBlockData[4][3][3];
				CenterOff /= 9;

				imageData << CenterOn << ",";
				imageData << CenterOff << std::endl;
			}
		}
		else
		{
			std::cout << "ImportData fail " << std::endl;
		}
	}
	delete gDVSInterface;
	imageData.close();
}

void GetImageSensitivityData(std::string datapath, std::string outpath)
{
	std::vector<std::string> FileQuene;

	gDVSInterface = CreateDVSAlgoInterface(ALP_003BA, "D:/");
	gDVSInterface->SetMultiThreadEnable(true);
	gDVSInterface->SetLogEnable(true);

	bool bRet = false;
	clock_t time = 0;
	FindFiles(datapath, FileQuene);

	std::ofstream imageData;
	imageData.open(outpath, std::ios::out | std::ios::trunc);

	for (uint32_t nIndex = 0; nIndex < FileQuene.size(); nIndex++)
	{
		std::ifstream infile;
		infile.open(FileQuene[nIndex], std::ios::binary | std::ios::in);
		if (!infile.fail())
		{
			infile.seekg(0, std::ios::end);
			uint64_t length = infile.tellg();
			infile.seekg(0, std::ios::beg);
			uint8_t* pRawData = new uint8_t[length];
			infile.read((char*)pRawData, length);
			infile.close();

			auto start = clock();
			bRet = gDVSInterface->ImportRawData(pRawData, length, 0, 160);
			auto end = clock();
			time = end - start;
			delete[] pRawData;
		}
		else
		{
			bRet = false;
		}
		if (bRet)
		{
			std::cout << "ImportData pass " << time << "," << nIndex + 1 << "/" << FileQuene.size() << std::endl;

			ImageContrastSensitivityData data;
			gDVSInterface->ImageContrastSensitivity(80, 80, nullptr, 3, On_OffEvents, data);

			std::string strSub1 = FileQuene[nIndex].substr(0, FileQuene[nIndex].find_last_of('/'));
			std::string strSub2 = strSub1.substr(0, strSub1.find_last_of('/'));
			std::string strSub3 = strSub2.substr(0, strSub2.find_last_of('/'));
			std::string Lux = strSub2.substr(strSub3.size() + 1);
			std::string Ratio = strSub1.substr(strSub2.size() + 1);

			if (!imageData.fail())
			{
				imageData << Lux << ", " << Ratio << ", " << std::to_string(data.OnEventsRatio[All]) << ", " << std::to_string(data.OnEventsRatio[0]) << ", " << std::to_string(data.OnEventsRatio[1]) << ", " << std::to_string(data.OnEventsRatio[2]) << ", " << std::to_string(data.OnEventsRatio[3]) << ", ";
				imageData << std::to_string(data.OffEventsRatio[All]) << ", " << std::to_string(data.OffEventsRatio[0]) << ", " << std::to_string(data.OffEventsRatio[1]) << ", " << std::to_string(data.OffEventsRatio[2]) << ", " << std::to_string(data.OffEventsRatio[3]) << ", " << std::endl;
			}
		}
		else
		{
			std::cout << "ImportData fail " << std::endl;
		}
	}
	delete gDVSInterface;
	imageData.close();
}

void GetSensitivityUniformityData(std::string datapath, std::string outpath)
{
	std::vector<std::string> FileQuene;

	gDVSInterface = CreateDVSAlgoInterface(ALP_003BA, "D:/");
	gDVSInterface->SetMultiThreadEnable(true);
	gDVSInterface->SetLogEnable(true);

	bool bRet = false;
	clock_t time = 0;
	FindFiles(datapath, FileQuene);

	std::ofstream imageData;
	imageData.open(outpath, std::ios::out | std::ios::trunc);

	for (uint32_t nIndex = 0; nIndex < FileQuene.size(); nIndex++)
	{
		std::ifstream infile;
		infile.open(FileQuene[nIndex], std::ios::binary | std::ios::in);
		if (!infile.fail())
		{
			infile.seekg(0, std::ios::end);
			uint64_t length = infile.tellg();
			infile.seekg(0, std::ios::beg);
			uint8_t* pRawData = new uint8_t[length];
			infile.read((char*)pRawData, length);
			infile.close();

			auto start = clock();
			bRet = gDVSInterface->ImportRawData(pRawData, length, 0, 160);
			auto end = clock();
			time = end - start;
			delete[] pRawData;
		}
		else
		{
			bRet = false;
		}
		if (bRet)
		{
			std::cout << "ImportData pass " << time << "," << nIndex + 1 << "/" << FileQuene.size() << std::endl;

			SpatialResponseUniformityData data;

			auto thre = gDVSInterface->GetAlgorithmThre();
			thre.nSpatialResponseUniformityColBlockNum = 5;
			thre.nSpatialResponseUniformityRowBlockNum = 5;
			gDVSInterface->SetAlgorithmThre(thre);
			gDVSInterface->SpatialResponseUniformity(70, 80, nullptr, 3, On_OffEvents, data);

			std::string strSub1 = FileQuene[nIndex].substr(0, FileQuene[nIndex].find_last_of('/'));
			std::string strSub2 = strSub1.substr(0, strSub1.find_last_of('/'));
			std::string strSub3 = strSub2.substr(0, strSub2.find_last_of('/'));
			std::string Lux = strSub2.substr(strSub3.size() + 1);
			std::string Ratio = strSub1.substr(strSub2.size() + 1);

			if (!imageData.fail())
			{
				imageData << Lux << ", " << Ratio << ", " << std::to_string(data.dOnEventsUniformityRatio[All]) << ", " << std::to_string(data.dOnEventsUniformityRatio[0]) << ", " << std::to_string(data.dOnEventsUniformityRatio[1]) << ", " << std::to_string(data.dOnEventsUniformityRatio[2]) << ", " << std::to_string(data.dOnEventsUniformityRatio[3]) << ", ";
				imageData << std::to_string(data.dOffEventsUniformityRatio[All]) << ", " << std::to_string(data.dOffEventsUniformityRatio[0]) << ", " << std::to_string(data.dOffEventsUniformityRatio[1]) << ", " << std::to_string(data.dOffEventsUniformityRatio[2]) << ", " << std::to_string(data.dOffEventsUniformityRatio[3]) << ", " << std::endl;
			}
		}
		else
		{
			std::cout << "ImportData fail " << std::endl;
		}
	}
	delete gDVSInterface;
	imageData.close();
}

void GetSensitivityUniformityData2(std::string datapath, std::string outpath)
{
	std::vector<std::string> FileQuene;

	gDVSInterface = CreateDVSAlgoInterface(ALP_003BA, "D:/");
	gDVSInterface->SetMultiThreadEnable(true);
	gDVSInterface->SetLogEnable(true);

	bool bRet = false;
	clock_t time = 0;
	FindFiles(datapath, FileQuene);

	std::ofstream imageData;
	imageData.open(outpath, std::ios::out | std::ios::trunc);

	for (uint32_t nIndex = 0; nIndex < FileQuene.size(); nIndex++)
	{
		std::ifstream infile;
		infile.open(FileQuene[nIndex], std::ios::binary | std::ios::in);
		if (!infile.fail())
		{
			infile.seekg(0, std::ios::end);
			uint64_t length = infile.tellg();
			infile.seekg(0, std::ios::beg);
			uint8_t* pRawData = new uint8_t[length];
			infile.read((char*)pRawData, length);
			infile.close();

			auto start = clock();
			bRet = gDVSInterface->ImportRawData(pRawData, length, 0, 1600);
			auto end = clock();
			time = end - start;
			delete[] pRawData;
		}
		else
		{
			bRet = false;
		}
		if (bRet)
		{
			std::cout << "ImportData pass " << time << "," << nIndex + 1 << "/" << FileQuene.size() << std::endl;

			SpatialResponseUniformityData data;

			auto thre = gDVSInterface->GetAlgorithmThre();
			thre.nSpatialResponseUniformityColBlockNum = 5;
			thre.nSpatialResponseUniformityRowBlockNum = 5;
			gDVSInterface->SetAlgorithmThre(thre);


			imageData << FileQuene[nIndex] << ", ";
			for (uint32_t n = 0; n < 10; n++)
			{
				gDVSInterface->SpatialResponseUniformity(n * 160 + 80, 80, nullptr, 3, On_OffEvents, data);
				if (!imageData.fail())
				{
					imageData << std::to_string(data.dOnEventsUniformityRatio[All]) << ", " << std::to_string(data.dOffEventsUniformityRatio[All]) << ", ";
				}

			}

			imageData << std::endl;
		}
		else
		{
			std::cout << "ImportData fail " << std::endl;
		}
	}
	delete gDVSInterface;
	imageData.close();
}

void GetStationaryNoise(std::string datapath, std::string outpath, uint32_t nIndexStart, uint32_t nDataLen)
{
	std::vector<std::string> FileQuene;

	gDVSInterface = CreateDVSAlgoInterface(ALP_003BA, "D:/");
	gDVSInterface->SetMultiThreadEnable(true);
	gDVSInterface->SetLogEnable(true);

	bool bRet = false;
	clock_t time = 0;
	FindFiles(datapath, FileQuene);

	std::ofstream imageData;
	imageData.open(outpath, std::ios::out | std::ios::trunc);

	for (uint32_t nIndex = 0; nIndex < FileQuene.size(); nIndex++)
	{
		std::ifstream infile;
		infile.open(FileQuene[nIndex], std::ios::binary | std::ios::in);
		if (!infile.fail())
		{
			infile.seekg(0, std::ios::end);
			uint64_t length = infile.tellg();
			infile.seekg(0, std::ios::beg);
			uint8_t* pRawData = new uint8_t[length];
			infile.read((char*)pRawData, length);
			infile.close();

			auto start = clock();
			bRet = gDVSInterface->ImportRawData(pRawData, length, 0, 400);
			auto end = clock();
			time = end - start;
			delete[] pRawData;
		}
		else
		{
			bRet = false;
		}
		if (bRet)
		{
			std::cout << "ImportData pass " << time << "," << nIndex + 1 << "/" << FileQuene.size() << std::endl;

			StationaryNoiseData Sdata;
			StationaryUniformityData SUdata;
			HotpixelData Hdata;

			auto thre = gDVSInterface->GetAlgorithmThre();
			thre.nStationaryUniformityColBlockNum = 5;
			thre.nStationaryUniformityRowBlockNum = 5;
			thre.dHotPixelThre = 0.7;
			gDVSInterface->SetAlgorithmThre(thre);

			gDVSInterface->StationaryNoise(nIndexStart, nDataLen, Sdata);
			gDVSInterface->StationaryUniformity(nIndexStart, nDataLen, SUdata);
			gDVSInterface->HotPixel(nIndexStart, nDataLen, Hdata);

			std::string strSub1 = FileQuene[nIndex].substr(0, FileQuene[nIndex].find_last_of('/'));
			std::string strSub2 = strSub1.substr(0, strSub1.find_last_of('/'));
			std::string strSub3 = strSub2.substr(0, strSub2.find_last_of('/'));
			std::string Lux = strSub2.substr(strSub3.size() + 1);
			std::string Ratio = strSub1.substr(strSub2.size() + 1);

			if (!imageData.fail())
			{
				imageData << Ratio << ",";
				imageData << Sdata.dStationaryNoiseMeanAll << ",";
				imageData << Sdata.dStationaryNoiseMeanOn << ",";
				imageData << Sdata.dStationaryNoiseMeanOff << ",";
				imageData << Sdata.dStationaryNoiseStdAll << ",";
				imageData << Sdata.dStationaryNoiseStdOn << ",";
				imageData << Sdata.dStationaryNoiseStdOff << ",";
				imageData << Sdata.dStationaryRowSNoise << ",";
				imageData << Sdata.dStationaryColSNoise << ",";

				imageData << SUdata.UniformityRatio << ",";

				imageData << Hdata.HotPixelNum << ",";

				imageData << std::endl;
			}


			if (!imageData.fail())
			{
				//imageData << Lux << ", " << Ratio << ", " << std::to_string(data.dOnEventsUniformityRatio[All]) << ", " << std::to_string(data.dOnEventsUniformityRatio[0]) << ", " << std::to_string(data.dOnEventsUniformityRatio[1]) << ", " << std::to_string(data.dOnEventsUniformityRatio[2]) << ", " << std::to_string(data.dOnEventsUniformityRatio[3]) << ", ";
				//imageData << std::to_string(data.dOffEventsUniformityRatio[All]) << ", " << std::to_string(data.dOffEventsUniformityRatio[0]) << ", " << std::to_string(data.dOffEventsUniformityRatio[1]) << ", " << std::to_string(data.dOffEventsUniformityRatio[2]) << ", " << std::to_string(data.dOffEventsUniformityRatio[3]) << ", " << std::endl;
			}
		}
		else
		{
			std::cout << "ImportData fail " << std::endl;
		}
	}
	delete gDVSInterface;
	imageData.close();
}


void GetStationaryNoise2(std::string datapath, std::string outpath, uint32_t nIndexStart)
{
	std::vector<std::string> FileQuene;

	gDVSInterface = CreateDVSAlgoInterface(ALP_003BA, "D:/");
	gDVSInterface->SetMultiThreadEnable(true);
	gDVSInterface->SetLogEnable(true);

	bool bRet = false;
	clock_t time = 0;
	FindFiles(datapath, FileQuene);

	std::ofstream imageData;
	imageData.open(outpath, std::ios::out | std::ios::trunc);

	for (uint32_t nIndex = 0; nIndex < FileQuene.size(); nIndex++)
	{
		std::ifstream infile;
		infile.open(FileQuene[nIndex], std::ios::binary | std::ios::in);
		if (!infile.fail())
		{
			infile.seekg(0, std::ios::end);
			uint64_t length = infile.tellg();
			infile.seekg(0, std::ios::beg);
			uint8_t* pRawData = new uint8_t[length];
			infile.read((char*)pRawData, length);
			infile.close();

			auto start = clock();
			bRet = gDVSInterface->ImportRawData(pRawData, length, 0, 400);
			auto end = clock();
			time = end - start;
			delete[] pRawData;
		}
		else
		{
			bRet = false;
		}
		if (bRet)
		{
			std::cout << "ImportData pass " << time << "," << nIndex + 1 << "/" << FileQuene.size() << std::endl;

			for (uint32_t nDataLen = 10; nDataLen <= 50; nDataLen += 10)
			{
				StationaryNoiseData Sdata;
				StationaryUniformityData SUdata;
				HotpixelData Hdata;

				auto thre = gDVSInterface->GetAlgorithmThre();
				thre.nStationaryUniformityColBlockNum = 5;
				thre.nStationaryUniformityRowBlockNum = 5;
				thre.dHotPixelThre = 0.7;
				gDVSInterface->SetAlgorithmThre(thre);

				gDVSInterface->StationaryNoise(nIndexStart, nDataLen, Sdata);
				gDVSInterface->StationaryUniformity(nIndexStart, nDataLen, SUdata);
				gDVSInterface->HotPixel(nIndexStart, nDataLen, Hdata);

				if (!imageData.fail())
				{
					imageData << nDataLen << ",";
					imageData << Sdata.dStationaryNoiseMeanAll << ",";
					imageData << Sdata.dStationaryNoiseMeanOn << ",";
					imageData << Sdata.dStationaryNoiseMeanOff << ",";
					imageData << Sdata.dStationaryNoiseStdAll << ",";
					imageData << Sdata.dStationaryNoiseStdOn << ",";
					imageData << Sdata.dStationaryNoiseStdOff << ",";
					imageData << Sdata.dStationaryRowSNoise << ",";
					imageData << Sdata.dStationaryColSNoise << ",";

					imageData << SUdata.UniformityRatio << ",";

					imageData << Hdata.HotPixelNum << ",";

					imageData << std::endl;
				}
			}

			//std::string strSub1 = FileQuene[nIndex].substr(0, FileQuene[nIndex].find_last_of('/'));
			//std::string strSub2 = strSub1.substr(0, strSub1.find_last_of('/'));
			//std::string strSub3 = strSub2.substr(0, strSub2.find_last_of('/'));
			//std::string Lux = strSub2.substr(strSub3.size() + 1);
			//std::string Ratio = strSub1.substr(strSub2.size() + 1);

			if (!imageData.fail())
			{
				//imageData << Lux << ", " << Ratio << ", " << std::to_string(data.dOnEventsUniformityRatio[All]) << ", " << std::to_string(data.dOnEventsUniformityRatio[0]) << ", " << std::to_string(data.dOnEventsUniformityRatio[1]) << ", " << std::to_string(data.dOnEventsUniformityRatio[2]) << ", " << std::to_string(data.dOnEventsUniformityRatio[3]) << ", ";
				//imageData << std::to_string(data.dOffEventsUniformityRatio[All]) << ", " << std::to_string(data.dOffEventsUniformityRatio[0]) << ", " << std::to_string(data.dOffEventsUniformityRatio[1]) << ", " << std::to_string(data.dOffEventsUniformityRatio[2]) << ", " << std::to_string(data.dOffEventsUniformityRatio[3]) << ", " << std::endl;
			}
		}
		else
		{
			std::cout << "ImportData fail " << std::endl;
		}
	}
	delete gDVSInterface;
	imageData.close();
}
