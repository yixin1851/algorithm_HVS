#include "DialogAPSShow.h"
#include <QStringList>
#include "WidgetChartView.h"

CDialogAPSShow::CDialogAPSShow(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);

	for (uint32_t nIndex = 0; nIndex < m_pAPSAlgoInterface->GetDataNum(); nIndex++)
	{
		ui.comboBoxIndex->addItem(QString::number(nIndex));
	}

	m_RawDataModel = new QStandardItemModel(this);
	ui.tableView->setModel(m_RawDataModel);
	uint32_t nRow = 0, nCol = 0;
	QStringList RowList, ColList;
	m_pAPSAlgoInterface->GetRawDataSize(nRow, nCol);
	nRow /= 2;
	nCol /= 2;
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

	m_CustomMenu = new QMenu(ui.tableView);
	m_DispRowData = new QAction(this);
	m_DispRowData->setText("Display Row Data");
	m_CustomMenu->addAction(m_DispRowData);
	m_DispColData = new QAction(this);
	m_DispColData->setText("Display Col Data");
	m_CustomMenu->addAction(m_DispColData);

	connect(ui.comboBoxIndex, SIGNAL(currentIndexChanged(int)), this, SLOT(Show(int)), Qt::QueuedConnection);
	connect(ui.comboBoxChannel, SIGNAL(currentIndexChanged(int)), this, SLOT(Show(int)), Qt::QueuedConnection);
	connect(ui.checkBoxNormalize, SIGNAL(stateChanged(int)), this, SLOT(Show(int)), Qt::QueuedConnection);
	connect(ui.comboBoxIndex, SIGNAL(currentIndexChanged(int)), this, SLOT(UpDateTable(int)), Qt::QueuedConnection);
	connect(ui.comboBoxChannel, SIGNAL(currentIndexChanged(int)), this, SLOT(UpDateTable(int)), Qt::QueuedConnection);
	connect(m_CustomMenu, SIGNAL(triggered(QAction *)), this, SLOT(MenuClicked(QAction *)), Qt::QueuedConnection);

	emit(ui.comboBoxIndex->currentIndexChanged(0));
}

void CDialogAPSShow::UpDateTable(int nIndex)
{
	APSType RawData;
	uint32_t nCurIndex = ui.comboBoxIndex->currentIndex();
	uint32_t nCurChannel = ui.comboBoxChannel->currentIndex();
	clock_t time = 0;
	auto start = clock();
	bool bRet = m_pAPSAlgoInterface->Show(nCurIndex, 1, nullptr, SubFrameIndex(nCurChannel), RawData);
	auto end = clock();
	time = end - start;
	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);

		uint32_t nRow = 0, nCol = 0;
		m_pAPSAlgoInterface->GetRawDataSize(nRow, nCol);
		nRow /= 2;
		nCol /= 2;
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

void CDialogAPSShow::Show(int nIndex)
{
	ImgType Img;
	uint32_t nCurIndex = ui.comboBoxIndex->currentIndex();
	uint32_t nCurChannel = ui.comboBoxChannel->currentIndex();
	bool bNormal = ui.checkBoxNormalize->isChecked();
	clock_t time = 0;
	auto start = clock();
	bool bRet = m_pAPSAlgoInterface->Show(nCurIndex, 1, nullptr, SubFrameIndex(nCurChannel), bNormal, Img);
	auto end = clock();
	time = end - start;

	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);

		uint32_t nRow = Img.size();
		uint32_t nCol = Img[0].size();

		uint8_t* pImage = new uint8_t[nRow * nCol];
		memset(pImage, 0, nRow * nCol);

		uint32_t nChannel = 0;

		for (uint32_t nRows = 0; nRows < nRow; nRows++)
		{
			for (uint32_t nCols = 0; nCols < nCol; nCols++)
			{
				pImage[nChannel] = Img[nRows][nCols];
				++nChannel;
			}
		}

		ui.widgetImageView->SetGrayData(pImage, nCol, nRow);
		delete[] pImage;
	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));

		ui.widgetImageView->Clear();
	}
}

void CDialogAPSShow::on_tableView_customContextMenuRequested(const QPoint& pos)
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

void CDialogAPSShow::MenuClicked(QAction* act)
{
	CWidgetChartView *DataView = new CWidgetChartView(this);
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
			DataView->SetLine(std::to_string(selectedRows[nIndex].row()+1), XData, YData);
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
			DataView->SetLine(std::to_string(selectedCols[nIndex].column()+1), XData, YData);
		}
		DataView->show();
	}
}
