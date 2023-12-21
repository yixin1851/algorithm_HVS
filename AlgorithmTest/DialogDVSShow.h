#pragma once
#include <qdialog.h>
#include "ui_DialogDVSShow.h"
#include "AlpMPAlgoInterface.h"
#include <qstandarditemmodel.h>
#include <QMenu>

class CDialogDVSShow :
    public QDialog
{
    Q_OBJECT
public:
    CDialogDVSShow(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    void Show(int nIndex);
    void UpDateTable(int nIndex);
    void on_tableView_customContextMenuRequested(const QPoint& pos);
    void MenuClicked(QAction* act);
    void on_pushButtonExport_clicked();
protected:
    bool eventFilter(QObject* object, QEvent* event);
private:
    void CopySelectFromTable();
private:
    Ui::DialogDCVSShow ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    QStandardItemModel* m_RawDataModel;
    QMenu* m_CustomMenu;
    QAction* m_DispRowData;
    QAction* m_DispColData;
};

