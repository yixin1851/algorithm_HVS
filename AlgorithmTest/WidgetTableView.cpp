#include "WidgetTableView.h"
#include <qabstractitemview.h>

CWidgetTableView::CWidgetTableView(QWidget* parent)
	:QWidget(parent)
{
	ui.setupUi(this);
	ui.tableWidget->setFocusPolicy(Qt::NoFocus);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectItems);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void CWidgetTableView::SetData(QStringList RowName, QStringList ColName, std::vector<std::vector<double>> Data)
{
	uint32_t nRowNum = RowName.count();
	uint32_t nColNum = ColName.count();
	ui.tableWidget->setColumnCount(nColNum);
	ui.tableWidget->setRowCount(nRowNum);
	ui.tableWidget->setHorizontalHeaderLabels(ColName);
	ui.tableWidget->setVerticalHeaderLabels(RowName);
	for (uint32_t nRows = 0; nRows < nRowNum; nRows++)
	{
		for (uint32_t nCols = 0; nCols < nColNum; nCols++)
		{
			ui.tableWidget->setItem(nRows, nCols, new QTableWidgetItem(QString::number(Data[nRows][nCols])));
		}
	}
	ui.tableWidget->resizeColumnsToContents();
	ui.tableWidget->resizeRowsToContents();
}

void CWidgetTableView::Clear()
{
	ui.tableWidget->clear();
}

