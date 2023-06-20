#pragma once
#include <qdialog.h>
#include "ui_DialogAPSPedestal.h"
#include "AlpMPAlgoInterface.h"

class CDialogAPSPedestal :
    public QDialog
{
    Q_OBJECT
public:
    CDialogAPSPedestal(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void Export();
    virtual void Pedestal();
private:
    Ui::DialogAPSPedestal ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    APSPedestalVariationType m_Pedestal;
};
#pragma once
