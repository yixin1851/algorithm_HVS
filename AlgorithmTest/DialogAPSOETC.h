#pragma once
#include <qdialog.h>
#include "ui_DialogAPSOETC.h"
#include "AlpMPAlgoInterface.h"
#include "WidgetTableView.h"
#include "WidgetChartView.h"

class CDialogAPSOETC :
    public QDialog
{
    Q_OBJECT
public:
    CDialogAPSOETC(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void Export();
    virtual void OETC();
private:
    Ui::DialogAPSOETC ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    APSOETCType m_OETCData;
    CWidgetTableView m_widgetTableView;
    CWidgetChartView m_widgetChartView;
    CWidgetTableView m_widgetTableDataView;
};
