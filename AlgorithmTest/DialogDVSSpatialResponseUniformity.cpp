#include "DialogDVSSpatialResponseUniformity.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>

CDialogDVSSpatialResponseUniformity::CDialogDVSSpatialResponseUniformity(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);

	ui.lineEditIndexStart->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditNumber->setValidator(new QIntValidator(1, 100000, this));
	ui.lineEditPeakNum->setValidator(new QIntValidator(1, 100000, this));
	ui.lineEditRowBlockNum->setValidator(new QIntValidator(1, 100000, this));
	ui.lineEditColBlockNum->setValidator(new QIntValidator(1, 100000, this));
	ui.lineEditRowBlockNum->setText(QString::number(m_pDVSAlgoInterface->GetAlgorithmThre().nSpatialResponseUniformityRowBlockNum));
	ui.lineEditColBlockNum->setText(QString::number(m_pDVSAlgoInterface->GetAlgorithmThre().nSpatialResponseUniformityColBlockNum));
	connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(Export()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(SpatialResponseUniformity()), Qt::QueuedConnection);

	ui.lineEditROIUp->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditROIRight->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditROILeft->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditROIDown->setValidator(new QIntValidator(0, 100000, this));

	ui.lineEditROIUp->setEnabled(false);
	ui.lineEditROIRight->setEnabled(false);
	ui.lineEditROILeft->setEnabled(false);
	ui.lineEditROIDown->setEnabled(false);
	auto temp = m_pDVSAlgoInterface->GetActiveArea();
	ui.lineEditROIUp->setText(QString::number(temp.Up));
	ui.lineEditROILeft->setText(QString::number(temp.Left));
	ui.lineEditROIDown->setText(QString::number(temp.Down));
	ui.lineEditROIRight->setText(QString::number(temp.Right));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIUp, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIDown, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROILeft, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIRight, SLOT(setEnabled(bool)));

}


