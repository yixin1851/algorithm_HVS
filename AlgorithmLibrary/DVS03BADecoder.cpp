#include "DVS03BADecoder.h"
#include <thread>
#include <iostream>

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

	if (sizeof(FrameTypeBlockDescriptor) > nRawLens)
	{
		return;
	}

	FrameTypeBlockDescriptor *BlockDescriptor = (FrameTypeBlockDescriptor*)RawData;
	BlockDescriptor = (FrameTypeBlockDescriptor *)RawData;

	if (0 == BlockDescriptor->ShortBlockDescriptor.BlockType)
	{
		m_BlockType = BlockType::OffsetBlock;
		m_pOffsetBlock = new COffsetBlock(RawData, nRawLens);
		m_nBlockLens = m_pOffsetBlock->GetBlockLens();
		m_SectionIndex = m_pOffsetBlock->GetSectionIndex();
	}
	else if (0 == BlockDescriptor->LongBlockDescriptor.LON && 1 == BlockDescriptor->LongBlockDescriptor.BSW && 0 == BlockDescriptor->LongBlockDescriptor.PN)
	{
		m_BlockType = BlockType::OffsetBlock;
		m_pOffsetBlock = new COffsetBlock(RawData, nRawLens);
		m_nBlockLens = m_pOffsetBlock->GetBlockLens();
		m_SectionIndex = m_pOffsetBlock->GetSectionIndex();
	}
	else if (1 == BlockDescriptor->LongBlockDescriptor.LON && 0 == BlockDescriptor->LongBlockDescriptor.BSW && 1 == BlockDescriptor->LongBlockDescriptor.PN && 0 == BlockDescriptor->LongBlockDescriptor.OT)
	{
		m_BlockType = BlockType::ContentBlock;
		m_pContentBlock = new CContentBlock(RawData, nRawLens);
		m_nBlockLens = m_pContentBlock->GetBlockLens();
		m_SectionIndex = m_pContentBlock->GetSectionIndex();
	}
	else if (1 == BlockDescriptor->LongBlockDescriptor.LON && 0 == BlockDescriptor->LongBlockDescriptor.BSW && 0 == BlockDescriptor->LongBlockDescriptor.PN && 0 == BlockDescriptor->LongBlockDescriptor.OT && BlockDescriptor->LongBlockDescriptor.SectionIndex == PADDING_BLOCK_SECTION_INDEX)
	{
		m_BlockType = BlockType::PaddingBlock;
		m_pPaddingBlock = new CPaddingBlock(RawData, nRawLens);
		m_nBlockLens = m_pPaddingBlock->GetBlockLens();
		m_SectionIndex = m_pPaddingBlock->GetSectionIndex();
	}
	else
	{
		return;
	}

	/*temporary method for data loss*/
	//if (BlockDescriptor.LongBlockDescriptor.SectionIndex != PADDING_BLOCK_SECTION_INDEX)
	//{
	//	m_BlockType = BlockType::ContentBlock;
	//	m_pContentBlock = std::make_shared<CContentBlock>(CContentBlock(RawData, nRawLens));
	//	m_nBlockLens = m_pContentBlock->GetBlockLens();
	//	m_SectionIndex = m_pContentBlock->GetSectionIndex();
	//}
	//else if (BlockDescriptor.LongBlockDescriptor.SectionIndex == PADDING_BLOCK_SECTION_INDEX)
	//{
	//	m_BlockType = BlockType::PaddingBlock;
	//	m_pPaddingBlock = std::make_shared<CPaddingBlock>(CPaddingBlock(RawData, nRawLens));
	//	m_nBlockLens = m_pPaddingBlock->GetBlockLens();
	//	m_SectionIndex = m_pPaddingBlock->GetSectionIndex();
	//}
	//else
	//{
	//	return;
	//}
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
	if (m_pContentBlock)
	{
		delete m_pContentBlock;
	}
	else if (m_pPaddingBlock)
	{
		delete m_pPaddingBlock;
	}
	else if (m_pOffsetBlock)
	{
		delete m_pOffsetBlock;
	}
}

COffsetBlock::COffsetBlock() :m_nBlockLens(0), m_nOffset(0), m_SectionIndex(0xFF)
{
}

