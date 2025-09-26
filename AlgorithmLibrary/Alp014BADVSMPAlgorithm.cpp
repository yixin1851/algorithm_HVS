#include "Alp014BADVSMPAlgorithm.h"

#define DVS_HEADER_014BA 0x0000FFFF
#define DVS_FOOTER_014BA 0x0101FFFF
#define DVS_FOOTER_DROP_014BA 0x0303FFFF

uint32_t EVS_only_readout_order[] = { 0 , 2 , 8 , 10 , 16 , 18 , 24 , 26 , 32 , 34 , 40 , 42 , 48 , 50 , 56 , 58,  1 , 3 , 9 , 11 , 17 , 19 , 25 , 27 , 33, 35, 41, 43, 49, 51, 57 , 59, 4 , 6 , 12 , 14 , 20 , 22 , 28, 30, 36, 38, 44, 46, 52, 54, 60 , 62, 5 , 7 , 13 , 15 , 21 , 23 , 29, 31, 37, 39, 45, 47, 53, 55, 61 , 63 };
uint32_t HVS_readout_order[] = {1 , 3 , 9 , 11 , 17 , 19 , 25 , 27 , 33, 35, 41, 43, 49, 51, 57 , 59, 4 , 6 , 12 , 14 , 20 , 22 , 28, 30, 36, 38, 44, 46, 52, 54, 60 , 62};


CAlp014BADVSMPAlgorithm::CAlp014BADVSMPAlgorithm(SensorType Sensortype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code)
	:CAlp014AADVSMPAlgorithm(Sensortype, strLogDir, nSiteNum, Pixelformat, code)
{
	if ((code & DVS_Code_HVS) == DVS_Code_HVS)
	{
		m_bHVS = true;
		m_nTotalRow = 512;
		m_nTotalCol = 1280;

		for (uint32_t i = 0; i < 32; i += 1)
		{
			m_subframe_order[i] = HVS_readout_order[i];

			switch (HVS_readout_order[i] % 8)
			{
			case 1:
				m_col_offset_table[i] = 1;
				break;
			case 3:
				m_col_offset_table[i] = 3;
				break;
			case 4:
				m_col_offset_table[i] = 0;
				break;
			case 6:
				m_col_offset_table[i] = 2;
				break;
			default:
				break;
			}
			m_row_offset_table[i] = HVS_readout_order[i] / 8;
		}
	}
	else
	{
		m_nTotalRow = 1024;
		m_nTotalCol = 1280;
		m_bHVS = false;

		for (uint32_t i = 0; i < 64; i++)
		{
			m_subframe_order[i] = EVS_only_readout_order[i];
			m_row_offset_table[i] = EVS_only_readout_order[i] / 4;
			m_col_offset_table[i] = EVS_only_readout_order[i] % 4;
		}
	}
	m_ActiveArea = { 0, m_nTotalRow - 1, 0, m_nTotalCol - 1 };
}

CAlp014BADVSMPAlgorithm::~CAlp014BADVSMPAlgorithm()
{
}

bool CAlp014BADVSMPAlgorithm::ImportRawData(uint8_t* pBinData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber)
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
		uint32_t nMaxSubFrame = m_bHVS ? 32 : 64;
		uint32_t nRow = m_bHVS ? m_nTotalRow / 8 : m_nTotalRow / 16;
		uint32_t nCol = m_nTotalCol / 4;

		while (nNeedSubFrameIndex != nMaxSubFrame)
		{
			if (Decode(pBinData, &m_RawDataContainer[nIndexStart + nIndex], nRow, nCol, &pos, nLens, nSubFrameIndex, nTimeStamp) && nSubFrameIndex == m_subframe_order[nNeedSubFrameIndex])
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

bool CAlp014BADVSMPAlgorithm::Decode(uint8_t* pucBinData, CDVSDataContainer* DVSData, uint32_t nRow, uint32_t nCol, size_t* pnPos, size_t nBinLens, uint8_t& nSubFrameIndex, uint64_t& nTimeStamp)
{
	size_t nCurIndex = *pnPos;
	Alp014BAFormatHeader* HeaderCode;
	while (nBinLens > nCurIndex + sizeof(Alp014AAFormatHeader))
	{
		HeaderCode = (Alp014BAFormatHeader*)(pucBinData + nCurIndex);

		if (HeaderCode->Header_vec != DVS_HEADER_014BA)
		{
			nCurIndex += 4;
		}
		else
		{
			break;
		}
	}
	if (nBinLens <= nCurIndex + sizeof(Alp014BAFormatHeader))
	{
		return false;
	}

	HeaderCode = (Alp014BAFormatHeader*)(pucBinData + nCurIndex);
	nTimeStamp = (uint64_t(HeaderCode->Timestamp_H) << 32) + HeaderCode->Timestamp_L;
	nCurIndex += sizeof(Alp014BAFormatHeader);

	Alp014BAFormatStatic* Static;

	if (nCurIndex + sizeof(Alp014BAFormatStatic) < nBinLens)
	{
		Static = (Alp014BAFormatStatic*)(pucBinData + nCurIndex);
		nCurIndex += sizeof(Alp014BAFormatStatic);
	}
	else
	{
		return false;
	}

	nSubFrameIndex = Static->Subframe;

	if ((Static->Roi_row_stop - Static->Roi_row_start + 1) > nRow || (Static->Roi_col_stop - Static->Roi_col_start + 1) > nCol)
	{
		return false;
	}

	bool bRet = true;

	if (Static->frame_mode == 0)
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

		Alp014BAFormatFooter* Footer = (Alp014BAFormatFooter*)(pucBinData + nCurIndex);

		if (Footer->Footer_vec == DVS_FOOTER_014BA && Footer->Dropflag == 0)
		{
			nCurIndex += sizeof(Alp014BAFormatFooter);
			*pnPos = nCurIndex;
			return true;
		}
	}
	return false;
}

