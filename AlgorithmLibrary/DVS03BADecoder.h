#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include "DVSDataContainer.h"

typedef struct
{
	uint8_t MinorVersion;
	uint8_t MajorVersion;
	uint8_t ST : 1;
	uint8_t ROI : 1;
	uint8_t FS : 1;
	uint8_t Reserve1 : 5;
	uint8_t HeaderSize;
	uint32_t Reserve2;
}FrameTypeHeaderStatic;

typedef struct
{
	uint32_t TimeStampL;
	uint16_t TimeStampH;
	uint8_t SubIndex;
	uint8_t Reserve;
}FrameTypeSubTime;

typedef struct
{
	uint16_t XStartInNumberOfGroups;
	uint16_t XEndInNumberOfGroups;
	uint16_t YStartInNumberOfGroups;
	uint16_t YEndInNumberOfGroups;
}FrameTypeROI;

typedef struct
{
	uint8_t NumberOfSectionsInX;
	uint8_t NumberOfSectionsInY;
	uint16_t Reserve;
	uint16_t NumberOfPixelsInX;
	uint16_t NumberOfPixelsInY;
	uint8_t UpPixelSize : 4;
	uint8_t GroupWidth : 4;
	uint8_t GroupsPerBlockInX;
	uint16_t NumberOfBlocksPerSectionInX;

	uint8_t DnPixelSize : 4;
	uint8_t GroupHeight : 4;
	uint8_t GroupsPerBlockInY;
	uint16_t NumberOfBlocksPerSectionInY;
}FrameTypeFrameStatic;

typedef struct
{
	uint64_t HeaderCode;
	FrameTypeHeaderStatic HeaderStatic;
}FrameTypeFrameHeader;

typedef struct
{
	uint8_t SectionIndex : 7;
	uint8_t BlockType : 1;
	uint8_t ImmediateOffset;
}FrameTypeShortBlockDescriptor;

typedef struct
{
	uint8_t SectionIndex : 7;
	uint8_t BlockType : 1;
	uint8_t LON : 1;
	uint8_t BSW : 1;
	uint8_t PN : 1;
	uint8_t OT : 1;
	uint8_t DCT : 1;
	uint8_t Reserve1 : 1;
	uint8_t UCT : 1;
	uint8_t Reserve2 : 1;
}FrameTypeLongBlockDescriptor;

typedef union
{
	FrameTypeShortBlockDescriptor ShortBlockDescriptor;
	FrameTypeLongBlockDescriptor LongBlockDescriptor;

}FrameTypeBlockDescriptor;

typedef struct
{
	uint8_t ST : 1;
	uint8_t CSZ : 1;
	uint8_t CH : 1;
	uint8_t Reserve1 : 5;
	uint8_t FooterSize;
	uint16_t Reserve2;
	uint32_t Reserve3;
}FrameTypeFooterStatic;

typedef struct
{
	uint16_t NumberOfSentGroups;
	uint16_t NumberOfDropedGroups;
	uint8_t SectionIndex;
	uint8_t Reserved1;
	uint16_t Reserved2;
}FrameTypeStats;

typedef struct
{
	uint32_t ByteCount;
}FrameTypeByteCount;

typedef struct
{
	uint32_t Crc;
}FrameTypeCrc;

typedef struct
{
	uint64_t FooterCode;
	FrameTypeFooterStatic FooterStatic;
}FrameTypeFrameFooter;

typedef enum
{
	OffsetBlock,
	ContentBlock,
	PaddingBlock,
}BlockType;

typedef struct
{
	uint8_t E0 : 1;
	uint8_t E1 : 1;
	uint8_t E2 : 1;
	uint8_t E3 : 1;
	uint8_t GroupIndex;
}FrameTypeOffsetGroup;

typedef struct
{
	uint8_t G0E0 : 1;
	uint8_t G0E1 : 1;
	uint8_t G0E2 : 1;
	uint8_t G0E3 : 1;
	uint8_t G1E0 : 1;
	uint8_t G1E1 : 1;
	uint8_t G1E2 : 1;
	uint8_t G1E3 : 1;
}FrameTypePackedGroup;

typedef struct
{
	FrameTypeLongBlockDescriptor BlockDescriptor;
	uint16_t ImmediateOffset;
}FrameTypeLongOffsetBlockBase;

typedef struct
{
	FrameTypeLongBlockDescriptor BlockDescriptor;
	uint8_t DNPageLength : 4;
	uint8_t UPPageLength : 4;

}FrameTypeContentBlockBase;

