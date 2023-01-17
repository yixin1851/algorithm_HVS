#include "DVS03BADecoder.h"
#include <thread>

#define PADDING_BLOCK_SECTION_INDEX 0x40
#define DVS_HEADER_003BA 0xF8F1F8F1F8F1F8F1
#define DVS_FOOTER_003BA 0xF4F2F4F2F4F2F4F2
#define DVS_THREAD_POOL 4


CBlockBase::CBlockBase() : m_BlockType(BlockType::PaddingBlock), m_SectionIndex(0), m_nBlockLens(0), m_pOffsetBlock(nullptr), m_pContentBlock(nullptr), m_pPaddingBlock(nullptr)
{

}

CBlockBase::CBlockBase(uint8_t* RawData, uint64_t nRawLens)
{
	m_nBlockLens = 0;
	m_SectionIndex = 0xFF;
	m_pOffsetBlock = nullptr;
	m_pContentBlock = nullptr;
	m_pPaddingBlock = nullptr;

	FrameTypeBlockDescriptor BlockDescriptor;
	if (sizeof(FrameTypeBlockDescriptor) > nRawLens)
	{
		return;
	}

	memcpy_s(&BlockDescriptor, sizeof(FrameTypeBlockDescriptor), RawData, sizeof(FrameTypeBlockDescriptor));

	if (0 == BlockDescriptor.ShortBlockDescriptor.BlockType)
	{
		m_BlockType = BlockType::OffsetBlock;
		m_pOffsetBlock = std::make_shared<COffsetBlock>(COffsetBlock(RawData, nRawLens));
		m_nBlockLens = m_pOffsetBlock->GetBlockLens();
		m_SectionIndex = m_pOffsetBlock->GetSectionIndex();
	}
	else if (0 == BlockDescriptor.LongBlockDescriptor.LON && 1 == BlockDescriptor.LongBlockDescriptor.BSW && 0 == BlockDescriptor.LongBlockDescriptor.PN)
	{
		m_BlockType = BlockType::OffsetBlock;
		m_pOffsetBlock = std::make_shared<COffsetBlock>(COffsetBlock(RawData, nRawLens));
		m_nBlockLens = m_pOffsetBlock->GetBlockLens();
		m_SectionIndex = m_pOffsetBlock->GetSectionIndex();
	}
	else if (1 == BlockDescriptor.LongBlockDescriptor.LON && 0 == BlockDescriptor.LongBlockDescriptor.BSW && 1 == BlockDescriptor.LongBlockDescriptor.PN && 0 == BlockDescriptor.LongBlockDescriptor.OT)
	{
		m_BlockType = BlockType::ContentBlock;
		m_pContentBlock = std::make_shared<CContentBlock>(CContentBlock(RawData, nRawLens));
		m_nBlockLens = m_pContentBlock->GetBlockLens();
		m_SectionIndex = m_pContentBlock->GetSectionIndex();
	}
	else if (1 == BlockDescriptor.LongBlockDescriptor.LON && 0 == BlockDescriptor.LongBlockDescriptor.BSW && 0 == BlockDescriptor.LongBlockDescriptor.PN && 0 == BlockDescriptor.LongBlockDescriptor.OT && BlockDescriptor.LongBlockDescriptor.SectionIndex == PADDING_BLOCK_SECTION_INDEX)
	{
		m_BlockType = BlockType::PaddingBlock;
		m_pPaddingBlock = std::make_shared<CPaddingBlock>(CPaddingBlock(RawData, nRawLens));
		m_nBlockLens = m_pPaddingBlock->GetBlockLens();
		m_SectionIndex = m_pPaddingBlock->GetSectionIndex();
	}
	else
	{
		return;
	}
}

BlockType CBlockBase::GetBlockType()
{
	return m_BlockType;
}

uint8_t CBlockBase::GetSectionIndex()
{
	return m_SectionIndex;
}

uint16_t CBlockBase::GetBlockLens()
{
	return m_nBlockLens;
}

CBlockBase::~CBlockBase()
{

}

COffsetBlock::COffsetBlock() :m_nBlockLens(0), m_nOffset(0), m_SectionIndex(0xFF)
{
}

