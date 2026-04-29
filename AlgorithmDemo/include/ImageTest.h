#pragma once

#include <string>
#include <vector>

void GetImageSensitivityData(std::string datapath, std::string outpath);

void GetSensitivityUniformityData(std::string datapath, std::string outpath);
void GetSensitivityUniformityData2(std::string datapath, std::string outpath);

void GetStationaryNoise(std::string datapath, std::string outpath, uint32_t nIndexStart, uint32_t nDataLen);

void GetCenterImageSensitivityData(std::string datapath, std::string outpath);

void Get16SubframeRaw(std::string datapath);

void GetOETC(std::string datapath);

void GetLinearity(std::string datapath);

void GetPedestalVariation(std::string datapath);

void GetDSNU(std::string datapath);

void GetFPN(std::string datapath);

void GetTNoise(std::string datapath);

void GetReadNoise(std::string datapath);

void GetDefectPixelsDark(std::string datapath);

void GetDefectPixelsLight(std::string datapath);

void DPC_On_ChipTest();

void FindFiles(std::string strPath, std::vector<std::string>& FileQuene);

void saveVectorToCSV(const std::vector<double>& data, int cols, const std::string& filename);

void CalcHotPixel();

void CalcBadpixelEVS();
