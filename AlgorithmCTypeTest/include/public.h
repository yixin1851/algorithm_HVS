//
// Created by xinfeng.weng on 2025/12/24.
//

#ifndef ALGORITHMLIBRARY_PUBLIC_H
#define ALGORITHMLIBRARY_PUBLIC_H
#include "stdio.h"
#include <string>
#include <iostream>
#include <fstream>

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
#endif //ALGORITHMLIBRARY_PUBLIC_H
