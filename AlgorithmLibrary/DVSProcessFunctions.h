#pragma once
#include "stdint.h"
#include "stdbool.h"

bool DVS_Decoder(uint8_t* pucBinData, uint8_t* pucRawData, size_t nRow, size_t nCol, size_t* pnPos, size_t nBinLens);
