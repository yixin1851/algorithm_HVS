#pragma once
#include <qdialog.h>
#include "ui_DialogDVSSpatialResponseUniformity.h"
#include "AlpMPAlgoInterface.h"

class CDialogDVSSpatialResponseUniformity :
    public QDialog
{
    Q_OBJECT
public:
    CDialogDVSSpatialResponseUniformity(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void Export();
    virtual void SpatialResponseUniformity();
private:
    Ui::DialogDVSSpatialResponseUniformity ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    DVSSpatialResponseUniformityType m_Data;
};
