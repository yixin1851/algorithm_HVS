#pragma once
#include <qdialog.h>
#include "ui_DialogAPSShading.h"
#include "AlpMPAlgoInterface.h"
#include "WidgetTableView.h"

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
    APSYShadingType m_YShadingData;
    APSColorShadingType m_ColorShadingData;
    APSOpticalCenterType m_OpticalCenterData;
    CWidgetTableView m_widgetTableView[4];
};
