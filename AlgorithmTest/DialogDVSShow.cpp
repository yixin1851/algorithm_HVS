#include "DialogDVSShow.h"
#include "WidgetChartView.h"
#include <fstream>

CDialogDVSShow::CDialogDVSShow(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);

	for (uint32_t nIndex = 0; nIndex < m_pDVSAlgoInterface->GetDataNum(); nIndex++)
	{
		ui.comboBoxIndex->addItem(QString::number(nIndex));
	}

	m_RawDataModel = new QStandardItemModel(this);
	ui.tableView->setModel(m_RawDataModel);
	uint32_t nRow = 0, nCol = 0;
	QStringList RowList, ColList;
	m_pDVSAlgoInterface->GetRawDataSize(nRow, nCol);
	for (uint32_t nRows = 1; nRows <= nRow; nRows++)
	{
		RowList << QString("%1").arg(nRows, 4, 10, QLatin1Char(' '));
	}
	for (uint32_t nCols = 1; nCols <= nCol; nCols++)
	{
		ColList << QString("%1").arg(nCols, 4, 10, QLatin1Char(' '));
	}
	m_RawDataModel->setHorizontalHeaderLabels(ColList);
	m_RawDataModel->setVerticalHeaderLabels(RowList);


	ui.tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	ui.tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	ui.tableView->setContextMenuPolicy(Qt::CustomContextMenu);
	ui.tableView->installEventFilter(this);

	m_CustomMenu = new QMenu(ui.tableView);
	m_DispRowData = new QAction(this);
	m_DispRowData->setText("Display Row Data");
	m_CustomMenu->addAction(m_DispRowData);
	m_DispColData = new QAction(this);
	m_DispColData->setText("Display Col Data");
	m_CustomMenu->addAction(m_DispColData);

	connect(ui.comboBoxIndex, SIGNAL(currentIndexChanged(int)), this, SLOT(Show(int)), Qt::QueuedConnection);
	connect(ui.comboBoxIndex, SIGNAL(currentIndexChanged(int)), this, SLOT(UpDateTable(int)), Qt::QueuedConnection);
	connect(m_CustomMenu, SIGNAL(triggered(QAction*)), this, SLOT(MenuClicked(QAction*)), Qt::QueuedConnection);

	emit(ui.comboBoxIndex->currentIndexChanged(0));
}

void CDialogDVSShow::Show(int nIndex)
{
	if (ui.tabWidget->currentIndex() != 0)
	{
		return;
	}
	ImgType OnEvents;
	ImgType OffEvents;

	clock_t time = 0;
	auto start = clock();
	bool bRet = m_pDVSAlgoInterface->Show(nIndex, 0, 255, 0, OnEvents) && m_pDVSAlgoInterface->Show(nIndex, 0, 0, 255, OffEvents);
	auto end = clock();
	time = end - start;

	if (bRet)
	{
		uint32_t nRow = OnEvents.size();
		uint32_t nCol = OnEvents[0].size();

		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);

		uint8_t* pImage = new uint8_t[3 * nRow * nCol];
		memset(pImage, 0, 3 * nRow * nCol);

		uint32_t nChannel = 0;

		for (uint32_t nRows = 0; nRows < nRow; nRows++)
		{
			for (uint32_t nCols = 0; nCols < nCol; nCols++)
			{
				pImage[nChannel] = OnEvents[nRows][nCols];
				pImage[nChannel + 1] = OffEvents[nRows][nCols];
				nChannel += 3;
			}
		}

		ui.widgetImageView->SetColorData(pImage, nCol, nRow);
		delete[] pImage;
	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));

		ui.widgetImageView->Clear();
	}
}

void CDialogDVSShow::UpDateTable(int nIndex)
{
	if (ui.tabWidget->currentIndex() != 1)
	{
		return;
	}
	ImgType RawData;
	uint32_t nCurIndex = ui.comboBoxIndex->currentIndex();
	clock_t time = 0;
	auto start = clock();
	bool bRet = m_pDVSAlgoInterface->Show(nCurIndex, 0, 2, 1, RawData);
	auto end = clock();
	time = end - start;
	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);

		uint32_t nRow = 0, nCol = 0;
		m_pDVSAlgoInterface->GetRawDataSize(nRow, nCol);
		m_RawDataModel->removeRows(0, nRow);
		for (uint32_t nRows = 0; nRows < nRow; nRows++)
		{
			QList<QStandardItem*> add_items;

			for (uint32_t nCols = 0; nCols < nCol; nCols++)
			{
				add_items << new QStandardItem(QString::number(RawData[nRows][nCols]));
			}

			m_RawDataModel->appendRow(add_items);
		}
	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));
		ui.tableView->setModel(nullptr);
	}

}

