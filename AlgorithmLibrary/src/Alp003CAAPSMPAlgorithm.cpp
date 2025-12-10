#include "Alp003CAAPSMPAlgorithm.h"
#include <map>

CAlp003CAAPSMPAlgorithm::CAlp003CAAPSMPAlgorithm(SensorType Sensortype, APSRawType Rawtype, std::string strLogDir,
                                                 uint32_t nSiteNum, PixelFormatType Pixelformat, int code)
    : CAlpAPSMPAlgorithm(Sensortype, Rawtype, strLogDir, nSiteNum, Pixelformat, code) {
    m_ActiveArea = {0, 1223, 0, 1631};
    m_nChannelRow = 1224;
    m_nChannelCol = 1632;
    m_nTotalRow = 2448;
    m_nTotalCol = 3264;
    m_AlgorithmThre.nDSNURowBlockNum = 30;
    m_AlgorithmThre.nDSNUColBlockNum = 40;
    m_AlgorithmThre.nDSNURowBlockSize = 40;
    m_AlgorithmThre.nDSNUColBlockSize = 40;

    if ((code & APS_Code_HVS) == APS_Code_HVS) {
        m_bHVS_DPC = true;
    } else {
        m_bHVS_DPC = false;
    }
}

CAlp003CAAPSMPAlgorithm::~CAlp003CAAPSMPAlgorithm() {
}

bool CAlp003CAAPSMPAlgorithm::ImportRawData(uint8_t *pRawData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber,
                                            bool bHeader_Footer) {
    uint32_t nOneFrameSize = 0;
    uint32_t nHeaderSize = 0;
    uint32_t nFooterSize = 0;

    uint8_t Header[8] = {0};
    uint8_t Footer[8] = {0};

    // 帧头, 帧尾定义, 用于验证数据帧的边界, 确保数据完整性
    uint8_t Header_003CA[] = {0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xe5};
    uint8_t Footer_003CA[] = {0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xe5};

    memcpy_s(Header, sizeof(Header), Header_003CA, sizeof(Header_003CA));
    memcpy_s(Footer, sizeof(Footer), Footer_003CA, sizeof(Footer_003CA));
    nHeaderSize = 64;
    nFooterSize = 8;

    if (m_RawType == RAW8) {
        nOneFrameSize = m_nTotalRow * m_nTotalCol;
    } else if (m_RawType == RAW10) {
        nOneFrameSize = m_nTotalRow * m_nTotalCol / 4 * 5;
    } else if (m_RawType == UNPACK10 || m_RawType == UNPACK12) {
        nOneFrameSize = m_nTotalRow * m_nTotalCol * 2;
    } else if (m_RawType == RAW12) {
        nOneFrameSize = m_nTotalRow * m_nTotalCol / 2 * 3;
    }
    // 如果有帧头帧尾, DataSize需要加上 64 Byte帧头 + 8 Byte帧尾
    if (bHeader_Footer) {
        nOneFrameSize += nHeaderSize + nFooterSize;
    }

    if ((nLens / nOneFrameSize) < nNumber) {
        std::string strErr = "ImportRawData: RawData buffer Lens less than frames number: Lens: " +
                             std::to_string(nLens) + ", Number: " + std::to_string(nNumber);
        WriteLog(strErr, SubFrameIndex::All);
        return false;
    }
    // 为所有通道预分配足够的存储空间
    for (uint32_t nChannelIndex = 0; nChannelIndex < SubFrameIndex::All; nChannelIndex++) {
        if (m_RawDataContainer[nChannelIndex].size() < nIndexStart + nNumber) {
            m_RawDataContainer[nChannelIndex].resize(nIndexStart + nNumber);
        }
    }

    uint32_t nIndex = 0;

    for (uint32_t i = 0; i < nNumber; i++) {
        for (uint32_t nChannelIndex = 0; nChannelIndex < SubFrameIndex::All; nChannelIndex++) {
            CAPSDataContainer &CurContainer = m_RawDataContainer[nChannelIndex][nIndexStart + i];
            if (CurContainer.m_nRow != m_nChannelRow || CurContainer.m_nCol != m_nChannelCol) {
                CurContainer.Init(m_nChannelRow, m_nChannelCol);
            }
        }

        if (bHeader_Footer) {
            while (nIndex < nLens - sizeof(Header) && 0 != memcmp(pRawData + nIndex, Header, sizeof(Header))) {
                // 每次跳过8 Byte搜索帧头
                nIndex += 8;
            }

            // 寻找帧头标识，如果找不到则返回错误
            if (nIndex >= nLens - sizeof(Header) || 0 != memcmp(pRawData + nIndex, Header, sizeof(Header))) {
                std::string strErr = "ImportRawData: Header Fail: Index " + std::to_string(i);
                WriteLog(strErr, SubFrameIndex::All);
                return false;
            } else {
                nIndex += nHeaderSize;
            }
        }

        uint32_t nRows = 0, nCols = 0;
        for (; nIndex < nLens;) {
            uint16_t tempData[4] = {0};
            if (m_RawType == RAW8) {
                tempData[0] = pRawData[nIndex];
                tempData[1] = pRawData[nIndex + 1];
                tempData[2] = pRawData[nIndex + 2];
                tempData[3] = pRawData[nIndex + 3];

                nIndex += 4;
            } else if (m_RawType == RAW10) {
                tempData[0] = ((uint16_t) (pRawData[nIndex])) << 2;
                tempData[1] = ((uint16_t) (pRawData[nIndex + 1])) << 2;
                tempData[2] = ((uint16_t) (pRawData[nIndex + 2])) << 2;
                tempData[3] = ((uint16_t) (pRawData[nIndex + 3])) << 2;

                tempData[0] += (((uint16_t) (pRawData[nIndex + 4])) & 3);
                tempData[1] += ((((uint16_t) (pRawData[nIndex + 4])) >> 2) & 3);
                tempData[2] += ((((uint16_t) (pRawData[nIndex + 4])) >> 4) & 3);
                tempData[3] += ((((uint16_t) (pRawData[nIndex + 4])) >> 6) & 3);

                nIndex += 5;
            } else if (m_RawType == UNPACK10 || m_RawType == UNPACK12) {
                tempData[0] = ((uint16_t) (pRawData[nIndex])) + ((uint16_t) (pRawData[nIndex + 1]) << 8);
                tempData[1] = ((uint16_t) (pRawData[nIndex + 2])) + ((uint16_t) (pRawData[nIndex + 3]) << 8);
                tempData[2] = ((uint16_t) (pRawData[nIndex + 4])) + ((uint16_t) (pRawData[nIndex + 5]) << 8);
                tempData[3] = ((uint16_t) (pRawData[nIndex + 6])) + ((uint16_t) (pRawData[nIndex + 7]) << 8);

                nIndex += 8;
            } else if (m_RawType == RAW12) {
                tempData[0] = ((uint16_t) (pRawData[nIndex])) << 4;
                tempData[1] = ((uint16_t) (pRawData[nIndex + 1])) << 4;
                tempData[0] += (((uint16_t) (pRawData[nIndex + 2])) & 15);
                tempData[1] += ((((uint16_t) (pRawData[nIndex + 2])) >> 4) & 15);

                tempData[2] = ((uint16_t) (pRawData[nIndex + 3])) << 4;
                tempData[3] = ((uint16_t) (pRawData[nIndex + 4])) << 4;
                tempData[2] += (((uint16_t) (pRawData[nIndex + 5])) & 15);
                tempData[3] += ((((uint16_t) (pRawData[nIndex + 5])) >> 4) & 15);

                nIndex += 6;
            }
            SetDataToSubFrame(nIndexStart + i, nRows, nCols, tempData[0]);
            SetDataToSubFrame(nIndexStart + i, nRows, nCols + 1, tempData[1]);
            SetDataToSubFrame(nIndexStart + i, nRows, nCols + 2, tempData[2]);
            SetDataToSubFrame(nIndexStart + i, nRows, nCols + 3, tempData[3]);
            nCols += 4;
            if (nCols == m_nTotalCol) {
                nRows += 1;
                nCols = 0;
            }
            if (nRows == m_nTotalRow) {
                break;
            }
        }

        if (bHeader_Footer) {
            // 寻找帧头标识，如果找不到则返回错误
            if (0 != memcmp(pRawData + nIndex, Footer, sizeof(Footer))) {
                std::string strErr = "ImportRawData: Footer Fail: Index " + std::to_string(i);
                WriteLog(strErr, SubFrameIndex::All);
                return false;
            } else {
                nIndex += nFooterSize;
            }
        }
    }
    if (m_bUse16SubFrame) {
        ImportDataTo16SubFrame(nIndexStart, nNumber);
    }
    return true;
}

