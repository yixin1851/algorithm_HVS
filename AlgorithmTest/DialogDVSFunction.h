#pragma once
#include <qdialog.h>
#include "ui_DialogDVSFunction.h"

class CAlpAPSMPAlgoInterface;
class CAlpDVSMPAlgoInterface;

class CDialogDVSFunction :
    public QDialog
{
    Q_OBJECT
public:
    CDialogDVSFunction(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
    void SetDVSAlgoInterface(CAlpDVSMPAlgoInterface* pDVSAlgoInterface);
    virtual ~CDialogDVSFunction();
private slots:
    void Init();
    void ImportData();
    void CountEvents();
    void StationaryNoise();
    void StationaryUniformity();
    void HotPixel();
    void FindPeak();
    void ImageContrastSensitivity();
    void AccompaniedPeakAndDelayedPeak();
    void SpatialResponseUniformity();
    void BadPixel();
    void Show();

private:
    Ui::DialogDVSFunction ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;

};

