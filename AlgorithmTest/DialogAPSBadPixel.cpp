#include "DialogAPSBadPixel.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>

CDialogAPSBadPixel::CDialogAPSBadPixel(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);

	ui.lineEditIndexStart->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditNumber->setValidator(new QIntValidator(1, 100000, this));
	ui.lineEditROIUp->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditROIRight->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditROILeft->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditROIDown->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditBadPixelRadius->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditBadPixelThre->setValidator(new QDoubleValidator(0, 1, 3, this));
	ui.lineEditDeadPixelThre->setValidator(new QDoubleValidator(0, 1, 3, this));
	ui.lineEditDeadLineThre->setValidator(new QDoubleValidator(0, 1, 3, this));

	ui.lineEditROIUp->setEnabled(false);
	ui.lineEditROIRight->setEnabled(false);
	ui.lineEditROILeft->setEnabled(false);
	ui.lineEditROIDown->setEnabled(false);

	auto temp = m_pAPSAlgoInterface->GetActiveArea();
	ui.lineEditROIUp->setText(QString::number(temp.Up));
	ui.lineEditROILeft->setText(QString::number(temp.Left));
	ui.lineEditROIDown->setText(QString::number(temp.Down));
	ui.lineEditROIRight->setText(QString::number(temp.Right));

	ui.lineEditBadPixelRadius->setText(QString::number(m_pAPSAlgoInterface->GetAlgorithmThre().nBadPixelRadius));
	ui.lineEditBadPixelThre->setText(QString::number(m_pAPSAlgoInterface->GetAlgorithmThre().dBadPixelThre));
	ui.lineEditDeadPixelThre->setText(QString::number(m_pAPSAlgoInterface->GetAlgorithmThre().dDeadPixelThre));
	ui.lineEditDeadLineThre->setText(QString::number(m_pAPSAlgoInterface->GetAlgorithmThre().dDeadLineThre));

	ui.tabWidgetView->insertTab(0, &m_widgetTableView, "BadPixelResult");
	ui.tabWidgetView->insertTab(1, &m_widgetImageView[0], "Gb1");
	ui.tabWidgetView->insertTab(2, &m_widgetImageView[1], "Gb2");
	ui.tabWidgetView->insertTab(3, &m_widgetImageView[2], "B1");
	ui.tabWidgetView->insertTab(4, &m_widgetImageView[3], "B2");
	ui.tabWidgetView->insertTab(5, &m_widgetImageView[4], "R1");
	ui.tabWidgetView->insertTab(6, &m_widgetImageView[5], "R2");
	ui.tabWidgetView->insertTab(7, &m_widgetImageView[6], "Gr1");
	ui.tabWidgetView->insertTab(8, &m_widgetImageView[7], "Gr2");

	connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(Export()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(BadPixel()), Qt::QueuedConnection);
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIUp, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIDown, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROILeft, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIRight, SLOT(setEnabled(bool)));

}


