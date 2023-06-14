#include "Alp003BBDVSMPAlgorithm.h"

CAlp003BBDVSMPAlgorithm::CAlp003BBDVSMPAlgorithm(SensorType Sensortype, std::string strLogDir, uint32_t nSiteNum)
	:CAlp003BADVSMPAlgorithm(Sensortype, strLogDir, nSiteNum)
{
	m_nTotalRow = 1224 - 54;
	m_nTotalCol = 1632;
	m_ActiveArea = { 0, m_nTotalRow - 1, 0, m_nTotalCol - 1 };
	m_03BADVSDecoder.SetHalfMode(false);
}

CAlp003BBDVSMPAlgorithm::~CAlp003BBDVSMPAlgorithm()
{
}
