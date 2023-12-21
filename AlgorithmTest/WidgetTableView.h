#pragma once
#include <qwidget.h>
#include "ui_WidgetTableView.h"
#include <vector>

class CWidgetTableView :
    public QWidget
{
    Q_OBJECT

public:
    CWidgetTableView(QWidget* parent = nullptr);
    void SetData(QStringList RowName, QStringList ColName, std::vector<std::vector<double>> Data);
    void Clear();
protected:
    bool eventFilter(QObject* object, QEvent* event);
private:
    void CopySelectFromTable();
private:
    Ui::TableViewWidget ui;
};

