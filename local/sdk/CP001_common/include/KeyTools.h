#ifndef __KEYTOOLS_H__
#define __KEYTOOLS_H__

#include <stddef.h>
#include <lumi_stdint.h>

bool CalculateKCV(u8* pKey, u16 nKeySize, u32 nBytesToCheck, u8* pKCV, int nAlgo);
bool CreateKeyCheckValue(u8* pKey, size_t nKey, size_t nZeros, size_t nVals, u8* pKCV, int nAlg);
u16 GetKCVTypeFromSlot(u16 nSlot, u16 nKeyMode);
u32 GetKCVTypeForVariableMode(u16 nKeyMode);
u16 GetKeySizeFromSlot(u16 nSlot, u16 nKeyMode);
u32 GetKeySizeForVariableMode(u16 nKeyMode);

#endif