bool CAlp003CAAPSMPAlgorithm::BadPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                       APSBadpixelType &BadpixelRes) {
    bool bRet = CAlpAPSMPAlgorithm::BadPixel(nIndexStart, nNumber, ROI, BadpixelRes);
    if (bRet) {
        std::vector<std::vector<uint32_t> > BadPixelMask(m_nTotalRow + 8);
        for (uint32_t i = 0; i < m_nTotalRow + 8; i++) {
            BadPixelMask[i].resize(m_nTotalCol + 8, 0);
        }

        for (uint32_t n = 0; n < BadpixelRes.BadPixelMask.BadPixelNum; n++) {
            Local temp = BadpixelRes.BadPixelMask.LocalData[n];
            BadPixelMask[temp.x + 4][temp.y + 4] = BadpixelRes.BadPixelMask.Flag[n];
        }

        for (uint32_t row = 0; row < 4; row++) {
            for (uint32_t col = 0; col < m_nTotalCol + 8; col++) {
                BadPixelMask[row][col] = BadPixelMask[row + 4][col];
                BadPixelMask[row + m_nTotalRow + 4][col] = BadPixelMask[row + m_nTotalRow][col];
            }
        }

        for (uint32_t col = 0; col < 4; col++) {
            for (uint32_t row = 0; row < m_nTotalRow + 8; row++) {
                BadPixelMask[row][col] = BadPixelMask[row][col + 4];
                BadPixelMask[row][col + m_nTotalCol + 4] = BadPixelMask[row][col + m_nTotalCol];
            }
        }

        int x0[] = {-3, 0, 1, -3, 1, 3, 0, 1, -2, -1, -2, -1, -2, -1, -3, -3, 0, 0, 1, 1, -3, -4, 1, 4, -4, -3, 4, 1};
        int y0[] = {-3, -3, -3, 0, 0, 1, 1, 1, -3, -3, 0, 0, 1, 1, -2, -1, -2, -1, -2, -1, -4, -3, -4, -3, 1, 4, 1, 4};
        int x1[] = {-1, 0, 3, -1, 3, -1, 0, 3, 1, 2, 1, 2, 1, 2, -1, -1, 0, 0, 3, 3, -1, 3};
        int y1[] = {-3, -3, -3, 0, 0, 1, 1, 1, -3, -3, 0, 0, 1, 1, -2, -1, -2, -1, -2, -1, -4, 4};
        int x2[] = {-3, 0, 1, -3, 1, -3, 0, 1, -2, -1, -2, -1, -2, -1, -3, -3, 0, 0, 1, 1, -4, 4};
        int y2[] = {-1, -1, -1, 0, 0, 3, 3, 3, -1, -1, 0, 0, 3, 3, 1, 2, 1, 2, 1, 2, -1, 3};
        int x3[] = {-1, 0, 3, -1, 3, -1, 0, 3, 1, 2, 1, 2, 1, 2, -1, -1, 0, 0, 3, 3};
        int y3[] = {-1, -1, -1, 0, 0, 3, 3, 3, -1, -1, 0, 0, 3, 3, 1, 2, 1, 2, 1, 2};

        for (uint32_t n = 0; n < BadpixelRes.BadPixelMask.BadPixelNum; n++) {
            Local temp = BadpixelRes.BadPixelMask.LocalData[n];

            int *x = nullptr;
            int *y = nullptr;
            int nCheckLen = 0;
            if (temp.x % 2 == 0 && temp.y % 2 == 0) {
                x = x0;
                y = y0;
                nCheckLen = sizeof(x0) / sizeof(x0[0]);
            } else if (temp.x % 2 == 0 && temp.y % 2 == 1) {
                x = x1;
                y = y1;
                nCheckLen = sizeof(x1) / sizeof(x1[0]);
            } else if (temp.x % 2 == 1 && temp.y % 2 == 0) {
                x = x2;
                y = y2;
                nCheckLen = sizeof(x2) / sizeof(x2[0]);
            } else {
                x = x3;
                y = y3;
                nCheckLen = sizeof(x3) / sizeof(x3[0]);
            }
            bool bFindBadPixel = false;
            for (int i = 0; i < nCheckLen; i++) {
                if (BadPixelMask[temp.x + 4 + y[i]][temp.y + 4 + x[i]] > 0) {
                    bFindBadPixel = true;
                    break;
                }
            }

            if (!bFindBadPixel) {
                BadpixelRes.BadPixelMask.Flag[n] = APX003CA_ON_CHIP_CALIBRATION_FLAG;
            }
        }
    }

    return bRet;
}

bool CAlp003CAAPSMPAlgorithm::HotPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                       APSBadpixelType &HotpixelRes) {
    bool bRet = CAlpAPSMPAlgorithm::HotPixel(nIndexStart, nNumber, ROI, HotpixelRes);
    if (bRet) {
        std::vector<std::vector<uint32_t> > BadPixelMask(m_nTotalRow + 8);
        for (uint32_t i = 0; i < m_nTotalRow + 8; i++) {
            BadPixelMask[i].resize(m_nTotalCol + 8, 0);
        }

        for (uint32_t n = 0; n < HotpixelRes.BadPixelMask.BadPixelNum; n++) {
            Local temp = HotpixelRes.BadPixelMask.LocalData[n];
            BadPixelMask[temp.x + 4][temp.y + 4] = HotpixelRes.BadPixelMask.Flag[n];
        }

        for (uint32_t row = 0; row < 4; row++) {
            for (uint32_t col = 0; col < m_nTotalCol + 8; col++) {
                BadPixelMask[row][col] = BadPixelMask[row + 4][col];
                BadPixelMask[row + m_nTotalRow + 4][col] = BadPixelMask[row + m_nTotalRow][col];
            }
        }

        for (uint32_t col = 0; col < 4; col++) {
            for (uint32_t row = 0; row < m_nTotalRow + 8; row++) {
                BadPixelMask[row][col] = BadPixelMask[row][col + 4];
                BadPixelMask[row][col + m_nTotalCol + 4] = BadPixelMask[row][col + m_nTotalCol];
            }
        }

        int x0[] = {-3, 0, 1, -3, 1, 3, 0, 1, -2, -1, -2, -1, -2, -1, -3, -3, 0, 0, 1, 1, -3, -4, 1, 4, -4, -3, 4, 1};
        int y0[] = {-3, -3, -3, 0, 0, 1, 1, 1, -3, -3, 0, 0, 1, 1, -2, -1, -2, -1, -2, -1, -4, -3, -4, -3, 1, 4, 1, 4};
        int x1[] = {-1, 0, 3, -1, 3, -1, 0, 3, 1, 2, 1, 2, 1, 2, -1, -1, 0, 0, 3, 3, -1, 3};
        int y1[] = {-3, -3, -3, 0, 0, 1, 1, 1, -3, -3, 0, 0, 1, 1, -2, -1, -2, -1, -2, -1, -4, 4};
        int x2[] = {-3, 0, 1, -3, 1, -3, 0, 1, -2, -1, -2, -1, -2, -1, -3, -3, 0, 0, 1, 1, -4, 4};
        int y2[] = {-1, -1, -1, 0, 0, 3, 3, 3, -1, -1, 0, 0, 3, 3, 1, 2, 1, 2, 1, 2, -1, 3};
        int x3[] = {-1, 0, 3, -1, 3, -1, 0, 3, 1, 2, 1, 2, 1, 2, -1, -1, 0, 0, 3, 3};
        int y3[] = {-1, -1, -1, 0, 0, 3, 3, 3, -1, -1, 0, 0, 3, 3, 1, 2, 1, 2, 1, 2};

        for (uint32_t n = 0; n < HotpixelRes.BadPixelMask.BadPixelNum; n++) {
            Local temp = HotpixelRes.BadPixelMask.LocalData[n];

            int *x = nullptr;
            int *y = nullptr;
            int nCheckLen = 0;
            if (temp.x % 2 == 0 && temp.y % 2 == 0) {
                x = x0;
                y = y0;
                nCheckLen = sizeof(x0) / sizeof(x0[0]);
            } else if (temp.x % 2 == 0 && temp.y % 2 == 1) {
                x = x1;
                y = y1;
                nCheckLen = sizeof(x1) / sizeof(x1[0]);
            } else if (temp.x % 2 == 1 && temp.y % 2 == 0) {
                x = x2;
                y = y2;
                nCheckLen = sizeof(x2) / sizeof(x2[0]);
            } else {
                x = x3;
                y = y3;
                nCheckLen = sizeof(x3) / sizeof(x3[0]);
            }
            bool bFindBadPixel = false;
            for (int i = 0; i < nCheckLen; i++) {
                if (BadPixelMask[temp.x + 4 + y[i]][temp.y + 4 + x[i]] > 0) {
                    bFindBadPixel = true;
                    break;
                }
            }

            if (!bFindBadPixel) {
                HotpixelRes.BadPixelMask.Flag[n] = APX003CA_ON_CHIP_CALIBRATION_FLAG;
            }
        }
    }

    return bRet;
}

