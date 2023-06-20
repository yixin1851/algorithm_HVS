#include "DVSDataContainer.h"

CDVSDataContainer::CDVSDataContainer()
{
	m_nRow = 0;
	m_nCol = 0;
	for (uint32_t nIndex = 0; nIndex <= SubFrameIndex::All; nIndex++)
	{
		m_NoEventsNum[nIndex] = 0;
		m_AllEventsNum[nIndex] = 0;
		m_OnEventsNum[nIndex] = 0;
		m_OffEventsNum[nIndex] = 0;
	}
	m_TimeStamp = 0;
	m_TriggerTime = 0;
	m_RawData.clear();
	m_PixelFormat = BayerGBRG;
}

void CDVSDataContainer::Init(uint32_t nRow, uint32_t nCol, bool bInitialize, PixelFormatType PixelFormat)
{
	m_RawData.resize(nRow);
	uint32_t nSizeOneRow = nCol >> 2;
	if (nCol & 3)
	{
		++nSizeOneRow;
	}
	for (uint32_t rows = 0; rows < nRow; rows++)
	{
		m_RawData[rows].resize(nSizeOneRow);
		if (bInitialize)
		{
			memset(&m_RawData[rows][0], 0, sizeof(m_RawData[rows][0]) * nSizeOneRow);
		}
	}
	m_nRow = nRow;
	m_nCol = nCol;
	for (uint32_t nIndex = 0; nIndex <= SubFrameIndex::All; nIndex++)
	{
		m_NoEventsNum[nIndex] = 0;
		m_AllEventsNum[nIndex] = 0;
		m_OnEventsNum[nIndex] = 0;
		m_OffEventsNum[nIndex] = 0;
	}
	m_TimeStamp = 0;
	m_TriggerTime = 0;
	m_RowAllEventsNum.resize(m_nRow);
	m_ColAllEventsNum.resize(m_nCol);
	m_PixelFormat = PixelFormat;
}

CDVSDataContainer::~CDVSDataContainer()
{
}

void CDVSDataContainer::SetData(uint32_t nRows, uint32_t nCols, uint8_t nValue)
{
	if (nRows >= m_nRow || nCols >= m_nCol)
	{
		return;
	}

	switch (nCols & 3)
	{
	case 0:
		m_RawData[nRows][nCols >> 2].event1 = nValue;
		break;
	case 1:
		m_RawData[nRows][nCols >> 2].event2 = nValue;
		break;
	case 2:
		m_RawData[nRows][nCols >> 2].event3 = nValue;
		break;
	case 3:
		m_RawData[nRows][nCols >> 2].event4 = nValue;
		break;
	}
}

uint8_t CDVSDataContainer::GetData(uint32_t nRows, uint32_t nCols)
{
	uint8_t nValue = 255;
	if (nRows >= m_nRow || nCols >= m_nCol)
	{
		return nValue;
	}

	switch (nCols & 3)
	{
	case 0:
		nValue = m_RawData[nRows][nCols >> 2].event1;
		break;
	case 1:
		nValue = m_RawData[nRows][nCols >> 2].event2;
		break;
	case 2:
		nValue = m_RawData[nRows][nCols >> 2].event3;
		break;
	case 3:
		nValue = m_RawData[nRows][nCols >> 2].event4;
		break;
	}
	return nValue;
}

