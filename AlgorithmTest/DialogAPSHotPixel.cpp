#include "DialogAPSHotPixel.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>

CDialogAPSHotPixel::CDialogAPSHotPixel(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
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
	ui.lineEditHotLineRadius->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditHotPixelThre->setValidator(new QDoubleValidator(0, 1, 3, this));
	ui.lineEditHotLineThre->setValidator(new QDoubleValidator(0, 1, 3, this));

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
	ui.lineEditHotLineRadius->setText(QString::number(m_pAPSAlgoInterface->GetAlgorithmThre().nBadLineRadius));
	ui.lineEditHotPixelThre->setText(QString::number(m_pAPSAlgoInterface->GetAlgorithmThre().dHotPixelThre));
	ui.lineEditHotLineThre->setText(QString::number(m_pAPSAlgoInterface->GetAlgorithmThre().dHotLineThre));

	ui.tabWidgetView->insertTab(0, &m_widgetTableView, "HotPixelResult");
	ui.tabWidgetView->insertTab(1, &m_widgetImageView[0], "Gb");
	ui.tabWidgetView->insertTab(2, &m_widgetImageView[1], "B");
	ui.tabWidgetView->insertTab(3, &m_widgetImageView[2], "R");
	ui.tabWidgetView->insertTab(4, &m_widgetImageView[3], "Gr");

	connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(Export()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(HotPixel()), Qt::QueuedConnection);
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIUp, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIDown, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROILeft, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIRight, SLOT(setEnabled(bool)));

}


void CDialogAPSHotPixel::HotPixel()
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
	tempThre.dHotPixelThre = ui.lineEditHotPixelThre->text().toDouble();
	tempThre.dHotLineThre = ui.lineEditHotLineThre->text().toDouble();
	tempThre.nBadPixelRadius = ui.lineEditBadPixelRadius->text().toUInt();
	tempThre.nBadLineRadius = ui.lineEditHotLineRadius->text().toUInt();

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
		bRet = m_pAPSAlgoInterface->HotPixel(nIndexStart, nNumber, roi, m_HotPixel);
		auto end = clock();
		time = end - start;
	}
	else
	{
		bRet = m_pAPSAlgoInterface->HotPixel(nIndexStart, nNumber, roi, m_HotPixel);
		if (bRet)
		{
			auto start = clock();
			bRet = m_pAPSAlgoInterface->DPC(nIndexStart, nNumber, roi, m_HotPixel);
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
			Data[0].push_back(m_HotPixel.SubFrameBadpixelData[nIndex].BadPixelNum);
			Data[1].push_back(m_HotPixel.SubFrameBadpixelData[nIndex].SingletNum);
			Data[2].push_back(m_HotPixel.SubFrameBadpixelData[nIndex].CoupletNum);
			Data[3].push_back(m_HotPixel.SubFrameBadpixelData[nIndex].ClusterNum);
			Data[4].push_back(0);
			Data[5].push_back(m_HotPixel.SubFrameBadpixelData[nIndex].DefectRowNum);
			Data[6].push_back(m_HotPixel.SubFrameBadpixelData[nIndex].DefectColNum);
		}
		Data[0].push_back(m_HotPixel.BadPixelNum);
		Data[1].push_back(m_HotPixel.SingletNum);
		Data[2].push_back(m_HotPixel.CoupletNum);
		Data[3].push_back(m_HotPixel.ClusterNum);
		Data[4].push_back(m_HotPixel.LadderNum);
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

			for (uint32_t nIndex = 0; nIndex < m_HotPixel.SubFrameBadpixelData[nChannel].BadPixelMask.BadPixelNum; nIndex++)
			{
				pImage[m_HotPixel.SubFrameBadpixelData[nChannel].BadPixelMask.LocalData[nIndex].x * nCol + m_HotPixel.SubFrameBadpixelData[nChannel].BadPixelMask.LocalData[nIndex].y] = 255;
			}
			m_widgetImageView[nChannel].SetGrayData(pImage, nCol, nRow);
		}
	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));
	}
	ui.pushButtonStart->setEnabled(true);
}