bool CAlp003CAAPSMPAlgorithm::DPC(uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                  std::vector<Local> &BadPixelLocal) {
    if (0 == nNumber || nIndexStart >= m_RawDataContainer[0].size() || (nIndexStart + nNumber) > m_RawDataContainer[0].
        size()) {
        std::string strErr = "DPC: Index error: nIndexStart: " + std::to_string(nIndexStart) + ", nNumber: " +
                             std::to_string(nNumber);
        WriteLog(strErr, SubFrameIndex::All);
        return false;
    }

    bool bRet = true;
    bool *bSubRes = new bool[nNumber];

    if (m_bMultiThreadEnable) {
        std::vector<std::thread *> t(nNumber);

        for (uint32_t i = 0; i < nNumber; i++) {
            if (m_bHVS_DPC) {
                t[i] = new std::thread(&CAlp003CAAPSMPAlgorithm::DPC_HVS, this, nIndexStart + i, ROI,
                                       std::ref(BadPixelLocal), std::ref(bSubRes[i]));
            } else {
                t[i] = new std::thread(&CAlp003CAAPSMPAlgorithm::DPC_APS_Only, this, nIndexStart + i, ROI,
                                       std::ref(BadPixelLocal), std::ref(bSubRes[i]));
            }
        }
        for (uint32_t i = 0; i < nNumber; i++) {
            t[i]->join();
            delete t[i];
        }
    } else {
        for (uint32_t i = 0; i < nNumber; i++) {
            if (m_bHVS_DPC) {
                DPC_HVS(nIndexStart + i, ROI, BadPixelLocal, bSubRes[i]);
            } else {
                DPC_APS_Only(nIndexStart + i, ROI, BadPixelLocal, bSubRes[i]);
            }
        }
    }
    for (uint32_t i = 0; i < nNumber; i++) {
        bRet = bRet && bSubRes[i];
    }
    delete[] bSubRes;

    return bRet;
}

void CAlp003CAAPSMPAlgorithm::DPC_APS_Only(uint32_t nIndex, ROIArea *ROI, std::vector<Local> &BadPixelLocal,
                                           bool &bRes) {
    bRes = false;

    int x0[] = {-3, 0, 1, -3, 1, 3, 0, 1, -2, -1, -2, -1, -2, -1, -3, -3, 0, 0, 1, 1, -3, -4, 1, 4, -4, -3, 4, 1};
    int y0[] = {-3, -3, -3, 0, 0, 1, 1, 1, -3, -3, 0, 0, 1, 1, -2, -1, -2, -1, -2, -1, -4, -3, -4, -3, 1, 4, 1, 4};
    int x1[] = {-1, 0, 3, -1, 3, -1, 0, 3, 1, 2, 1, 2, 1, 2, -1, -1, 0, 0, 3, 3, -1, 3};
    int y1[] = {-3, -3, -3, 0, 0, 1, 1, 1, -3, -3, 0, 0, 1, 1, -2, -1, -2, -1, -2, -1, -4, 4};
    int x2[] = {-3, 0, 1, -3, 1, -3, 0, 1, -2, -1, -2, -1, -2, -1, -3, -3, 0, 0, 1, 1, -4, 4};
    int y2[] = {-1, -1, -1, 0, 0, 3, 3, 3, -1, -1, 0, 0, 3, 3, 1, 2, 1, 2, 1, 2, -1, 3};
    int x3[] = {-1, 0, 3, -1, 3, -1, 0, 3, 1, 2, 1, 2, 1, 2, -1, -1, 0, 0, 3, 3};
    int y3[] = {-1, -1, -1, 0, 0, 3, 3, 3, -1, -1, 0, 0, 3, 3, 1, 2, 1, 2, 1, 2};

    for (uint32_t n = 0; n < BadPixelLocal.size(); n++) {
        auto totalLocal = BadPixelLocal[n];
        int nRow = totalLocal.x;
        int nCol = totalLocal.y;

        Local PixelLocal[20];
        int tempRow = 0;
        int tempCol = 0;
        for (int i = 0; i < 20; i++) {
            if (nRow % 2 == 0 && nCol % 2 == 0) {
                tempRow = nRow + y0[i];
                tempCol = nCol + x0[i];
            } else if (nRow % 2 == 0 && nCol % 2 == 1) {
                tempRow = nRow + y1[i];
                tempCol = nCol + x1[i];
            } else if (nRow % 2 == 1 && nCol % 2 == 0) {
                tempRow = nRow + y2[i];
                tempCol = nCol + x2[i];
            } else {
                tempRow = nRow + y3[i];
                tempCol = nCol + x3[i];
            }
            if (tempRow < 0) {
                tempRow += 4;
            } else if (tempRow >= m_nTotalRow) {
                tempRow -= 4;
            }
            if (tempCol < 0) {
                tempCol += 4;
            } else if (tempCol >= m_nTotalCol) {
                tempCol -= 4;
            }
            PixelLocal[i].x = tempRow;
            PixelLocal[i].y = tempCol;
        }
        double p0, p1, p2, p3, p5, p6, p7, p8, g0, g1, g2, g3, g4, g5, g6, g7, g8, g9, g10, g11;
        GetDataFromSubFrame(nIndex, PixelLocal[0].x, PixelLocal[0].y, p0);
        GetDataFromSubFrame(nIndex, PixelLocal[1].x, PixelLocal[1].y, p1);
        GetDataFromSubFrame(nIndex, PixelLocal[2].x, PixelLocal[2].y, p2);
        GetDataFromSubFrame(nIndex, PixelLocal[3].x, PixelLocal[3].y, p3);
        GetDataFromSubFrame(nIndex, PixelLocal[4].x, PixelLocal[4].y, p5);
        GetDataFromSubFrame(nIndex, PixelLocal[5].x, PixelLocal[5].y, p6);
        GetDataFromSubFrame(nIndex, PixelLocal[6].x, PixelLocal[6].y, p7);
        GetDataFromSubFrame(nIndex, PixelLocal[7].x, PixelLocal[7].y, p8);
        GetDataFromSubFrame(nIndex, PixelLocal[8].x, PixelLocal[8].y, g0);
        GetDataFromSubFrame(nIndex, PixelLocal[9].x, PixelLocal[9].y, g1);
        GetDataFromSubFrame(nIndex, PixelLocal[10].x, PixelLocal[10].y, g2);
        GetDataFromSubFrame(nIndex, PixelLocal[11].x, PixelLocal[11].y, g3);
        GetDataFromSubFrame(nIndex, PixelLocal[12].x, PixelLocal[12].y, g4);
        GetDataFromSubFrame(nIndex, PixelLocal[13].x, PixelLocal[13].y, g5);
        GetDataFromSubFrame(nIndex, PixelLocal[14].x, PixelLocal[14].y, g6);
        GetDataFromSubFrame(nIndex, PixelLocal[15].x, PixelLocal[15].y, g7);
        GetDataFromSubFrame(nIndex, PixelLocal[16].x, PixelLocal[16].y, g8);
        GetDataFromSubFrame(nIndex, PixelLocal[17].x, PixelLocal[17].y, g9);
        GetDataFromSubFrame(nIndex, PixelLocal[18].x, PixelLocal[18].y, g10);
        GetDataFromSubFrame(nIndex, PixelLocal[19].x, PixelLocal[19].y, g11);

        double dh4 = abs(g2 + g3 - (g0 + g1) / 2 - (g4 + g5) / 2);

        double dv4 = abs(g8 + g9 - (g6 + g7) / 2 - (g10 + g11) / 2);

        double dhori = (abs(2 * p1 - p0 - p2) + abs(p3 - p5) * 2 + abs(2 * p7 - p6 - p8) + dh4) / 4;

        double dvert = (abs(2 * p3 - p0 - p6) + abs(p1 - p7) * 2 + abs(2 * p5 - p2 - p8) + dv4) / 4;

        double hori_value = 0, vert_value = 0;

        if (nRow % 2 == 0 && nCol % 2 == 0) {
            hori_value = 0.75 * p5 + 0.25 * p3;

            vert_value = 0.75 * p7 + 0.25 * p1;
        } else if (nRow % 2 == 0 && nCol % 2 == 1) {
            hori_value = 0.75 * p3 + 0.25 * p5;

            vert_value = 0.75 * p7 + 0.25 * p1;
        } else if (nRow % 2 == 1 && nCol % 2 == 0) {
            hori_value = 0.75 * p5 + 0.25 * p3;

            vert_value = 0.75 * p1 + 0.25 * p7;
        } else {
            hori_value = 0.75 * p3 + 0.25 * p5;

            vert_value = 0.75 * p1 + 0.25 * p7;
        }

        if ((dvert + dhori) == 0) {
            std::string strErr = "DPC: dvert + dhori == 0 ";
            WriteLog(strErr, SubFrameIndex::All);
            bRes = false;
            return;
        }

        double p4 = (hori_value * dvert + vert_value * dhori) / (dvert + dhori);

        SetDataToSubFrame(nIndex, nRow, nCol, int(p4));
    }

    bRes = true;
}

