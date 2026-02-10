#include "Alp014BADVSMPAlgorithm.h"

#define DVS_HEADER_014BA 0x0000FFFF
#define DVS_FOOTER_014BA 0x0101FFFF
#define DVS_FOOTER_DROP_014BA 0x0303FFFF

uint32_t EVS_only_readout_order[] = { 0 , 2 , 8 , 10 , 16 , 18 , 24 , 26 , 32 , 34 , 40 , 42 , 48 , 50 , 56 , 58,  1 , 3 , 9 , 11 , 17 , 19 , 25 , 27 , 33, 35, 41, 43, 49, 51, 57 , 59, 4 , 6 , 12 , 14 , 20 , 22 , 28, 30, 36, 38, 44, 46, 52, 54, 60 , 62, 5 , 7 , 13 , 15 , 21 , 23 , 29, 31, 37, 39, 45, 47, 53, 55, 61 , 63 };
uint32_t HVS_readout_order[] = {1 , 3 , 9 , 11 , 17 , 19 , 25 , 27 , 33, 35, 41, 43, 49, 51, 57 , 59, 4 , 6 , 12 , 14 , 20 , 22 , 28, 30, 36, 38, 44, 46, 52, 54, 60 , 62};
uint32_t subsample_1_2_readout_order[] = { 1 , 3 , 9 , 11 , 17 , 19 , 25 , 27 , 33, 35, 41, 43, 49, 51, 57 , 59};
uint32_t subsample_1_4_readout_order[] = { 1 , 17, 33 , 49};
uint32_t subsample_1_8_readout_order[] = { 1 };