void CDialogAPSHotPixel::Export()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "")
	{
		std::string strFile = dir.toStdString() + "//HotPixel.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			outfile << "Gb,B,R,Gr,Total" << std::endl;
			for (uint32_t nIndex = 0; nIndex < SubFrameIndex::All; nIndex++)
			{
				outfile << std::to_string(m_HotPixel.SubFrameBadpixelData[nIndex].BadPixelNum) << ",";
			}
			outfile << std::to_string(m_HotPixel.BadPixelNum) << ",";
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < SubFrameIndex::All; nIndex++)
			{
				outfile << std::to_string(m_HotPixel.SubFrameBadpixelData[nIndex].SingletNum) << ",";
			}
			outfile << std::to_string(m_HotPixel.SingletNum) << ",";
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < SubFrameIndex::All; nIndex++)
			{
				outfile << std::to_string(m_HotPixel.SubFrameBadpixelData[nIndex].CoupletNum) << ",";
			}
			outfile << std::to_string(m_HotPixel.CoupletNum) << ",";
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < SubFrameIndex::All; nIndex++)
			{
				outfile << std::to_string(m_HotPixel.SubFrameBadpixelData[nIndex].ClusterNum) << ",";
			}
			outfile << std::to_string(m_HotPixel.ClusterNum) << ",";
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < SubFrameIndex::All; nIndex++)
			{
				outfile << std::to_string(m_HotPixel.SubFrameBadpixelData[nIndex].DefectRowNum) << ",";
			}
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < SubFrameIndex::All; nIndex++)
			{
				outfile << std::to_string(m_HotPixel.SubFrameBadpixelData[nIndex].DefectColNum) << ",";
			}
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < SubFrameIndex::All; nIndex++)
			{
				for (uint32_t i = 0; i < m_HotPixel.SubFrameBadpixelData[nIndex].BadPixelMask.BadPixelNum; i++)
				{
					outfile << std::to_string(m_HotPixel.SubFrameBadpixelData[nIndex].BadPixelMask.LocalData[i].x) << ",";
				}
				outfile << std::endl;
				for (uint32_t i = 0; i < m_HotPixel.SubFrameBadpixelData[nIndex].BadPixelMask.BadPixelNum; i++)
				{
					outfile << std::to_string(m_HotPixel.SubFrameBadpixelData[nIndex].BadPixelMask.LocalData[i].y) << ",";
				}
				outfile << std::endl;
				for (uint32_t i = 0; i < m_HotPixel.SubFrameBadpixelData[nIndex].BadPixelMask.BadPixelNum; i++)
				{
					outfile << std::to_string(m_HotPixel.SubFrameBadpixelData[nIndex].BadPixelMask.Flag[i]) << ",";
				}
				outfile << std::endl;
			}

			for (uint32_t i = 0; i < m_HotPixel.BadPixelMask.BadPixelNum; i++)
			{
				outfile << std::to_string(m_HotPixel.BadPixelMask.LocalData[i].x) << ",";
			}
			outfile << std::endl;
			for (uint32_t i = 0; i < m_HotPixel.BadPixelMask.BadPixelNum; i++)
			{
				outfile << std::to_string(m_HotPixel.BadPixelMask.LocalData[i].y) << ",";
			}
			outfile << std::endl;
			for (uint32_t i = 0; i < m_HotPixel.BadPixelMask.BadPixelNum; i++)
			{
				outfile << std::to_string(m_HotPixel.BadPixelMask.Flag[i]) << ",";
			}
			outfile << std::endl;

			std::vector<uint8_t> OtpData;

			if (m_pAPSAlgoInterface->BadPixelLocalToOtpType(m_HotPixel.BadPixelMask.LocalData, OtpData))
			{
				for (uint32_t i = 0; i < OtpData.size(); i++)
				{
					outfile << QString::number(OtpData[i], 16).toStdString() << ",";
				}
				outfile << std::endl;
			}

			outfile.close();
		}
	}
}