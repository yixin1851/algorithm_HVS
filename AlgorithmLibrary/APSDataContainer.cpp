#include "APSDataContainer.h"

CAPSDataContainer::CAPSDataContainer()
{
    m_nRow = 0;
    m_nCol = 0;
    m_RawData.clear();
}

void CAPSDataContainer::Init(uint32_t nRow, uint32_t nCol, bool bInitialize)
{
    m_RawData.resize(nRow);
    for (uint32_t rows = 0; rows < nRow; rows++)
    {
        if (bInitialize)
        {
            m_RawData[rows].resize(nCol, 0);
        }
        else
        {
            m_RawData[rows].resize(nCol);
        }
    }
    m_nRow = nRow;
    m_nCol = nCol;
}

CAPSDataContainer::~CAPSDataContainer()
{

}

CAPSDataContainer CAPSDataContainer::operator+(const CAPSDataContainer& rh)
{
    CAPSDataContainer res;
    res.Init(m_nRow, m_nCol);
    res.m_nRow = 0;
    res.m_nCol = 0;

    if (m_nRow != rh.m_nRow || m_nCol != rh.m_nCol)
    {
        return res;
    }
    for (uint32_t rows = 0; rows < m_nRow; rows++)
    {
        for (uint32_t cols = 0; cols < m_nCol; cols++)
        {
            res.m_RawData[rows][cols] = m_RawData[rows][cols] + rh.m_RawData[rows][cols];
        }
    }
    res.m_nRow = m_nRow;
    res.m_nCol = m_nCol;
    return res;
}

CAPSDataContainer& CAPSDataContainer::operator+=(const CAPSDataContainer& rh)
{
    if (m_nRow != rh.m_nRow || m_nCol != rh.m_nCol)
    {
        return *this;
    }
    for (uint32_t rows = 0; rows < m_nRow; rows++)
    {
        for (uint32_t cols = 0; cols < m_nCol; cols++)
        {
            m_RawData[rows][cols] += rh.m_RawData[rows][cols];
        }
    }
    return *this;
}

CAPSDataContainer CAPSDataContainer::operator-(const CAPSDataContainer& rh)
{
    CAPSDataContainer res;
    res.Init(m_nRow, m_nCol);
    res.m_nRow = 0;
    res.m_nCol = 0;
    if (m_nRow != rh.m_nRow || m_nCol != rh.m_nCol)
    {
        return res;
    }
    for (uint32_t rows = 0; rows < m_nRow; rows++)
    {
        for (uint32_t cols = 0; cols < m_nCol; cols++)
        {
            res.m_RawData[rows][cols] = m_RawData[rows][cols] - rh.m_RawData[rows][cols];
        }
    }
    res.m_nRow = m_nRow;
    res.m_nCol = m_nCol;
    return res;
}

CAPSDataContainer& CAPSDataContainer::operator-=(const CAPSDataContainer& rh)
{
    if (m_nRow != rh.m_nRow || m_nCol != rh.m_nCol)
    {
        return *this;
    }
    for (uint32_t rows = 0; rows < m_nRow; rows++)
    {
        for (uint32_t cols = 0; cols < m_nCol; cols++)
        {
            m_RawData[rows][cols] -= rh.m_RawData[rows][cols];
        }
    }
    return *this;
}

CAPSDataContainer& CAPSDataContainer::operator*=(const CAPSDataContainer& rh)
{
    if (m_nRow != rh.m_nRow || m_nCol != rh.m_nCol)
    {
        return *this;
    }
    for (uint32_t rows = 0; rows < m_nRow; rows++)
    {
        for (uint32_t cols = 0; cols < m_nCol; cols++)
        {
            m_RawData[rows][cols] *= rh.m_RawData[rows][cols];
        }
    }
    return *this;
}

CAPSDataContainer& CAPSDataContainer::operator/=(const CAPSDataContainer& rh)
{
    if (m_nRow != rh.m_nRow || m_nCol != rh.m_nCol)
    {
        return *this;
    }
    for (uint32_t rows = 0; rows < m_nRow; rows++)
    {
        for (uint32_t cols = 0; cols < m_nCol; cols++)
        {
            if (0 == rh.m_RawData[rows][cols])
            {
                m_nRow = 0;
                m_nCol = 0;
                return *this;
            }
            m_RawData[rows][cols] /= rh.m_RawData[rows][cols];
        }
    }
    return *this;
}

CAPSDataContainer& CAPSDataContainer::operator+=(double rh)
{
    for (uint32_t rows = 0; rows < m_nRow; rows++)
    {
        for (uint32_t cols = 0; cols < m_nCol; cols++)
        {
            m_RawData[rows][cols] += rh;
        }
    }
    return *this;
}

