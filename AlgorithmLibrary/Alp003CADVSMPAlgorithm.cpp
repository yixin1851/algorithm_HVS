#include "Alp003CADVSMPAlgorithm.h"

#define DVS_HEADER_003CA 0x00FFFF
#define DVS_FOOTER_003CA 0x0101FFFF
#define DVS_FOOTER_DROP_003CA 0x0303FFFF

CAlp003CADVSMPAlgorithm::CAlp003CADVSMPAlgorithm(SensorType Sensortype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat)
	:CAlpDVSMPAlgorithm(Sensortype, strLogDir, nSiteNum, Pixelformat)
{
	m_nTotalRow = 1224;
	m_nTotalCol = 1632;
	m_ActiveArea = { 0, m_nTotalRow - 1, 0, m_nTotalCol - 1 };
}

CAlp003CADVSMPAlgorithm::~CAlp003CADVSMPAlgorithm()
{
}

bool CAlp003CADVSMPAlgorithm::ImportRawData(uint8_t* pBinData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber)
{
	size_t pos = 0;
	if (m_RawDataContainer.size() < nIndexStart + nNumber)
	{
		m_RawDataContainer.resize(nIndexStart + nNumber);
	}

	for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
	{
		uint8_t nNeedSubFrameIndex = 0;
		m_RawDataContainer[nIndexStart + nIndex].Init(m_nTotalRow, m_nTotalCol, true, m_PixelFormat);
		uint8_t nSubFrameIndex = 0;
		uint64_t nTimeStamp = 0;
		while (nNeedSubFrameIndex != 16)
		{
			if (Decode(pBinData, &m_RawDataContainer[nIndexStart + nIndex], m_nTotalRow / 4, m_nTotalCol / 4, &pos, nLens, nSubFrameIndex, nTimeStamp) && nSubFrameIndex == nNeedSubFrameIndex)
			{
				++nNeedSubFrameIndex;
			}
			else
			{
				std::string strErr = "ImportRawData: DVS Decoder error: Index: " + std::to_string(nIndex) + ", Pos: " + std::to_string(pos);
				WriteLog(strErr);
				m_nErrCode = EVS_DECODE_ERROR;
				return false;
			}
		}
	}
	return true;
}

bool CAlp003CADVSMPAlgorithm::Decode(uint8_t* pucBinData, CDVSDataContainer* DVSData, uint32_t nRow, uint32_t nCol, size_t* pnPos, size_t nBinLens, uint8_t& nSubFrameIndex, uint64_t& nTimeStamp)
{
	size_t nCurIndex = *pnPos;
	Alp003CAFormatHeader* HeaderCode;
	while (nBinLens > nCurIndex + sizeof(Alp003CAFormatHeader))
	{
		HeaderCode = (Alp003CAFormatHeader*)(pucBinData + nCurIndex);

		if (HeaderCode->Header_vec != DVS_HEADER_003CA)
		{
			nCurIndex += 4;
		}
		else
		{
			break;
		}
	}
	if (nBinLens <= nCurIndex + sizeof(Alp003CAFormatHeader))
	{
		return false;
	}

	HeaderCode = (Alp003CAFormatHeader*)(pucBinData + nCurIndex);
	nTimeStamp = HeaderCode->Timestamp;
	nCurIndex += sizeof(Alp003CAFormatHeader);

	Alp003CAFormatStatic* Static;

	if (nCurIndex + sizeof(Alp003CAFormatStatic) < nBinLens)
	{
		Static = (Alp003CAFormatStatic*)(pucBinData + nCurIndex);
		nCurIndex += sizeof(Alp003CAFormatStatic);
	}
	else
	{
		return false;
	}

	nSubFrameIndex = Static->Subframe;

	if (Static->Roi_row_stop > nRow || Static->Roi_col_stop > nCol)
	{
		return false;
	}

	bool bRet = true;

	if (Static->Framemode == 0)
	{
		bRet = EventModeDecode(pucBinData, DVSData, Static->Roi_row_start, Static->Roi_row_stop, Static->Roi_col_start, Static->Roi_col_stop, &nCurIndex, nBinLens, nSubFrameIndex);
	}
	else
	{
		bRet = FrameModeDecode(pucBinData, DVSData, Static->Roi_row_start, Static->Roi_row_stop, Static->Roi_col_start, Static->Roi_col_stop, &nCurIndex, nBinLens, nSubFrameIndex);
	}
	if (bRet)
	{
		if (nCurIndex % 8 != 0)
		{
			nCurIndex = (nCurIndex / 8 + 1) * 8;
		}

		Alp003CAFormatFooter * Footer = (Alp003CAFormatFooter*)(pucBinData + nCurIndex);

		if (Footer->Footer_vec == DVS_FOOTER_003CA && Footer->Dropflag == 0)
		{
			nCurIndex += sizeof(Alp003CAFormatFooter);
			*pnPos = nCurIndex;
			return true;
		}
	}
	return false;
}

