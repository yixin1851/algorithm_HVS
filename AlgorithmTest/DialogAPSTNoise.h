#pragma once
#include <qdialog.h>
#include "ui_DialogAPSTNoise.h"
#include "AlpMPAlgoInterface.h"

class CDialogAPSTNoise :
    public QDialog
{
    Q_OBJECT
public:
    CDialogAPSTNoise(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void Export();
    virtual void TNoise();
private:
    Ui::DialogAPSTNoise ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    APSTNoiseType m_TNoiseData;
};
