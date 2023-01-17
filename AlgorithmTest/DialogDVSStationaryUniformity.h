#pragma once
#include <qdialog.h>
#include "ui_DialogDVSStationaryUniformity.h"
#include "AlpMPAlgoInterface.h"

class CDialogStationaryUniformity :
    public QDialog
{
    Q_OBJECT
public:
    CDialogStationaryUniformity(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void Export();
    virtual void StationaryUniformity();
private:
    Ui::DialogStationaryUniformity ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    StationaryUniformityData m_Data;
};
