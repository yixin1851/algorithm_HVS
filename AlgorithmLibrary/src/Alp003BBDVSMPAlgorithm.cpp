#include "Alp003BBDVSMPAlgorithm.h"

CAlp003BBDVSMPAlgorithm::CAlp003BBDVSMPAlgorithm(SensorType Sensortype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code)
	:CAlp003BADVSMPAlgorithm(Sensortype, strLogDir, nSiteNum, Pixelformat, code)
{
	m_nTotalRow = 1224;
	m_nTotalCol = 1632;
	m_ActiveArea = { 0, m_nTotalRow - 1, 0, m_nTotalCol - 1 };
	m_03BADVSDecoder.SetHalfMode(false);
}

CAlp003BBDVSMPAlgorithm::~CAlp003BBDVSMPAlgorithm()
{
}

bool CAlp003BBDVSMPAlgorithm::ImportRawData_DropSubFrame(uint8_t *pBinData, uint64_t nLens, uint32_t nIndexStart,
    uint32_t nNumber, uint32_t nMode, size_t &nDropSubFrameNum) {
    return CAlp003BADVSMPAlgorithm::ImportRawData_DropSubFrame(pBinData, nLens, nIndexStart, nNumber, nMode,
                                                               nDropSubFrameNum);
}