void CDialogDVSShow::on_tableView_customContextMenuRequested(const QPoint& pos)
{
	QModelIndex index = ui.tableView->indexAt(pos);
	if (index.isValid())
	{
		if (ui.tableView->selectionModel()->selectedRows().size() == 0)
		{
			m_DispRowData->setEnabled(false);
		}
		else
		{
			m_DispRowData->setEnabled(true);
		}
		if (ui.tableView->selectionModel()->selectedColumns().size() == 0)
		{
			m_DispColData->setEnabled(false);
		}
		else
		{
			m_DispColData->setEnabled(true);
		}
		m_CustomMenu->exec(QCursor::pos());
	}
}

void CDialogDVSShow::MenuClicked(QAction* act)
{
	CWidgetChartView* DataView = new CWidgetChartView(this);
	DataView->setWindowFlags(Qt::WindowCloseButtonHint | Qt::Dialog | Qt::WindowMinMaxButtonsHint);
	DataView->setWindowTitle("Line Data");
	if (act == m_DispRowData)
	{
		auto selectedRows = ui.tableView->selectionModel()->selectedRows();
		for (uint32_t nIndex = 0; nIndex < selectedRows.size(); nIndex++)
		{
			QVector<double> XData, YData;
			for (uint32_t nCols = 0; nCols < m_RawDataModel->columnCount(); nCols++)
			{
				XData.push_back(nCols + 1);
				YData.push_back(m_RawDataModel->item(selectedRows[nIndex].row(), nCols)->data(0).toDouble());
			}
			DataView->SetLine(std::to_string(selectedRows[nIndex].row() + 1), XData, YData);
		}
		DataView->show();
	}

	if (act == m_DispColData)
	{
		auto selectedCols = ui.tableView->selectionModel()->selectedColumns();
		for (uint32_t nIndex = 0; nIndex < selectedCols.size(); nIndex++)
		{
			QVector<double> XData, YData;
			for (uint32_t nRows = 0; nRows < m_RawDataModel->rowCount(); nRows++)
			{
				XData.push_back(nRows + 1);
				YData.push_back(m_RawDataModel->item(nRows, selectedCols[nIndex].column())->data(0).toDouble());
			}
			DataView->SetLine(std::to_string(selectedCols[nIndex].column() + 1), XData, YData);
		}
		DataView->show();
	}
}

void CDialogDVSShow::on_pushButtonExport_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "")
	{

		ImgType Img;
		uint32_t nCurIndex = ui.comboBoxIndex->currentIndex();
		if (m_pDVSAlgoInterface->Show(nCurIndex, 0, 2, 1, Img))
		{
			std::string strFile = dir.toLocal8Bit().toStdString() + "//dvs_16bit_" + std::to_string(Img.size()) + "_" + std::to_string(Img[0].size()) + ".raw";
			std::ofstream outfile;
			outfile.open(strFile, std::ios::binary | std::ios::trunc);
			if (!outfile.fail())
			{
				for (uint32_t nRow = 0; nRow < Img.size(); nRow++)
				{
					for (uint32_t nCol = 0; nCol < Img[0].size(); nCol++)
					{
						uint8_t a = Img[nRow][nCol];
						uint8_t b = 0;

						outfile << a;
						outfile << b;
					}
				}
				outfile.close();
			}
		}
	}


}

bool CDialogDVSShow::eventFilter(QObject* object, QEvent* event)
{
	if (object == ui.tableView)
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
	return QDialog::eventFilter(object, event);
}

void CDialogDVSShow::CopySelectFromTable()
{
	QModelIndexList indexList = ui.tableView->selectionModel()->selectedIndexes();
	if (indexList.isEmpty())
		return;
	int startRow = indexList.first().row();
	int endRow = indexList.last().row();
	int startCol = indexList.first().column();
	int endCol = indexList.last().column();

	//从tableview界面拿数据
	QStringList clipboardTextList;
	for (int i = startRow; i <= endRow; i++)
	{
		QStringList rowText;
		for (int j = startCol; j <= endCol; j++)
		{
			rowText.append(m_RawDataModel->data(m_RawDataModel->index(i, j)).toString());
		}
		clipboardTextList.append(rowText.join('\t'));
	}
	QString clipboardText = clipboardTextList.join('\n');

	//将数据放入剪贴板
	QApplication::clipboard()->setText(clipboardText);
}