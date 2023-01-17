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
	ui.lineEditBlockNum->setValidator(new QIntValidator(1, 100000, this));
	ui.lineEditBlockNum->setText(QString::number(m_pDVSAlgoInterface->GetAlgorithmThre().nSpatialResponseUniformityBlockNum));
	connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(Export()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(SpatialResponseUniformity()), Qt::QueuedConnection);
}


void CDialogDVSSpatialResponseUniformity::SpatialResponseUniformity()
{
	bool bRet = true;
	uint32_t nIndexStart = ui.lineEditIndexStart->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();
	uint32_t nPeakNum = ui.lineEditPeakNum->text().toUInt();
	uint32_t nLigtType = ui.comboBoxLightType->currentIndex();
	auto temp = m_pDVSAlgoInterface->GetAlgorithmThre();
	uint32_t nBlockNum = ui.lineEditBlockNum->text().toUInt();
	temp.nSpatialResponseUniformityBlockNum = nBlockNum;
	m_pDVSAlgoInterface->SetAlgorithmThre(temp);

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
	bRet = m_pDVSAlgoInterface->SpatialResponseUniformity(nIndexStart, nNumber, nullptr, nPeakNum, LightTrigerType(nLigtType + 1), m_Data);
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
		for (uint32_t nIndex = 0; nIndex < nBlockNum; nIndex++)
		{
			UniformRowName << QString::number(nIndex);
			UniformColName << QString::number(nIndex);
		}

		std::vector<std::vector<double>> UniformityRes;
		if ((nLigtType + 1) & 1)
		{
			ResRowName << "OffUniformityRatio";
			std::vector<double> OffUniformityRes(DVSSubFrameIndex::All + 1, 0);
			for (uint32_t nChannel = 0; nChannel <= DVSSubFrameIndex::All; nChannel++)
			{
				OffUniformityRes[nChannel] = m_Data.dOffEventsUniformityRatio[nChannel];
			}
			UniformityRes.push_back(OffUniformityRes);

			ui.tabOffEventsTableWidegetAll->SetData(UniformRowName, UniformColName, m_Data.OffEventsUniformityBlockData[DVSSubFrameIndex::All]);
			ui.tabOffEventsTableWidegetGb->SetData(UniformRowName, UniformColName, m_Data.OffEventsUniformityBlockData[DVSSubFrameIndex::Gb]);
			ui.tabOffEventsTableWidegetB->SetData(UniformRowName, UniformColName, m_Data.OffEventsUniformityBlockData[DVSSubFrameIndex::B]);
			ui.tabOffEventsTableWidegetR->SetData(UniformRowName, UniformColName, m_Data.OffEventsUniformityBlockData[DVSSubFrameIndex::R]);
			ui.tabOffEventsTableWidegetGr->SetData(UniformRowName, UniformColName, m_Data.OffEventsUniformityBlockData[DVSSubFrameIndex::Gr]);
		}

		if ((nLigtType + 1) & 2)
		{
			ResRowName << "OnUniformityRatio";
			std::vector<double> OnUniformityRes(DVSSubFrameIndex::All + 1, 0);
			for (uint32_t nChannel = 0; nChannel <= DVSSubFrameIndex::All; nChannel++)
			{
				OnUniformityRes[nChannel] = m_Data.dOnEventsUniformityRatio[nChannel];
			}
			UniformityRes.push_back(OnUniformityRes);

			ui.tabOnEventsTableWidegetAll->SetData(UniformRowName, UniformColName, m_Data.OnEventsUniformityBlockData[DVSSubFrameIndex::All]);
			ui.tabOnEventsTableWidegetGb->SetData(UniformRowName, UniformColName, m_Data.OnEventsUniformityBlockData[DVSSubFrameIndex::Gb]);
			ui.tabOnEventsTableWidegetB->SetData(UniformRowName, UniformColName, m_Data.OnEventsUniformityBlockData[DVSSubFrameIndex::B]);
			ui.tabOnEventsTableWidegetR->SetData(UniformRowName, UniformColName, m_Data.OnEventsUniformityBlockData[DVSSubFrameIndex::R]);
			ui.tabOnEventsTableWidegetGr->SetData(UniformRowName, UniformColName, m_Data.OnEventsUniformityBlockData[DVSSubFrameIndex::Gr]);
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
		std::string strFile = dir.toStdString() + "//SpatialResponseUniformity.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			outfile << "OffEvents" << std::endl;
			for (uint32_t nChannel = 0; nChannel <= DVSSubFrameIndex::All; nChannel++)
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
			for (uint32_t nChannel = 0; nChannel <= DVSSubFrameIndex::All; nChannel++)
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