COffsetBlock::COffsetBlock(uint8_t* RawData, uint64_t nRawLens)
{
	m_nBlockLens = 0;
	m_SectionIndex = 0xFF;

	FrameTypeBlockDescriptor BlockDescriptor;
	if (sizeof(FrameTypeBlockDescriptor) > nRawLens)
	{
		return;
	}

	memcpy_s(&BlockDescriptor, sizeof(FrameTypeBlockDescriptor), RawData, sizeof(FrameTypeBlockDescriptor));

	if (0 == BlockDescriptor.ShortBlockDescriptor.BlockType)
	{
		m_nOffset = BlockDescriptor.ShortBlockDescriptor.ImmediateOffset;
		m_SectionIndex = BlockDescriptor.ShortBlockDescriptor.SectionIndex;
		m_nBlockLens = sizeof(FrameTypeBlockDescriptor);
	}
	else if (0 == BlockDescriptor.LongBlockDescriptor.LON && 1 == BlockDescriptor.LongBlockDescriptor.BSW && 0 == BlockDescriptor.LongBlockDescriptor.PN)
	{
		if (sizeof(FrameTypeLongOffsetBlockBase) > nRawLens)
		{
			return;
		}

		FrameTypeLongOffsetBlockBase LongOffsetBlock;

		memcpy_s(&LongOffsetBlock, sizeof(LongOffsetBlock), RawData, sizeof(LongOffsetBlock));

		m_nOffset = LongOffsetBlock.ImmediateOffset;
		m_SectionIndex = LongOffsetBlock.BlockDescriptor.SectionIndex;
		m_nBlockLens = sizeof(FrameTypeLongOffsetBlockBase);
	}
	else
	{
		return;
	}
}

COffsetBlock::~COffsetBlock()
{
}

uint16_t COffsetBlock::GetBlockLens()
{
	return m_nBlockLens;
}

uint16_t COffsetBlock::GetOffset()
{
	return m_nOffset;
}

uint8_t COffsetBlock::GetSectionIndex()
{
	return m_SectionIndex;
}

CContentBlock::CContentBlock() :m_nBlockLens(0), m_SectionIndex(0xFF)
{
}

CContentBlock::CContentBlock(uint8_t* RawData, uint64_t nRawLens)
{
	m_nBlockLens = 0;
	m_SectionIndex = 0xFF;
	FrameTypeContentBlockBase ContentBlockBase;

	if (sizeof(FrameTypeContentBlockBase) > nRawLens)
	{
		return;
	}

	memcpy_s(&ContentBlockBase, sizeof(FrameTypeContentBlockBase), RawData, sizeof(FrameTypeContentBlockBase));

	uint16_t nSize = sizeof(FrameTypeContentBlockBase) + ContentBlockBase.DNPageLength + ContentBlockBase.UPPageLength;

	if (nSize > nRawLens)
	{
		return;
	}

	m_nBlockLens = nSize;
	m_SectionIndex = ContentBlockBase.BlockDescriptor.SectionIndex;
	m_UpCompType = ContentBlockBase.BlockDescriptor.UCT;
	m_DownCompType = ContentBlockBase.BlockDescriptor.DCT;
	m_DownPageLength = ContentBlockBase.DNPageLength;
	m_UpPageLength = ContentBlockBase.UPPageLength;

	m_DownPage.resize(m_DownPageLength);
	m_UpPage.resize(m_UpPageLength);

	uint32_t nIndex = sizeof(FrameTypeContentBlockBase);

	for (uint32_t i = 0; i < m_DownPageLength; i++)
	{
		m_DownPage[i] = RawData[nIndex++];
	}
	for (uint32_t i = 0; i < m_UpPageLength; i++)
	{
		m_UpPage[i] = RawData[nIndex++];
	}
}

CContentBlock::~CContentBlock()
{
}

uint16_t CContentBlock::GetBlockLens()
{
	return m_nBlockLens;
}

uint8_t CContentBlock::GetSectionIndex()
{
	return m_SectionIndex;
}

CPaddingBlock::CPaddingBlock() :m_nBlockLens(0), m_SectionIndex(0xFF)
{
}

