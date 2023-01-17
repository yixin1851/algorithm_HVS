#pragma once
#include "AlpMPAlgoInterface.h"
#include <cstdint>
#include <vector>

#define OFF_EVENT_FLAG 1
#define ON_EVENT_FLAG 2

typedef struct
{
	uint8_t event1 : 2;
	uint8_t event2 : 2;
	uint8_t event3 : 2;
	uint8_t event4 : 2;
}EventsInOneByte;

typedef std::vector<std::vector<EventsInOneByte>> DVSType;

class CDVSDataContainer
{
public:
	CDVSDataContainer();
	void Init(uint32_t nRow, uint32_t nCol, bool bInitialize = false);
	virtual ~CDVSDataContainer();
	virtual void SetData(uint32_t nRows, uint32_t nCols, uint8_t nValue);
	virtual uint8_t GetData(uint32_t nRows, uint32_t nCols);
	virtual void CountEvents();
	uint32_t m_nRow;
	uint32_t m_nCol;
	uint32_t m_NoEventsNum[DVSSubFrameIndex::All + 1];
	uint32_t m_AllEventsNum[DVSSubFrameIndex::All + 1];
	uint32_t m_OnEventsNum[DVSSubFrameIndex::All + 1];
	uint32_t m_OffEventsNum[DVSSubFrameIndex::All + 1];
	std::vector<uint32_t> m_RowAllEventsNum;
	std::vector<uint32_t> m_ColAllEventsNum;
	uint64_t m_TimeStamp;
	uint64_t m_TriggerTime;
	DVSType m_RawData;
	const uint32_t m_nDataNumberInOneByte = 4;
};