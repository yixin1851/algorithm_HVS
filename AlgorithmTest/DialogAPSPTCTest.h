#pragma once
#include <qdialog.h>
#include "ui_DialogAPSPTC.h"
#include "AlpMPAlgoInterface.h"
#include "WidgetTableView.h"
#include "WidgetChartView.h"

typedef struct
{
    APSTNoiseType tnoise;
    APSDataMeanType datamean;
}PTCType;

class CDialogAPSPTC :
    public QDialog
{
    Q_OBJECT
public:
    CDialogAPSPTC(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void Export();
    virtual void PTC();
    virtual void MultiBrowser();
private:
    bool ReadFile(QDir DataDir, QStringList DataList, int nSaveIndex, int nFileIndex);
private:
    Ui::DialogAPSPTC ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    std::vector<PTCType> m_PTCData;
    CWidgetTableView m_widgetTableView[17];
    CWidgetChartView m_widgetChartView;
};
