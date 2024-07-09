#include "AlpMPAlgoInterface.h"
#include "AlpAPSMPAlgorithm.h"
#include "AlpDVSMPAlgorithm.h"
#include "Alp003AADVSMPAlgorithm.h"
#include "Alp003BADVSMPAlgorithm.h"
#include "Alp003BBDVSMPAlgorithm.h"
#include "Alp003CADVSMPAlgorithm.h"
#include "Alp003AAAPSMPAlgorithm.h"
#include "Alp003BAAPSMPAlgorithm.h"
#include "Alp003BBAPSMPAlgorithm.h"
#include "Alp003CAAPSMPAlgorithm.h"
#include "Alp004ABAPSMPAlgorithm.h"
#include "Alp004ABDVSMPAlgorithm.h"
#include "Alp014AAAPSMPAlgorithm.h"

//#include "vld.h"

uint32_t CAlpAPSMPAlgoInterface::m_nSiteNumber = 0;
uint32_t CAlpDVSMPAlgoInterface::m_nSiteNumber = 0;

CAlpAPSMPAlgoInterface* CAlpAPSMPAlgoInterface::CreateAPSAlgoInterface(SensorType Sensortype, APSRawType Rawtype, std::string strLogDir, PixelFormatType Pixelformat, int code)
{
	if (Sensortype == ALP_003AA)
	{
		return new CAlp003AAAPSMPAlgorithm(Sensortype, Rawtype, strLogDir, m_nSiteNumber++, Pixelformat, code);
	}
	else if (Sensortype == ALP_003BA)
	{
		return new CAlp003BAAPSMPAlgorithm(Sensortype, Rawtype, strLogDir, m_nSiteNumber++, Pixelformat, code);
	}
	else if (Sensortype == ALP_003BB)
	{
		return new CAlp003BBAPSMPAlgorithm(Sensortype, Rawtype, strLogDir, m_nSiteNumber++, Pixelformat, code);
	}
	else if (Sensortype == ALP_003CA)
	{
		return new CAlp003CAAPSMPAlgorithm(Sensortype, Rawtype, strLogDir, m_nSiteNumber++, Pixelformat, code);
	}
	else if (Sensortype == ALP_004AB)
	{
		return new CAlp004ABAPSMPAlgorithm(Sensortype, Rawtype, strLogDir, m_nSiteNumber++, Pixelformat, code);
	}
	else if (Sensortype == ALP_014AA)
	{
		return new CAlp014AAAPSMPAlgorithm(Sensortype, Rawtype, strLogDir, m_nSiteNumber++, Pixelformat, code);
	}
	else
	{
		return nullptr;
	}
}

CAlpAPSMPAlgoInterface::~CAlpAPSMPAlgoInterface()
{
}

CAlpDVSMPAlgoInterface::~CAlpDVSMPAlgoInterface()
{

}

CAlpDVSMPAlgoInterface* CAlpDVSMPAlgoInterface::CreateDVSAlgoInterface(SensorType Sensortype, std::string strLogDir, PixelFormatType Pixelformat, int code)
{
	if (Pixelformat < BayerGBRG || Pixelformat > BayerGRBG)
	{
		return nullptr;
	}

	if (Sensortype == ALP_003AA)
	{
		return new CAlp003AADVSMPAlgorithm(Sensortype, strLogDir, m_nSiteNumber++, Pixelformat, code);
	}
	else if (Sensortype == ALP_003BA)
	{
		return new CAlp003BADVSMPAlgorithm(Sensortype, strLogDir, m_nSiteNumber++, Pixelformat, code);
	}
	else if (Sensortype == ALP_003BB)
	{
		return new CAlp003BBDVSMPAlgorithm(Sensortype, strLogDir, m_nSiteNumber++, Pixelformat, code);
	}
	else if (Sensortype == ALP_003CA)
	{
		return new CAlp003CADVSMPAlgorithm(Sensortype, strLogDir, m_nSiteNumber++, Pixelformat, code);
	}
	else if (Sensortype == ALP_004AB)
	{
		return new CAlp004ABDVSMPAlgorithm(Sensortype, strLogDir, m_nSiteNumber++, Pixelformat, code);
	}
	else
	{
		return nullptr;
	}
}

bool operator==(const Local& lh, const Local& rh)
{
	if (lh.x == rh.x && lh.y == rh.y)
	{
		return true;
	}
	else
	{
		return false;
	}
}

ALP_ALGO_DLL_API CAlpAPSMPAlgoInterface* CreateAPSAlgoInterface(SensorType Sensortype, APSRawType Rawtype, std::string strLogDir, PixelFormatType Pixelformat, int code)
{
	return CAlpAPSMPAlgoInterface::CreateAPSAlgoInterface(Sensortype, Rawtype, strLogDir, Pixelformat, code);
}

ALP_ALGO_DLL_API CAlpDVSMPAlgoInterface* CreateDVSAlgoInterface(SensorType Sensortype, std::string strLogDir, PixelFormatType Pixelformat, int code)
{
	return CAlpDVSMPAlgoInterface::CreateDVSAlgoInterface(Sensortype, strLogDir, Pixelformat, code);
}
