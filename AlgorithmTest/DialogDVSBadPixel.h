#pragma once
#include <qdialog.h>
#include "ui_DialogDVSBadPixel.h"
#include "AlpMPAlgoInterface.h"

class CDialogDVSBadPixel :
    public QDialog
{
    Q_OBJECT
public:
    CDialogDVSBadPixel(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void Export();
    virtual void BadPixel();
private:
    Ui::DialogDVSBadPixel ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    DVSBadpixelType m_Data;
};
