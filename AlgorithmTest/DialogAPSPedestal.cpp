#include "DialogAPSPedestal.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>

CDialogAPSPedestal::CDialogAPSPedestal(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
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
	ui.lineEditPedestalRowNum->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditPedestalColNum->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditPedestalRowSize->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditPedestalColSize->setValidator(new QIntValidator(0, 100000, this));

	ui.lineEditROIUp->setEnabled(false);
	ui.lineEditROIRight->setEnabled(false);
	ui.lineEditROILeft->setEnabled(false);
	ui.lineEditROIDown->setEnabled(false);

	auto temp = m_pAPSAlgoInterface->GetActiveArea();
	ui.lineEditROIUp->setText(QString::number(temp.Up));
	ui.lineEditROILeft->setText(QString::number(temp.Left));
	ui.lineEditROIDown->setText(QString::number(temp.Down));
	ui.lineEditROIRight->setText(QString::number(temp.Right));

	ui.lineEditPedestalRowNum->setText(QString::number(m_pAPSAlgoInterface->GetAlgorithmThre().nPedestalVariationRowBlockNum));
	ui.lineEditPedestalColNum->setText(QString::number(m_pAPSAlgoInterface->GetAlgorithmThre().nPedestalVariationColBlockNum));
	ui.lineEditPedestalRowSize->setText(QString::number(m_pAPSAlgoInterface->GetAlgorithmThre().nPedestalVariationRowBlockSize));
	ui.lineEditPedestalColSize->setText(QString::number(m_pAPSAlgoInterface->GetAlgorithmThre().nPedestalVariationColBlockSize));

	connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(Export()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(Pedestal()), Qt::QueuedConnection);
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIUp, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIDown, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROILeft, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIRight, SLOT(setEnabled(bool)));

}

void CDialogAPSPedestal::Pedestal()
{
	ui.widgetTableView->Clear();
	bool bRet = true;
	uint32_t nIndexStart = ui.lineEditIndexStart->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();
	ui.label_Res->setText(tr(" "));
	ui.pushButtonStart->setEnabled(false);

	ROIArea* roi = nullptr;
	ROIArea temp;

	if (ui.checkBoxROI->isChecked())
	{
		temp.Up = ui.lineEditROIUp->text().toUInt();
		temp.Down = ui.lineEditROIDown->text().toUInt();
		temp.Left = ui.lineEditROILeft->text().toUInt();
		temp.Right = ui.lineEditROIRight->text().toUInt();
		roi = &temp;
	}

	auto tempThre = m_pAPSAlgoInterface->GetAlgorithmThre();
	tempThre.nPedestalVariationRowBlockNum = ui.lineEditPedestalRowNum->text().toUInt();
	tempThre.nPedestalVariationColBlockNum = ui.lineEditPedestalColNum->text().toUInt();
	tempThre.nPedestalVariationRowBlockSize = ui.lineEditPedestalRowSize->text().toUInt();
	tempThre.nPedestalVariationColBlockSize = ui.lineEditPedestalColSize->text().toUInt();
	m_pAPSAlgoInterface->SetAlgorithmThre(tempThre);

	clock_t time = 0;
	auto start = clock();
	bRet = m_pAPSAlgoInterface->PedestalVariation(nIndexStart, nNumber, roi, m_Pedestal);
	auto end = clock();
	time = end - start;
	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);

		QStringList RowName, ColName;
		RowName << "Max" << "Min";
		ColName << "Gb" << "B" << "R" << "Gr";

		std::vector<std::vector<double>> Data(2);
		for (uint32_t nIndex = 0; nIndex < SubFrameIndex::All; nIndex++)
		{
			Data[0].push_back(m_Pedestal.PedestalMax[nIndex]);
			Data[1].push_back(m_Pedestal.PedestalMin[nIndex]);
		}
		ui.widgetTableView->SetData(RowName, ColName, Data);
	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));
	}
	ui.pushButtonStart->setEnabled(true);
}

void CDialogAPSPedestal::Export()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "")
	{
		std::string strFile = dir.toStdString() + "//PedestalVariation.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			outfile << "Gb,B,R,Gr" << std::endl;
			for (uint32_t nIndex = 0; nIndex < SubFrameIndex::All; nIndex++)
			{
				outfile << std::to_string(m_Pedestal.PedestalMax[nIndex]) << ",";
			}
			outfile << std::endl;
			for (uint32_t nIndex = 0; nIndex < SubFrameIndex::All; nIndex++)
			{
				outfile << std::to_string(m_Pedestal.PedestalMin[nIndex]) << ",";
			}
			outfile << std::endl;

			outfile.close();
		}
	}
}