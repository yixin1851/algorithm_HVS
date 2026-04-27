//
// Created by xinfeng.weng on 2026/1/23.
//

#ifndef ALGORITHMLIBRARY_CALCLIGHTINTENSITY_H
#define ALGORITHMLIBRARY_CALCLIGHTINTENSITY_H

#include "public.h"
#include "AlpMPAlgoCTypeInterface.h"
#include "../public.h"

int CalcLightIntensity() {
    auto calcEventRatio = [&](std::string rawDataPath, std::vector<double> &vecOnEventRatio,
                              std::vector<double> &vecOffEventRatio) -> bool {
        uint8_t useFrameNum = 70;
        uint8_t *rawDataBuf = nullptr;
        uint64_t rawDataRealLen = 0;

        // 1. Inupt EVS RawData
        int ret = ImportData(rawDataPath, 0, useFrameNum, rawDataBuf, rawDataRealLen);
        if (ret != 0 || rawDataBuf == nullptr) {
            return false;
        }

        // 2. Init Handle
        HANDLE handle = InitHandleDVS(SensorType::ALP_014BA, "./", BayerGBRG, 0);
        SetLogEnableDVS(handle, true);
        // 3. import RawData
        check_ret(__func__, ImportRawDataDVS(handle, rawDataBuf,
                                             rawDataRealLen, 0, useFrameNum));
        // 4. ImageContrastSensitivity
        DVSPeakInfo PeakInfoRes;
        DVSImageContrastSensitivityType ImageContrastSensitivityRes;
        check_ret(__func__, FindPeakDVS(handle, 0, useFrameNum, 3, &PeakInfoRes, On_OffEvents));
        if (PeakInfoRes.nOnEventsPeakNumber >= 3 && PeakInfoRes.nOffEventsPeakNumber >= 3) {
            uint32_t nPeakNum = 3;
            if (check_ret(__func__, ImageContrastSensitivityDVS(handle, 0, 70, &PeakInfoRes, nPeakNum, On_OffEvents,
                                                                &ImageContrastSensitivityRes))) {
                setResult("OnEvents_EventsRatio_All", (double) ImageContrastSensitivityRes.OnEventsRatio[All]);
                setResult("OffEvents_EventsRatio_All", (double) ImageContrastSensitivityRes.OffEventsRatio[All]);
                vecOnEventRatio.push_back(ImageContrastSensitivityRes.OnEventsRatio[All]);
                vecOffEventRatio.push_back(ImageContrastSensitivityRes.OffEventsRatio[All]);
            }
        }

        // Release memory
        delete [] rawDataBuf;
        rawDataBuf = NULL;
        DeleteHandleDVS(handle);
        return true;
    };

    // 1. Import EVS RawData
    std::string F1 =
            "D:/Work/Tmp/APX014BA/EVS/CalcLightInsentity/FrameID001458__EVS_W1288_H256_Fx30_50000_50_500_500_7__20260114-18-18-09_d1.raw";
    std::string F2 =
            "D:/Work/Tmp/APX014BA/EVS/CalcLightInsentity/FrameID002524__EVS_W1288_H256_Fx30_50000_50_500_500_7__20260114-18-18-27_d2.raw";
    std::string F3 =
            "D:/Work/Tmp/APX014BA/EVS/CalcLightInsentity/FrameID003398__EVS_W1288_H256_Fx30_50000_50_500_500_7__20260114-18-18-41_d3.raw";
    std::string F4 =
            "D:/Work/Tmp/APX014BA/EVS/CalcLightInsentity/FrameID004024__EVS_W1288_H256_Fx30_50000_50_500_500_7__20260114-18-18-52_d4.raw";


    // 2. Init
    HANDLE handle = InitHandleDVS(SensorType::ALP_014BA, "./", BayerGBRG, 0);
    double onEventPercent = 50.0; // 求当On事件量为50%时对应的光强
    double offEventPercent = 50.0; // 求当Off事件量为50%时对应的光强
    double onTargetLightIntensity = 0.0;
    double offTargetLightIntensity = 0.0;
    // Light Intensity jump points
    std::vector<std::pair<double, double> > vecLightIntensity = {
        {100, 105}, {100, 120}, {100, 150}, {100, 200}
    };

    // 3. calcEventRatio
    std::vector<double> vecOnEvent;
    std::vector<double> vecOffEvent;
    calcEventRatio(F1, vecOnEvent, vecOffEvent);
    calcEventRatio(F2, vecOnEvent, vecOffEvent);
    calcEventRatio(F3, vecOnEvent, vecOffEvent);
    calcEventRatio(F4, vecOnEvent, vecOffEvent);

    // Set AlgorithmLibrary Log Enabled
    SetLogEnableDVS(handle, true);
    // 4. CalcLightIntensity
    check_ret(__func__, CalcLightIntensityDVS(handle, onEventPercent, vecOnEvent, vecLightIntensity,
                                              onTargetLightIntensity));
    check_ret(__func__, CalcLightIntensityDVS(handle, offEventPercent, vecOffEvent, vecLightIntensity,
                                              offTargetLightIntensity));

    // std::cout << "onTargetLightIntensity = " << onTargetLightIntensity << std::endl;
    // std::cout << "offTargetLightIntensity = " << offTargetLightIntensity << std::endl;
    setResult("onTargetLightIntensity", (double) onTargetLightIntensity);
    setResult("offTargetLightIntensity", (double) offTargetLightIntensity);

    // 5. Delete handle
    DeleteHandleDVS(handle);
}

#endif //ALGORITHMLIBRARY_CALCLIGHTINTENSITY_H
