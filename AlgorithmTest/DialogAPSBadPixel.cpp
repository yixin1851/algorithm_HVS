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
	ui.lineEditBadLineRadius->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditBadPixelThre->setValidator(new QDoubleValidator(0, 1, 3, this));
	ui.lineEditBadLineThre->setValidator(new QDoubleValidator(0, 1, 3, this));

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
	ui.lineEditBadLineRadius->setText(QString::number(m_pAPSAlgoInterface->GetAlgorithmThre().nBadLineRadius));
	ui.lineEditBadLineThre->setText(QString::number(m_pAPSAlgoInterface->GetAlgorithmThre().dBadLineThre));

	ui.tabWidgetView->insertTab(0, &m_widgetTableView, "BadPixelResult");
	ui.tabWidgetView->insertTab(1, &m_widgetImageView[0], "Gb");
	ui.tabWidgetView->insertTab(2, &m_widgetImageView[1], "B");
	ui.tabWidgetView->insertTab(3, &m_widgetImageView[2], "R");
	ui.tabWidgetView->insertTab(4, &m_widgetImageView[3], "Gr");

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

	for (uint32_t nIndex = 0; nIndex < SubFrameIndex::All; nIndex++)
	{
		m_widgetImageView[nIndex].Clear();
	}

	ROIArea* roi = nullptr;
	ROIArea tempROI;

	auto tempThre = m_pAPSAlgoInterface->GetAlgorithmThre();
	tempThre.dBadPixelThre = ui.lineEditBadPixelThre->text().toDouble();
	tempThre.dBadLineThre = ui.lineEditBadLineThre->text().toDouble();
	tempThre.nBadLineRadius = ui.lineEditBadLineRadius->text().toUInt();
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
			auto start = clock();
			std::vector<Local> PixelsInfo;
			for (int i = 0; i < m_BadPixel.BadPixelMask.BadPixelNum; i++)
			{
				if (APX003CA_ON_CHIP_CALIBRATION_FLAG == m_BadPixel.BadPixelMask.Flag[i])
				{
					PixelsInfo.push_back(m_BadPixel.BadPixelMask.LocalData[i]);
				}
			}
			bRet = m_pAPSAlgoInterface->DPC(nIndexStart, nNumber, roi, PixelsInfo);
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
		RowName << "BadPixelNum" << "SingletNum" << "CoupletNum" << "ClusterNum" << "LadderNum" << "DefectRowNum" << "DefectColNum";
		ColName << "Gb" << "B" << "R" << "Gr" << "Total";

		std::vector<std::vector<double>> Data(7);

		for (uint32_t nIndex = 0; nIndex < SubFrameIndex::All; nIndex++)
		{
			Data[0].push_back(m_BadPixel.SubFrameBadpixelData[nIndex].BadPixelNum);
			Data[1].push_back(m_BadPixel.SubFrameBadpixelData[nIndex].SingletNum);
			Data[2].push_back(m_BadPixel.SubFrameBadpixelData[nIndex].CoupletNum);
			Data[3].push_back(m_BadPixel.SubFrameBadpixelData[nIndex].ClusterNum);
			Data[4].push_back(0);
			Data[5].push_back(m_BadPixel.SubFrameBadpixelData[nIndex].DefectRowNum);
			Data[6].push_back(m_BadPixel.SubFrameBadpixelData[nIndex].DefectColNum);
		}
		Data[0].push_back(m_BadPixel.BadPixelNum);
		Data[1].push_back(m_BadPixel.SingletNum);
		Data[2].push_back(m_BadPixel.CoupletNum);
		Data[3].push_back(m_BadPixel.ClusterNum);
		Data[4].push_back(m_BadPixel.LadderNum);
		Data[5].push_back(0);
		Data[6].push_back(0);
		m_widgetTableView.SetData(RowName, ColName, Data);

		uint32_t nRow = 0, nCol = 0;
		m_pAPSAlgoInterface->GetRawDataSize(nRow, nCol);
		nRow /= 2;
		nCol /= 2;

		uint8_t* pImage = new uint8_t[nRow * nCol];
		for (uint32_t nChannel = 0; nChannel < SubFrameIndex::All; nChannel++)
		{
			memset(pImage, 0, nRow * nCol);

			for (uint32_t nIndex = 0; nIndex < m_BadPixel.SubFrameBadpixelData[nChannel].BadPixelMask.BadPixelNum; nIndex++)
			{
				pImage[m_BadPixel.SubFrameBadpixelData[nChannel].BadPixelMask.LocalData[nIndex].x * nCol + m_BadPixel.SubFrameBadpixelData[nChannel].BadPixelMask.LocalData[nIndex].y] = 255;
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
		std::string strFile = dir.toLocal8Bit().toStdString() + "//BadPixel.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			outfile << "Gb,B,R,Gr,Total" << std::endl;
			for (uint32_t nIndex = 0; nIndex < SubFrameIndex::All; nIndex++)
			{
				outfile << std::to_string(m_BadPixel.SubFrameBadpixelData[nIndex].BadPixelNum) << ",";
			}
			outfile << std::to_string(m_BadPixel.BadPixelNum) << ",";
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < SubFrameIndex::All; nIndex++)
			{
				outfile << std::to_string(m_BadPixel.SubFrameBadpixelData[nIndex].SingletNum) << ",";
			}
			outfile << std::to_string(m_BadPixel.SingletNum) << ",";
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < SubFrameIndex::All; nIndex++)
			{
				outfile << std::to_string(m_BadPixel.SubFrameBadpixelData[nIndex].CoupletNum) << ",";
			}
			outfile << std::to_string(m_BadPixel.CoupletNum) << ",";
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < SubFrameIndex::All; nIndex++)
			{
				outfile << std::to_string(m_BadPixel.SubFrameBadpixelData[nIndex].ClusterNum) << ",";
			}
			outfile << std::to_string(m_BadPixel.ClusterNum) << ",";
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < SubFrameIndex::All; nIndex++)
			{
				outfile << std::to_string(m_BadPixel.SubFrameBadpixelData[nIndex].DefectRowNum) << ",";
			}
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < SubFrameIndex::All; nIndex++)
			{
				outfile << std::to_string(m_BadPixel.SubFrameBadpixelData[nIndex].DefectColNum) << ",";
			}
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < SubFrameIndex::All; nIndex++)
			{
				for (uint32_t i = 0; i < m_BadPixel.SubFrameBadpixelData[nIndex].BadPixelMask.BadPixelNum; i++)
				{
					outfile << std::to_string(m_BadPixel.SubFrameBadpixelData[nIndex].BadPixelMask.LocalData[i].x) << ",";
				}
				outfile << std::endl;
				for (uint32_t i = 0; i < m_BadPixel.SubFrameBadpixelData[nIndex].BadPixelMask.BadPixelNum; i++)
				{
					outfile << std::to_string(m_BadPixel.SubFrameBadpixelData[nIndex].BadPixelMask.LocalData[i].y) << ",";
				}
				outfile << std::endl;
				for (uint32_t i = 0; i < m_BadPixel.SubFrameBadpixelData[nIndex].BadPixelMask.BadPixelNum; i++)
				{
					outfile << std::to_string(m_BadPixel.SubFrameBadpixelData[nIndex].BadPixelMask.Flag[i]) << ",";
				}
				outfile << std::endl;
			}

			for (uint32_t i = 0; i < m_BadPixel.BadPixelMask.BadPixelNum; i++)
			{
				outfile << std::to_string(m_BadPixel.BadPixelMask.LocalData[i].x) << ",";
			}
			outfile << std::endl;
			for (uint32_t i = 0; i < m_BadPixel.BadPixelMask.BadPixelNum; i++)
			{
				outfile << std::to_string(m_BadPixel.BadPixelMask.LocalData[i].y) << ",";
			}
			outfile << std::endl;
			for (uint32_t i = 0; i < m_BadPixel.BadPixelMask.BadPixelNum; i++)
			{
				outfile << std::to_string(m_BadPixel.BadPixelMask.Flag[i]) << ",";
			}
			outfile << std::endl;

			outfile.close();
		}
	}
}