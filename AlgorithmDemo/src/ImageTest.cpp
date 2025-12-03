#include "../ImageTest.h"
#include <string>
#include <vector>
#include <io.h>
#include <fstream>
#include <iostream>
#include "../AlpMPAlgoInterface.h"

static CAlpDVSMPAlgoInterface* gDVSInterface = nullptr;
static CAlpAPSMPAlgoInterface* gAPSInterface = nullptr;

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
		if ((file_info.attrib & _A_SUBDIR) != _A_SUBDIR)
		{
			std::string name = file_info.name;
			if (name.find(".raw") != std::string::npos)
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

			DVSSpatialResponseUniformityType data;
			gDVSInterface->SpatialResponseUniformity(20, 60, nullptr, nullptr, 3, On_OffEvents, data);

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

void Get16SubframeRaw(std::string datapath)
{
	std::vector<std::string> FileQuene;

	gAPSInterface = CreateAPSAlgoInterface(ALP_003CA, UNPACK12, "D:/", QuadBayerRGGB, 1);
	gAPSInterface->SetMultiThreadEnable(true);
	gAPSInterface->SetLogEnable(true);
	gAPSInterface->SetRawDataSize(2064,3088);
	gAPSInterface->SetActiveArea({ 0, 2064 / 2 - 1, 0, 3088 / 2 - 1 });
	bool bRet = false;

	std::ifstream infile;
	infile.open(datapath, std::ios::binary | std::ios::in);
	if (!infile.fail())
	{
		infile.seekg(0, std::ios::end);
		uint64_t length = infile.tellg();
		infile.seekg(0, std::ios::beg);
		uint8_t* pRawData = new uint8_t[length];
		infile.read((char*)pRawData, length);
		infile.close();

		bRet = gAPSInterface->ImportRawData(pRawData, length, 0, 1);
		delete[] pRawData;
	}
	else
	{
		bRet = false;
	}
	if (bRet)
	{
		std::ofstream outfile;
		for (uint32_t n = 0; n < 16; n++)
		{
			std::string outpath = ".\\" + std::to_string(n) + ".raw";
			outfile.open(outpath, std::ios::binary | std::ios::trunc);
			APSType Data;
			gAPSInterface->Show(0, 1, nullptr, SubFrameIndex(n), Data);

			for (uint32_t nRow = 0; nRow < Data.size(); nRow++)
			{
				for (uint32_t nCol = 0; nCol < Data[0].size(); nCol++)
				{
					uint8_t a = uint16_t(Data[nRow][nCol]) & 0xFF;
					uint8_t b = (uint16_t(Data[nRow][nCol]) >> 8)& 0xFF;

					outfile << a;
					outfile << b;
				}
			}
			outfile.close();
		}
	}
	else
	{
		std::cout << "ImportData fail " << std::endl;
	}
}

void GetLinearity(std::string datapath)
{
	std::vector<std::string> FileQuene;

	gAPSInterface = CreateAPSAlgoInterface(ALP_014AA, UNPACK10, "D:/", QuadBayerGBRG, 0);
	gAPSInterface->SetLogEnable(true);

	bool bRet = false;

	FindFiles(datapath, FileQuene);

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

			bRet = gAPSInterface->ImportRawData(pRawData, length, nIndex, 1);
			delete[] pRawData;
		}
		else
		{
			bRet = false;
			break;
		}
	}
	if (bRet)
	{
		APSSSNRType res;
		gAPSInterface->Linearity(0, 121, nullptr, SubFrameIndex::Gb, res);

		std::cout << "maxSSNR: " << res.MaxSSNR << std::endl;

		std::ofstream outfile;
		outfile.open("./Linearity.csv", std::ios::trunc);
		outfile << "SNoise, DataMean, SSNR" << std::endl;
		for (uint32_t n = 0; n < res.SNoiseData.size(); n++)
		{
			outfile << res.SNoiseData[n] << ",";
			outfile << res.DataMean[n] << ",";
			outfile << res.SSNR[n] << ",";
			outfile << std::endl;
		}
		outfile.close();
	}
	else
	{
		std::cout << "ImportData fail " << std::endl;
	}
}

void GetPedestalVariation(std::string datapath)
{
	std::vector<std::string> FileQuene;

	gAPSInterface = CreateAPSAlgoInterface(ALP_014AA, UNPACK10, "D:/", QuadBayerGBRG, 0);
	gAPSInterface->SetLogEnable(true);

	bool bRet = false;

	if (datapath.find(".raw") != -1)
	{
		FileQuene.push_back(datapath);
	}
	else
	{
		FindFiles(datapath, FileQuene);
	}
	//FindFiles(datapath, FileQuene);

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

			bRet = gAPSInterface->ImportRawData(pRawData, length, nIndex, 1);
			delete[] pRawData;
		}
		else
		{
			bRet = false;
			break;
		}
	}
	if (bRet)
	{
		APSPedestalVariationType res;
		gAPSInterface->PedestalVariation(0, 1, nullptr, res);

		std::cout << "pedestalGb_Max:" << res.PedestalMax[0] << std::endl;
		std::cout << "pedestalGb_Min:" << res.PedestalMin[0] << std::endl;
	}
	else
	{
		std::cout << "ImportData fail " << std::endl;
	}
}

