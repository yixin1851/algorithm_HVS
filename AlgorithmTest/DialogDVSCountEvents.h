#pragma once
#include <qdialog.h>
#include "ui_DialogDVSCountEvents.h"
#include "AlpMPAlgoInterface.h"

class CDialogCountEvents :
    public QDialog
{
    Q_OBJECT
public:
    CDialogCountEvents(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
    ~CDialogCountEvents();
private slots:
    virtual void Export();
    virtual void CountEvents();
private:
    Ui::DialogCountEvents ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    DVSEventsNumberCountType m_Data;
};
