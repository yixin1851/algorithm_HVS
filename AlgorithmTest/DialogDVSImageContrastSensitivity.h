#pragma once
#include <qdialog.h>
#include "ui_DialogDVSImageContrastSensitivity.h"
#include "AlpMPAlgoInterface.h"

class CDialogDVSImageContrastSensitivity :
    public QDialog
{
    Q_OBJECT
public:
    CDialogDVSImageContrastSensitivity(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void Export();
    virtual void ImageContrastSensitivity();
private:
    Ui::DialogDVSImageContrastSensitivityiformity ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    ImageContrastSensitivityData m_Data;
};
