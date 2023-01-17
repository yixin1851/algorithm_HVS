#pragma once
#include <qdialog.h>
#include "ui_DialogAPSShow.h"
#include "AlpMPAlgoInterface.h"
#include <qstandarditemmodel.h>

class CDialogAPSShow :
    public QDialog
{
    Q_OBJECT
public:
    CDialogAPSShow(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    void Show(int nIndex);
    void UpDateTable(int nIndex);
private:
    Ui::DialogAPSShow ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    QStandardItemModel *m_RawDataModel;
};

