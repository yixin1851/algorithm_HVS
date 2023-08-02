#include "DialogAPSShading.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>

CDialogAPSShading::CDialogAPSShading(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
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
	ui.lineEditYShadingRowNum->setValidator(new QIntValidator(1, 100000, this));
	ui.lineEditYShadingColNum->setValidator(new QIntValidator(1, 100000, this));
	ui.lineEditColorShadingRowNum->setValidator(new QIntValidator(1, 100000, this));
	ui.lineEditColorShadingColNum->setValidator(new QIntValidator(1, 100000, this));

	ui.lineEditROIUp->setEnabled(false);
	ui.lineEditROIRight->setEnabled(false);
	ui.lineEditROILeft->setEnabled(false);
	ui.lineEditROIDown->setEnabled(false);

	auto temp = m_pAPSAlgoInterface->GetActiveArea();
	ui.lineEditROIUp->setText(QString::number(temp.Up));
	ui.lineEditROILeft->setText(QString::number(temp.Left));
	ui.lineEditROIDown->setText(QString::number(temp.Down));
	ui.lineEditROIRight->setText(QString::number(temp.Right));

	ui.lineEditYShadingRowNum->setText(QString::number(m_pAPSAlgoInterface->GetAlgorithmThre().nYShadingRowBlockNum));
	ui.lineEditYShadingColNum->setText(QString::number(m_pAPSAlgoInterface->GetAlgorithmThre().nYShadingColBlockNum));
	ui.lineEditColorShadingRowNum->setText(QString::number(m_pAPSAlgoInterface->GetAlgorithmThre().nColorShadingRowBlockNum));
	ui.lineEditColorShadingColNum->setText(QString::number(m_pAPSAlgoInterface->GetAlgorithmThre().nColorShadingColBlockNum));

	connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(Export()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(Shading()), Qt::QueuedConnection);
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIUp, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIDown, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROILeft, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIRight, SLOT(setEnabled(bool)));

	ui.tabWidgetView->insertTab(0, &m_widgetTableView[0], "YShading");
	ui.tabWidgetView->insertTab(1, &m_widgetTableView[1], "ColorShading(RG)");
	ui.tabWidgetView->insertTab(2, &m_widgetTableView[2], "ColorShading(BG)");
	ui.tabWidgetView->insertTab(3, &m_widgetTableView[3], "OpticalCenter");
}


void CDialogAPSShading::Shading()
{
	bool bRet = true;
	uint32_t nIndexStart = ui.lineEditIndexStart->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();
	ui.label_Res->setText(tr(" "));
	ui.pushButtonStart->setEnabled(false);

	for (uint32_t nIndex = 0; nIndex < SubFrameIndex::All; nIndex++)
	{
		m_widgetTableView[nIndex].Clear();
	}


	ROIArea* roi = nullptr;
	ROIArea tempROI;

	auto tempThre = m_pAPSAlgoInterface->GetAlgorithmThre();
	tempThre.nYShadingRowBlockNum = ui.lineEditYShadingRowNum->text().toUInt();
	tempThre.nYShadingColBlockNum = ui.lineEditYShadingColNum->text().toUInt();
	tempThre.nColorShadingRowBlockNum = ui.lineEditColorShadingRowNum->text().toUInt();
	tempThre.nColorShadingColBlockNum = ui.lineEditColorShadingColNum->text().toUInt();
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
	auto start = clock();
	bRet = m_pAPSAlgoInterface->YShading(nIndexStart, nNumber, roi, m_YShadingData)
		&& m_pAPSAlgoInterface->ColorShading(nIndexStart, nNumber, roi, m_ColorShadingData)
		&& m_pAPSAlgoInterface->OpticalCenter(nIndexStart, nNumber, roi, m_OpticalCenterData);
	auto end = clock();
	time = end - start;
	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);

		QStringList RowName, ColName;

		for (uint32_t nRows = 0; nRows < m_YShadingData.YShadingData.size(); nRows++)
		{
			RowName << QString::number(nRows + 1);
		}
		for (uint32_t nCols = 0; nCols < m_YShadingData.YShadingData[0].size(); nCols++)
		{
			ColName << QString::number(nCols + 1);
		}
		m_widgetTableView[0].SetData(RowName, ColName, m_YShadingData.YShadingData);

		RowName.clear();
		ColName.clear();

		for (uint32_t nRows = 0; nRows < m_ColorShadingData.ColorShadingRGData.size(); nRows++)
		{
			RowName << QString::number(nRows + 1);
		}
		for (uint32_t nCols = 0; nCols < m_ColorShadingData.ColorShadingRGData[0].size(); nCols++)
		{
			ColName << QString::number(nCols + 1);
		}

		m_widgetTableView[1].SetData(RowName, ColName, m_ColorShadingData.ColorShadingRGData);
		m_widgetTableView[2].SetData(RowName, ColName, m_ColorShadingData.ColorShadingBGData);

		RowName.clear();
		ColName.clear();
		RowName << "";
		ColName << "CenterRow" << "CenterCol";

		std::vector<std::vector<double>> Data(1);
		Data[0].push_back(m_OpticalCenterData.CenterRow);
		Data[0].push_back(m_OpticalCenterData.CenterCol);

		m_widgetTableView[3].SetData(RowName, ColName, Data);
	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));
	}
	ui.pushButtonStart->setEnabled(true);
}

void CDialogAPSShading::Export()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "")
	{
		std::string strFile = dir.toLocal8Bit().toStdString() + "//Shading.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			outfile << "YShading" << std::endl;
			for (uint32_t nRows = 0; nRows < m_YShadingData.YShadingData.size(); nRows++)
			{
				for (uint32_t nCols = 0; nCols < m_YShadingData.YShadingData[0].size(); nCols++)
				{
					outfile << m_YShadingData.YShadingData[nRows][nCols] << ",";
				}
				outfile << std::endl;
			}
			outfile << "ColorShading(RG)" << std::endl;
			for (uint32_t nRows = 0; nRows < m_ColorShadingData.ColorShadingRGData.size(); nRows++)
			{
				for (uint32_t nCols = 0; nCols < m_ColorShadingData.ColorShadingRGData[0].size(); nCols++)
				{
					outfile << m_ColorShadingData.ColorShadingRGData[nRows][nCols] << ",";
				}
				outfile << std::endl;
			}

			outfile << "ColorShading(BG)" << std::endl;
			for (uint32_t nRows = 0; nRows < m_ColorShadingData.ColorShadingBGData.size(); nRows++)
			{
				for (uint32_t nCols = 0; nCols < m_ColorShadingData.ColorShadingBGData[0].size(); nCols++)
				{
					outfile << m_ColorShadingData.ColorShadingBGData[nRows][nCols] << ",";
				}
				outfile << std::endl;
			}
			outfile << "CenterRow, CenterCol" << std::endl;
			outfile << std::to_string(m_OpticalCenterData.CenterRow) << "," << std::to_string(m_OpticalCenterData.CenterCol) << ",";
			outfile << std::endl;

			outfile.close();
		}
	}
}