void CAlp003CAAPSMPAlgorithm::DPC_HVS(uint32_t nIndex, ROIArea *ROI, std::vector<Local> &BadPixelLocal, bool &bRes) {
    bRes = false;

    int x0[] = {-3, 0, 1, -3, 1, 3, 0, 1, -2, -1, -2, -1, -2, -1, -3, -3, 0, 0, 1, 1, -3, -4, 1, 4, -4, -3, 4, 1};
    int y0[] = {-3, -3, -3, 0, 0, 1, 1, 1, -3, -3, 0, 0, 1, 1, -2, -1, -2, -1, -2, -1, -4, -3, -4, -3, 1, 4, 1, 4};
    int x1[] = {-1, 0, 3, -1, 3, -1, 0, 3, 1, 2, 1, 2, 1, 2, -1, -1, 0, 0, 3, 3, -1, 3};
    int y1[] = {-3, -3, -3, 0, 0, 1, 1, 1, -3, -3, 0, 0, 1, 1, -2, -1, -2, -1, -2, -1, -4, 4};
    int x2[] = {-3, 0, 1, -3, 1, -3, 0, 1, -2, -1, -2, -1, -2, -1, -3, -3, 0, 0, 1, 1, -4, 4};
    int y2[] = {-1, -1, -1, 0, 0, 3, 3, 3, -1, -1, 0, 0, 3, 3, 1, 2, 1, 2, 1, 2, -1, 3};
    int x3[] = {-1, 0, 3, -1, 3, -1, 0, 3, 1, 2, 1, 2, 1, 2, -1, -1, 0, 0, 3, 3};
    int y3[] = {-1, -1, -1, 0, 0, 3, 3, 3, -1, -1, 0, 0, 3, 3, 1, 2, 1, 2, 1, 2};

    for (uint32_t n = 0; n < BadPixelLocal.size(); n++) {
        auto totalLocal = BadPixelLocal[n];
        int nRow = totalLocal.x;
        int nCol = totalLocal.y;

        Local PixelLocal[28];
        int tempRow = 0;
        int tempCol = 0;
        int nLenPoints = 0;
        if (nRow % 2 == 0 && nCol % 2 == 0) {
            nLenPoints = sizeof(x0) / sizeof(x0[0]);
        } else if (nRow % 2 == 0 && nCol % 2 == 1) {
            nLenPoints = sizeof(x1) / sizeof(x1[0]);
        } else if (nRow % 2 == 1 && nCol % 2 == 0) {
            nLenPoints = sizeof(x2) / sizeof(x2[0]);
        } else {
            nLenPoints = sizeof(x3) / sizeof(x3[0]);
        }
        for (int i = 0; i < nLenPoints; i++) {
            if (nRow % 2 == 0 && nCol % 2 == 0) {
                tempRow = nRow + y0[i];
                tempCol = nCol + x0[i];
            } else if (nRow % 2 == 0 && nCol % 2 == 1) {
                tempRow = nRow + y1[i];
                tempCol = nCol + x1[i];
            } else if (nRow % 2 == 1 && nCol % 2 == 0) {
                tempRow = nRow + y2[i];
                tempCol = nCol + x2[i];
            } else {
                tempRow = nRow + y3[i];
                tempCol = nCol + x3[i];
            }
            if (tempRow < 0) {
                tempRow += 4;
            } else if (tempRow >= m_nTotalRow) {
                tempRow -= 4;
            }
            if (tempCol < 0) {
                tempCol += 4;
            } else if (tempCol >= m_nTotalCol) {
                tempCol -= 4;
            }
            PixelLocal[i].x = tempRow;
            PixelLocal[i].y = tempCol;
        }
        double p0, p1, p2, p3, p5, p6, p7, p8, g0, g1, g2, g3, g4, g5, g6, g7, g8, g9, g10, g11, A, B, C, D, E, F, G, H;
        GetDataFromSubFrame(nIndex, PixelLocal[0].x, PixelLocal[0].y, p0);
        GetDataFromSubFrame(nIndex, PixelLocal[1].x, PixelLocal[1].y, p1);
        GetDataFromSubFrame(nIndex, PixelLocal[2].x, PixelLocal[2].y, p2);
        GetDataFromSubFrame(nIndex, PixelLocal[3].x, PixelLocal[3].y, p3);
        GetDataFromSubFrame(nIndex, PixelLocal[4].x, PixelLocal[4].y, p5);
        GetDataFromSubFrame(nIndex, PixelLocal[5].x, PixelLocal[5].y, p6);
        GetDataFromSubFrame(nIndex, PixelLocal[6].x, PixelLocal[6].y, p7);
        GetDataFromSubFrame(nIndex, PixelLocal[7].x, PixelLocal[7].y, p8);
        GetDataFromSubFrame(nIndex, PixelLocal[8].x, PixelLocal[8].y, g0);
        GetDataFromSubFrame(nIndex, PixelLocal[9].x, PixelLocal[9].y, g1);
        GetDataFromSubFrame(nIndex, PixelLocal[10].x, PixelLocal[10].y, g2);
        GetDataFromSubFrame(nIndex, PixelLocal[11].x, PixelLocal[11].y, g3);
        GetDataFromSubFrame(nIndex, PixelLocal[12].x, PixelLocal[12].y, g4);
        GetDataFromSubFrame(nIndex, PixelLocal[13].x, PixelLocal[13].y, g5);
        GetDataFromSubFrame(nIndex, PixelLocal[14].x, PixelLocal[14].y, g6);
        GetDataFromSubFrame(nIndex, PixelLocal[15].x, PixelLocal[15].y, g7);
        GetDataFromSubFrame(nIndex, PixelLocal[16].x, PixelLocal[16].y, g8);
        GetDataFromSubFrame(nIndex, PixelLocal[17].x, PixelLocal[17].y, g9);
        GetDataFromSubFrame(nIndex, PixelLocal[18].x, PixelLocal[18].y, g10);
        GetDataFromSubFrame(nIndex, PixelLocal[19].x, PixelLocal[19].y, g11);
        if (nRow % 2 == 0 && nCol % 2 == 0) {
            GetDataFromSubFrame(nIndex, PixelLocal[20].x, PixelLocal[20].y, A);
            GetDataFromSubFrame(nIndex, PixelLocal[21].x, PixelLocal[21].y, B);
            GetDataFromSubFrame(nIndex, PixelLocal[22].x, PixelLocal[22].y, C);
            GetDataFromSubFrame(nIndex, PixelLocal[23].x, PixelLocal[23].y, D);
            GetDataFromSubFrame(nIndex, PixelLocal[24].x, PixelLocal[24].y, E);
            GetDataFromSubFrame(nIndex, PixelLocal[25].x, PixelLocal[25].y, F);
            GetDataFromSubFrame(nIndex, PixelLocal[26].x, PixelLocal[26].y, G);
            GetDataFromSubFrame(nIndex, PixelLocal[27].x, PixelLocal[27].y, H);

            p0 = abs(A - p3) > abs(B - p1) ? (3 * B + p1) / 4 : (3 * A + p3) / 4;

            p2 = abs(C - p5) > abs(p1 - D) ? (3 * p1 + D) / 4 : (3 * C + p5) / 4;

            p6 = abs(p3 - F) > abs(E - p7) ? (3 * E + p7) / 4 : (3 * p3 + F) / 4;

            p8 = abs(p5 - H) > abs(p7 - G) ? (3 * p7 + G) / 4 : (3 * p5 + H) / 4;
        } else if (nRow % 2 == 0 && nCol % 2 == 1) {
            GetDataFromSubFrame(nIndex, PixelLocal[20].x, PixelLocal[20].y, A);
            GetDataFromSubFrame(nIndex, PixelLocal[21].x, PixelLocal[21].y, B);

            p1 = abs(p0 - p2) > abs(A - p5) ? (3 * A + p5) / 4 : (3 * p0 + p2) / 4;

            p7 = abs(p6 - p8) > abs(p3 - B) ? (3 * p3 + B) / 4 : (3 * p6 + p8) / 4;
        } else if (nRow % 2 == 1 && nCol % 2 == 0) {
            GetDataFromSubFrame(nIndex, PixelLocal[20].x, PixelLocal[20].y, A);
            GetDataFromSubFrame(nIndex, PixelLocal[21].x, PixelLocal[21].y, B);

            p3 = abs(p0 - p6) > abs(A - p7) ? (3 * A + p7) / 4 : (3 * p0 + p6) / 4;

            p5 = abs(p2 - p8) > abs(p1 - B) ? (3 * p1 + B) / 4 : (3 * p2 + p8) / 4;
        } else {
        }

        double dhori = (abs(2 * p1 - p0 - p2) + abs(p3 - p5) * 2 + abs(2 * p7 - p6 - p8)) / 3;

        double dvert = (abs(2 * p3 - p0 - p6) + abs(p1 - p7) * 2 + abs(2 * p5 - p2 - p8)) / 3;

        double hori_value = 0, vert_value = 0;

        if (nRow % 2 == 0 && nCol % 2 == 0) {
            hori_value = 0.75 * p5 + 0.25 * p3;

            vert_value = 0.75 * p7 + 0.25 * p1;
        } else if (nRow % 2 == 0 && nCol % 2 == 1) {
            hori_value = 0.75 * p3 + 0.25 * p5;

            vert_value = 0.75 * p7 + 0.25 * p1;
        } else if (nRow % 2 == 1 && nCol % 2 == 0) {
            hori_value = 0.75 * p5 + 0.25 * p3;

            vert_value = 0.75 * p1 + 0.25 * p7;
        } else {
            hori_value = 0.75 * p3 + 0.25 * p5;

            vert_value = 0.75 * p1 + 0.25 * p7;
        }

        if ((dvert + dhori) == 0) {
            std::string strErr = "DPC: dvert + dhori == 0 ";
            WriteLog(strErr, SubFrameIndex::All);
            bRes = false;
            return;
        }

        double p4 = (hori_value * dvert + vert_value * dhori) / (dvert + dhori);

        SetDataToSubFrame(nIndex, nRow, nCol, int(p4));
    }
    bRes = true;
}

