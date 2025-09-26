#include "Alp014BAAPSMPAlgorithm.h"

CAlp014BAAPSMPAlgorithm::CAlp014BAAPSMPAlgorithm(SensorType Sensortype, APSRawType Rawtype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code)
	: CAlp014AAAPSMPAlgorithm(Sensortype, Rawtype, strLogDir, nSiteNum, Pixelformat, code)
{
	if ((code & APSCodeType::APS_Code_HVS) == APSCodeType::APS_Code_HVS)
	{
		m_ActiveArea = { 0, 1023, 0, 639 };
		m_nChannelRow = 1024;
		m_nChannelCol = 640;
		m_nTotalRow = 1024;
		m_nTotalCol = 640;

		m_AlgorithmThre.nDSNURowBlockNum = 25;
		m_AlgorithmThre.nDSNUColBlockNum = 16;
		m_AlgorithmThre.nDSNURowBlockSize = 40;
		m_AlgorithmThre.nDSNUColBlockSize = 40;
		m_AlgorithmThre.nPedestalVariationRowBlockNum = 8;
		m_AlgorithmThre.nPedestalVariationColBlockNum = 8;
		m_AlgorithmThre.nPedestalVariationRowBlockSize = 128;
		m_AlgorithmThre.nPedestalVariationColBlockSize = 80;
	}
	else
	{
		m_ActiveArea = { 0, 1023, 0, 1279 };
		m_nChannelRow = 1024;
		m_nChannelCol = 1280;
		m_nTotalRow = 1024;
		m_nTotalCol = 1280;

		m_AlgorithmThre.nDSNURowBlockNum = 25;
		m_AlgorithmThre.nDSNUColBlockNum = 32;
		m_AlgorithmThre.nDSNURowBlockSize = 40;
		m_AlgorithmThre.nDSNUColBlockSize = 40;
		m_AlgorithmThre.nPedestalVariationRowBlockNum = 8;
		m_AlgorithmThre.nPedestalVariationColBlockNum = 8;
		m_AlgorithmThre.nPedestalVariationRowBlockSize = 128;
		m_AlgorithmThre.nPedestalVariationColBlockSize = 160;
	}

	m_AlgorithmThre.nOETCRadius = 128;
	m_AlgorithmThre.nLinearityRadius = 32;
}

CAlp014BAAPSMPAlgorithm::~CAlp014BAAPSMPAlgorithm()
{
}