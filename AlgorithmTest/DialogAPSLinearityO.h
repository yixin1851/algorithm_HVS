#pragma once
#include <qdialog.h>
#include "ui_DialogAPSLinearityO.h"
#include "AlpMPAlgoInterface.h"
#include "WidgetTableView.h"
#include "WidgetChartView.h"

class CDialogAPSLinearityO :
    public QDialog
{
    Q_OBJECT
public:
    CDialogAPSLinearityO(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void Export();
    virtual void Linearity();
private:
    Ui::DialogAPSLinearityO ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    APSSSNRType m_LinearityData;
    CWidgetTableView m_widgetTableView;
    CWidgetChartView m_widgetChartView;
};