void CAlp003CAAPSMPAlgorithm::SubFrameBadPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                               SubFrameIndex nChannelIndex, APSSubFrameBadpixelType &BadpixelRes,
                                               bool &bRes) {
    // ��ĳ��ͨ��(nChannelIndex) ��Rawͼ��֡���н��л����⣬��������㡢���С����е���Ϣ��
    /*
    * �������ݰ�����
    * �����⣨�����쳣����
    * �쳣����ͨ����������㡢˫�㡢cluster�ȣ���
    * ���м�⣻
    * ���м�⣻
    * ���ػ���ӳ��ͼ��ͳ����Ϣ��
    */
    bRes = true;
    ROIArea RealRoi = {0};
    // ROI && ����������
    if (ROI == nullptr) {
        RealRoi = m_ActiveArea;
    } else {
        RealRoi = *ROI;
    }
    // ���֡�����������Ƿ�Ϸ�
    if (0 == nNumber || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) >
        m_RawDataContainer[nChannelIndex].size()) {
        std::string strErr = "SubFrameBadPixel: Index error: nIndexStart: " + std::to_string(nIndexStart) +
                             ", nNumber: " + std::to_string(nNumber);
        WriteLog(strErr, nChannelIndex);
        bRes = false;
        return;
    }
    // ���ROI�Ƿ�Խ��
    if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right <
        RealRoi.Left) {
        std::string strErr = "SubFrameBadPixel: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " +
                             std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " +
                             std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " +
                             std::to_string(m_nChannelCol);
        WriteLog(strErr, nChannelIndex);
        bRes = false;
        return;
    }
    uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
    uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;

    // �����ڴ� PixelMeanArray �洢ÿ��������nNumber֡�е�ƽ��ֵ
    CAPSDataContainer PixelMeanArray;
    PixelMeanArray.Init(nRow, nCol, true);
    std::vector<std::vector<uint32_t> > BadPixelMask(nRow);
    for (uint32_t i = 0; i < nRow; i++) {
        BadPixelMask[i].resize(nCol, 0);
    }
    BadpixelRes.BadPixelNum = 0;
    BadpixelRes.DefectColNum = 0;
    BadpixelRes.DefectRowNum = 0;
    BadpixelRes.SingletNum = 0;
    BadpixelRes.CoupletNum = 0;
    BadpixelRes.ClusterNum = 0;
    BadpixelRes.MaxClusterSize = 0;
    BadpixelRes.BadPixelMask.LocalData.clear();
    BadpixelRes.BadPixelMask.Flag.clear();
    BadpixelRes.BadPixelMask.DiffData.clear();
    BadpixelRes.BadPixelMask.BadPixelNum = 0;
    std::map<Local, float> DiffMap;

    for (uint32_t nRows = 0; nRows < nRow + m_AlgorithmThre.nBadPixelRadius; nRows++) {
        for (uint32_t nCols = 0; nCols < nCol + m_AlgorithmThre.nBadPixelRadius; nCols++) {
            if (nRows < nRow && nCols < nCol) {
                // ��ʹ��HVS DPCģʽ(m_bHVS_DPC == true)������λ��Ϊ������/��(odd,odd)����ֵΪ10000��Ϊ��Чֵ
                if (m_bHVS_DPC && nRows % 2 == 1 && nCols % 2 == 1) {
                    PixelMeanArray.m_RawData[nRows][nCols] = 10000;
                }
                // ����������������ֵ
                else {
                    double dValue = 0;
                    for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++) {
                        // value = sum over frames
                        dValue += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows + RealRoi.Up][
                            nCols + RealRoi.Left];
                    }
                    PixelMeanArray.m_RawData[nRows][nCols] = round(dValue / nNumber);
                }
            }
            // ��������Ļ�����
            // ����ROI��ÿ�����أ�
            // ʹ��һ���������򣨴�С��2*radius+1�����������ĵ㣬������ƽ��
            if ((nRows - m_AlgorithmThre.nBadPixelRadius < nRow) && (nCols - m_AlgorithmThre.nBadPixelRadius < nCol) &&
                PixelMeanArray.m_RawData[nRows - m_AlgorithmThre.nBadPixelRadius][
                    nCols - m_AlgorithmThre.nBadPixelRadius] != 10000) {
                uint32_t uSize = 0;
                double dSurroundPixle = 0;
                double dMax = -1;
                double dMin = 10000;
                for (uint32_t i = 0; i < 2 * m_AlgorithmThre.nBadPixelRadius + 1; i++) {
                    for (uint32_t j = 0; j < 2 * m_AlgorithmThre.nBadPixelRadius + 1; j++) {
                        if ((nRows - i < nRow) && (nCols - j < nCol) && (
                                i != m_AlgorithmThre.nBadPixelRadius || j != m_AlgorithmThre.nBadPixelRadius) &&
                            PixelMeanArray.m_RawData[nRows - i][nCols - j] != 10000) {
                            dSurroundPixle += PixelMeanArray.m_RawData[nRows - i][nCols - j];
                            //SortData[uSize] = PixelMeanArray.m_RawData[nRows - i][nCols - j];
                            // �ҵ����ֵ����Сֵ
                            if (dMax < PixelMeanArray.m_RawData[nRows - i][nCols - j]) {
                                dMax = PixelMeanArray.m_RawData[nRows - i][nCols - j];
                            }
                            if (dMin > PixelMeanArray.m_RawData[nRows - i][nCols - j]) {
                                dMin = PixelMeanArray.m_RawData[nRows - i][nCols - j];
                            }
                            uSize++;
                        }
                    }
                }
                if (uSize > 2) {
                    // ȥ�����ֵ����Сֵ��������ƽ��dSurroundPixel
                    dSurroundPixle = (dSurroundPixle - dMax - dMin) / (uSize - 2);
                }
                // ��� uSize <= 2, ʹ�� center value ��Ϊ������Χֵ
                else {
                    dSurroundPixle = PixelMeanArray.m_RawData[nRows - m_AlgorithmThre.nBadPixelRadius][
                        nCols - m_AlgorithmThre.nBadPixelRadius];
                }
                //std::sort(SortData.begin(), SortData.begin() + uSize);
                //double dSurroundPixle = SortData[uSize / 2];
                double dCurrentPixel = PixelMeanArray.m_RawData[nRows - m_AlgorithmThre.nBadPixelRadius][
                    nCols - m_AlgorithmThre.nBadPixelRadius];

                // �жϻ�����������ֵ�жϻ���
                if (abs(dCurrentPixel - dSurroundPixle) / dSurroundPixle > m_AlgorithmThre.dBadPixelThre) {
                    //BadpixelRes.BadPixelMask.BadPixelNum++;
                    //BadpixelRes.BadPixelMask.LocalData.push_back({ nRows - m_AlgorithmThre.nBadPixelRadius + RealRoi.Up, nCols - m_AlgorithmThre.nBadPixelRadius + RealRoi.Left });
                    //BadpixelRes.BadPixelMask.Flag.push_back(APS_BAD_PIXEL_FLAG);

                    BadpixelRes.BadPixelNum++;
                    BadPixelMask[nRows - m_AlgorithmThre.nBadPixelRadius][nCols - m_AlgorithmThre.nBadPixelRadius] = 1;
                    Local temp = {nRows - m_AlgorithmThre.nBadPixelRadius, nCols - m_AlgorithmThre.nBadPixelRadius};
                    DiffMap[temp] = abs(dCurrentPixel - dSurroundPixle) / dSurroundPixle;
                }
            }
        }
    }

    // ��ͨ���������ʶ��Singlet��Couplet��Cluster
    // �� BadPixelMask ����8���� BFS/Flood Fill��
    // �ҳ����л������ͨ�أ�
    // ͳ�������С AreaSize��
    uint32_t ConnectedAreaFlag = 0xFFFFFFFF;

    for (uint32_t nRows = 0; nRows < nRow; nRows++) {
        for (uint32_t nCols = 0; nCols < nCol; nCols++) {
            // ���� BadPixelMask, ����ֵΪ1�����������⴦��
            if (BadPixelMask[nRows][nCols] != 0 && BadPixelMask[nRows][nCols] < ConnectedAreaFlag) {
                uint32_t AreaSize = 0;
                uint32_t nCur = 0;
                std::vector<Local> Search;
                BadPixelMask[nRows][nCols] = ConnectedAreaFlag;
                Search.push_back({nRows, nCols});
                while (nCur != Search.size()) {
                    Local temp = Search[nCur];
                    nCur++;
                    AreaSize++;
                    for (int nTempRows = (int) temp.x - 1; nTempRows <= (int) temp.x + 1; nTempRows++) {
                        if (nTempRows >= 0 && nTempRows < nRow) {
                            for (int nTempCols = (int) temp.y - 1; nTempCols <= (int) temp.y + 1; nTempCols++) {
                                if (nTempCols >= 0 && nTempCols < nCol && BadPixelMask[nTempRows][nTempCols] != 0 &&
                                    BadPixelMask[nTempRows][nTempCols] < ConnectedAreaFlag) {
                                    BadPixelMask[nTempRows][nTempCols] = ConnectedAreaFlag;
                                    Search.push_back({(uint32_t) nTempRows, (uint32_t) nTempCols});
                                }
                            }
                        }
                    }
                }
                uint8_t uFlag = 0;
                // �����߼�
                // AreaSize = 1 -> �����㣻
                // AreaSize = 2 -> ˫���㣻
                // AreaSize >= 3 -> Cluster��
                if (AreaSize == 1) {
                    BadpixelRes.SingletNum++;
                    uFlag = APS_BAD_PIXEL_SINGLET_FLAG;
                } else if (AreaSize == 2) {
                    BadpixelRes.CoupletNum++;
                    uFlag = APS_BAD_PIXEL_COUPLET_FLAG;
                } else {
                    BadpixelRes.ClusterNum++;
                    uFlag = APS_BAD_PIXEL_CLUSTER_FLAG;
                }
                if (AreaSize > BadpixelRes.MaxClusterSize) {
                    BadpixelRes.MaxClusterSize = AreaSize;
                }
                // ���л���ض���¼��BadpixelRes.BadPixelMask��, ��¼���������Ӧ�����ͱ�ʶ�Ͳ����
                for (uint32_t n = 0; n < Search.size(); n++) {
                    BadpixelRes.BadPixelMask.LocalData.push_back(Search[n]);
                    BadpixelRes.BadPixelMask.Flag.push_back(uFlag);
                    BadpixelRes.BadPixelMask.DiffData.push_back(DiffMap[Search[n]]);
                    BadpixelRes.BadPixelMask.BadPixelNum++;
                }
                ConnectedAreaFlag--;
            }
        }
    }

    // ���С����м��
    std::vector<double> RowMean(nRow, 0);
    std::vector<double> ColMean(nCol, 0);

    // ����ROI��ÿ��/�е�ƽ���Ҷ�ֵ��
    for (uint32_t nRows = 0; nRows < nRow; nRows++) {
        for (uint32_t nCols = 0; nCols < nCol; nCols++) {
            double value = 0;
            for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++) {
                value += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows + RealRoi.Up][
                    nCols + RealRoi.Left];
            }
            RowMean[nRows] += round(value / nNumber);
            ColMean[nCols] += round(value / nNumber);
        }
    }
    for (uint32_t nRows = 0; nRows < nRow; nRows++) {
        RowMean[nRows] /= nCol;
    }
    for (uint32_t nCols = 0; nCols < nCol; nCols++) {
        ColMean[nCols] /= nRow;
    }
    // ʹ�ð뾶��Χ��������/��ƽ��ֵ�������ж��Ƿ��쳣
    for (uint32_t nRows = m_AlgorithmThre.nBadLineRadius; nRows < nRow - m_AlgorithmThre.nBadLineRadius; nRows++) {
        double dBaseMean = 0;
        for (uint32_t n = nRows - m_AlgorithmThre.nBadLineRadius; n <= nRows + m_AlgorithmThre.nBadLineRadius; n++) {
            if (n != nRows) {
                dBaseMean += RowMean[n];
            }
        }
        dBaseMean /= 2 * m_AlgorithmThre.nBadLineRadius;
        // abs(RowMean - ��Χ�о�ֵ) / ��Χ�о�ֵ > ��ֵ �� ����
        if (abs(RowMean[nRows] - dBaseMean) / dBaseMean > m_AlgorithmThre.dBadLineThre) {
            BadpixelRes.DefectRowNum++;
        }
    }
    for (uint32_t nCols = m_AlgorithmThre.nBadLineRadius; nCols < nCol - m_AlgorithmThre.nBadLineRadius; nCols++) {
        double dBaseMean = 0;
        for (uint32_t n = nCols - m_AlgorithmThre.nBadLineRadius; n <= nCols + m_AlgorithmThre.nBadLineRadius; n++) {
            if (n != nCols) {
                dBaseMean += ColMean[n];
            }
        }
        dBaseMean /= 2 * m_AlgorithmThre.nBadLineRadius;
        // abs(ColMean - ��Χ�о�ֵ) / ��Χ�о�ֵ > ��ֵ �� ����
        if (abs(ColMean[nCols] - dBaseMean) / dBaseMean > m_AlgorithmThre.dBadLineThre) {
            BadpixelRes.DefectColNum++;
        }
    }
    return;
}

