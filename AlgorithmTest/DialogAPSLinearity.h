#pragma once
#include <qdialog.h>
#include "ui_DialogAPSLinearity.h"
#include "AlpMPAlgoInterface.h"
#include "WidgetTableView.h"
#include "WidgetChartView.h"

class CDialogAPSLinearity :
    public QDialog
{
    Q_OBJECT
public:
    CDialogAPSLinearity(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void Export();
    virtual void Linearity();
private:
    Ui::DialogAPSLinearity ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    APSLinearityType m_LinearityData;
    CWidgetTableView m_widgetTableView;
    CWidgetChartView m_widgetChartView;
};
