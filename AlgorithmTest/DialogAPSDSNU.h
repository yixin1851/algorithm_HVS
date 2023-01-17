#pragma once
#include <qdialog.h>
#include "ui_DialogAPSDSNU.h"
#include "AlpMPAlgoInterface.h"

class CDialogAPSDSNU :
    public QDialog
{
    Q_OBJECT
public:
    CDialogAPSDSNU(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void Export();
    virtual void DSNU();
private:
    Ui::DialogAPSDSNU ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    DSNUData m_DSNUData;
};
