#include "DVSProcessFunctions.h"
#include "memory.h"

bool DVS_Decoder(uint8_t *pucBinData, uint8_t *pucRawData, size_t nRow, size_t nCol, size_t *pnPos, size_t nBinLens)
{  
    uint8_t ucHead[] = { 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xFD };
    uint8_t ucTail[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    uint8_t ucHeadLength = 32;
    uint64_t ulTrigerTime = 0;
    uint64_t ulTimestamp = 0;
    uint32_t unGapInfo = 0;
    uint8_t* pucBinHeader = pucBinData + *pnPos;
    size_t nIndex = 0;
    while (0 != memcmp(ucHead, pucBinHeader + nIndex, sizeof(ucHead)))
    {
        nIndex += 8;

        if (nBinLens - sizeof(ucTail)  <= *pnPos + nIndex)
        {
            return false;
        }
    }
    pucBinHeader += nIndex;
    ulTrigerTime = *((uint64_t*)&pucBinHeader[8]);
    unGapInfo = *((uint32_t*)&pucBinHeader[20]);
    ulTimestamp = ulTrigerTime + unGapInfo / 1000;
    uint8_t* pucBinBody = pucBinHeader + ucHeadLength;
    memset(pucRawData, 0, nRow * nCol);
    size_t nRowIndex = 0;
    size_t nColIndex = 0;
    for (nIndex = 0; nIndex < nBinLens - *pnPos - sizeof(ucTail); nIndex += 5)
    {
        if (0 == memcmp(ucTail, pucBinBody + nIndex, sizeof(ucTail)))
        {
            break;
        }

        uint8_t ucGroupHead = pucBinBody[nIndex];
        if (0xFF == ucGroupHead)
        {
            nRowIndex += pucBinBody[nIndex + 1];
            if (nRowIndex >= nRow)
            {
                return false;
            }
            nColIndex = 0;
        }
        else
        {
            nColIndex += 16 * ucGroupHead;
            if (nColIndex >= nCol)
            {
                return false;
            }
            size_t nPolarityIndex = 0;
            for (int i = 1; i < 5; i++)
            {
                pucRawData[nRowIndex * nCol + nColIndex + nPolarityIndex] = (pucBinBody[nIndex + i] & 3);
                nPolarityIndex++;
                pucRawData[nRowIndex * nCol + nColIndex + nPolarityIndex] = ((pucBinBody[nIndex + i] >> 2) & 3);
                nPolarityIndex++;
                pucRawData[nRowIndex * nCol + nColIndex + nPolarityIndex] = ((pucBinBody[nIndex + i] >> 4) & 3);
                nPolarityIndex++;
                pucRawData[nRowIndex * nCol + nColIndex + nPolarityIndex] = ((pucBinBody[nIndex + i] >> 6) & 3);
                nPolarityIndex++;
            }
        }
    }
    if (0 == memcmp(ucTail, pucBinBody + nIndex, sizeof(ucTail)))
    {
        *pnPos += ucHeadLength + nIndex + sizeof(ucTail);
        while (*pnPos < nBinLens && pucBinData[*pnPos] == 0xFF)
        {
            (* pnPos)++;
        }
        return true;
    }
    else
    {
        return false;
    }
}

