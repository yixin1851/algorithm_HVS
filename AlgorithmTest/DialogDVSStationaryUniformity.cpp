#include "DialogDVSStationaryUniformity.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>

CDialogStationaryUniformity::CDialogStationaryUniformity(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);

	ui.lineEditIndexStart->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditNumber->setValidator(new QIntValidator(1, 100000, this));
	ui.lineEditBlockNum->setValidator(new QIntValidator(1, 100000, this));
	ui.lineEditBlockNum->setText(QString::number(m_pDVSAlgoInterface->GetAlgorithmThre().nStationaryUniformityBlockNum));

	connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(Export()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(StationaryUniformity()), Qt::QueuedConnection);
}


void CDialogStationaryUniformity::StationaryUniformity()
{
	bool bRet = true;
	uint32_t nIndexStart = ui.lineEditIndexStart->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();
	uint32_t nBlockNum = ui.lineEditBlockNum->text().toUInt();
	auto temp = m_pDVSAlgoInterface->GetAlgorithmThre();
	temp.nStationaryUniformityBlockNum = nBlockNum;
	m_pDVSAlgoInterface->SetAlgorithmThre(temp);
	ui.widgetTableView->Clear();
	ui.label_Res->setText(tr(" "));
	ui.pushButtonStart->setEnabled(false);
	clock_t time = 0;
	auto start = clock();
	bRet = m_pDVSAlgoInterface->StationaryUniformity(nIndexStart, nNumber, m_Data);
	auto end = clock();
	time = end - start;
	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);

		QStringList RowName, ColName;
		for (uint32_t nIndex = 0; nIndex < nBlockNum; nIndex++)
		{
			RowName << QString::number(nIndex);
			ColName << QString::number(nIndex);
		}
		RowName << "UniformityRatio";

		auto Data = m_Data.UniformityBlockData;
		std::vector<double> ratio(nBlockNum, 0);
		ratio[0] = m_Data.UniformityRatio;
		Data.push_back(ratio);
		ui.widgetTableView->SetData(RowName, ColName, Data);

	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));
	}
	ui.pushButtonStart->setEnabled(true);
}

void CDialogStationaryUniformity::Export()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "")
	{
		std::string strFile = dir.toStdString() + "//StationaryUniformity.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			for (uint32_t nRows = 0; nRows < m_Data.UniformityBlockData.size(); nRows++)
			{
				for (uint32_t nCols = 0; nCols < m_Data.UniformityBlockData[0].size(); nCols++)
				{
					outfile << std::to_string(m_Data.UniformityBlockData[nRows][nCols]) << ",";
				}
				outfile << std::endl;
			}
			outfile << std::to_string(m_Data.UniformityRatio) << std::endl;
			outfile.close();
		}
	}
}