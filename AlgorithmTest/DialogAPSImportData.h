#pragma once
#include <qdialog.h>
#include "ui_DialogAPSImportData.h"

class CAlpAPSMPAlgoInterface;
class CAlpDVSMPAlgoInterface;

class CDialogAPSImportData :
    public QDialog
{
    Q_OBJECT
public:
    CDialogAPSImportData(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void SingleBrowser();
    virtual void MultiBrowser();

    virtual void ImportData();

private:
    virtual void SingleImportData();
    virtual void MultiImportData();

private:
    Ui::DialogAPSImportData ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;

};

