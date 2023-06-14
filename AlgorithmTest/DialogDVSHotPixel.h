#pragma once
#include <qdialog.h>
#include "ui_DialogDVSHotPixel.h"
#include "AlpMPAlgoInterface.h"

class CDialogDVSHotPixel :
    public QDialog
{
    Q_OBJECT
public:
    CDialogDVSHotPixel(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void Export();
    virtual void HotPixel();
private:
    Ui::DialogHotPixel ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    DVSHotpixelData m_Data;
};
