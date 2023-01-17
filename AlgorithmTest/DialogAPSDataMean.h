#pragma once
#include <qdialog.h>
#include "ui_DialogAPSDataMean.h"
#include "AlpMPAlgoInterface.h"

class CDialogAPSDataMean :
    public QDialog
{
    Q_OBJECT
public:
    CDialogAPSDataMean(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void Export();
    virtual void DataMean();
private:
    Ui::DialogAPSDataMean ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    std::vector<double> m_DataMean;
};
