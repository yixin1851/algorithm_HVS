#pragma once
#include <qdialog.h>
#include "ui_DialogDVSImportData.h"

class CAlpAPSMPAlgoInterface;
class CAlpDVSMPAlgoInterface;

class CDialogDVSImportData :
    public QDialog
{
    Q_OBJECT
public:
    CDialogDVSImportData(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);

private slots:
    virtual void Browser();
    virtual void ImportData();
private:
    Ui::DialogImportData ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;

};

