#pragma once
#include <qdialog.h>
#include "ui_DialogAPSReadNoise.h"
#include "AlpMPAlgoInterface.h"

class CDialogAPSReadNoise :
    public QDialog
{
    Q_OBJECT
public:
    CDialogAPSReadNoise(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void Export();
    virtual void ReadNoise();
private:
    Ui::DialogAPSReadNoise ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    APSReadNoiseType m_ReadNoiseData;
};
