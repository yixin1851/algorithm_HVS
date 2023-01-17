#pragma once
#include <qdialog.h>
#include "ui_DialogAPSBLC.h"

class CAlpAPSMPAlgoInterface;
class CAlpDVSMPAlgoInterface;

class CDialogAPSBLC :
    public QDialog
{
    Q_OBJECT
public:
    CDialogAPSBLC(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void BLC();
private:
    Ui::DialogAPSBLC ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;

};