void CDVSDataContainer::CountEvents(ROIArea& Roi)
{
	for (uint32_t nIndex = 0; nIndex <= SubFrameIndex::All; nIndex++)
	{
		m_NoEventsNum[nIndex] = 0;
		m_AllEventsNum[nIndex] = 0;
		m_OnEventsNum[nIndex] = 0;
		m_OffEventsNum[nIndex] = 0;
	}
	for (uint32_t nRows = 0; nRows < m_nRow; nRows++)
	{
		m_RowAllEventsNum[nRows] = 0;
	}
	for (uint32_t nCols = 0; nCols < m_nCol; nCols++)
	{
		m_ColAllEventsNum[nCols] = 0;
	}

	for (uint32_t nRows = Roi.Up; nRows <= Roi.Down; nRows++)
	{
		for (uint32_t nCols = Roi.Left; nCols <= Roi.Right; nCols++)
		{
			uint8_t nValue = 0;
			switch (nCols & 3)
			{
			case 0:
				nValue = m_RawData[nRows][nCols >> 2].event1;
				break;
			case 1:
				nValue = m_RawData[nRows][nCols >> 2].event2;
				break;
			case 2:
				nValue = m_RawData[nRows][nCols >> 2].event3;
				break;
			case 3:
				nValue = m_RawData[nRows][nCols >> 2].event4;
				break;
			}

			SubFrameIndex nChannel = All;
			GetChannel(nRows, nCols, nChannel);

			if (0 == nValue)
			{
				++m_NoEventsNum[SubFrameIndex::All];
				++m_NoEventsNum[nChannel];
			}
			else if (ON_EVENT_FLAG == nValue)
			{
				++m_OnEventsNum[SubFrameIndex::All];
				++m_AllEventsNum[SubFrameIndex::All];
				++m_OnEventsNum[nChannel];
				++m_AllEventsNum[nChannel];
				++m_RowAllEventsNum[nRows];
				++m_ColAllEventsNum[nCols];
			}
			else if (OFF_EVENT_FLAG == nValue)
			{
				++m_OffEventsNum[SubFrameIndex::All];
				++m_AllEventsNum[SubFrameIndex::All];
				++m_OffEventsNum[nChannel];
				++m_AllEventsNum[nChannel];
				++m_RowAllEventsNum[nRows];
				++m_ColAllEventsNum[nCols];
			}
		}
	}
}

void CDVSDataContainer::GetChannel(uint32_t nRows, uint32_t nCols, SubFrameIndex& nChannel)
{
	switch (m_PixelFormat)
	{
	case BayerGBRG:
		if (0 == (nRows & 1) && 0 == (nCols & 1))
		{
			nChannel = SubFrameIndex::Gb;
		}
		else if (0 == (nRows & 1) && 1 == (nCols & 1))
		{
			nChannel = SubFrameIndex::B;
		}
		else if (1 == (nRows & 1) && 0 == (nCols & 1))
		{
			nChannel = SubFrameIndex::R;
		}
		else if (1 == (nRows & 1) && 1 == (nCols & 1))
		{
			nChannel = SubFrameIndex::Gr;
		}
		break;
	case BayerBGGR:
		if (0 == (nRows & 1) && 0 == (nCols & 1))
		{
			nChannel = SubFrameIndex::B;
		}
		else if (0 == (nRows & 1) && 1 == (nCols & 1))
		{
			nChannel = SubFrameIndex::Gb;
		}
		else if (1 == (nRows & 1) && 0 == (nCols & 1))
		{
			nChannel = SubFrameIndex::Gr;
		}
		else if (1 == (nRows & 1) && 1 == (nCols & 1))
		{
			nChannel = SubFrameIndex::R;
		}
		break;
	case BayerRGGB:
		if (0 == (nRows & 1) && 0 == (nCols & 1))
		{
			nChannel = SubFrameIndex::R;
		}
		else if (0 == (nRows & 1) && 1 == (nCols & 1))
		{
			nChannel = SubFrameIndex::Gr;
		}
		else if (1 == (nRows & 1) && 0 == (nCols & 1))
		{
			nChannel = SubFrameIndex::Gb;
		}
		else if (1 == (nRows & 1) && 1 == (nCols & 1))
		{
			nChannel = SubFrameIndex::B;
		}
		break;
	case BayerGRBG:
		if (0 == (nRows & 1) && 0 == (nCols & 1))
		{
			nChannel = SubFrameIndex::Gr;
		}
		else if (0 == (nRows & 1) && 1 == (nCols & 1))
		{
			nChannel = SubFrameIndex::R;
		}
		else if (1 == (nRows & 1) && 0 == (nCols & 1))
		{
			nChannel = SubFrameIndex::B;
		}
		else if (1 == (nRows & 1) && 1 == (nCols & 1))
		{
			nChannel = SubFrameIndex::Gb;
		}
		break;
	}
}