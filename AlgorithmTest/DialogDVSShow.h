#pragma once
#include <qdialog.h>
#include "ui_DialogDVSShow.h"
#include "AlpMPAlgoInterface.h"

class CDialogDVSShow :
    public QDialog
{
    Q_OBJECT
public:
    CDialogDVSShow(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    void Show(int nIndex);

private:
    Ui::DialogDCVSShow ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;

};

