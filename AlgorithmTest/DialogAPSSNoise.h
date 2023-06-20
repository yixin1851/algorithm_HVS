#pragma once
#include <qdialog.h>
#include "ui_DialogAPSSNoise.h"
#include "AlpMPAlgoInterface.h"

class CDialogAPSSNoise :
    public QDialog
{
    Q_OBJECT
public:
    CDialogAPSSNoise(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void Export();
    virtual void SNoise();
private:
    Ui::DialogAPSSNoise ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    APSSNoiseType m_SNoiseData;
};
