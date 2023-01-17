#pragma once
#include <qdialog.h>
#include "ui_DialogAPSShading.h"
#include "AlpMPAlgoInterface.h"

class CDialogAPSShading :
    public QDialog
{
    Q_OBJECT
public:
    CDialogAPSShading(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void Export();
    virtual void Shading();
private:
    Ui::DialogAPSShading ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    ShadingData m_Data;
};
