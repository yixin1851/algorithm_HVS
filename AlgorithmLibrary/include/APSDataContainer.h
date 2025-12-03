#pragma once
#include <cstdint>
#include <vector>
typedef std::vector<std::vector<double>> APSType;

class CAPSDataContainer
{
public:
	CAPSDataContainer();
	void Init(uint32_t nRow, uint32_t nCol, bool bInitialize = false);
	virtual ~CAPSDataContainer();
	virtual CAPSDataContainer operator+(const CAPSDataContainer& rh);
	virtual CAPSDataContainer operator-(const CAPSDataContainer& rh);
	virtual CAPSDataContainer operator*(const CAPSDataContainer& rh);
	virtual CAPSDataContainer operator/(const CAPSDataContainer& rh);
	virtual CAPSDataContainer operator+(double rh);
	virtual CAPSDataContainer operator-(double rh);
	virtual CAPSDataContainer operator*(double rh);
	virtual CAPSDataContainer operator/(double rh);
	virtual CAPSDataContainer& operator+=(const CAPSDataContainer& rh);
	virtual CAPSDataContainer& operator-=(const CAPSDataContainer& rh);
	virtual CAPSDataContainer& operator*=(const CAPSDataContainer& rh);
	virtual CAPSDataContainer& operator/=(const CAPSDataContainer& rh);
	virtual CAPSDataContainer& operator+=(double rh);
	virtual CAPSDataContainer& operator-=(double rh);
	virtual CAPSDataContainer& operator*=(double rh);
	virtual CAPSDataContainer& operator/=(double rh);
	uint32_t m_nRow;
	uint32_t m_nCol;
	APSType m_RawData;
};