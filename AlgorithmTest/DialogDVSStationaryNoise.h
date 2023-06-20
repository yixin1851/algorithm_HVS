#pragma once
#include <qdialog.h>
#include "ui_DialogDVSStationaryNoise.h"
#include "AlpMPAlgoInterface.h"

class CDialogStationaryNoise :
    public QDialog
{
    Q_OBJECT
public:
    CDialogStationaryNoise(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
    ~CDialogStationaryNoise();
private slots:
    virtual void Export();
    virtual void StationaryNoise();
private:
    Ui::DialogStationaryNoise ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    DVSStationaryNoiseType m_Data;
};
