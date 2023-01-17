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
private:
    Ui::TableViewWidget ui;
};