void GetDSNU(std::string datapath)
{
	std::vector<std::string> FileQuene;

	gAPSInterface = CreateAPSAlgoInterface(ALP_014AA, UNPACK10, "D:/", QuadBayerGBRG, 0);
	gAPSInterface->SetLogEnable(true);
	gAPSInterface->SetMultiThreadEnable(true);
	bool bRet = false;

	FindFiles(datapath, FileQuene);

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

			bRet = gAPSInterface->ImportRawData(pRawData, length, nIndex, 1);
			delete[] pRawData;
		}
		else
		{
			bRet = false;
			break;
		}
	}
	if (bRet)
	{
		APSDSNUType res;
		bool bRet = gAPSInterface->DSNU(0, 5, nullptr, res);

		std::cout << "RangeG:" << res.RangeG << std::endl;
		std::cout << "signal_max:" << res.SignalMax << std::endl;
		std::cout << "deltaSignal_max:" << res.DeltaSignalMax << std::endl;
		std::cout << "deltaSignalLocal_centre_max:" << res.DeltaSignalCentreMax << std::endl;
		std::cout << "deltaSignalLocal_edge_max:" << res.DeltaSignalEdgeMax << std::endl;
		std::cout << "deltaSignalLocal_corner_max:" << res.DeltaSignalCornerMax << std::endl;
	}
	else
	{
		std::cout << "ImportData fail " << std::endl;
	}
}

void GetFPN(std::string datapath)
{
	std::vector<std::string> FileQuene;

	gAPSInterface = CreateAPSAlgoInterface(ALP_014AA, UNPACK10, "D:/", QuadBayerGBRG, 0);
	gAPSInterface->SetLogEnable(true);

	bool bRet = false;

	FindFiles(datapath, FileQuene);

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

			bRet = gAPSInterface->ImportRawData(pRawData, length, nIndex, 1);
			delete[] pRawData;
		}
		else
		{
			bRet = false;
			break;
		}
	}
	if (bRet)
	{
		APSSNoiseType res;
		gAPSInterface->SNoise(0, 5, nullptr, res);

		std::cout << "frame:" << res.SNoiseFrame << std::endl;
		std::cout << "row_Gb:" << res.SubFrameSNoiseData[0].RowSNoise << std::endl;
		std::cout << "col_Gb:" << res.SubFrameSNoiseData[0].ColSNoise << std::endl;
	}
	else
	{
		std::cout << "ImportData fail " << std::endl;
	}
}

void GetTNoise(std::string datapath)
{
	std::vector<std::string> FileQuene;

	gAPSInterface = CreateAPSAlgoInterface(ALP_014AA, UNPACK10, "D:/", QuadBayerGBRG, 0);
	gAPSInterface->SetLogEnable(true);

	bool bRet = false;

	FindFiles(datapath, FileQuene);

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

			bRet = gAPSInterface->ImportRawData(pRawData, length, nIndex, 1);
			delete[] pRawData;
		}
		else
		{
			bRet = false;
			break;
		}
	}
	if (bRet)
	{
		APSTNoiseType res;
		gAPSInterface->TNoise(0, 5, nullptr, res);


		std::cout << "rowTemp_Gb:" << res.SubFrameTNoiseData[0].RowTemp << std::endl;
		std::cout << "colTemp_Gb:" << res.SubFrameTNoiseData[0].ColTemp << std::endl;
		std::cout << "tempRNRatio_Gb:" << res.SubFrameTNoiseData[0].TempRNRatio << std::endl;
		std::cout << "tempRNRatio_Gb:" << res.SubFrameTNoiseData[0].TempCNRatio << std::endl;

	}
	else
	{
		std::cout << "ImportData fail " << std::endl;
	}
}

void GetReadNoise(std::string datapath)
{
	std::vector<std::string> FileQuene;

	gAPSInterface = CreateAPSAlgoInterface(ALP_014AA, UNPACK10, "D:/", QuadBayerGBRG, 0);
	gAPSInterface->SetLogEnable(true);

	bool bRet = false;

	FindFiles(datapath, FileQuene);

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

			bRet = gAPSInterface->ImportRawData(pRawData, length, nIndex, 1);
			delete[] pRawData;
		}
		else
		{
			bRet = false;
			break;
		}
	}
	if (bRet)
	{
		APSReadNoiseType res;
		gAPSInterface->ReadNoise(0, 1, nullptr, res);

		std::cout << "ReadNoise:" << res << std::endl;
	}
	else
	{
		std::cout << "ImportData fail " << std::endl;
	}
}