CAlp014BADVSMPAlgorithm::CAlp014BADVSMPAlgorithm(SensorType Sensortype, std::string strLogDir, uint32_t nSiteNum, PixelFormatType Pixelformat, int code)
	:CAlp014AADVSMPAlgorithm(Sensortype, strLogDir, nSiteNum, Pixelformat, code)
{
	if ((code & DVS_Code_HVS) == DVS_Code_HVS)
	{
		m_subsample_num = 32;
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
	else if ((code & DVS_Code_1_2_Subsample) == DVS_Code_1_2_Subsample)
	{
		m_subsample_num = 16;
		m_nTotalRow = 512;
		m_nTotalCol = 640;

		for (uint32_t i = 0; i < 16; i += 1)
		{
			m_subframe_order[i] = subsample_1_2_readout_order[i];

			switch (subsample_1_2_readout_order[i] % 8)
			{
			case 1:
				m_col_offset_table[i] = 0;
				break;
			case 3:
				m_col_offset_table[i] = 1;
				break;
			default:
				break;
			}
			m_row_offset_table[i] = subsample_1_2_readout_order[i] / 8;
		}
	} else if ((code & DVS_Code_1_4_Subsample) == DVS_Code_1_4_Subsample)
	{
	    m_subsample_num = 4;
	    m_nTotalRow = 256;
	    m_nTotalCol = 320;

	    for (uint32_t i = 0; i < 4; i += 1)
	    {
	        m_subframe_order[i] = subsample_1_4_readout_order[i];

	        switch (subsample_1_4_readout_order[i] % 8)
	        {
	            case 1:
	                m_col_offset_table[i] = 0;
	                break;
	            default:
	                break;
	        }
	        m_row_offset_table[i] = subsample_1_4_readout_order[i] / 8;
	    }
	} else if ((code & DVS_Code_1_8_Subsample) == DVS_Code_1_8_Subsample)
	{
	    m_subsample_num = 1;
	    m_nTotalRow = 64;
	    m_nTotalCol = 320;

	    for (uint32_t i = 0; i < 1; i += 1)
	    {
	        m_subframe_order[i] = subsample_1_8_readout_order[i];

	        switch (subsample_1_8_readout_order[i] % 8)
	        {
	            case 1:
	                m_col_offset_table[i] = 0;
	                break;
	            default:
	                break;
	        }
	        m_row_offset_table[i] = subsample_1_8_readout_order[i] / 8;
	    }
	}
	else
	{
		m_nTotalRow = 1024;
		m_nTotalCol = 1280;
		m_subsample_num = 64;

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
		uint32_t nMaxSubFrame = m_subsample_num;
		uint32_t nRow = 0;
		
		if (m_subsample_num == 64)
		{
			nRow = m_nTotalRow / 16;
		}
		else if (m_subsample_num == 32)
		{
			nRow = m_nTotalRow / 8;
		}
		else if (m_subsample_num == 16)
		{
			nRow = m_nTotalRow / 8;
		}

		uint32_t nCol = 0;
		if (m_subsample_num == 64)
		{
			nCol = m_nTotalCol / 4;
		}
		else if (m_subsample_num == 32)
		{
			nCol = m_nTotalCol / 4;
		}
		else if (m_subsample_num == 16)
		{
			nCol = m_nTotalCol / 2;
		}

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

#if 0
bool CAlp014BADVSMPAlgorithm::ImportRawData_DropSubFrame(uint8_t* pBinData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber, uint32_t nMode, size_t &nDropSubFrameNum)
{
    nDropSubFrameNum = 0;
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
        uint32_t nMaxSubFrame = m_subsample_num;
        uint32_t nRow = 0;

        if (m_subsample_num == 64)
        {
            nRow = m_nTotalRow / 16;
        }
        else if (m_subsample_num == 32)
        {
            nRow = m_nTotalRow / 8;
        }
        else if (m_subsample_num == 16)
        {
            nRow = m_nTotalRow / 8;
        }

        uint32_t nCol = 0;
        if (m_subsample_num == 64)
        {
            nCol = m_nTotalCol / 4;
        }
        else if (m_subsample_num == 32)
        {
            nCol = m_nTotalCol / 4;
        }
        else if (m_subsample_num == 16)
        {
            nCol = m_nTotalCol / 2;
        }

        while (nNeedSubFrameIndex != nMaxSubFrame)
        {
            std::string str = "nSubFrameIndex:"+std::to_string(nSubFrameIndex) +"m_subframe_order[nNeedSubFrameIndex]:"+std::to_string(m_subframe_order[nNeedSubFrameIndex]);
            WriteLog(str);
            if (Decode_DropSubFrame(pBinData, &m_RawDataContainer[nIndexStart + nIndex], nRow, nCol, &pos, nLens, nSubFrameIndex, nTimeStamp, nDropSubFrameNum) && nSubFrameIndex == m_subframe_order[nNeedSubFrameIndex])
            {
                std::string strErr = "LINE: " + std::to_string(__LINE__) +
                                     " ImportRawData_DropSubFrame: m_subsample_num: " + std::to_string(m_subsample_num);
                WriteLog(strErr);
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
#endif

bool CAlp014BADVSMPAlgorithm::ImportRawData_DropSubFrame(uint8_t* pBinData, uint64_t nLens, uint32_t nIndexStart, uint32_t nNumber, uint32_t &nDropSubFrameNum) {
    size_t pos = 0;
    if (m_RawDataContainer.size() < nIndexStart + nNumber)
    {
        m_RawDataContainer.resize(nIndexStart + nNumber);
    }

    // 计算行列参数
    uint32_t nRow = 0;
    uint32_t nCol = 0;

    if (m_subsample_num == 64)
    {
        nRow = m_nTotalRow / 16;
        nCol = m_nTotalCol / 4;
    }
    else if (m_subsample_num == 32)
    {
        nRow = m_nTotalRow / 8;
        nCol = m_nTotalCol / 4;
    }
    else if (m_subsample_num == 16)
    {
        nRow = m_nTotalRow / 8;
        nCol = m_nTotalCol / 2;
    }
    else if (m_subsample_num == 4)
    {
        nRow = m_nTotalRow / 4;
        nCol = m_nTotalCol / 2;
    }
    else if (m_subsample_num == 1)
    {
        nRow = m_nTotalRow / 2;
        nCol = m_nTotalCol / 1;
    }
    else
    {
        WriteLog("ImportRawData: Unsupported subsample_num: " + std::to_string(m_subsample_num));
        m_nErrCode = EVS_DECODE_ERROR;
        return false;
    }


    uint32_t nMaxSubFrame = m_subsample_num;
    bool bFirstDecode = true;  // 标记是否是第一次解码

    for (uint32_t nIndex = 0; nIndex < nNumber; nIndex++)
    {
        uint8_t nNeedSubFrameIndex = 0;
        m_RawDataContainer[nIndexStart + nIndex].Init(m_nTotalRow, m_nTotalCol, true, m_PixelFormat);
        uint8_t nSubFrameIndex = 0;
        uint64_t nTimeStamp = 0;

        while (nNeedSubFrameIndex != nMaxSubFrame)
        {
            bool bDecodeSuccess = false;

            // ============ 关键修改:只在第一次解码时使用 Decode_DropSubFrame ============
            if (bFirstDecode)
            {
                nDropSubFrameNum = 0;

                bDecodeSuccess = Decode_DropSubFrame(
                    pBinData,
                    &m_RawDataContainer[nIndexStart + nIndex],
                    nRow, nCol,
                    &pos, nLens,
                    nSubFrameIndex, nTimeStamp,
                    nDropSubFrameNum
                );

                bFirstDecode = false;  // 只执行一次

                // 记录丢弃信息
                if (bDecodeSuccess && nDropSubFrameNum > 0)
                {
                    std::string strInfo = "ImportRawData: Synchronized buffer, dropped " +
                        std::to_string(nDropSubFrameNum) +
                        " frames before Subframe=" + std::to_string(m_subframe_order[0]);
                    WriteLog(strInfo);
                }
            }
            else
            {
                // 后续使用原来的 Decode
                bDecodeSuccess = Decode(
                    pBinData,
                    &m_RawDataContainer[nIndexStart + nIndex],
                    nRow, nCol,
                    &pos, nLens,
                    nSubFrameIndex, nTimeStamp
                );
            }
            // ============ 修改结束 ============

            // 验证子帧顺序
            if (bDecodeSuccess && nSubFrameIndex == m_subframe_order[nNeedSubFrameIndex])
            {
                ++nNeedSubFrameIndex;
            }
            else
            {
                std::string strErr = "ImportRawData: DVS Decoder error\n";
                strErr += "  Frame Index: " + std::to_string(nIndex) + "\n";
                strErr += "  SubFrame Index: " + std::to_string(nNeedSubFrameIndex) + "\n";
                strErr += "  Byte Position: " + std::to_string(pos) + "\n";
                strErr += "  Expected Subframe: " + std::to_string(m_subframe_order[nNeedSubFrameIndex]) + "\n";
                strErr += "  Received Subframe: " + std::to_string(nSubFrameIndex) + "\n";
                strErr += "  Decode Success: " + std::string(bDecodeSuccess ? "true" : "false");
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
    // 查找Header
	while (nBinLens > nCurIndex + sizeof(Alp014AAFormatHeader))
	{
		HeaderCode = (Alp014BAFormatHeader*)(pucBinData + nCurIndex);

		if (HeaderCode->Header_vec != DVS_HEADER_014BA)
		{
			nCurIndex += 4; // 每次前进4字节继续搜索
		}
		else
		{
			break; // 找到有效Header
		}
	}
	if (nBinLens <= nCurIndex + sizeof(Alp014BAFormatHeader))
	{
		return false;
	}

    // Decode Header and TimeStamp
	HeaderCode = (Alp014BAFormatHeader*)(pucBinData + nCurIndex);
    // 64bit TimeStamp, 32-bit Timestamp_H and 32-bit Timestamp_L.
	nTimeStamp = (uint64_t(HeaderCode->Timestamp_H) << 32) + HeaderCode->Timestamp_L;
    // 移动解码时越过Header
	nCurIndex += sizeof(Alp014BAFormatHeader);

	Alp014BAFormatStatic* Static;

    // Decode Static
	if (nCurIndex + sizeof(Alp014BAFormatStatic) < nBinLens) // 检查ROI是否超出范围
	{
		Static = (Alp014BAFormatStatic*)(pucBinData + nCurIndex);
		nCurIndex += sizeof(Alp014BAFormatStatic);
	}
	else
	{
		return false;
	}

    // 获取SubFrame[5:0]
	nSubFrameIndex = Static->Subframe;

	uint8_t nSubFrameInPixelArray = 0;

    // 查找SubFrame在像素数组中的位置, 将SubFrameIndex映射到像素数组位置
	for (; nSubFrameInPixelArray < m_subsample_num; nSubFrameInPixelArray++)
	{
		if (nSubFrameIndex == m_subframe_order[nSubFrameInPixelArray])
		{
			break;
		}
	}

	if ((Static->Roi_row_stop - Static->Roi_row_start + 1) > nRow || (Static->Roi_col_stop - Static->Roi_col_start + 1) > nCol)
	{
		return false;
	}

	bool bRet = true;

	if (Static->frame_mode == 0)
	{
		bRet = EventModeDecode(pucBinData, DVSData, 0, nRow, 0, nCol, &nCurIndex, nBinLens, nSubFrameInPixelArray);
	}
	else
	{
		bRet = FrameModeDecode(pucBinData, DVSData, 0, nRow, 0, nCol, &nCurIndex, nBinLens, nSubFrameInPixelArray);
	}
	if (bRet)
	{
	    // 验证尾部和对齐
		if (nCurIndex % 8 != 0)
		{
			nCurIndex = (nCurIndex / 8 + 1) * 8;
		}

		Alp014BAFormatFooter* Footer = (Alp014BAFormatFooter*)(pucBinData + nCurIndex);

	    // 验证Footer和DropFlag
	    // Dropflag == 0: 确保这一帧数据完整, 未丢失
		if (Footer->Footer_vec == DVS_FOOTER_014BA && Footer->Dropflag == 0)
		{
			nCurIndex += sizeof(Alp014BAFormatFooter);
			*pnPos = nCurIndex;
			return true;
		}
	}
	return false;
}

// ============ 辅助函数:跳过当前子帧 ============
bool CAlp014BADVSMPAlgorithm::SkipCurrentSubframe(uint8_t* pucBinData, size_t* pnPos, size_t nBinLens)
{
    size_t nCurIndex = *pnPos;

    // 搜索Footer,最多搜索合理的距离(例如最大子帧大小)
    const size_t MAX_SUBFRAME_SIZE = 1024 * 1024;  // 1MB,根据实际情况调整
    size_t nSearchLimit = nCurIndex + MAX_SUBFRAME_SIZE;
    if (nSearchLimit > nBinLens)
    {
        nSearchLimit = nBinLens;
    }

    while (nCurIndex + sizeof(Alp014BAFormatFooter) <= nSearchLimit)
    {
        // 尝试8字节对齐位置
        if (nCurIndex % 8 != 0)
        {
            nCurIndex = (nCurIndex / 8 + 1) * 8;
        }

        if (nCurIndex + sizeof(Alp014BAFormatFooter) > nSearchLimit)
        {
            break;
        }

        Alp014BAFormatFooter* Footer = (Alp014BAFormatFooter*)(pucBinData + nCurIndex);

        if (Footer->Footer_vec == DVS_FOOTER_014BA ||
            Footer->Footer_vec == DVS_FOOTER_DROP_014BA)
        {
            // 找到Footer,跳到Footer之后
            nCurIndex += sizeof(Alp014BAFormatFooter);
            *pnPos = nCurIndex;
            return true;
        }

        // 继续搜索,向前移动8字节
        nCurIndex += 8;
    }

    return false;
}

bool CAlp014BADVSMPAlgorithm::Decode_DropSubFrame(
    uint8_t* pucBinData,
    CDVSDataContainer* DVSData,
    uint32_t nRow,
    uint32_t nCol,
    size_t* pnPos,
    size_t nBinLens,
    uint8_t& nSubFrameIndex,
    uint64_t& nTimeStamp,
    uint32_t& nDropSubFrameNum)
{
    WriteLog("Decode_DropSubFrame");
    size_t nCurIndex = *pnPos;
    Alp014BAFormatHeader* HeaderCode;

    // 获取期望的第一个子帧索引
    uint8_t nExpectedFirstSubframe = m_subframe_order[0];  // 应该是0
    // uint8_t nExpectedFirstSubframe = 0;  // 应该是0

    // 初始化丢弃计数器
    nDropSubFrameNum = 0;

    // ============ 寻找第一个匹配 m_subframe_order[0] 的数据包 ============
    bool bFoundTargetSubframe = false;

    while (nBinLens > nCurIndex + sizeof(Alp014BAFormatHeader))
    {
        // 查找头部
        while (nBinLens > nCurIndex + sizeof(Alp014AAFormatHeader))
        {
            HeaderCode = (Alp014BAFormatHeader*)(pucBinData + nCurIndex);

            if (HeaderCode->Header_vec != DVS_HEADER_014BA)
            {
                nCurIndex += 4;
            }
            else
            {
                WriteLog("HeaderCode->Header_vec == DVS_HEADER_014BA, Find HEADER");
                break;
            }
        }

        if (nBinLens <= nCurIndex + sizeof(Alp014BAFormatHeader))
        {
            // 到达buffer末尾仍未找到
            WriteLog("到达buffer末尾仍未找到");
            return false;
        }

        // 找到一个有效的Header
        HeaderCode = (Alp014BAFormatHeader*)(pucBinData + nCurIndex);
        size_t nHeaderPos = nCurIndex;
        nCurIndex += sizeof(Alp014BAFormatHeader);

        // 解析Static信息
        if (nCurIndex + sizeof(Alp014BAFormatStatic) >= nBinLens)
        {
            WriteLog("nCurIndex + sizeof(Alp014BAFormatStatic) >= nBinLens");
            return false;
        }

        Alp014BAFormatStatic* Static = (Alp014BAFormatStatic*)(pucBinData + nCurIndex);

        // 检查是否为期望的第一个子帧
        if (Static->Subframe == nExpectedFirstSubframe)
        {
            // 找到目标子帧!
            bFoundTargetSubframe = true;
            WriteLog("bFoundTargetSubframe = true");

            // 将指针重置到这个数据包的头部
            nCurIndex = nHeaderPos;
            *pnPos = nCurIndex;

            // 现在执行正常的Decode流程,解码这一帧
            break;
        }
        else
        {
            WriteLog("bFoundTargetSubframe = false");
            // 这不是期望的子帧,需要丢弃
            nDropSubFrameNum++;

            // 跳过这个完整的数据包
            nCurIndex += sizeof(Alp014BAFormatStatic);

            if (!SkipCurrentSubframe(pucBinData, &nCurIndex, nBinLens))
            {
                // 跳过失败,尝试下一个位置
                nCurIndex = nHeaderPos + 4;
            }
            // 继续搜索下一个数据包
        }
    }

    if (!bFoundTargetSubframe)
    {
        // 未找到期望的子帧
        std::string str = "!bFoundTargetSubframe";
        WriteLog(str);
        return false;
    }

    // ============ 找到目标子帧,现在执行正常解码 ============
    // 从这里开始,nCurIndex 已经指向期望的第一个子帧的头部
    // 下面的代码与原Decode函数完全相同

    // 重新解析头部
    while (nBinLens > nCurIndex + sizeof(Alp014AAFormatHeader))
    {
        HeaderCode = (Alp014BAFormatHeader*)(pucBinData + nCurIndex);

        if (HeaderCode->Header_vec != DVS_HEADER_014BA)
        {
            nCurIndex += 4;
        }
        else
        {
            WriteLog("HeaderCode->Header_vec == DVS_HEADER_014BA");
            break;
        }
    }

    if (nBinLens <= nCurIndex + sizeof(Alp014BAFormatHeader))
    {
        WriteLog("nBinLens <= nCurIndex + sizeof(Alp014BAFormatHeader");
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
        WriteLog("nCurIndex + sizeof(Alp014BAFormatStatic) >= nBinLens");
        return false;
    }

    nSubFrameIndex = Static->Subframe;

    // 验证:解码的子帧应该等于期望的第一个子帧
    if (nSubFrameIndex != nExpectedFirstSubframe)
    {
        WriteLog("nSubFrameIndex != nExpectedFirstSubframe");
        // 逻辑错误,不应该发生
        return false;
    }

    uint8_t nSubFrameInPixelArray = 0;

    for (; nSubFrameInPixelArray < m_subsample_num; nSubFrameInPixelArray++)
    {
        if (nSubFrameIndex == m_subframe_order[nSubFrameInPixelArray])
        {
            break;
        }
    }

    std::string tstr = "Static->Roi_row_stop: " + std::to_string(Static->Roi_row_stop);
    tstr += "Static->Roi_row_start: " + std::to_string(Static->Roi_row_start);
    tstr += "Static->Roi_col_stop:" + std::to_string(Static->Roi_col_stop);
    tstr += "Static->Roi_col_start:" + std::to_string(Static->Roi_col_start);
    tstr += "nRow:" + std::to_string(nRow);
    tstr += "nCol:" + std::to_string(nCol);
    WriteLog(tstr);
    if ((Static->Roi_row_stop - Static->Roi_row_start + 1) > nRow ||
        (Static->Roi_col_stop - Static->Roi_col_start + 1) > nCol)
    {
        WriteLog("((Static->Roi_row_stop - Static->Roi_row_start + 1) > nRow ||(Static->Roi_col_stop - Static->Roi_col_start + 1) > nCol)");
        return false;
    }

    bool bRet = true;

    if (Static->frame_mode == 0)
    {
        bRet = EventModeDecode(pucBinData, DVSData, 0, nRow, 0, nCol, &nCurIndex, nBinLens, nSubFrameInPixelArray);
        std::string str = "EventModeDecode"+ std::to_string(bRet);
        WriteLog(str);
    }
    else
    {
        bRet = FrameModeDecode(pucBinData, DVSData, 0, nRow, 0, nCol, &nCurIndex, nBinLens, nSubFrameInPixelArray);
        std::string str = "FrameModeDecode"+ std::to_string(bRet);
        WriteLog(str);
    }

    if (bRet)
    {
        if (nCurIndex % 8 != 0)
        {
            nCurIndex = (nCurIndex / 8 + 1) * 8;
        }

        Alp014BAFormatFooter* Footer = (Alp014BAFormatFooter*)(pucBinData + nCurIndex);

        std::string str ="Footer->Footer_vec == DVS_FOOTER_014BA ?:"+std::to_string(Footer->Footer_vec == DVS_FOOTER_014BA);
        str += "Footer->Dropflag == 0?:" +std::to_string(Footer->Dropflag == 0);
        WriteLog(str);
        if (Footer->Footer_vec == DVS_FOOTER_014BA && Footer->Dropflag == 0)
        {
            nCurIndex += sizeof(Alp014BAFormatFooter);
            *pnPos = nCurIndex;
            return true;
        }
    }

    WriteLog("End Error");
    return false;
}

bool CAlp014BADVSMPAlgorithm::FrameModeDecode(uint8_t* pucBinData, CDVSDataContainer* DVSData, uint32_t nRowStart, uint32_t nRowStop, uint32_t nColStart, uint32_t nColStop, size_t* pnPos, size_t nBinLens, uint8_t& nSubFrameIndex)
{
	bool bRet = false;

	uint32_t nRowStep = 16;
	uint32_t nColStep = 4;
	if (m_subsample_num == 16 || m_subsample_num == 32)
	{
		nRowStep = 8;
	}
	if (m_subsample_num == 16)
	{
		nColStep = 2;
	}
    if (m_subsample_num == 4)
	{
	    nColStep = 1;
        nRowStep = 4;
	}
    if (m_subsample_num == 1)
    {
        nColStep = 1;
        nRowStep = 1;
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
	if (m_subsample_num == 16 || m_subsample_num == 32)
	{
		nRowStep = 8;
	}
	if (m_subsample_num == 16)
	{
		nColStep = 2;
	}
    if (m_subsample_num == 4)
    {
        nColStep = 1;
        nRowStep = 4;
    }
    if (m_subsample_num == 1)
    {
        nColStep = 1;
        nRowStep = 1;
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