void CDialogAPSBadPixel::BadPixel()
{
	bool bRet = true;
	uint32_t nIndexStart = ui.lineEditIndexStart->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();
	ui.label_Res->setText(tr(" "));
	ui.pushButtonStart->setEnabled(false);
	m_widgetTableView.Clear();

	for (uint32_t nIndex = 0; nIndex < APSSubFrameIndex::SubFrameNum; nIndex++)
	{
		m_widgetImageView[nIndex].Clear();
	}

	ROIArea* roi = nullptr;
	ROIArea tempROI;

	auto tempThre = m_pAPSAlgoInterface->GetAlgorithmThre();
	tempThre.dBadPixelThre = ui.lineEditBadPixelThre->text().toDouble();
	tempThre.dDeadPixelThre = ui.lineEditDeadPixelThre->text().toDouble();
	tempThre.dDeadLineThre = ui.lineEditDeadLineThre->text().toDouble();
	tempThre.nBadPixelRadius = ui.lineEditBadPixelRadius->text().toUInt();
	m_pAPSAlgoInterface->SetAlgorithmThre(tempThre);

	if (ui.checkBoxROI->isChecked())
	{
		tempROI.Up = ui.lineEditROIUp->text().toUInt();
		tempROI.Down = ui.lineEditROIDown->text().toUInt();
		tempROI.Left = ui.lineEditROILeft->text().toUInt();
		tempROI.Right = ui.lineEditROIRight->text().toUInt();
		roi = &tempROI;
	}

	clock_t time = 0;
	if (!ui.checkBoxDPC->isChecked())
	{
		auto start = clock();
		bRet = m_pAPSAlgoInterface->BadPixel(nIndexStart, nNumber, roi, m_BadPixel);
		auto end = clock();
		time = end - start;
	}
	else
	{
		bRet = m_pAPSAlgoInterface->BadPixel(nIndexStart, nNumber, roi, m_BadPixel);
		if (bRet)
		{
			std::vector<BadPixelMaskData> MaskData(APSSubFrameIndex::SubFrameNum);
			for (uint32_t nIndex = 0; nIndex < APSSubFrameIndex::SubFrameNum; nIndex++)
			{
				MaskData[nIndex] = m_BadPixel[nIndex].BadPixelMask;
			}
			auto start = clock();
			bRet = m_pAPSAlgoInterface->DPC(nIndexStart, nNumber, roi, MaskData);
			auto end = clock();
			time = end - start;
		}
	}
	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);

		QStringList RowName, ColName;
		RowName << "BadPixelNum" << "SingletNum" << "CoupletNum" << "ClusterNum" << "DeadPixelNum" << "DeadLineNum";
		ColName << "Gb1" << "Gb2" << "B1" << "B2" << "R1" << "R2" << "Gr1" << "Gr2";

		std::vector<std::vector<double>> Data(6);

		for (uint32_t nIndex = 0; nIndex < APSSubFrameIndex::SubFrameNum; nIndex++)
		{
			Data[0].push_back(m_BadPixel[nIndex].BadPixelNum);
			Data[1].push_back(m_BadPixel[nIndex].SingletNum);
			Data[2].push_back(m_BadPixel[nIndex].CoupletNum);
			Data[3].push_back(m_BadPixel[nIndex].ClusterNum);
			Data[4].push_back(m_BadPixel[nIndex].DeadPixelNum);
			Data[5].push_back(m_BadPixel[nIndex].DeadLineNum);
		}

		m_widgetTableView.SetData(RowName, ColName, Data);

		uint32_t nRow = 0, nCol = 0;
		m_pAPSAlgoInterface->GetRawDataSize(nRow, nCol);

		uint8_t* pImage = new uint8_t[nRow * nCol];
		for (uint32_t nChannel = 0; nChannel < APSSubFrameIndex::SubFrameNum; nChannel++)
		{
			memset(pImage, 0, nRow * nCol);

			for (uint32_t nIndex = 0; nIndex < m_BadPixel[nChannel].BadPixelMask.BadPixelNum; nIndex++)
			{
				pImage[m_BadPixel[nChannel].BadPixelMask.LocalData[nIndex].x * nCol + m_BadPixel[nChannel].BadPixelMask.LocalData[nIndex].y] = 255;
			}
			m_widgetImageView[nChannel].SetGrayData(pImage, nCol, nRow);
		}
		delete[] pImage;

	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));
	}
	ui.pushButtonStart->setEnabled(true);
}

void CDialogAPSBadPixel::Export()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "")
	{
		std::string strFile = dir.toStdString() + "//BadPixel.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			outfile << "Gb1,Gb2,B1,B2,R1,R2,Gr1,Gr2" << std::endl;
			for (uint32_t nIndex = 0; nIndex < m_BadPixel.size(); nIndex++)
			{
				outfile << std::to_string(m_BadPixel[nIndex].BadPixelNum) << ",";
			}
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < m_BadPixel.size(); nIndex++)
			{
				outfile << std::to_string(m_BadPixel[nIndex].SingletNum) << ",";
			}
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < m_BadPixel.size(); nIndex++)
			{
				outfile << std::to_string(m_BadPixel[nIndex].CoupletNum) << ",";
			}
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < m_BadPixel.size(); nIndex++)
			{
				outfile << std::to_string(m_BadPixel[nIndex].ClusterNum) << ",";
			}
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < m_BadPixel.size(); nIndex++)
			{
				outfile << std::to_string(m_BadPixel[nIndex].DeadPixelNum) << ",";
			}
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < m_BadPixel.size(); nIndex++)
			{
				outfile << std::to_string(m_BadPixel[nIndex].DeadLineNum) << ",";
			}
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < m_BadPixel.size(); nIndex++)
			{
				for (uint32_t i = 0; i < m_BadPixel[nIndex].BadPixelMask.BadPixelNum; i++)
				{
					outfile << std::to_string(m_BadPixel[nIndex].BadPixelMask.LocalData[i].x) << ",";
				}
				outfile << std::endl;
				for (uint32_t i = 0; i < m_BadPixel[nIndex].BadPixelMask.BadPixelNum; i++)
				{
					outfile << std::to_string(m_BadPixel[nIndex].BadPixelMask.LocalData[i].y) << ",";
				}
				outfile << std::endl;
				for (uint32_t i = 0; i < m_BadPixel[nIndex].BadPixelMask.BadPixelNum; i++)
				{
					outfile << std::to_string(m_BadPixel[nIndex].BadPixelMask.Flag[i]) << ",";
				}
				outfile << std::endl;
			}

			outfile.close();
		}
	}
}