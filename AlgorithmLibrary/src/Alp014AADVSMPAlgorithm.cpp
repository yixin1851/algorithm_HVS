#include "Alp014AADVSMPAlgorithm.h"

#define DVS_HEADER_014AA 0x0000FFFF
#define DVS_FOOTER_014AA 0x0101FFFF
#define DVS_FOOTER_DROP_014AA 0x0303FFFF

CAlp014AADVSMPAlgorithm::CAlp014AADVSMPAlgorithm(SensorType Sensortype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code)
	:CAlpDVSMPAlgorithm(Sensortype, strLogDir, nSiteNum, Pixelformat, code)
{
	if ((code & DVS_Code_HVS) == DVS_Code_HVS)
	{
		m_bHVS = true;
		m_nTotalRow = 480;
		m_nTotalCol = 1280;
	}
	else
	{
		m_nTotalRow = 960;
		m_nTotalCol = 1280;
		m_bHVS = false;
	}
	m_ActiveArea = { 0, m_nTotalRow - 1, 0, m_nTotalCol - 1 };
}

CAlp014AADVSMPAlgorithm::~CAlp014AADVSMPAlgorithm()
{
}

bool CAlp014AADVSMPAlgorithm::ImportRawData(uint8_t* pBinData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber)
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
		uint32_t nMaxSubFrame = m_bHVS ? 16 : 32;
		uint32_t nRow = m_bHVS ? m_nTotalRow / 4 : m_nTotalRow / 8;
		uint32_t nCol = m_nTotalCol / 4;

		while (nNeedSubFrameIndex != nMaxSubFrame)
		{
			if (Decode(pBinData, &m_RawDataContainer[nIndexStart + nIndex], nRow, nCol, &pos, nLens, nSubFrameIndex, nTimeStamp) && nSubFrameIndex == nNeedSubFrameIndex)
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

bool CAlp014AADVSMPAlgorithm::ImportRawData_DropSubFrame(uint8_t *pBinData, uint64_t nLens, uint32_t nIndexStart,
    uint32_t nNumber, uint32_t nMode, size_t &nDropSubFrameNum) {
    return true;
}

bool CAlp014AADVSMPAlgorithm::Decode(uint8_t* pucBinData, CDVSDataContainer* DVSData, uint32_t nRow, uint32_t nCol, size_t* pnPos, size_t nBinLens, uint8_t& nSubFrameIndex, uint64_t& nTimeStamp)
{
	size_t nCurIndex = *pnPos;
	Alp014AAFormatHeader* HeaderCode;
	while (nBinLens > nCurIndex + sizeof(Alp014AAFormatHeader))
	{
		HeaderCode = (Alp014AAFormatHeader*)(pucBinData + nCurIndex);

		if (HeaderCode->Header_vec != DVS_HEADER_014AA)
		{
			nCurIndex += 4;
		}
		else
		{
			break;
		}
	}
	if (nBinLens <= nCurIndex + sizeof(Alp014AAFormatHeader))
	{
		return false;
	}

	HeaderCode = (Alp014AAFormatHeader*)(pucBinData + nCurIndex);
	nTimeStamp = (uint64_t(HeaderCode->Timestamp_H) << 32) + HeaderCode->Timestamp_L;
	nCurIndex += sizeof(Alp014AAFormatHeader);

	Alp014AAFormatStatic* Static;

	if (nCurIndex + sizeof(Alp014AAFormatStatic) < nBinLens)
	{
		Static = (Alp014AAFormatStatic*)(pucBinData + nCurIndex);
		nCurIndex += sizeof(Alp014AAFormatStatic);
	}
	else
	{
		return false;
	}

	nSubFrameIndex = Static->Subframe;

	if (m_bHVS)
	{
		nSubFrameIndex -= 16;
	}

	if ((Static->Roi_row_stop - Static->Roi_row_start + 1) > nRow || (Static->Roi_col_stop - Static->Roi_col_start + 1) > nCol)
	{
		return false;
	}

	bool bRet = true;

	if (Static->Framemode == 0)
	{
		bRet = EventModeDecode(pucBinData, DVSData, 0, nRow, 0, nCol, &nCurIndex, nBinLens, nSubFrameIndex);
	}
	else
	{
		bRet = FrameModeDecode(pucBinData, DVSData, 0, nRow, 0, nCol, &nCurIndex, nBinLens, nSubFrameIndex);
	}
	if (bRet)
	{
		if (nCurIndex % 8 != 0)
		{
			nCurIndex = (nCurIndex / 8 + 1) * 8;
		}

		Alp014AAFormatFooter* Footer = (Alp014AAFormatFooter*)(pucBinData + nCurIndex);

		if (Footer->Footer_vec == DVS_FOOTER_014AA && Footer->Dropflag == 0)
		{
			nCurIndex += sizeof(Alp014AAFormatFooter);
			*pnPos = nCurIndex;
			return true;
		}
	}
	return false;
}

bool CAlp014AADVSMPAlgorithm::FrameModeDecode(uint8_t* pucBinData, CDVSDataContainer* DVSData, uint32_t nRowStart, uint32_t nRowStop, uint32_t nColStart, uint32_t nColStop, size_t* pnPos, size_t nBinLens, uint8_t& nSubFrameIndex)
{
	bool bRet = false;

	uint32_t nRowStep = 8;
	uint32_t nColStep = 4;
	if (m_bHVS)
	{
		nRowStep = 4;
	}
	nRowStart *= nRowStep;
	nColStart *= nColStep;
	nRowStop *= nRowStep;
	nColStop *= nColStep;

	uint32_t nRow = nRowStart;
	uint32_t nCol = nColStart;
	size_t& nCurIndex = *pnPos;

	Alp014AAFormatFooter* Footer;
	Alp014AAFormatEventGroup* EventGroup;

	while (nCurIndex + sizeof(Alp014AAFormatFooter) < nBinLens)
	{
		Footer = (Alp014AAFormatFooter*)(pucBinData + nCurIndex);

		if (Footer->Footer_vec == DVS_FOOTER_014AA || Footer->Footer_vec == DVS_FOOTER_DROP_014AA)
		{
			return false;
		}

		for (uint32_t n = 0; n < 8; n++)
		{
			EventGroup = (Alp014AAFormatEventGroup*)(pucBinData + nCurIndex);

			SetData(DVSData, nRow, nCol, nSubFrameIndex, EventGroup->Pix0);
			nCol += nColStep;
			SetData(DVSData, nRow, nCol, nSubFrameIndex, EventGroup->Pix1);
			nCol += nColStep;
			SetData(DVSData, nRow, nCol, nSubFrameIndex, EventGroup->Pix2);
			nCol += nColStep;
			SetData(DVSData, nRow, nCol, nSubFrameIndex, EventGroup->Pix3);
			nCol += nColStep;

			nCurIndex++;

			if (nCol == nColStop)
			{
				nRow += nRowStep;
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

bool CAlp014AADVSMPAlgorithm::EventModeDecode(uint8_t* pucBinData, CDVSDataContainer* DVSData, uint32_t nRowStart, uint32_t nRowStop, uint32_t nColStart, uint32_t nColStop, size_t* pnPos, size_t nBinLens, uint8_t& nSubFrameIndex)
{
	bool bRet = false;

	uint32_t nRowStep = 8;
	uint32_t nColStep = 4;
	if (m_bHVS)
	{
		nRowStep = 4;
	}
	nRowStart *= nRowStep;
	nColStart *= nColStep;
	nRowStop *= nRowStep;
	nColStop *= nColStep;

	uint32_t nRow = nRowStart;
	uint32_t nCol = nColStart;
	size_t& nCurIndex = *pnPos;

	Alp014AAFormatFooter* Footer;
	Alp014AAFormatEventGroup* EventGroup;
	Alp014AAFormatVoidByte* VoidByte;

	while (nCurIndex + sizeof(Alp014AAFormatFooter) < nBinLens)
	{
		Footer = (Alp014AAFormatFooter*)(pucBinData + nCurIndex);

		if (Footer->Footer_vec == DVS_FOOTER_014AA || Footer->Footer_vec == DVS_FOOTER_DROP_014AA)
		{
			return false;
		}

		for (uint32_t n = 0; n < 8; n++)
		{
			EventGroup = (Alp014AAFormatEventGroup*)(pucBinData + nCurIndex);

			if (EventGroup->Pix3 != 3)
			{
				if (EventGroup->Pix0)
				{
					SetData(DVSData, nRow, nCol, nSubFrameIndex, EventGroup->Pix0);
				}
				nCol += nColStep;
				if (EventGroup->Pix1)
				{
					SetData(DVSData, nRow, nCol, nSubFrameIndex, EventGroup->Pix1);
				}
				nCol += nColStep;
				if (EventGroup->Pix2)
				{
					SetData(DVSData, nRow, nCol, nSubFrameIndex, EventGroup->Pix2);
				}
				nCol += nColStep;
				if (EventGroup->Pix3)
				{
					SetData(DVSData, nRow, nCol, nSubFrameIndex, EventGroup->Pix3);
				}
				nCol += nColStep;

				if (nCol == nColStop)
				{
					nRow += nRowStep;
					nCol = nColStart;
				}
				else if (nCol > nColStop)
				{
					return false;
				}
			}
			else
			{
				VoidByte = (Alp014AAFormatVoidByte*)(pucBinData + nCurIndex);

				nCol += ((static_cast<size_t>(VoidByte->Voidbytelen) + 1) << 2) * nColStep;
				if (nCol == nColStop)
				{
					nRow += nRowStep;
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

void CAlp014AADVSMPAlgorithm::SetData(CDVSDataContainer* DVSData, uint32_t nRow, uint32_t nCol, uint8_t nSubFrameIndex, uint8_t nEventFlag)
{
	if (m_bHVS)
	{
		switch (nSubFrameIndex)
		{
		case 0:
			break;
		case 1:
			nCol += 2;
			break;
		case 2:
			nRow += 1;
			break;
		case 3:
			nRow += 1;
			nCol += 2;
			break;
		case 4:
			nRow += 2;
			break;
		case 5:
			nRow += 2;
			nCol += 2;
			break;
		case 6:
			nRow += 3;
			break;
		case 7:
			nRow += 3;
			nCol += 2;
			break;
		case 8:
			nCol += 1;
			break;
		case 9:
			nCol += 3;
			break;
		case 10:
			nRow += 1;
			nCol += 1;
			break;
		case 11:
			nRow += 1;
			nCol += 3;
			break;
		case 12:
			nRow += 2;
			nCol += 1;
			break;
		case 13:
			nRow += 2;
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
	}
	else
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
			nRow += 4;
			break;
		case 5:
			nRow += 4;
			nCol += 2;
			break;
		case 6:
			nRow += 6;
			break;
		case 7:
			nRow += 6;
			nCol += 2;
			break;
		case 8:
			nCol += 1;
			break;
		case 9:
			nCol += 3;
			break;
		case 10:
			nRow += 2;
			nCol += 1;
			break;
		case 11:
			nRow += 2;
			nCol += 3;
			break;
		case 12:
			nRow += 4;
			nCol += 1;
			break;
		case 13:
			nRow += 4;
			nCol += 3;
			break;
		case 14:
			nRow += 6;
			nCol += 1;
			break;
		case 15:
			nRow += 6;
			nCol += 3;
			break;

		case 16:
			nRow += 1;
			break;
		case 17:
			nRow += 1;
			nCol += 2;
			break;
		case 18:
			nRow += 3;
			break;
		case 19:
			nRow += 3;
			nCol += 2;
			break;
		case 20:
			nRow += 5;
			nCol += 0;
			break;
		case 21:
			nRow += 5;
			nCol += 2;
			break;
		case 22:
			nRow += 7;
			nCol += 0;
			break;
		case 23:
			nRow += 7;
			nCol += 2;
			break;
		case 24:
			nRow += 1;
			nCol += 1;
			break;
		case 25:
			nRow += 1;
			nCol += 3;
			break;
		case 26:
			nRow += 3;
			nCol += 1;
			break;
		case 27:
			nRow += 3;
			nCol += 3;
			break;
		case 28:
			nRow += 5;
			nCol += 1;
			break;
		case 29:
			nRow += 5;
			nCol += 3;
			break;
		case 30:
			nRow += 7;
			nCol += 1;
			break;
		case 31:
			nRow += 7;
			nCol += 3;
			break;
		}
	}
	DVSData->SetData(nRow, nCol, nEventFlag);
}