typedef struct
{
	FrameTypeLongBlockDescriptor BlockDescriptor;
	uint8_t PaddingPageLength;
}FrameTypePaddingBlockBase;

class COffsetBlock
{
public:
	COffsetBlock();
	COffsetBlock(uint8_t* RawData, uint64_t nRawLens);
	~COffsetBlock();
	uint16_t GetBlockLens();
	uint16_t GetOffset();
	uint8_t GetSectionIndex();
public:
	uint16_t m_nBlockLens;
	uint16_t m_nOffset;
	uint8_t m_SectionIndex;
};

class CContentBlock
{
public:
	CContentBlock();
	CContentBlock(uint8_t* RawData, uint64_t nRawLens);
	~CContentBlock();
	uint16_t GetBlockLens();
	uint8_t GetSectionIndex();
public:
	uint16_t m_nBlockLens;
	uint8_t m_SectionIndex;
	uint8_t m_DownCompType;
	uint8_t m_UpCompType;
	uint8_t m_DownPageLength;
	uint8_t m_UpPageLength;
	uint8_t m_DownPage[16];
	uint8_t m_UpPage[16];
};

typedef enum
{
	packed = 0,
	offset,
}GroupCompType;

class CPaddingBlock
{
public:
	CPaddingBlock();
	CPaddingBlock(uint8_t* RawData, uint64_t nRawLens);
	~CPaddingBlock();
	uint16_t GetBlockLens();
	uint8_t GetSectionIndex();
public:
	uint16_t m_nBlockLens;
	uint8_t m_SectionIndex;
	uint8_t m_nPaddingPageLength;
};

class CBlockBase
{
public:
	CBlockBase();
	CBlockBase(uint8_t * RawData, uint64_t nRawLens);
	BlockType GetBlockType();
	uint8_t GetSectionIndex();
	uint16_t GetBlockLens();
	~CBlockBase();
public:
	BlockType m_BlockType;
	uint8_t m_SectionIndex;
	uint16_t m_nBlockLens;
	COffsetBlock * m_pOffsetBlock;
	CContentBlock * m_pContentBlock;
	CPaddingBlock * m_pPaddingBlock;
};

class CDVS03BADecoder
{
public:
	CDVS03BADecoder();
	void Init();
	bool DVS_Decode(uint8_t* pucBinData, CDVSDataContainer* DVSData, size_t nRow, size_t nCol, size_t* pnPos, size_t nBinLens, uint8_t &nSubFrameIndex, uint64_t &nTimeStamp);
	void SetCheckSimpleFooter(bool bCheckSimpleFooter) { m_bCheckSimpleFooter = bCheckSimpleFooter; }
protected:
	bool CheckFrameHeader(uint8_t* pucBinData, size_t nBinLens, uint16_t& nHeaderLens);
	bool CheckFrameFooter(uint8_t* pucBinData, size_t nBinLens, uint16_t& nFooterLens, bool &bFindFrameLens, bool &bFindCRC);
	bool CheckBlock(uint8_t* pucBinData, size_t nBinLens, uint16_t& nBlockLens, bool& bPaddingFlag);
	inline Local LocalBlock(uint8_t nSectionIndex, uint16_t nBlockIndex);
	inline Local LocalGroup(Local& BlockLocal, uint8_t nGroupIndex);
	inline Local LocalPixel(Local &GroupLocal, uint8_t nEventIndex, uint8_t nSubFrameIndex);
	void BlockProcess(uint8_t Section, CBlockBase & Block);
	uint32_t GetCrc32(uint8_t* data, size_t length);
private:
	uint32_t m_nTotalRow;
	uint32_t m_nTotalCol;
	uint8_t m_nSubFrameIndex;
	uint64_t m_nTimeStamp;
	ROIArea m_Roi;
	uint8_t m_nRowSectionNum;
	uint8_t m_nColSectionNum;
	uint16_t m_nSubFrameRow;
	uint16_t m_nSubFrameCol;
	uint16_t m_nRowBlockNumInSection;
	uint16_t m_nColBlockNumInSection;
	uint16_t m_nRowGroupNumInBlock;
	uint16_t m_nColGroupNumInBlock;
	uint8_t m_nSectionTotalNum;
	uint16_t m_nBlockTotalNumInSection;
	uint16_t m_nGroupTotalNumInBlock;
	uint8_t m_nGroupRow;
	uint8_t m_nGroupCol;
	uint32_t m_nFrameSize;
	std::vector<uint16_t> m_SectionBlock;
	std::vector<FrameTypeStats> m_Stats;
	uint32_t m_nCrc;
	CDVSDataContainer *m_RawData;
	bool m_bCheckSimpleFooter;
};