bool CAlp003CADVSMPAlgorithm::FrameModeDecode(uint8_t* pucBinData, CDVSDataContainer* DVSData, uint32_t nRowStart, uint32_t nRowStop, uint32_t nColStart, uint32_t nColStop, size_t* pnPos, size_t nBinLens, uint8_t& nSubFrameIndex)
{
	bool bRet = false;

	nRowStart *= 4;
	nColStart *= 4;
	nRowStop *= 4;
	nColStop *= 4;

	uint32_t nRow = nRowStart;
	uint32_t nCol = nColStart;
	size_t& nCurIndex = *pnPos;

	Alp003CAFormatFooter* Footer;
	Alp003CAFormatEventGroup* EventGroup;

	while (nCurIndex + sizeof(Alp003CAFormatFooter) < nBinLens)
	{
		Footer = (Alp003CAFormatFooter*)(pucBinData + nCurIndex);

		if (Footer->Footer_vec == DVS_FOOTER_003CA || Footer->Footer_vec == DVS_FOOTER_DROP_003CA)
		{
			return false;
		}

		for (uint32_t n = 0; n < 8; n++)
		{
			EventGroup = (Alp003CAFormatEventGroup*)(pucBinData + nCurIndex);

			SetData(DVSData, nRow, nCol, nSubFrameIndex, EventGroup->Pix0);
			nCol += 4;
			SetData(DVSData, nRow, nCol, nSubFrameIndex, EventGroup->Pix1);
			nCol += 4;
			SetData(DVSData, nRow, nCol, nSubFrameIndex, EventGroup->Pix2);
			nCol += 4;
			SetData(DVSData, nRow, nCol, nSubFrameIndex, EventGroup->Pix3);
			nCol += 4;

			nCurIndex++;

			if (nCol == nColStop)
			{
				nRow += 4;
				nCol = nColStart;
			}
			else if (nCol > nColStop)
			{
				return false;
			}

			if (nRow == nRowStop)
			{
				return true;
			}
			else if (nRow > nRowStop)
			{
				return false;
			}
		}
	}
	return false;
}

bool CAlp003CADVSMPAlgorithm::EventModeDecode(uint8_t* pucBinData, CDVSDataContainer* DVSData, uint32_t nRowStart, uint32_t nRowStop, uint32_t nColStart, uint32_t nColStop, size_t* pnPos, size_t nBinLens, uint8_t& nSubFrameIndex)
{
	bool bRet = false;

	nRowStart *= 4;
	nColStart *= 4;
	nRowStop *= 4;
	nColStop *= 4;

	uint32_t nRow = nRowStart;
	uint32_t nCol = nColStart;
	size_t& nCurIndex = *pnPos;

	Alp003CAFormatFooter* Footer;
	Alp003CAFormatEventGroup* EventGroup;
	Alp003CAFormatVoidByte* VoidByte;

	while (nCurIndex + sizeof(Alp003CAFormatFooter) < nBinLens)
	{
		Footer = (Alp003CAFormatFooter*)(pucBinData + nCurIndex);

		if (Footer->Footer_vec == DVS_FOOTER_003CA || Footer->Footer_vec == DVS_FOOTER_DROP_003CA)
		{
			return false;
		}

		for (uint32_t n = 0; n < 8; n++)
		{
			EventGroup = (Alp003CAFormatEventGroup*)(pucBinData + nCurIndex);

			if (EventGroup->Pix3 != 3)
			{
				if (EventGroup->Pix0)
				{
					SetData(DVSData, nRow, nCol, nSubFrameIndex, EventGroup->Pix0);
				}
				nCol += 4;
				if (EventGroup->Pix1)
				{
					SetData(DVSData, nRow, nCol, nSubFrameIndex, EventGroup->Pix1);
				}
				nCol += 4;
				if (EventGroup->Pix2)
				{
					SetData(DVSData, nRow, nCol, nSubFrameIndex, EventGroup->Pix2);
				}
				nCol += 4;
				if (EventGroup->Pix3)
				{
					SetData(DVSData, nRow, nCol, nSubFrameIndex, EventGroup->Pix3);
				}
				nCol += 4;

				if (nCol == nColStop)
				{
					nRow += 4;
					nCol = nColStart;
				}
				else if (nCol > nColStop)
				{
					return false;
				}
			}
			else
			{
				VoidByte = (Alp003CAFormatVoidByte*)(pucBinData + nCurIndex);

				nCol += ((static_cast<size_t>(VoidByte->Voidbytelen) + 1) << 4);
				if (nCol == nColStop)
				{
					nRow += 4;
					nCol = nColStart;
				}
				else if (nCol > nColStop)
				{
					return false;
				}
			}
			nCurIndex++;

			if (nRow == nRowStop)
			{
				return true;
			}
			else if (nRow > nRowStop)
			{
				return false;
			}
		}
	}
	return false;
}

void CAlp003CADVSMPAlgorithm::SetData(CDVSDataContainer* DVSData, uint32_t nRow, uint32_t nCol, uint8_t nSubFrameIndex, uint8_t nEventFlag)
{
	switch (nSubFrameIndex)
	{
	case 0:
		break;
	case 1:
		nCol += 2;
		break;
	case 2:
		nRow += 2;
		break;
	case 3:
		nRow += 2;
		nCol += 2;
		break;
	case 4:
		nCol += 1;
		break;
	case 5:
		nCol += 3;
		break;
	case 6:
		nRow += 2;
		nCol += 1;
		break;
	case 7:
		nRow += 2;
		nCol += 3;
		break;
	case 8:
		nRow += 1;
		break;
	case 9:
		nRow += 1;
		nCol += 2;
		break;
	case 10:
		nRow += 3;
		break;
	case 11:
		nRow += 3;
		nCol += 2;
		break;
	case 12:
		nRow += 1;
		nCol += 1;
		break;
	case 13:
		nRow += 1;
		nCol += 3;
		break;
	case 14:
		nRow += 3;
		nCol += 1;
		break;
	case 15:
		nRow += 3;
		nCol += 3;
		break;
	}
	DVSData->SetData(nRow, nCol, nEventFlag);
}
