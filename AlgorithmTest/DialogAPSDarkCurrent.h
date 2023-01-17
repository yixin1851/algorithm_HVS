#pragma once
#include <qdialog.h>
#include "ui_DialogAPSDarkCurrent.h"
#include "AlpMPAlgoInterface.h"
#include "WidgetTableView.h"
#include "WidgetChartView.h"

class CDialogAPSDarkCurrent :
    public QDialog
{
    Q_OBJECT
public:
    CDialogAPSDarkCurrent(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void Export();
    virtual void DarkCurrent();
private:
    Ui::DialogAPSDarkCurrent ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    std::vector<double> m_DarkCurrent;
    CWidgetTableView m_widgetTableView;
    CWidgetChartView m_widgetChartView;
};