void CDialogDVSSpatialResponseUniformity::SpatialResponseUniformity()
{
	bool bRet = true;
	uint32_t nIndexStart = ui.lineEditIndexStart->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();
	uint32_t nPeakNum = ui.lineEditPeakNum->text().toUInt();
	uint32_t nLigtType = ui.comboBoxLightType->currentIndex();
	auto temp = m_pDVSAlgoInterface->GetAlgorithmThre();
	uint32_t nRowBlockNum = ui.lineEditRowBlockNum->text().toUInt();
	uint32_t nColBlockNum = ui.lineEditColBlockNum->text().toUInt();
	temp.nSpatialResponseUniformityRowBlockNum = nRowBlockNum;
	temp.nSpatialResponseUniformityColBlockNum = nColBlockNum;
	m_pDVSAlgoInterface->SetAlgorithmThre(temp);

	ROIArea* roi = nullptr;
	ROIArea tempRoi;

	if (ui.checkBoxROI->isChecked())
	{
		tempRoi.Up = ui.lineEditROIUp->text().toUInt();
		tempRoi.Down = ui.lineEditROIDown->text().toUInt();
		tempRoi.Left = ui.lineEditROILeft->text().toUInt();
		tempRoi.Right = ui.lineEditROIRight->text().toUInt();
		roi = &tempRoi;
	}

	ui.tabUniformityRes->Clear();

	ui.tabOffEventsTableWidegetAll->Clear();
	ui.tabOffEventsTableWidegetGb->Clear();
	ui.tabOffEventsTableWidegetB->Clear();
	ui.tabOffEventsTableWidegetR->Clear();
	ui.tabOffEventsTableWidegetGr->Clear();

	ui.tabOnEventsTableWidegetAll->Clear();
	ui.tabOnEventsTableWidegetGb->Clear();
	ui.tabOnEventsTableWidegetB->Clear();
	ui.tabOnEventsTableWidegetR->Clear();
	ui.tabOnEventsTableWidegetGr->Clear();

	ui.label_Res->setText(tr(" "));
	ui.pushButtonStart->setEnabled(false);
	clock_t time = 0;
	auto start = clock();
	bRet = m_pDVSAlgoInterface->SpatialResponseUniformity(nIndexStart, nNumber, roi, nullptr, nPeakNum, DVSLightTrigerType(nLigtType + 1), m_Data);
	auto end = clock();
	time = end - start;
	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);
		QStringList ResRowName, ResColName;
		ResColName << "Gb" << "B" << "R" << "Gr" << "All";

		QStringList UniformRowName, UniformColName;
		for (uint32_t nIndex = 0; nIndex < nRowBlockNum; nIndex++)
		{
			UniformRowName << QString::number(nIndex);
		}
		for (uint32_t nIndex = 0; nIndex < nColBlockNum; nIndex++)
		{
			UniformColName << QString::number(nIndex);
		}

		std::vector<std::vector<double>> UniformityRes;
		if ((nLigtType + 1) & 1)
		{
			ResRowName << "OffUniformityRatio";
			std::vector<double> OffUniformityRes(SubFrameIndex::All + 1, 0);
			for (uint32_t nChannel = 0; nChannel <= SubFrameIndex::All; nChannel++)
			{
				OffUniformityRes[nChannel] = m_Data.dOffEventsUniformityRatio[nChannel];
			}
			UniformityRes.push_back(OffUniformityRes);

			ui.tabOffEventsTableWidegetAll->SetData(UniformRowName, UniformColName, m_Data.OffEventsUniformityBlockData[SubFrameIndex::All]);
			ui.tabOffEventsTableWidegetGb->SetData(UniformRowName, UniformColName, m_Data.OffEventsUniformityBlockData[SubFrameIndex::Gb]);
			ui.tabOffEventsTableWidegetB->SetData(UniformRowName, UniformColName, m_Data.OffEventsUniformityBlockData[SubFrameIndex::B]);
			ui.tabOffEventsTableWidegetR->SetData(UniformRowName, UniformColName, m_Data.OffEventsUniformityBlockData[SubFrameIndex::R]);
			ui.tabOffEventsTableWidegetGr->SetData(UniformRowName, UniformColName, m_Data.OffEventsUniformityBlockData[SubFrameIndex::Gr]);
		}

		if ((nLigtType + 1) & 2)
		{
			ResRowName << "OnUniformityRatio";
			std::vector<double> OnUniformityRes(SubFrameIndex::All + 1, 0);
			for (uint32_t nChannel = 0; nChannel <= SubFrameIndex::All; nChannel++)
			{
				OnUniformityRes[nChannel] = m_Data.dOnEventsUniformityRatio[nChannel];
			}
			UniformityRes.push_back(OnUniformityRes);

			ui.tabOnEventsTableWidegetAll->SetData(UniformRowName, UniformColName, m_Data.OnEventsUniformityBlockData[SubFrameIndex::All]);
			ui.tabOnEventsTableWidegetGb->SetData(UniformRowName, UniformColName, m_Data.OnEventsUniformityBlockData[SubFrameIndex::Gb]);
			ui.tabOnEventsTableWidegetB->SetData(UniformRowName, UniformColName, m_Data.OnEventsUniformityBlockData[SubFrameIndex::B]);
			ui.tabOnEventsTableWidegetR->SetData(UniformRowName, UniformColName, m_Data.OnEventsUniformityBlockData[SubFrameIndex::R]);
			ui.tabOnEventsTableWidegetGr->SetData(UniformRowName, UniformColName, m_Data.OnEventsUniformityBlockData[SubFrameIndex::Gr]);
		}

		ui.tabUniformityRes->SetData(ResRowName, ResColName, UniformityRes);
	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));
	}
	ui.pushButtonStart->setEnabled(true);
}

void CDialogDVSSpatialResponseUniformity::Export()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "")
	{
		std::string strFile = dir.toLocal8Bit().toStdString() + "//SpatialResponseUniformity.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			outfile << "OffEvents" << std::endl;
			for (uint32_t nChannel = 0; nChannel <= SubFrameIndex::All; nChannel++)
			{
				for (uint32_t nRows = 0; nRows < m_Data.OffEventsUniformityBlockData[nChannel].size(); nRows++)
				{
					for (uint32_t nCols = 0; nCols < m_Data.OffEventsUniformityBlockData[nChannel][0].size(); nCols++)
					{
						outfile << std::to_string(m_Data.OffEventsUniformityBlockData[nChannel][nRows][nCols]) << ",";
					}
					outfile << std::endl;
				}
				outfile << std::to_string(m_Data.dOffEventsUniformityRatio[nChannel]) << std::endl;

			}

			outfile << "OnEvents" << std::endl;
			for (uint32_t nChannel = 0; nChannel <= SubFrameIndex::All; nChannel++)
			{
				for (uint32_t nRows = 0; nRows < m_Data.OnEventsUniformityBlockData[nChannel].size(); nRows++)
				{
					for (uint32_t nCols = 0; nCols < m_Data.OnEventsUniformityBlockData[nChannel][0].size(); nCols++)
					{
						outfile << std::to_string(m_Data.OnEventsUniformityBlockData[nChannel][nRows][nCols]) << ",";
					}
					outfile << std::endl;
				}
				outfile << std::to_string(m_Data.dOnEventsUniformityRatio[nChannel]) << std::endl;
			}

			outfile.close();
		}
	}
}