COffsetBlock::COffsetBlock(uint8_t* RawData, uint64_t nRawLens)
{
	m_nBlockLens = 0;
	m_SectionIndex = 0xFF;

	if (sizeof(FrameTypeBlockDescriptor) > nRawLens)
	{
		return;
	}

	FrameTypeBlockDescriptor *BlockDescriptor = (FrameTypeBlockDescriptor*)RawData;

	if (0 == BlockDescriptor->ShortBlockDescriptor.BlockType)
	{
		m_nOffset = BlockDescriptor->ShortBlockDescriptor.ImmediateOffset;
		m_SectionIndex = BlockDescriptor->ShortBlockDescriptor.SectionIndex;
		m_nBlockLens = sizeof(FrameTypeBlockDescriptor);
	}
	else if (0 == BlockDescriptor->LongBlockDescriptor.LON && 1 == BlockDescriptor->LongBlockDescriptor.BSW && 0 == BlockDescriptor->LongBlockDescriptor.PN)
	{
		if (sizeof(FrameTypeLongOffsetBlockBase) > nRawLens)
		{
			return;
		}

		FrameTypeLongOffsetBlockBase *LongOffsetBlock = (FrameTypeLongOffsetBlockBase * )RawData;

		m_nOffset = LongOffsetBlock->ImmediateOffset;
		m_SectionIndex = LongOffsetBlock->BlockDescriptor.SectionIndex;
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

	if (sizeof(FrameTypeContentBlockBase) > nRawLens)
	{
		return;
	}

	FrameTypeContentBlockBase *ContentBlockBase = (FrameTypeContentBlockBase * )RawData;

	/*temporary method for data loss*/
	//if (ContentBlockBase.DNPageLength == 8 || ContentBlockBase.UPPageLength == 8)
	//{
	//	ContentBlockBase.DNPageLength = 8;
	//	ContentBlockBase.UPPageLength = 8;
	//}

	uint16_t nSize = sizeof(FrameTypeContentBlockBase) + ContentBlockBase->DNPageLength + ContentBlockBase->UPPageLength;

	if (nSize > nRawLens)
	{
		return;
	}

	m_nBlockLens = nSize;
	m_SectionIndex = ContentBlockBase->BlockDescriptor.SectionIndex;
	m_UpCompType = ContentBlockBase->BlockDescriptor.UCT;
	m_DownCompType = ContentBlockBase->BlockDescriptor.DCT;
	m_DownPageLength = ContentBlockBase->DNPageLength;
	m_UpPageLength = ContentBlockBase->UPPageLength;

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

	FrameTypePaddingBlockBase *PaddingBlock = (FrameTypePaddingBlockBase *)RawData;

	m_SectionIndex = PaddingBlock->BlockDescriptor.SectionIndex;
	m_nPaddingPageLength = PaddingBlock->PaddingPageLength;
	m_nBlockLens = sizeof(FrameTypePaddingBlockBase) + PaddingBlock->PaddingPageLength;
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
	m_bCheckSimpleFooter = false;
}

void CDVS03BADecoder::Init()
{
	m_nTotalRow = 1224;
	m_nTotalCol = 1632;
	//m_SectionBlockQueue.clear();
	m_nSubFrameIndex = 0;
	m_nTimeStamp = 0;
	m_nRowSectionNum = 1;
	m_nColSectionNum = 17;
	m_nSubFrameRow = 612;
	m_nSubFrameCol = 816;
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
	m_Roi = { 0, m_nSubFrameRow, 0, m_nSubFrameCol };
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
	FrameTypeFrameHeader *FrameHeader = (FrameTypeFrameHeader *)pucBinData;

	if (FrameHeader->HeaderCode != DVS_HEADER_003BA)
	{
		return false;
	}
	nHeaderLens = FrameHeader->HeaderStatic.HeaderSize;
	nIndex += sizeof(FrameTypeFrameHeader);
	if (FrameHeader->HeaderStatic.ST)
	{
		if ((nBinLens - nIndex) < sizeof(FrameTypeSubTime))
		{
			return false;
		}
		FrameTypeSubTime *SubTime = (FrameTypeSubTime*)(pucBinData + nIndex);
		m_nSubFrameIndex = SubTime->SubIndex;
		m_nTimeStamp = SubTime->TimeStampL + (SubTime->TimeStampH << 16);
		nIndex += sizeof(FrameTypeSubTime);
	}

	if (FrameHeader->HeaderStatic.ROI)
	{
		if ((nBinLens - nIndex) < sizeof(FrameTypeROI))
		{
			return false;
		}
		FrameTypeROI *Roi = (FrameTypeROI *)(pucBinData + nIndex);
		m_Roi = {Roi->YStartInNumberOfGroups, Roi->YEndInNumberOfGroups, Roi->XStartInNumberOfGroups, Roi->XEndInNumberOfGroups};
		nIndex += sizeof(FrameTypeROI);
	}

	if (FrameHeader->HeaderStatic.FS)
	{
		if ((nBinLens - nIndex) < sizeof(FrameTypeFrameStatic))
		{
			return false;
		}
		FrameTypeFrameStatic *FrameStatic = (FrameTypeFrameStatic*)(pucBinData + nIndex);
		m_nRowSectionNum = FrameStatic->NumberOfSectionsInY;
		m_nColSectionNum = FrameStatic->NumberOfSectionsInX;
		m_nSubFrameRow = FrameStatic->NumberOfPixelsInY;
		m_nSubFrameCol = FrameStatic->NumberOfPixelsInX;
		m_nRowBlockNumInSection = FrameStatic->NumberOfBlocksPerSectionInY;
		m_nColBlockNumInSection = FrameStatic->NumberOfBlocksPerSectionInX;
		m_nRowGroupNumInBlock = FrameStatic->GroupsPerBlockInY;
		m_nColGroupNumInBlock = FrameStatic->GroupsPerBlockInX;
		m_nSectionTotalNum = m_nRowSectionNum * m_nColSectionNum;
		m_nBlockTotalNumInSection = m_nRowBlockNumInSection * m_nColBlockNumInSection;
		m_nGroupTotalNumInBlock = m_nRowGroupNumInBlock * m_nColGroupNumInBlock;
		m_nGroupRow = FrameStatic->GroupHeight;
		m_nGroupCol = FrameStatic->GroupWidth;
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

bool CDVS03BADecoder::CheckFrameFooter(uint8_t* pucBinData, size_t nBinLens, uint16_t& nFooterLens, bool &bFindFrameLens, bool & bFindCRC)
{
	bFindFrameLens = false;
	bFindCRC = false;
	if (nBinLens < sizeof(DVS_FOOTER_003BA))
	{
		return false;
	}
	m_nFrameSize = 0;
	size_t nIndex = 0;
	FrameTypeFrameFooter *FrameFooter = (FrameTypeFrameFooter *)pucBinData;

	if (FrameFooter->FooterCode != DVS_FOOTER_003BA)
	{
		return false;
	}

	if (m_bCheckSimpleFooter)
	{
		nFooterLens = 8;
		return true;
	}

	nFooterLens = FrameFooter->FooterStatic.FooterSize; 
	nIndex += sizeof(FrameTypeFrameFooter);
	if (FrameFooter->FooterStatic.ST)
	{
		for (uint32_t i = 0; i < m_nSectionTotalNum; i++)
		{
			if ((nBinLens - nIndex) < sizeof(FrameTypeStats))
			{
				return false;
			}

			FrameTypeStats *Stats = (FrameTypeStats*)(pucBinData + nIndex);
			m_Stats[i] = *Stats;
			nIndex += sizeof(FrameTypeStats);
		}
	}
	if (FrameFooter->FooterStatic.CSZ)
	{
		if ((nBinLens - nIndex) < sizeof(FrameTypeByteCount))
		{
			return false;
		}
		bFindFrameLens = true;
		FrameTypeByteCount *ByteCount = (FrameTypeByteCount*)(pucBinData + nIndex);
		m_nFrameSize = ByteCount->ByteCount;
		nIndex += sizeof(FrameTypeByteCount);
	}

	if (FrameFooter->FooterStatic.CH)
	{
		if ((nBinLens - nIndex) < sizeof(FrameTypeCrc))
		{
			return false;
		}
		bFindCRC = true;
		FrameTypeCrc *Crc = (FrameTypeCrc*)(pucBinData + nIndex);
		m_nCrc = Crc->Crc;
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

		BlockProcess(nSectionIndex, BlockBase);
	}

	return true;
}


void CDVS03BADecoder::BlockProcess(uint8_t nCurSection, CBlockBase& Block)
{
	if (Block.m_BlockType == BlockType::OffsetBlock)
	{
		auto OffsetBlock = Block.m_pOffsetBlock;
		m_SectionBlock[nCurSection] += OffsetBlock->GetOffset() + 1;
	}
	else
	{
		auto ContentBlock = Block.m_pContentBlock;
		auto BlockLocal = LocalBlock(nCurSection, m_SectionBlock[nCurSection]);
		if (0 != ContentBlock->m_DownPageLength)
		{
			if (ContentBlock->m_DownCompType == GroupCompType::packed)
			{
				uint8_t uGroupIndex = 0;
				FrameTypePackedGroup *PackedGroup;
				Local l;
				for (uint32_t nContentIndex = 0; nContentIndex < ContentBlock->m_DownPageLength; nContentIndex++)
				{
					PackedGroup = (FrameTypePackedGroup*)(&ContentBlock->m_DownPage[nContentIndex]);

					auto GroupLocal1 = LocalGroup(BlockLocal, uGroupIndex);
					auto GroupLocal2 = LocalGroup(BlockLocal, uGroupIndex + 1);

					if (PackedGroup->G0E0)
					{
						l = LocalPixel(GroupLocal1, 0, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, OFF_EVENT_FLAG);
					}
					if (PackedGroup->G0E1)
					{
						l = LocalPixel(GroupLocal1, 1, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, OFF_EVENT_FLAG);
					}
					if (PackedGroup->G0E2)
					{
						l = LocalPixel(GroupLocal1, 2, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, OFF_EVENT_FLAG);
					}
					if (PackedGroup->G0E3)
					{
						l = LocalPixel(GroupLocal1, 3, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, OFF_EVENT_FLAG);
					}
					if (PackedGroup->G1E0)
					{
						l = LocalPixel(GroupLocal2, 0, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, OFF_EVENT_FLAG);
					}
					if (PackedGroup->G1E1)
					{
						l = LocalPixel(GroupLocal2, 1, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, OFF_EVENT_FLAG);
					}
					if (PackedGroup->G1E2)
					{
						l = LocalPixel(GroupLocal2, 2, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, OFF_EVENT_FLAG);
					}
					if (PackedGroup->G1E3)
					{
						l = LocalPixel(GroupLocal2, 3, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, OFF_EVENT_FLAG);
					}
					uGroupIndex += 2;
				}
			}
			else
			{
				FrameTypeOffsetGroup *OffsetGroup;
				Local l;
				for (uint32_t nContentIndex = 0; nContentIndex < ContentBlock->m_DownPageLength; nContentIndex++)
				{
					OffsetGroup = (FrameTypeOffsetGroup  *)( & ContentBlock->m_DownPage[nContentIndex]);
					auto GroupLocal = LocalGroup(BlockLocal, OffsetGroup->GroupIndex);

					if (OffsetGroup->E0)
					{
						l = LocalPixel(GroupLocal, 0, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, OFF_EVENT_FLAG);
					}
					if (OffsetGroup->E1)
					{
						l = LocalPixel(GroupLocal, 1, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, OFF_EVENT_FLAG);
					}
					if (OffsetGroup->E0)
					{
						l = LocalPixel(GroupLocal, 2, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, OFF_EVENT_FLAG);
					}
					if (OffsetGroup->E3)
					{
						l = LocalPixel(GroupLocal, 3, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, OFF_EVENT_FLAG);
					}
				}
			}
		}
		if (0 != ContentBlock->m_UpPageLength)
		{
			if (ContentBlock->m_UpCompType == GroupCompType::packed)
			{
				uint8_t uGroupIndex = 0;
				FrameTypePackedGroup *PackedGroup;
				Local l;
				for (uint32_t nContentIndex = 0; nContentIndex < ContentBlock->m_UpPageLength; nContentIndex++)
				{
					PackedGroup = (FrameTypePackedGroup*)(&ContentBlock->m_UpPage[nContentIndex]);
					auto GroupLocal1 = LocalGroup(BlockLocal, uGroupIndex);
					auto GroupLocal2 = LocalGroup(BlockLocal, uGroupIndex + 1);

					if (PackedGroup->G0E0)
					{
						l = LocalPixel(GroupLocal1, 0, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, ON_EVENT_FLAG);
					}
					if (PackedGroup->G0E1)
					{
						l = LocalPixel(GroupLocal1, 1, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, ON_EVENT_FLAG);
					}
					if (PackedGroup->G0E2)
					{
						l = LocalPixel(GroupLocal1, 2, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, ON_EVENT_FLAG);
					}
					if (PackedGroup->G0E3)
					{
						l = LocalPixel(GroupLocal1, 3, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, ON_EVENT_FLAG);
					}
					if (PackedGroup->G1E0)
					{
						l = LocalPixel(GroupLocal2, 0, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, ON_EVENT_FLAG);
					}
					if (PackedGroup->G1E1)
					{
						l = LocalPixel(GroupLocal2, 1, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, ON_EVENT_FLAG);
					}
					if (PackedGroup->G1E2)
					{
						l = LocalPixel(GroupLocal2, 2, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, ON_EVENT_FLAG);
					}
					if (PackedGroup->G1E3)
					{
						l = LocalPixel(GroupLocal2, 3, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, ON_EVENT_FLAG);
					}
					uGroupIndex += 2;
				}
			}
			else
			{
				FrameTypeOffsetGroup *OffsetGroup;
				Local l;
				for (uint32_t nContentIndex = 0; nContentIndex < ContentBlock->m_UpPageLength; nContentIndex++)
				{
					OffsetGroup = (FrameTypeOffsetGroup * )(&ContentBlock->m_UpPage[nContentIndex]);
					auto GroupLocal = LocalGroup(BlockLocal, OffsetGroup->GroupIndex);

					if (OffsetGroup->E0)
					{
						l = LocalPixel(GroupLocal, 0, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, ON_EVENT_FLAG);
					}
					if (OffsetGroup->E1)
					{
						l = LocalPixel(GroupLocal, 1, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, ON_EVENT_FLAG);
					}
					if (OffsetGroup->E0)
					{
						l = LocalPixel(GroupLocal, 2, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, ON_EVENT_FLAG);
					}
					if (OffsetGroup->E3)
					{
						l = LocalPixel(GroupLocal, 3, m_nSubFrameIndex);
						if ((l.x >= 56) && (l.x % 8 < 4))
							m_RawData->SetData(l.x / 8 * 4 + l.x % 8 - 28, l.y, ON_EVENT_FLAG);
					}
				}
			}
		}
		++m_SectionBlock[nCurSection];
	}
}

Local CDVS03BADecoder::LocalBlock(uint8_t nSectionIndex, uint16_t nBlockIndex)
{
	Local res;

	res.x = (nSectionIndex / m_nColSectionNum * m_nRowBlockNumInSection + nBlockIndex / m_nColBlockNumInSection) * m_nRowGroupNumInBlock;
	res.y = (nSectionIndex % m_nColSectionNum * m_nColBlockNumInSection + nBlockIndex % m_nColBlockNumInSection) * m_nColGroupNumInBlock;

	return res;
}

Local CDVS03BADecoder::LocalGroup(Local& BlockLocal, uint8_t nGroupIndex)
{
	Local res;

	res.x = (BlockLocal.x + nGroupIndex / m_nColGroupNumInBlock) * m_nGroupRow;
	res.y = (BlockLocal.y + nGroupIndex % m_nColGroupNumInBlock) * m_nGroupCol;

	return res;
}

Local CDVS03BADecoder::LocalPixel(Local& GroupLocal, uint8_t nEventIndex, uint8_t nSubFrameIndex)
{
	/*
		1 | 3	 0 | 2
		！ ！ -> ！ ！
		0 | 2	 1 | 3
	*/
	Local res;

	res.x = GroupLocal.x + nEventIndex % m_nGroupCol;
	res.y = GroupLocal.y + nEventIndex / m_nGroupCol;

	res.x = res.x * 2 + nSubFrameIndex / 2;
	res.y = res.y * 2 + nSubFrameIndex % 2;

	return res;
}

uint32_t CDVS03BADecoder::GetCrc32(uint8_t* data, size_t length)
{
	uint32_t crc = 0xffffffff; // same as previousCrc32 ^ 0xFFFFFFFF
	const uint8_t* current = (const uint8_t*)data;

	while (length-- != 0)
	{
		uint8_t s = uint8_t(crc) ^ *current++;

		// Hagai Gold made me aware of this table-less algorithm and send me code

		// polynomial 0xEDB88320 can be written in binary as 11101101101110001000001100100000b
		// reverse the bits (or just assume bit 0 is the first one)
		// and we have bits set at position 0, 1, 2, 4, 5, 7, 8, 10, 11, 12, 16, 22, 23, 26
		// => those are the shift offsets:
		//crc = (crc >> 8) ^
		//       t ^
		//      (t >>  1) ^ (t >>  2) ^ (t >>  4) ^ (t >>  5) ^  // == y
		//      (t >>  7) ^ (t >>  8) ^ (t >> 10) ^ (t >> 11) ^  // == y >> 6
		//      (t >> 12) ^ (t >> 16) ^                          // == z
		//      (t >> 22) ^ (t >> 26) ^                          // == z >> 10
		//      (t >> 23);

		// the fastest I can come up with:
		uint32_t low = (s ^ (s << 6)) & 0xFF;
		uint32_t a = (low * ((1 << 23) + (1 << 14) + (1 << 2)));
		crc = (crc >> 8) ^
			(low * ((1 << 24) + (1 << 16) + (1 << 8))) ^
			a ^
			(a >> 1) ^
			(low * ((1 << 20) + (1 << 12))) ^
			(low << 19) ^
			(low << 17) ^
			(low >> 2);

		// Hagai's code:
		/*uint32_t t = (s ^ (s << 6)) << 24;
		// some temporaries to optimize XOR
		uint32_t x = (t >> 1) ^ (t >> 2);
		uint32_t y = x ^ (x >> 3);
		uint32_t z = (t >> 12) ^ (t >> 16);
		crc = (crc >> 8) ^
			   t ^ (t >> 23) ^
			   y ^ (y >>  6) ^
			   z ^ (z >> 10);*/
	}

	return ~crc; // same as crc ^ 0xFFFFFFFF
}


bool CDVS03BADecoder::DVS_Decode(uint8_t* pucBinData, CDVSDataContainer* DVSData, size_t nRow, size_t nCol, size_t* pnPos, size_t nBinLens, uint8_t& nSubFrameIndex, uint64_t & ntimeStamp)
{
	Init();
	m_nTotalRow = nRow;
	m_nTotalCol = nCol;
	m_RawData = DVSData;
	size_t nCurIndex = *pnPos;
	uint16_t nHeaderLens = 0;
	uint64_t *HeaderCode;
	while (nBinLens - nCurIndex > sizeof(HeaderCode))
	{
		HeaderCode = (uint64_t *)(pucBinData + nCurIndex);

		if (*HeaderCode != DVS_HEADER_003BA) 
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
	std::vector<uint16_t>(m_nSectionTotalNum, 0).swap(m_SectionBlock);
	m_Stats.resize(m_nSectionTotalNum);
	uint16_t nFooterLens = 0;
	uint16_t nBlockLens = 0;
	bool bPadding = false;
	bool bFindFooter = false;
	while (nBinLens > nCurIndex)
	{
		bool bFindFrameLens = false, bFindCRC = false;
		if (CheckFrameFooter(pucBinData + nCurIndex, nBinLens - nCurIndex, nFooterLens, bFindFrameLens, bFindCRC))
		{
			nCurIndex += nFooterLens;
			bFindFooter = true;
			if (bFindFrameLens)
			{
				if (m_nFrameSize != (nCurIndex - nFrameStart))
				{
					return false;
				}
			}
			if (bFindCRC)
			{
				//uint32_t uFrameCrc = GetCrc32(pucBinData + nFrameStart, nCurIndex - nFrameStart - sizeof(m_nCrc));
				//if(m_nCrc != uFrameCrc)
				//{
				//	return false;
				//}
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
	nSubFrameIndex = m_nSubFrameIndex;
	ntimeStamp = m_nTimeStamp;
	*pnPos = nCurIndex;
	return true;
}

