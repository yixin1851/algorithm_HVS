#pragma once
#include <qdialog>
#include "ui_DialogDVSInit.h"

class CAlpAPSMPAlgoInterface;
class CAlpDVSMPAlgoInterface;

class CWidgetDVSInit :
    public QDialog
{
    Q_OBJECT
public:
    CWidgetDVSInit(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
    virtual CAlpDVSMPAlgoInterface* GetDVSAlgoInterface() { return m_pDVSAlgoInterface; }

private slots:
    virtual void Init();
    virtual void Browser();
private:
    Ui::DialogInit ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
};

