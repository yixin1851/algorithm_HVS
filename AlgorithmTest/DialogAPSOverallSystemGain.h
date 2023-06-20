#pragma once
#include <qdialog.h>
#include "ui_DialogAPSOverallSystemGain.h"
#include "AlpMPAlgoInterface.h"
#include "WidgetTableView.h"
#include "WidgetChartView.h"

class CDialogAPSOverallSystemGain :
    public QDialog
{
    Q_OBJECT
public:
    CDialogAPSOverallSystemGain(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void Export();
    virtual void OverallSystemGain();
private:
    Ui::DialogAPSOverallSystemGain ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    APSOverallSystemGainType m_GainK;
    CWidgetTableView m_widgetTableView;
    CWidgetChartView m_widgetChartView;
};
