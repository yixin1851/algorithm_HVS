#pragma once
#include <qdialog.h>
#include "ui_DialogDVSFindPeak.h"
#include "AlpMPAlgoInterface.h"

class CDialogDVSFindPeak :
    public QDialog
{
    Q_OBJECT
public:
    CDialogDVSFindPeak(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void Export();
    virtual void FindPeak();
private:
    Ui::DialogFindPeak ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    PeakInfo m_Data;
};
