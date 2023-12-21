#include "WidgetTableView.h"
#include <qabstractitemview.h>
#include <qclipboard.h>
#include <qkeysequence.h>
#include <qevent.h>

CWidgetTableView::CWidgetTableView(QWidget* parent)
	:QWidget(parent)
{
	ui.setupUi(this);
	ui.tableWidget->setFocusPolicy(Qt::ClickFocus);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectItems);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.tableWidget->installEventFilter(this);
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

bool CWidgetTableView::eventFilter(QObject* object, QEvent* event)
{
    if (object == ui.tableWidget)
    {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->matches(QKeySequence::Copy)) {
                CopySelectFromTable();
                event->accept();
                return true;
            }
        }
    }
    return QWidget::eventFilter(object, event);
}

// 复制选中内容
void CWidgetTableView::CopySelectFromTable()
{
    QList<QTableWidgetSelectionRange> sRangeList = ui.tableWidget->selectedRanges();
    for (const auto& p : qAsConst(sRangeList)) {
        QString str;
        for (auto i = p.topRow(); i <= p.bottomRow(); i++) {
            QString rowStr;
            for (auto j = p.leftColumn(); j <= p.rightColumn(); j++) {
                QTableWidgetItem* item = ui.tableWidget->item(i, j);
                if (item != nullptr) {
                    if (j == p.leftColumn())
                        rowStr = item->text() + "\t";
                    else if (j == p.rightColumn())
                        rowStr = rowStr + item->text() + "\n";
                    else
                        rowStr = rowStr + item->text() + "\t";
                }
                else {
                    break;
                }
            }
            str += rowStr;
        }
        auto clipboard = QApplication::clipboard();
        clipboard->setText(str);
    }
}
