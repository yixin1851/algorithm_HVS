#pragma once
#include <qdialog.h>
#include "ui_DialogAPSSaturation.h"
#include "AlpMPAlgoInterface.h"

class CDialogAPSSaturation :
    public QDialog
{
    Q_OBJECT
public:
    CDialogAPSSaturation(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void Export();
    virtual void Saturation();
private:
    Ui::DialogAPSSaturation ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    SaturationData m_Data;
};
