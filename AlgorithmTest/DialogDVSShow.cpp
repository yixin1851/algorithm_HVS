#include "DialogDVSShow.h"

CDialogDVSShow::CDialogDVSShow(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);

	for (uint32_t nIndex = 0; nIndex < m_pDVSAlgoInterface->GetDataNum(); nIndex++)
	{
		ui.comboBoxIndex->addItem(QString::number(nIndex));
	}
	connect(ui.comboBoxIndex, SIGNAL(currentIndexChanged(int)), this, SLOT(Show(int)), Qt::QueuedConnection);

	emit(ui.comboBoxIndex->currentIndexChanged(0));
}

void CDialogDVSShow::Show(int nIndex)
{
	ImgType OnEvents;
	ImgType OffEvents;

	clock_t time = 0;
	auto start = clock();
	bool bRet = m_pDVSAlgoInterface->Show(nIndex, OnEvents, OffEvents);
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