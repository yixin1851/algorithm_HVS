#pragma once
#include <qdialog>
#include "ui_DialogAPSInit.h"

class CAlpAPSMPAlgoInterface;
class CAlpDVSMPAlgoInterface;

class CDialogAPSInit :
    public QDialog
{
    Q_OBJECT
public:
    CDialogAPSInit(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
    virtual CAlpAPSMPAlgoInterface* GetAPSAlgoInterface() { return m_pAPSAlgoInterface; }

private slots:
    virtual void Init();
    virtual void Browser();
    virtual void RawDataInfoInit(int nIndex);
    virtual void ChangeUpDown();
    virtual void ChangeLeftRight();
private:
    Ui::DialogAPSInit ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
};

