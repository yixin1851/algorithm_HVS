#include "../ImageTest.h"
#include <string>
#include <vector>
#include <io.h>
#include <fstream>
#include <iostream>
#include "../AlpMPAlgoInterface.h"

void DPC_On_ChipTest()
{
	std::string darkdatapath = "D:/Data/FPN&Temporal_noise&Defect_Pixels_dark/65mS_15fps_16xGain";
	std::string lightdatapath = "D:/Data/Defect_Pixels_light&light_noise/RAW_GRAY";

	std::vector<std::string> FileQuene;

	CAlpAPSMPAlgoInterface *gAPSInterface = CreateAPSAlgoInterface(ALP_003CA, UNPACK10, "D:/", QuadBayerGBRG, 0);
	gAPSInterface->SetMultiThreadEnable(true);
	gAPSInterface->SetLogEnable(true);

	APSBadpixelType hotpixel;
	APSBadpixelType badpixel;

	bool bRet = false;

	FindFiles(darkdatapath, FileQuene);

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
		gAPSInterface->HotPixel(0, 5, nullptr, hotpixel);

		std::cout << "Singlets:" << hotpixel.SingletNum << std::endl;
		std::cout << "Couplets:" << hotpixel.CoupletNum << std::endl;
		std::cout << "Ladders:" << hotpixel.LadderNum << std::endl;
		std::cout << "Clusters:" << hotpixel.ClusterNum << std::endl;

	}
	else
	{
		std::cout << "ImportData fail " << std::endl;
	}

	FileQuene.clear();
	FindFiles(lightdatapath, FileQuene);

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

			bRet = gAPSInterface->ImportRawData(pRawData, length, nIndex + 5, 1);
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
		gAPSInterface->BLC(5, 5);
		gAPSInterface->BadPixel(5, 5, nullptr, badpixel);

		std::cout << "Singlets:" << badpixel.SingletNum << std::endl;
		std::cout << "Couplets:" << badpixel.CoupletNum << std::endl;
		std::cout << "Ladders:" << badpixel.LadderNum << std::endl;
		std::cout << "Clusters:" << badpixel.ClusterNum << std::endl;

	}
	else
	{
		std::cout << "ImportData fail " << std::endl;
	}

	int max_pixel_in_otp = 200;

	std::vector<Local> pixel_local;
	for (int n = 0; n < badpixel.BadPixelMask.BadPixelNum; n++)
	{
		if (pixel_local.size() >= max_pixel_in_otp)
		{
			break;
		}
		if (APX003CA_ON_CHIP_CALIBRATION_FLAG == badpixel.BadPixelMask.Flag[n])
		{
			pixel_local.push_back(badpixel.BadPixelMask.LocalData[n]);
		}
	}
	for (int n = 0; n < hotpixel.BadPixelMask.BadPixelNum; n++)
	{
		if (pixel_local.size() >= max_pixel_in_otp)
		{
			break;
		}
		if (APX003CA_ON_CHIP_CALIBRATION_FLAG == hotpixel.BadPixelMask.Flag[n])
		{
			pixel_local.push_back(hotpixel.BadPixelMask.LocalData[n]);
		}
	}
	gAPSInterface->DPC(0, 10, nullptr, pixel_local);
	std::vector<uint8_t> otpdata;
	gAPSInterface->BadPixelLocalToOtpType(pixel_local, otpdata);

}