CPaddingBlock::CPaddingBlock(uint8_t* RawData, uint64_t nRawLens)
{
	m_nBlockLens = 0;
	m_SectionIndex = 0xFF;
	m_nPaddingPageLength = 0;
	if (sizeof(FrameTypePaddingBlockBase) > nRawLens)
	{
		return;
	}

	FrameTypePaddingBlockBase PaddingBlock;
	memcpy_s(&PaddingBlock, sizeof(FrameTypePaddingBlockBase), RawData, sizeof(FrameTypePaddingBlockBase));

	m_SectionIndex = PaddingBlock.BlockDescriptor.SectionIndex;
	m_nPaddingPageLength = PaddingBlock.PaddingPageLength;
	m_nBlockLens = sizeof(FrameTypePaddingBlockBase) + PaddingBlock.PaddingPageLength;
}

CPaddingBlock::~CPaddingBlock()
{
}

uint16_t CPaddingBlock::GetBlockLens()
{
	return m_nBlockLens;
}

uint8_t CPaddingBlock::GetSectionIndex()
{
	return m_SectionIndex;
}

CDVS03BADecoder::CDVS03BADecoder()
{
	Init();
	m_bMultiThreadEnable = false;
}

void CDVS03BADecoder::Init()
{
	m_nTotalRow = 1224;
	m_nTotalCol = 1632;
	m_SectionBlockQueue.clear();
	m_nSubFrameIndex = 0;
	m_nTimeStamp = 0;
	m_nRowSectionNum = 1;
	m_nColSectionNum = 17;
	m_nSubFrameRow = 612;
	m_nSubFrameCow = 816;
	m_nRowBlockNumInSection = 77;
	m_nColBlockNumInSection = 6;
	m_nRowGroupNumInBlock = 4;
	m_nColGroupNumInBlock = 4;
	m_nSectionTotalNum = m_nRowSectionNum * m_nColSectionNum;
	m_nBlockTotalNumInSection = m_nRowBlockNumInSection * m_nColBlockNumInSection;
	m_nGroupTotalNumInBlock = m_nRowGroupNumInBlock * m_nColGroupNumInBlock;
	m_nGroupRow = 2;
	m_nGroupCol = 2;
	m_nFrameSize = 0;
	m_nCrc = 0;
	m_Roi = { 0, m_nSubFrameRow, 0, m_nSubFrameCow };
	m_Stats.clear();
	m_RawData = nullptr;
}

