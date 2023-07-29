#include "Alp003BBAPSMPAlgorithm.h"

CAlp003BBAPSMPAlgorithm::CAlp003BBAPSMPAlgorithm(SensorType Sensortype, APSRawType Rawtype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code)
	: CAlp003BAAPSMPAlgorithm(Sensortype, Rawtype, strLogDir, nSiteNum, Pixelformat, code)
{
	m_AlgorithmThre.nDSNURowBlockNum = 30;
	m_AlgorithmThre.nDSNUColBlockNum = 40;
	m_AlgorithmThre.nDSNURowBlockSize = 40;
	m_AlgorithmThre.nDSNUColBlockSize = 40;
}

CAlp003BBAPSMPAlgorithm::~CAlp003BBAPSMPAlgorithm()
{
}
