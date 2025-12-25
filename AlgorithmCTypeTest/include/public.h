//
// Created by xinfeng.weng on 2025/12/24.
//

#ifndef ALGORITHMLIBRARY_PUBLIC_H
#define ALGORITHMLIBRARY_PUBLIC_H
#include "stdio.h"
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include "AlpMPAlgoCTypeInterface.h"

struct d_003ca_point3
{
    int x;
    int y;
    int z;

    d_003ca_point3() :x(0), y(0), z(0) {}
    d_003ca_point3(int v_x, int v_y, int v_z) :x(v_x), y(v_y), z(v_z) {}
};

namespace {
    std::map<std::string, APSDataMeanType> RegisterData;
    std::map<std::string, APSBadpixelType> APSBadPixelData;
    std::map<std::string, APSBadpixelType*> APSBadPixelDataPtr;
    std::map<std::string, std::vector<Local>> DPSBadPixelArray;
    std::map<std::string, std::vector<d_003ca_point3>> DPS3DPointArray;
}

int ImageCapture_capture(std::string path, unsigned char rawDataBuf[], unsigned long rawDataBufLen,
                         unsigned long &rawDataRealLen, void *frameInfo) {
    std::ifstream infile;
    infile.open(path, std::ios::binary | std::ios::in);
    if (!infile.fail()) {
        infile.seekg(0, std::ios::end);
        rawDataRealLen = infile.tellg();
        infile.seekg(0, std::ios::beg);
        if (rawDataRealLen > rawDataBufLen)
            rawDataRealLen = rawDataBufLen;
        infile.read((char *) rawDataBuf, rawDataRealLen);
        infile.close();
        return 0;
    } else {
        return -1;
    }
}

void setResult(std::string param, double value) {
    //函数目的，将参数以及值存到寄存器中，这里只是打印值
    std::cout << param << ": " << value << std::endl;
}

bool check_ret(std::string func, int func_ret) {
    if (func_ret != 0) {
        std::cout << func << " error ret:" << func_ret << std::endl;
        return false;
    }
    return true;
};

bool Set_RegisterData(std::string stKey, int iduts, APSDataMeanType& dblValue)
{
    std::string regName;
    regName = stKey + "_" + std::to_string(iduts);

    if (RegisterData.find(regName) == RegisterData.end())
        RegisterData.insert(make_pair(regName, dblValue));
    else
        RegisterData.find(regName)->second = dblValue;

    return true;
}

bool Get_RegisterData(std::string stKey, int iduts, APSDataMeanType& dblValue)
{
    std::string regName;
    regName = stKey + "_" + std::to_string(iduts);

    if (RegisterData.find(regName) == RegisterData.end())
        return false;

    dblValue = RegisterData[regName];
    //dblValue.DataMeanFrame = RegisterData[regName].DataMeanFrame;
    //dblValue.SubFrameDataMean = RegisterData[regName].SubFrameDataMean;
    //std::cout << "DataMean_Total: " << dblValue.DataMeanFrame << std::endl;
    //std::cout << "DataMean_Gb: " << dblValue.SubFrameDataMean[0] << std::endl;
    //std::cout << "DataMean_B: " << dblValue.SubFrameDataMean[1] << std::endl;
    //std::cout << "DataMean_R: " << dblValue.SubFrameDataMean[2] << std::endl;
    //std::cout << "DataMean_Gr: " << dblValue.SubFrameDataMean[3] << std::endl;
    return true;
}

void Reset_RegisterData(void)
{
    RegisterData.clear();
}
#endif //ALGORITHMLIBRARY_PUBLIC_H
