#include "DialogDVSImportData.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>
#include "AlpMPAlgoInterface.h"
#include "time.h"

CDialogDVSImportData::CDialogDVSImportData(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);

	ui.lineEditIndexStart->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditNumber->setValidator(new QIntValidator(1, 100000, this));

	connect(ui.pushButtonBrowser, SIGNAL(clicked()), this, SLOT(Browser()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(ImportData()), Qt::QueuedConnection);


}

void CDialogDVSImportData::Browser()
{
	QString strFileName = QFileDialog::getOpenFileName(nullptr, tr("Open DVS File"), ui.lineEditDataFile->text(), tr("All Files (*.*) \n Bin File (*.bin)"));
	ui.lineEditDataFile->setText(strFileName);
}

void CDialogDVSImportData::ImportData()
{
	bool bRet = true;
	uint32_t nIndexStart = ui.lineEditIndexStart->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();
	std::string strFileName(ui.lineEditDataFile->text().toLocal8Bit());
	ui.label_Res->setText(tr(" "));
	ui.pushButtonStart->setEnabled(false);
	clock_t time = 0;
    uint32_t dropSubFrameNumber = 0;
	std::ifstream infile;
	infile.open(strFileName, std::ios::binary | std::ios::in);
	if (!infile.fail())
	{
		infile.seekg(0, std::ios::end);
		uint64_t length = infile.tellg();
		infile.seekg(0, std::ios::beg);
		uint8_t *pRawData = new uint8_t[length];
		infile.read((char *)pRawData, length);
		infile.close();

		auto start = clock();
		bRet = m_pDVSAlgoInterface->ImportRawData(pRawData, length, nIndexStart, nNumber);
		auto end = clock();
		time = end - start;
		delete[] pRawData;
	}
	else
	{
		bRet = false;
	}
	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
	    QString res = "use time: " + QString::number(time) + " ms";
        // QString res = "use time: " + QString::number(time) + " ms, " +
                      // "drop sub frame num: " + QString::number(dropSubFrameNumber);
		ui.label_Res->setText(res);
	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));
	}
	ui.pushButtonStart->setEnabled(true);
}