bool CDVS03BADecoder::CheckFrameHeader(uint8_t* pucBinData, size_t nBinLens, uint16_t& nHeaderLens)
{
	if (nBinLens < sizeof(FrameTypeFrameHeader))
	{
		return false;
	}

	size_t nIndex = 0;
	FrameTypeFrameHeader FrameHeader;
	memcpy_s(&FrameHeader, sizeof(FrameTypeFrameHeader), pucBinData, sizeof(FrameTypeFrameHeader));

	if (FrameHeader.HeaderCode != DVS_HEADER_003BA)
	{
		return false;
	}
	nHeaderLens = FrameHeader.HeaderStatic.HeaderSize;
	nIndex += sizeof(FrameTypeFrameHeader);
	if (FrameHeader.HeaderStatic.ST)
	{
		if ((nBinLens - nIndex) < sizeof(FrameTypeSubTime))
		{
			return false;
		}
		FrameTypeSubTime SubTime;
		memcpy_s(&SubTime, sizeof(FrameTypeSubTime), pucBinData + nIndex, sizeof(FrameTypeSubTime));
		m_nSubFrameIndex = SubTime.SubIndex;
		m_nTimeStamp = SubTime.TimeStampL + (SubTime.TimeStampH << 16);
		nIndex += sizeof(FrameTypeSubTime);
	}

	if (FrameHeader.HeaderStatic.ROI)
	{
		if ((nBinLens - nIndex) < sizeof(FrameTypeROI))
		{
			return false;
		}
		FrameTypeROI Roi;
		memcpy_s(&Roi, sizeof(FrameTypeROI), pucBinData + nIndex, sizeof(FrameTypeROI));
		m_Roi = {Roi.YStartInNumberOfGroups, Roi.YEndInNumberOfGroups, Roi.XStartInNumberOfGroups, Roi.XEndInNumberOfGroups};
		nIndex += sizeof(FrameTypeROI);
	}

	if (FrameHeader.HeaderStatic.FS)
	{
		if ((nBinLens - nIndex) < sizeof(FrameTypeFrameStatic))
		{
			return false;
		}
		FrameTypeFrameStatic FrameStatic;
		memcpy_s(&FrameStatic, sizeof(FrameTypeFrameStatic), pucBinData + nIndex, sizeof(FrameTypeFrameStatic));
		m_nRowSectionNum = FrameStatic.NumberOfSectionsInY;
		m_nColSectionNum = FrameStatic.NumberOfSectionsInX;
		m_nSubFrameRow = FrameStatic.NumberOfPixelsInY;
		m_nSubFrameCow = FrameStatic.NumberOfPixelsInX;
		m_nRowBlockNumInSection = FrameStatic.NumberOfBlocksPerSectionInY;
		m_nColBlockNumInSection = FrameStatic.NumberOfBlocksPerSectionInX;
		m_nRowGroupNumInBlock = FrameStatic.GroupsPerBlockInY;
		m_nColGroupNumInBlock = FrameStatic.GroupsPerBlockInX;
		m_nSectionTotalNum = m_nRowSectionNum * m_nColSectionNum;
		m_nBlockTotalNumInSection = m_nRowBlockNumInSection * m_nColBlockNumInSection;
		m_nGroupTotalNumInBlock = m_nRowGroupNumInBlock * m_nColGroupNumInBlock;
		m_nGroupRow = FrameStatic.GroupHeight;
		m_nGroupCol = FrameStatic.GroupWidth;
		nIndex += sizeof(FrameTypeFrameStatic);
	}

	if (nIndex == nHeaderLens)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool CDVS03BADecoder::CheckFrameFooter(uint8_t* pucBinData, size_t nBinLens, uint16_t& nFooterLens)
{
	if (nBinLens < sizeof(FrameTypeFrameFooter))
	{
		return false;
	}
	m_nFrameSize = 0;
	size_t nIndex = 0;
	FrameTypeFrameFooter FrameFooter;
	memcpy_s(&FrameFooter, sizeof(FrameFooter), pucBinData, sizeof(FrameFooter));

	if (FrameFooter.FooterCode != DVS_FOOTER_003BA)
	{
		return false;
	}
	nFooterLens = FrameFooter.FooterStatic.FooterSize; 
	nIndex += sizeof(FrameTypeFrameFooter);
	if (FrameFooter.FooterStatic.ST)
	{
		m_Stats.resize(m_nSectionTotalNum);

		for (uint32_t i = 0; i < m_nSectionTotalNum; i++)
		{
			if ((nBinLens - nIndex) < sizeof(FrameTypeStats))
			{
				return false;
			}

			FrameTypeStats Stats;
			memcpy_s(&Stats, sizeof(FrameTypeStats), pucBinData + nIndex, sizeof(FrameTypeStats));
			m_Stats[i] = Stats;
			nIndex += sizeof(FrameTypeStats);
		}
	}
	if (FrameFooter.FooterStatic.CSZ)
	{
		if ((nBinLens - nIndex) < sizeof(FrameTypeByteCount))
		{
			return false;
		}
		FrameTypeByteCount ByteCount;
		memcpy_s(&ByteCount, sizeof(FrameTypeByteCount), pucBinData + nIndex, sizeof(FrameTypeByteCount));
		m_nFrameSize = ByteCount.ByteCount;
		nIndex += sizeof(FrameTypeByteCount);
	}

	if (FrameFooter.FooterStatic.CH)
	{
		if ((nBinLens - nIndex) < sizeof(FrameTypeCrc))
		{
			return false;
		}
		FrameTypeCrc Crc;
		memcpy_s(&Crc, sizeof(FrameTypeCrc), pucBinData + nIndex, sizeof(FrameTypeCrc));
		m_nCrc = Crc.Crc;
		nIndex += sizeof(FrameTypeCrc);

	}
	if (nIndex == nFooterLens)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool CDVS03BADecoder::CheckBlock(uint8_t* pucBinData, size_t nBinLens, uint16_t& nBlockLens, bool &bPaddingFlag)
{
	if (nBinLens < sizeof(FrameTypeBlockDescriptor))
	{
		return false;
	}
	CBlockBase BlockBase(pucBinData, nBinLens);

	nBlockLens = BlockBase.GetBlockLens();
	uint16_t nSectionIndex = BlockBase.GetSectionIndex();
	BlockType Type = BlockBase.GetBlockType();

	if (nBlockLens == 0)
	{
		return false;
	}

	if (Type == BlockType::PaddingBlock)
	{
		bPaddingFlag = true;
	}
	else
	{
		if (nSectionIndex >= m_nSectionTotalNum)
		{
			return false;
		}

		m_SectionBlockQueue[nSectionIndex].push_back(BlockBase);
	}

	return true;
}

Local CDVS03BADecoder::LocalSwitch(uint8_t nSectionIndex, uint16_t nBlockIndex, uint16_t nGroupIndex, uint8_t nEventIndex, uint8_t nSubFrameIndex)
{
	Local res;

	res.x = ((nSectionIndex / m_nColSectionNum * m_nRowBlockNumInSection + nBlockIndex / m_nColBlockNumInSection) * m_nRowGroupNumInBlock + nGroupIndex / m_nColGroupNumInBlock) * m_nGroupRow + nEventIndex / m_nGroupCol;
	res.y = ((nSectionIndex % m_nColSectionNum * m_nColBlockNumInSection + nBlockIndex % m_nColBlockNumInSection) * m_nColGroupNumInBlock + nGroupIndex % m_nColGroupNumInBlock) * m_nGroupCol + nEventIndex % m_nGroupCol;

	res.x = res.x * 2 + nSubFrameIndex / 2;
	res.y = res.y * 2 + nSubFrameIndex % 2;

	return res;
}

void CDVS03BADecoder::SectionProcess(uint8_t nSectionStart, uint8_t nSectionEnd)
{
	for (uint8_t nCurSection = nSectionStart; nCurSection < nSectionEnd; nCurSection++)
	{
		uint16_t nCurBlock = 0;

		for (uint32_t nIndex = 0; nIndex < m_SectionBlockQueue[nCurSection].size(); nIndex++)
		{
			CBlockBase& Block = m_SectionBlockQueue[nCurSection][nIndex];
			if (Block.m_BlockType == BlockType::OffsetBlock)
			{
				auto OffsetBlock = Block.m_pOffsetBlock;
				nCurBlock += OffsetBlock->GetOffset() + 1;
			}
			else
			{
				auto ContentBlock = Block.m_pContentBlock;
				if (0 != ContentBlock->m_DownPageLength)
				{
					if (ContentBlock->m_DownCompType == GroupCompType::packed)
					{
						uint8_t uGroupIndex = 0;
						FrameTypePackedGroup PackedGroup;
						Local l;
						for (uint32_t nContentIndex = 0; nContentIndex < ContentBlock->m_DownPageLength; nContentIndex++)
						{
							memcpy_s(&PackedGroup, sizeof(FrameTypePackedGroup), &ContentBlock->m_DownPage[nContentIndex], sizeof(FrameTypePackedGroup));

							if (PackedGroup.G0E0)
							{
								 l = LocalSwitch(nCurSection, nCurBlock, uGroupIndex, 0, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, OFF_EVENT_FLAG);
							}
							if (PackedGroup.G0E1)
							{
								 l = LocalSwitch(nCurSection, nCurBlock, uGroupIndex, 1, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, OFF_EVENT_FLAG);
							}
							if (PackedGroup.G0E2)
							{
								 l = LocalSwitch(nCurSection, nCurBlock, uGroupIndex, 2, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, OFF_EVENT_FLAG);
							}
							if (PackedGroup.G0E3)
							{
								 l = LocalSwitch(nCurSection, nCurBlock, uGroupIndex, 3, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, OFF_EVENT_FLAG);
							}
							if (PackedGroup.G1E0)
							{
								 l = LocalSwitch(nCurSection, nCurBlock, uGroupIndex + 1, 0, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, OFF_EVENT_FLAG);
							}
							if (PackedGroup.G1E1)
							{
								 l = LocalSwitch(nCurSection, nCurBlock, uGroupIndex + 1, 1, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, OFF_EVENT_FLAG);
							}
							if (PackedGroup.G1E2)
							{
								 l = LocalSwitch(nCurSection, nCurBlock, uGroupIndex + 1, 2, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, OFF_EVENT_FLAG);
							}
							if (PackedGroup.G1E3)
							{
								 l = LocalSwitch(nCurSection, nCurBlock, uGroupIndex + 1, 3, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, OFF_EVENT_FLAG);
							}
							uGroupIndex += 2;
						}
					}
					else
					{
						FrameTypeOffsetGroup OffsetGroup;
						Local l;
						for (uint32_t nContentIndex = 0; nContentIndex < ContentBlock->m_DownPageLength; nContentIndex++)
						{
							memcpy_s(&OffsetGroup, sizeof(FrameTypeOffsetGroup), &ContentBlock->m_DownPage[nContentIndex], sizeof(FrameTypeOffsetGroup));
							if (OffsetGroup.E0)
							{
								l = LocalSwitch(nCurSection, nCurBlock, OffsetGroup.GroupIndex, 0, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, OFF_EVENT_FLAG);
							}
							if (OffsetGroup.E1)
							{
								l = LocalSwitch(nCurSection, nCurBlock, OffsetGroup.GroupIndex, 1, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, OFF_EVENT_FLAG);
							}
							if (OffsetGroup.E0)
							{
								l = LocalSwitch(nCurSection, nCurBlock, OffsetGroup.GroupIndex, 2, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, OFF_EVENT_FLAG);
							}
							if (OffsetGroup.E3)
							{
								l = LocalSwitch(nCurSection, nCurBlock, OffsetGroup.GroupIndex, 3, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, OFF_EVENT_FLAG);
							}
						}
					}
				}
				if (0 != ContentBlock->m_UpPageLength)
				{
					if (ContentBlock->m_UpCompType == GroupCompType::packed)
					{
						uint8_t uGroupIndex = 0;
						FrameTypePackedGroup PackedGroup;
						Local l;
						for (uint32_t nContentIndex = 0; nContentIndex < ContentBlock->m_UpPageLength; nContentIndex++)
						{
							memcpy_s(&PackedGroup, sizeof(FrameTypePackedGroup), &ContentBlock->m_UpPage[nContentIndex], sizeof(FrameTypePackedGroup));

							if (PackedGroup.G0E0)
							{
								l = LocalSwitch(nCurSection, nCurBlock, uGroupIndex, 0, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, ON_EVENT_FLAG);
							}
							if (PackedGroup.G0E1)
							{
								l = LocalSwitch(nCurSection, nCurBlock, uGroupIndex, 1, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, ON_EVENT_FLAG);
							}
							if (PackedGroup.G0E2)
							{
								l = LocalSwitch(nCurSection, nCurBlock, uGroupIndex, 2, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, ON_EVENT_FLAG);
							}
							if (PackedGroup.G0E3)
							{
								l = LocalSwitch(nCurSection, nCurBlock, uGroupIndex, 3, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, ON_EVENT_FLAG);
							}
							if (PackedGroup.G1E0)
							{
								l = LocalSwitch(nCurSection, nCurBlock, uGroupIndex + 1, 0, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, ON_EVENT_FLAG);
							}
							if (PackedGroup.G1E1)
							{
								l = LocalSwitch(nCurSection, nCurBlock, uGroupIndex + 1, 1, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, ON_EVENT_FLAG);
							}
							if (PackedGroup.G1E2)
							{
								l = LocalSwitch(nCurSection, nCurBlock, uGroupIndex + 1, 2, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, ON_EVENT_FLAG);
							}
							if (PackedGroup.G1E3)
							{
								l = LocalSwitch(nCurSection, nCurBlock, uGroupIndex + 1, 3, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, ON_EVENT_FLAG);
							}
							uGroupIndex += 2;
						}
					}
					else
					{
						FrameTypeOffsetGroup OffsetGroup;
						Local l;
						for (uint32_t nContentIndex = 0; nContentIndex < ContentBlock->m_UpPageLength; nContentIndex++)
						{
							memcpy_s(&OffsetGroup, sizeof(FrameTypeOffsetGroup), &ContentBlock->m_UpPage[nContentIndex], sizeof(FrameTypeOffsetGroup));
							if (OffsetGroup.E0)
							{
								l = LocalSwitch(nCurSection, nCurBlock, OffsetGroup.GroupIndex, 0, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, ON_EVENT_FLAG);
							}
							if (OffsetGroup.E1)
							{
								l = LocalSwitch(nCurSection, nCurBlock, OffsetGroup.GroupIndex, 1, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, ON_EVENT_FLAG);
							}
							if (OffsetGroup.E0)
							{
								l = LocalSwitch(nCurSection, nCurBlock, OffsetGroup.GroupIndex, 2, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, ON_EVENT_FLAG);
							}
							if (OffsetGroup.E3)
							{
								l = LocalSwitch(nCurSection, nCurBlock, OffsetGroup.GroupIndex, 3, m_nSubFrameIndex);
								m_RawData->SetData(l.x, l.y, ON_EVENT_FLAG);
							}
						}
					}
				}
				++nCurBlock;
			}
		}
	}
}

bool CDVS03BADecoder::DVS_Decode(uint8_t* pucBinData, CDVSDataContainer* DVSData, size_t nRow, size_t nCol, size_t* pnPos, size_t nBinLens, uint8_t& nSubFrameIndex, uint64_t & ntimeStamp)
{
	Init();
	m_nTotalRow = nRow;
	m_nTotalCol = nCol;
	m_RawData = DVSData;
	size_t nCurIndex = *pnPos;
	uint16_t nHeaderLens = 0;
	uint64_t HeaderCode = 0;
	while (nBinLens - nCurIndex > sizeof(HeaderCode))
	{
		memcpy_s(&HeaderCode, sizeof(HeaderCode), pucBinData + nCurIndex, sizeof(HeaderCode));

		if (HeaderCode != DVS_HEADER_003BA) 
		{
			nCurIndex += sizeof(DVS_HEADER_003BA);
		}
		else
		{
			break;
		}
	}
	size_t nFrameStart = nCurIndex;
	if (nBinLens <= nCurIndex || !CheckFrameHeader(pucBinData + nCurIndex, nBinLens - nCurIndex, nHeaderLens))
	{
		return false;
	}
	nCurIndex += nHeaderLens;
	m_SectionBlockQueue.resize(m_nSectionTotalNum);
	uint16_t nFooterLens = 0;
	uint16_t nBlockLens = 0;
	bool bPadding = false;
	bool bFindFooter = false;
	while (nBinLens > nCurIndex)
	{
		if (CheckFrameFooter(pucBinData + nCurIndex, nBinLens - nCurIndex, nFooterLens))
		{
			nCurIndex += nFooterLens;
			bFindFooter = true;
			if (m_nFrameSize != 0)
			{
				if (m_nFrameSize != (nCurIndex - nFrameStart))
				{
					return false;
				}
			}
			break;
		}
		else
		{
			if (bPadding)
			{
				return false;
			}
		}

		if (!CheckBlock(pucBinData + nCurIndex, nBinLens - nCurIndex, nBlockLens, bPadding))
		{
			return false;
		}
		else
		{
			nCurIndex += nBlockLens;
		}
	}

	if (!bFindFooter)
	{
		return false;
	}

	if (m_bMultiThreadEnable)
	{
		uint32_t nThreadPool = m_nSectionTotalNum < DVS_THREAD_POOL ? m_nSectionTotalNum : DVS_THREAD_POOL;

		uint32_t nSectionStep = m_nSectionTotalNum / nThreadPool;

		std::thread* t[DVS_THREAD_POOL] = { 0 };

		for (uint32_t i = 0; i < nThreadPool; i++)
		{
			if (i == nThreadPool - 1)
			{
				t[i] = new std::thread(&CDVS03BADecoder::SectionProcess, this, i * nSectionStep, m_nSectionTotalNum);
			}
			else
			{
				t[i] = new std::thread(&CDVS03BADecoder::SectionProcess, this, i * nSectionStep, (i + 1) * nSectionStep);
			}
		}
		for (uint32_t i = 0; i < nThreadPool; i++)
		{
			t[i]->join();
			delete t[i];
		}
	}
	else
	{
		SectionProcess(0, m_nSectionTotalNum);
	}
	nSubFrameIndex = m_nSubFrameIndex;
	ntimeStamp = m_nTimeStamp;
	*pnPos = nCurIndex;
	return true;
}

