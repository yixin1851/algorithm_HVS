#include "AlpMPAlgoInterface.h"
#include "AlpAPSMPAlgorithm.h"
#include "AlpDVSMPAlgorithm.h"
//#include "vld.h"

uint32_t CAlpAPSMPAlgoInterface::m_nSiteNumber = 0;
uint32_t CAlpDVSMPAlgoInterface::m_nSiteNumber = 0;

CAlpAPSMPAlgoInterface* CAlpAPSMPAlgoInterface::CreateAPSAlgoInterface(SensorType Sensortype, RawType Rawtype, std::string strLogDir)
{
    return new CAlpAPSMPAlgorithm(Sensortype, Rawtype, strLogDir, m_nSiteNumber++);
}

CAlpAPSMPAlgoInterface::~CAlpAPSMPAlgoInterface()
{
}

CAlpDVSMPAlgoInterface::~CAlpDVSMPAlgoInterface()
{

}

CAlpDVSMPAlgoInterface* CAlpDVSMPAlgoInterface::CreateDVSAlgoInterface(SensorType Sensortype, std::string strLogDir)
{
    return new CAlpDVSMPAlgorithm(Sensortype, strLogDir, m_nSiteNumber++);
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

ALP_ALGO_DLL_API CAlpAPSMPAlgoInterface* CreateAPSAlgoInterface(SensorType Sensortype, RawType Rawtype, std::string strLogDir)
{
	return CAlpAPSMPAlgoInterface::CreateAPSAlgoInterface(Sensortype, Rawtype, strLogDir);
}

ALP_ALGO_DLL_API CAlpDVSMPAlgoInterface* CreateDVSAlgoInterface(SensorType Sensortype, std::string strLogDir)
{
	return CAlpDVSMPAlgoInterface::CreateDVSAlgoInterface(Sensortype, strLogDir);
}
