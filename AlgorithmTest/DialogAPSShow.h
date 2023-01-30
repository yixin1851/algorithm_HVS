#pragma once
#include <qdialog.h>
#include "ui_DialogAPSShow.h"
#include "AlpMPAlgoInterface.h"
#include <qstandarditemmodel.h>
#include <QMenu>

class CDialogAPSShow :
    public QDialog
{
    Q_OBJECT
public:
    CDialogAPSShow(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    void Show(int nIndex);
    void UpDateTable(int nIndex);
    void on_tableView_customContextMenuRequested(const QPoint& pos);
    void MenuClicked(QAction* act);
private:
    Ui::DialogAPSShow ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    QStandardItemModel *m_RawDataModel;
    QMenu * m_CustomMenu;
    QAction* m_DispRowData;
    QAction* m_DispColData;
};

