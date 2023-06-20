#pragma once
#include <qdialog.h>
#include "ui_DialogDVSAccompaniedPeakAndDelayedPeak.h"
#include "AlpMPAlgoInterface.h"

class CDialogDVSAccompaniedPeakAndDelayedPeak :
    public QDialog
{
    Q_OBJECT
public:
    CDialogDVSAccompaniedPeakAndDelayedPeak(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void Export();
    virtual void AccompaniedPeakAndDelayedPeak();
private:
    Ui::DialogDVSAccompaniedPeakAndDelayedPeak ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    DVSAccompaniedPeakAndDelayedPeakType m_Data;
};
