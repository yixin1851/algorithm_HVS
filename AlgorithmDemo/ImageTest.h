#pragma once

#include <string>

void GetImageSensitivityData(std::string datapath, std::string outpath);

void GetSensitivityUniformityData(std::string datapath, std::string outpath);
void GetSensitivityUniformityData2(std::string datapath, std::string outpath);

void GetStationaryNoise(std::string datapath, std::string outpath, uint32_t nIndexStart, uint32_t nDataLen);

void GetCenterImageSensitivityData(std::string datapath, std::string outpath);