void CAlp003CAAPSMPAlgorithm::SubFrameHotPixel(uint32_t nIndexStart, uint32_t nNumber, ROIArea *ROI,
                                               SubFrameIndex nChannelIndex, APSSubFrameBadpixelType &HotpixelRes,
                                               bool &bRes) {
    bRes = true;
    ROIArea RealRoi = {0};
    if (ROI == nullptr) {
        RealRoi = m_ActiveArea;
    } else {
        RealRoi = *ROI;
    }
    if (0 == nNumber || nIndexStart >= m_RawDataContainer[nChannelIndex].size() || (nIndexStart + nNumber) >
        m_RawDataContainer[nChannelIndex].size()) {
        std::string strErr = "SubFrameHotpixel: Index error: nIndexStart: " + std::to_string(nIndexStart) +
                             ", nNumber: " + std::to_string(nNumber);
        WriteLog(strErr, nChannelIndex);
        bRes = false;
        return;
    }
    if (RealRoi.Down >= m_nChannelRow || RealRoi.Right >= m_nChannelCol || RealRoi.Down < RealRoi.Up || RealRoi.Right <
        RealRoi.Left) {
        std::string strErr = "SubFrameHotPixel: ROI error: ROI: " + std::to_string(RealRoi.Up) + ", " +
                             std::to_string(RealRoi.Down) + ", " + std::to_string(RealRoi.Left) + ", " +
                             std::to_string(RealRoi.Right) + ", Row: " + std::to_string(m_nChannelRow) + ", Col: " +
                             std::to_string(m_nChannelCol);
        WriteLog(strErr, nChannelIndex);
        bRes = false;
        return;
    }
    uint32_t nRow = RealRoi.Down - RealRoi.Up + 1;
    uint32_t nCol = RealRoi.Right - RealRoi.Left + 1;

    CAPSDataContainer PixelMeanArray;
    PixelMeanArray.Init(nRow, nCol, true);
    std::vector<std::vector<uint32_t> > BadPixelMask(nRow);
    for (uint32_t i = 0; i < nRow; i++) {
        BadPixelMask[i].resize(nCol, 0);
    }

    HotpixelRes.BadPixelNum = 0;
    HotpixelRes.DefectColNum = 0;
    HotpixelRes.DefectRowNum = 0;
    HotpixelRes.SingletNum = 0;
    HotpixelRes.CoupletNum = 0;
    HotpixelRes.ClusterNum = 0;
    HotpixelRes.MaxClusterSize = 0;
    HotpixelRes.BadPixelMask.LocalData.clear();
    HotpixelRes.BadPixelMask.Flag.clear();
    HotpixelRes.BadPixelMask.DiffData.clear();
    HotpixelRes.BadPixelMask.BadPixelNum = 0;
    std::map<Local, float> DiffMap;

    for (uint32_t nRows = 0; nRows < nRow + m_AlgorithmThre.nBadPixelRadius; nRows++) {
        for (uint32_t nCols = 0; nCols < nCol + m_AlgorithmThre.nBadPixelRadius; nCols++) {
            if (nRows < nRow && nCols < nCol) {
                if (m_bHVS_DPC && nRows % 2 == 1 && nCols % 2 == 1) {
                    PixelMeanArray.m_RawData[nRows][nCols] = 10000;
                } else {
                    double dValue = 0;
                    for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++) {
                        dValue += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows + RealRoi.Up][
                            nCols + RealRoi.Left];
                    }
                    PixelMeanArray.m_RawData[nRows][nCols] = round(dValue / nNumber);
                }
            }
            if ((nRows - m_AlgorithmThre.nBadPixelRadius < nRow) && (nCols - m_AlgorithmThre.nBadPixelRadius < nCol) &&
                PixelMeanArray.m_RawData[nRows - m_AlgorithmThre.nBadPixelRadius][
                    nCols - m_AlgorithmThre.nBadPixelRadius] != 10000) {
                uint32_t uSize = 0;
                double dSurroundPixle = 0;
                double dMax = -1;
                double dMin = 10000;
                for (uint32_t i = 0; i < 2 * m_AlgorithmThre.nBadPixelRadius + 1; i++) {
                    for (uint32_t j = 0; j < 2 * m_AlgorithmThre.nBadPixelRadius + 1; j++) {
                        if ((nRows - i < nRow) && (nCols - j < nCol) && (
                                i != m_AlgorithmThre.nBadPixelRadius || j != m_AlgorithmThre.nBadPixelRadius) &&
                            PixelMeanArray.m_RawData[nRows - i][nCols - j] != 10000) {
                            dSurroundPixle += PixelMeanArray.m_RawData[nRows - i][nCols - j];
                            //SortData[uSize] = PixelMeanArray.m_RawData[nRows - i][nCols - j];
                            if (dMax < PixelMeanArray.m_RawData[nRows - i][nCols - j]) {
                                dMax = PixelMeanArray.m_RawData[nRows - i][nCols - j];
                            }
                            if (dMin > PixelMeanArray.m_RawData[nRows - i][nCols - j]) {
                                dMin = PixelMeanArray.m_RawData[nRows - i][nCols - j];
                            }
                            uSize++;
                        }
                    }
                }
                if (uSize > 2) {
                    dSurroundPixle = (dSurroundPixle - dMax - dMin) / (uSize - 2);
                } else {
                    dSurroundPixle = PixelMeanArray.m_RawData[nRows - m_AlgorithmThre.nBadPixelRadius][
                        nCols - m_AlgorithmThre.nBadPixelRadius];
                }
                //std::sort(SortData.begin(), SortData.begin() + uSize);
                //dSurroundPixle = SortData[uSize / 2];
                double dCurrentPixel = PixelMeanArray.m_RawData[nRows - m_AlgorithmThre.nBadPixelRadius][
                    nCols - m_AlgorithmThre.nBadPixelRadius];

                if (abs(dCurrentPixel - dSurroundPixle) > m_AlgorithmThre.dHotPixelThre) {
                    HotpixelRes.BadPixelNum++;
                    //HotpixelRes.BadPixelMask.LocalData.push_back({ nRows - m_AlgorithmThre.nBadPixelRadius + RealRoi.Up, nCols - m_AlgorithmThre.nBadPixelRadius + RealRoi.Left });
                    //HotpixelRes.BadPixelMask.Flag.push_back(APS_HOT_PIXEL_FLAG);
                    //HotpixelRes.BadPixelMask.BadPixelNum++;
                    BadPixelMask[nRows - m_AlgorithmThre.nBadPixelRadius][nCols - m_AlgorithmThre.nBadPixelRadius] = 1;
                    Local temp = {nRows - m_AlgorithmThre.nBadPixelRadius, nCols - m_AlgorithmThre.nBadPixelRadius};
                    DiffMap[temp] = abs(dCurrentPixel - dSurroundPixle);
                }
            }
        }
    }

    uint32_t ConnectedAreaFlag = 0xFFFFFFFF;

    for (uint32_t nRows = 0; nRows < nRow; nRows++) {
        for (uint32_t nCols = 0; nCols < nCol; nCols++) {
            if (BadPixelMask[nRows][nCols] != 0 && BadPixelMask[nRows][nCols] < ConnectedAreaFlag) {
                uint32_t AreaSize = 0;
                std::vector<Local> Search;
                uint32_t nCur = 0;
                BadPixelMask[nRows][nCols] = ConnectedAreaFlag;
                Search.push_back({nRows, nCols});
                while (nCur != Search.size()) {
                    Local temp = Search[nCur];
                    nCur++;
                    AreaSize++;
                    for (int nTempRows = (int) temp.x - 1; nTempRows <= (int) temp.x + 1; nTempRows++) {
                        if (nTempRows >= 0 && nTempRows < nRow) {
                            for (int nTempCols = (int) temp.y - 1; nTempCols <= (int) temp.y + 1; nTempCols++) {
                                if (nTempCols >= 0 && nTempCols < nCol && BadPixelMask[nTempRows][nTempCols] != 0 &&
                                    BadPixelMask[nTempRows][nTempCols] < ConnectedAreaFlag) {
                                    BadPixelMask[nTempRows][nTempCols] = ConnectedAreaFlag;
                                    Search.push_back({(uint32_t) nTempRows, (uint32_t) nTempCols});
                                }
                            }
                        }
                    }
                }
                uint8_t uFlag = 0;
                if (AreaSize == 1) {
                    HotpixelRes.SingletNum++;
                    uFlag = APS_HOT_PIXEL_SINGLET_FLAG;
                } else if (AreaSize == 2) {
                    HotpixelRes.CoupletNum++;
                    uFlag = APS_HOT_PIXEL_COUPLET_FLAG;
                } else {
                    HotpixelRes.ClusterNum++;
                    uFlag = APS_HOT_PIXEL_CLUSTER_FLAG;
                }
                if (AreaSize > HotpixelRes.MaxClusterSize) {
                    HotpixelRes.MaxClusterSize = AreaSize;
                }
                for (uint32_t n = 0; n < Search.size(); n++) {
                    HotpixelRes.BadPixelMask.LocalData.push_back(Search[n]);
                    HotpixelRes.BadPixelMask.Flag.push_back(uFlag);

                    HotpixelRes.BadPixelMask.DiffData.push_back(DiffMap[Search[n]]);
                    HotpixelRes.BadPixelMask.BadPixelNum++;
                }
                ConnectedAreaFlag--;
            }
        }
    }
    std::vector<double> RowMean(nRow, 0);
    std::vector<double> ColMean(nCol, 0);

    double dBaseMean = 0;
    SubFrameDataMean(nIndexStart, nNumber, &RealRoi, nChannelIndex, dBaseMean, bRes, m_RawDataContainer);

    if (!bRes) {
        return;
    }

    for (uint32_t nRows = 0; nRows < nRow; nRows++) {
        for (uint32_t nCols = 0; nCols < nCol; nCols++) {
            double value = 0;
            for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++) {
                value += m_RawDataContainer[nChannelIndex][nIndexStart + nIndex].m_RawData[nRows + RealRoi.Up][
                    nCols + RealRoi.Left];
            }
            RowMean[nRows] += round(value / nNumber);
            ColMean[nCols] += round(value / nNumber);
        }
    }
    for (uint32_t nRows = 0; nRows < nRow; nRows++) {
        RowMean[nRows] /= nCol;
        if (abs(RowMean[nRows] - dBaseMean) > m_AlgorithmThre.dHotLineThre) {
            HotpixelRes.DefectRowNum++;
        }
    }
    for (uint32_t nCols = 0; nCols < nCol; nCols++) {
        ColMean[nCols] /= nRow;
        if (abs(ColMean[nCols] - dBaseMean) > m_AlgorithmThre.dHotLineThre) {
            HotpixelRes.DefectColNum++;
        }
    }
    //for (uint32_t nRows = m_AlgorithmThre.nBadLineRadius; nRows < nRow - m_AlgorithmThre.nBadLineRadius; nRows++)
    //{
    //	double dBaseMean = 0;
    //	for (uint32_t n = nRows - m_AlgorithmThre.nBadLineRadius; n <= nRows + m_AlgorithmThre.nBadLineRadius; n++)
    //	{
    //		if (n != nRows)
    //		{
    //			dBaseMean += RowMean[n];
    //		}
    //	}
    //	dBaseMean /= 2 * m_AlgorithmThre.nBadLineRadius;
    //	if (abs(RowMean[nRows] - dBaseMean) > m_AlgorithmThre.dHotLineThre)
    //	{
    //		HotpixelRes.DefectRowNum++;
    //	}
    //}
    //for (uint32_t nCols = m_AlgorithmThre.nBadLineRadius; nCols < nCol - m_AlgorithmThre.nBadLineRadius; nCols++)
    //{
    //	double dBaseMean = 0;
    //	for (uint32_t n = nCols - m_AlgorithmThre.nBadLineRadius; n <= nCols + m_AlgorithmThre.nBadLineRadius; n++)
    //	{
    //		if (n != nCols)
    //		{
    //			dBaseMean += ColMean[n];
    //		}
    //	}
    //	dBaseMean /= 2 * m_AlgorithmThre.nBadLineRadius;
    //	if (abs(ColMean[nCols] - dBaseMean) > m_AlgorithmThre.dHotLineThre)
    //	{
    //		HotpixelRes.DefectColNum++;
    //	}
    //}
    return;
}