void GetDefectPixelsDark(std::string datapath)
{
	std::vector<std::string> FileQuene;

	gAPSInterface = CreateAPSAlgoInterface(ALP_014AA, UNPACK10, "D:/", QuadBayerGBRG, 0);
	gAPSInterface->SetLogEnable(true);
	bool bRet = false;

	FindFiles(datapath, FileQuene);

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

			bRet = gAPSInterface->ImportRawData(pRawData, length, nIndex, 1);
			delete[] pRawData;
		}
		else
		{
			bRet = false;
			break;
		}
	}
	if (bRet)
	{
		APSBadpixelType res;
		gAPSInterface->HotPixel(0, 20, nullptr, res);

		std::cout << "Singlets:" << res.SingletNum << std::endl;
		std::cout << "Couplets:" << res.CoupletNum << std::endl;
		std::cout << "Ladders:" << res.LadderNum << std::endl;
		std::cout << "Clusters:" << res.ClusterNum << std::endl;
		std::cout << "MaxClusterSize:" << res.MaxClusterSize << std::endl;
	}
	else
	{
		std::cout << "ImportData fail " << std::endl;
	}
}

void GetDefectPixelsLight(std::string datapath)
{
	std::vector<std::string> FileQuene;

	gAPSInterface = CreateAPSAlgoInterface(ALP_014AA, UNPACK10, "D:/", QuadBayerGBRG, 0);
	gAPSInterface->SetLogEnable(true);

	bool bRet = false;

	FindFiles(datapath, FileQuene);

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

			bRet = gAPSInterface->ImportRawData(pRawData, length, nIndex, 1);
			delete[] pRawData;
		}
		else
		{
			bRet = false;
			break;
		}
	}
	if (bRet)
	{
		APSBadpixelType res;
		auto thre = gAPSInterface->GetAlgorithmThre();
		thre.dBadPixelThre = 0.18;
		gAPSInterface->SetAlgorithmThre(thre);
		gAPSInterface->BadPixel(0, 5, nullptr, res);

		std::cout << "Singlets:" << res.SingletNum << std::endl;
		std::cout << "Couplets:" << res.CoupletNum << std::endl;
		std::cout << "Ladders:" << res.LadderNum << std::endl;
		std::cout << "Clusters:" << res.ClusterNum << std::endl;

	}
	else
	{
		std::cout << "ImportData fail " << std::endl;
	}
}

void GetOETC(std::string datapath)
{
	std::vector<std::string> FileQuene;

	gAPSInterface = CreateAPSAlgoInterface(ALP_014AA, UNPACK10, "D:/", QuadBayerGBRG, 0);
	gAPSInterface->SetLogEnable(true);

	bool bRet = false;

	FindFiles(datapath, FileQuene);

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

			bRet = gAPSInterface->ImportRawData(pRawData, length, nIndex, 1);
			delete[] pRawData;
		}
		else
		{
			bRet = false;
			break;
		}
	}
	if (bRet)
	{
		APSOETCType res;
		gAPSInterface->OETC(0, 46, nullptr, SubFrameIndex::Gb, res);

		std::cout << "DR(db): " << res.DR_dB << std::endl;
		std::cout << "ReadNoise(e-): " << res.ReadNoise_e << std::endl;
		std::cout << "ReadNoise(DN): " << res.ReadNoise << std::endl;
		std::cout << "FWC(e-): " << res.FWC_e << std::endl;
		std::cout << "FWC(DN): " << res.FWC << std::endl;
		std::cout << "ConversionGain: " << res.ConversionGain << std::endl;

		std::ofstream outfile;
		outfile.open("./OETC.csv", std::ios::trunc);
		outfile << "DataMean, ReadNoise, TNoise," << std::endl;
		for (uint32_t n = 0; n < res.ReadNoiseData.size(); n++)
		{
			outfile << res.DataMean[n] << ",";
			outfile << res.ReadNoiseData[n] << ",";
			outfile << res.TNoiseData[n] << ",";
			outfile << std::endl;
		}
		outfile.close();

	}
	else
	{
		std::cout << "ImportData fail " << std::endl;
	}
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

			DVSImageContrastSensitivityType data;
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

			DVSSpatialResponseUniformityType data;

			auto thre = gDVSInterface->GetAlgorithmThre();
			thre.nSpatialResponseUniformityColBlockNum = 5;
			thre.nSpatialResponseUniformityRowBlockNum = 5;
			gDVSInterface->SetAlgorithmThre(thre);
			gDVSInterface->SpatialResponseUniformity(70, 80, nullptr, nullptr, 3, On_OffEvents, data);

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

			DVSSpatialResponseUniformityType data;

			auto thre = gDVSInterface->GetAlgorithmThre();
			thre.nSpatialResponseUniformityColBlockNum = 5;
			thre.nSpatialResponseUniformityRowBlockNum = 5;
			gDVSInterface->SetAlgorithmThre(thre);


			imageData << FileQuene[nIndex] << ", ";
			for (uint32_t n = 0; n < 10; n++)
			{
				gDVSInterface->SpatialResponseUniformity(n * 160 + 80, 80, nullptr, nullptr, 3, On_OffEvents, data);
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

			DVSStationaryNoiseType Sdata;
			DVSStationaryUniformityType SUdata;
			DVSHotpixelType Hdata;

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
				DVSStationaryNoiseType Sdata;
				DVSStationaryUniformityType SUdata;
				DVSHotpixelType Hdata;

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