CAPSDataContainer& CAPSDataContainer::operator-=(double rh)
{
    for (uint32_t rows = 0; rows < m_nRow; rows++)
    {
        for (uint32_t cols = 0; cols < m_nCol; cols++)
        {
            m_RawData[rows][cols] -= rh;
        }
    }
    return *this;
}

CAPSDataContainer& CAPSDataContainer::operator*=(double rh)
{
    for (uint32_t rows = 0; rows < m_nRow; rows++)
    {
        for (uint32_t cols = 0; cols < m_nCol; cols++)
        {
            m_RawData[rows][cols] *= rh;
        }
    }
    return *this;
}

CAPSDataContainer& CAPSDataContainer::operator/=(double rh)
{
    if (0 == rh)
    {
        m_nRow = 0;
        m_nCol = 0;
        return *this;
    }
    for (uint32_t rows = 0; rows < m_nRow; rows++)
    {
        for (uint32_t cols = 0; cols < m_nCol; cols++)
        {
            m_RawData[rows][cols] /= rh;
        }
    }
    return *this;
}

CAPSDataContainer CAPSDataContainer::operator*(const CAPSDataContainer& rh)
{
    CAPSDataContainer res;
    res.Init(m_nRow, m_nCol);
    res.m_nRow = 0;
    res.m_nCol = 0;
    if (m_nRow != rh.m_nRow || m_nCol != rh.m_nCol)
    {
        return res;
    }
    for (uint32_t rows = 0; rows < m_nRow; rows++)
    {
        for (uint32_t cols = 0; cols < m_nCol; cols++)
        {
            res.m_RawData[rows][cols] = m_RawData[rows][cols] * rh.m_RawData[rows][cols];
        }
    }
    res.m_nRow = m_nRow;
    res.m_nCol = m_nCol;
    return res;
}

CAPSDataContainer CAPSDataContainer::operator/(const CAPSDataContainer& rh)
{
    CAPSDataContainer res;
    res.Init(m_nRow, m_nCol);
    res.m_nRow = 0;
    res.m_nCol = 0;
    if (m_nRow != rh.m_nRow || m_nCol != rh.m_nCol)
    {
        return res;
    }
    for (uint32_t rows = 0; rows < m_nRow; rows++)
    {
        for (uint32_t cols = 0; cols < m_nCol; cols++)
        {
            if (0 == rh.m_RawData[rows][cols])
            {
                return res;
            }
            res.m_RawData[rows][cols] = m_RawData[rows][cols] / rh.m_RawData[rows][cols];
        }
    }
    res.m_nRow = m_nRow;
    res.m_nCol = m_nCol;
    return res;
}

CAPSDataContainer CAPSDataContainer::operator+(double rh)
{
    CAPSDataContainer res;
    res.Init(m_nRow, m_nCol);
    res.m_nRow = 0;
    res.m_nCol = 0;
    for (uint32_t rows = 0; rows < m_nRow; rows++)
    {
        for (uint32_t cols = 0; cols < m_nCol; cols++)
        {
            res.m_RawData[rows][cols] = m_RawData[rows][cols] + rh;
        }
    }
    res.m_nRow = m_nRow;
    res.m_nCol = m_nCol;
    return res;
}

CAPSDataContainer CAPSDataContainer::operator-(double rh)
{
    CAPSDataContainer res;
    res.Init(m_nRow, m_nCol);
    res.m_nRow = 0;
    res.m_nCol = 0;
    for (uint32_t rows = 0; rows < m_nRow; rows++)
    {
        for (uint32_t cols = 0; cols < m_nCol; cols++)
        {
            res.m_RawData[rows][cols] = m_RawData[rows][cols] - rh;
        }
    }
    res.m_nRow = m_nRow;
    res.m_nCol = m_nCol;
    return res;
}

CAPSDataContainer CAPSDataContainer::operator*(double rh)
{
    CAPSDataContainer res;
    res.Init(m_nRow, m_nCol);
    res.m_nRow = 0;
    res.m_nCol = 0;
    for (uint32_t rows = 0; rows < m_nRow; rows++)
    {
        for (uint32_t cols = 0; cols < m_nCol; cols++)
        {
            res.m_RawData[rows][cols] = m_RawData[rows][cols] * rh;
        }
    }
    res.m_nRow = m_nRow;
    res.m_nCol = m_nCol;
    return res;
}

CAPSDataContainer CAPSDataContainer::operator/(double rh)
{
    CAPSDataContainer res;
    res.Init(m_nRow, m_nCol);
    res.m_nRow = 0;
    res.m_nCol = 0;
    if (rh == 0)
    {
        return res;
    }
    for (uint32_t rows = 0; rows < m_nRow; rows++)
    {
        for (uint32_t cols = 0; cols < m_nCol; cols++)
        {
            res.m_RawData[rows][cols] = m_RawData[rows][cols] / rh;
        }
    }
    res.m_nRow = m_nRow;
    res.m_nCol = m_nCol;
    return res;
}
