#include "DialogAPSShow.h"
#include <QStringList>

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

	connect(ui.comboBoxIndex, SIGNAL(currentIndexChanged(int)), this, SLOT(Show(int)), Qt::QueuedConnection);
	connect(ui.comboBoxChannel, SIGNAL(currentIndexChanged(int)), this, SLOT(Show(int)), Qt::QueuedConnection);
	connect(ui.checkBoxNormalize, SIGNAL(stateChanged(int)), this, SLOT(Show(int)), Qt::QueuedConnection);
	connect(ui.comboBoxIndex, SIGNAL(currentIndexChanged(int)), this, SLOT(UpDateTable(int)), Qt::QueuedConnection);
	connect(ui.comboBoxChannel, SIGNAL(currentIndexChanged(int)), this, SLOT(UpDateTable(int)), Qt::QueuedConnection);

	emit(ui.comboBoxIndex->currentIndexChanged(0));
}

void CDialogAPSShow::UpDateTable(int nIndex)
{
	APSType RawData;
	uint32_t nCurIndex = ui.comboBoxIndex->currentIndex();
	uint32_t nCurChannel = ui.comboBoxChannel->currentIndex();
	clock_t time = 0;
	auto start = clock();
	bool bRet = m_pAPSAlgoInterface->Show(nCurIndex, 1, nullptr, APSSubFrameIndex(nCurChannel), RawData);
	auto end = clock();
	time = end - start;
	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);

		uint32_t nRow = 0, nCol = 0;
		m_pAPSAlgoInterface->GetRawDataSize(nRow, nCol);

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
	bool bRet = m_pAPSAlgoInterface->Show(nCurIndex, 1, nullptr, APSSubFrameIndex(nCurChannel), bNormal, Img);
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