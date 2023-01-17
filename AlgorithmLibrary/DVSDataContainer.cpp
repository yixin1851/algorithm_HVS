#include "DVSDataContainer.h"

CDVSDataContainer::CDVSDataContainer()
{
	m_nRow = 0;
	m_nCol = 0;
	for (uint32_t nIndex = 0; nIndex <= DVSSubFrameIndex::All; nIndex++)
	{
		m_NoEventsNum[nIndex] = 0;
		m_AllEventsNum[nIndex] = 0;
		m_OnEventsNum[nIndex] = 0;
		m_OffEventsNum[nIndex] = 0;
	}
	m_TimeStamp = 0;
	m_TriggerTime = 0;
	m_RawData.clear();
}

void CDVSDataContainer::Init(uint32_t nRow, uint32_t nCol, bool bInitialize)
{
	m_RawData.resize(nRow);
	uint32_t nSizeOneRow = nCol / m_nDataNumberInOneByte;
	if (nCol % m_nDataNumberInOneByte)
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
	for (uint32_t nIndex = 0; nIndex <= DVSSubFrameIndex::All; nIndex++)
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

	switch (nCols % m_nDataNumberInOneByte)
	{
	case 0:
		m_RawData[nRows][nCols / m_nDataNumberInOneByte].event1 = nValue;
		break;
	case 1:
		m_RawData[nRows][nCols / m_nDataNumberInOneByte].event2 = nValue;
		break;
	case 2:
		m_RawData[nRows][nCols / m_nDataNumberInOneByte].event3 = nValue;
		break;
	case 3:
		m_RawData[nRows][nCols / m_nDataNumberInOneByte].event4 = nValue;
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

	switch (nCols % m_nDataNumberInOneByte)
	{
	case 0:
		nValue = m_RawData[nRows][nCols / m_nDataNumberInOneByte].event1;
		break;
	case 1:
		nValue = m_RawData[nRows][nCols / m_nDataNumberInOneByte].event2;
		break;
	case 2:
		nValue = m_RawData[nRows][nCols / m_nDataNumberInOneByte].event3;
		break;
	case 3:
		nValue = m_RawData[nRows][nCols / m_nDataNumberInOneByte].event4;
		break;
	}
	return nValue;
}

void CDVSDataContainer::CountEvents()
{
	for (uint32_t nIndex = 0; nIndex <= DVSSubFrameIndex::All; nIndex++)
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

	for (uint32_t nRows = 0; nRows < m_nRow; nRows++)
	{
		for (uint32_t nCols = 0; nCols < m_nCol; nCols++)
		{
			uint8_t nValue = 0;
			switch (nCols % m_nDataNumberInOneByte)
			{
			case 0:
				nValue = m_RawData[nRows][nCols / m_nDataNumberInOneByte].event1;
				break;
			case 1:
				nValue = m_RawData[nRows][nCols / m_nDataNumberInOneByte].event2;
				break;
			case 2:
				nValue = m_RawData[nRows][nCols / m_nDataNumberInOneByte].event3;
				break;
			case 3:
				nValue = m_RawData[nRows][nCols / m_nDataNumberInOneByte].event4;
				break;
			}

			uint32_t nChannel = 0;
			if (0 == (nRows % 2) && 0 == (nCols % 2))
			{
				nChannel = DVSSubFrameIndex::Gb;
			}
			else if (0 == (nRows % 2) && 1 == (nCols % 2))
			{
				nChannel = DVSSubFrameIndex::B;
			}
			else if (1 == (nRows % 2) && 0 == (nCols % 2))
			{
				nChannel = DVSSubFrameIndex::R;
			}
			else if (1 == (nRows % 2) && 1 == (nCols % 2))
			{
				nChannel = DVSSubFrameIndex::Gr;
			}

			if (0 == nValue)
			{
				++m_NoEventsNum[DVSSubFrameIndex::All];
				++m_NoEventsNum[nChannel];
			}
			else if (ON_EVENT_FLAG == nValue)
			{
				++m_OnEventsNum[DVSSubFrameIndex::All];
				++m_AllEventsNum[DVSSubFrameIndex::All];
				++m_OnEventsNum[nChannel];
				++m_AllEventsNum[nChannel];
				++m_RowAllEventsNum[nRows];
				++m_ColAllEventsNum[nCols];
			}
			else if (OFF_EVENT_FLAG == nValue)
			{
				++m_OffEventsNum[DVSSubFrameIndex::All];
				++m_AllEventsNum[DVSSubFrameIndex::All];
				++m_OffEventsNum[nChannel];
				++m_AllEventsNum[nChannel];
				++m_RowAllEventsNum[nRows];
				++m_ColAllEventsNum[nCols];
			}
		}
	}
}