bool CAlp014BADVSMPAlgorithm::FrameModeDecode(uint8_t* pucBinData, CDVSDataContainer* DVSData, uint32_t nRowStart, uint32_t nRowStop, uint32_t nColStart, uint32_t nColStop, size_t* pnPos, size_t nBinLens, uint8_t& nSubFrameIndex)
{
	bool bRet = false;

	uint32_t nRowStep = 16;
	uint32_t nColStep = 4;
	if (m_bHVS)
	{
		nRowStep = 8;
	}
	nRowStart *= nRowStep;
	nColStart *= nColStep;
	nRowStop *= nRowStep;
	nColStop *= nColStep;

	uint32_t nRow = nRowStart;
	uint32_t nCol = nColStart;
	size_t& nCurIndex = *pnPos;

	Alp014BAFormatFooter* Footer;
	Alp014BAFormatEventGroup* EventGroup;

	while (nCurIndex + sizeof(Alp014BAFormatFooter) < nBinLens)
	{
		Footer = (Alp014BAFormatFooter*)(pucBinData + nCurIndex);

		if (Footer->Footer_vec == DVS_FOOTER_014BA || Footer->Footer_vec == DVS_FOOTER_DROP_014BA)
		{
			return false;
		}

		for (uint32_t n = 0; n < 8; n++)
		{
			EventGroup = (Alp014BAFormatEventGroup*)(pucBinData + nCurIndex);

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

bool CAlp014BADVSMPAlgorithm::EventModeDecode(uint8_t* pucBinData, CDVSDataContainer* DVSData, uint32_t nRowStart, uint32_t nRowStop, uint32_t nColStart, uint32_t nColStop, size_t* pnPos, size_t nBinLens, uint8_t& nSubFrameIndex)
{
	bool bRet = false;

	uint32_t nRowStep = 16;
	uint32_t nColStep = 4;
	if (m_bHVS)
	{
		nRowStep = 8;
	}
	nRowStart *= nRowStep;
	nColStart *= nColStep;
	nRowStop *= nRowStep;
	nColStop *= nColStep;

	uint32_t nRow = nRowStart;
	uint32_t nCol = nColStart;
	size_t& nCurIndex = *pnPos;

	Alp014BAFormatFooter* Footer;
	Alp014BAFormatEventGroup* EventGroup;
	Alp014BAFormatVoidByte* VoidByte;

	while (nCurIndex + sizeof(Alp014BAFormatFooter) < nBinLens)
	{
		Footer = (Alp014BAFormatFooter*)(pucBinData + nCurIndex);

		if (Footer->Footer_vec == DVS_FOOTER_014BA || Footer->Footer_vec == DVS_FOOTER_DROP_014BA)
		{
			return false;
		}

		for (uint32_t n = 0; n < 8; n++)
		{
			EventGroup = (Alp014BAFormatEventGroup*)(pucBinData + nCurIndex);

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
				VoidByte = (Alp014BAFormatVoidByte*)(pucBinData + nCurIndex);

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

void CAlp014BADVSMPAlgorithm::SetData(CDVSDataContainer* DVSData, uint32_t nRow, uint32_t nCol, uint8_t nSubFrameIndex, uint8_t nEventFlag)
{
	nRow += m_row_offset_table[nSubFrameIndex];
	nCol += m_col_offset_table[nSubFrameIndex];

	DVSData->SetData(nRow, nCol, nEventFlag);
}
