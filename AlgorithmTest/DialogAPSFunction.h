#pragma once
#include <qdialog.h>
#include "ui_DialogAPSFunction.h"

class CAlpAPSMPAlgoInterface;
class CAlpDVSMPAlgoInterface;

class CDialogAPSFunction :
    public QDialog
{
    Q_OBJECT
public:
    CDialogAPSFunction(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
    void SetAPSAlgoInterface(CAlpAPSMPAlgoInterface* pAPSAlgoInterface);
    virtual ~CDialogAPSFunction();
private slots:
    void Init();
    void ImportData();
    void Show();
    void TNoise();
    void SNoise();
    void HotPixel();
    void BLC();
    void Shading();
    void BadPixel();
    void DarkCurrent();
    void DSNU();
    void Linearity();
    void OverallSystemGain();
    void DataMean();
    void Saturation();
    void Pedestal();
    void ReadNoise();
private:
    Ui::DialogAPSFunction ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;

};

