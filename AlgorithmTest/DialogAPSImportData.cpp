#include "DialogAPSImportData.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>
#include "AlpMPAlgoInterface.h"
#include "time.h"
#include <thread>
#include<io.h>

CDialogAPSImportData::CDialogAPSImportData(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);

	ui.lineEditIndexStart->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditNumber->setValidator(new QIntValidator(1, 100000, this));

	connect(ui.pushButtonBrowser, SIGNAL(clicked()), this, SLOT(Browser()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(ImportData()), Qt::QueuedConnection);


}

void CDialogAPSImportData::FindFiles(std::string strPath, std::vector<std::string>& FileQuene)
{
	_finddata_t file_info;
	std::string current_path = strPath + "/*.raw";
	long long handle = _findfirst(current_path.c_str(), &file_info);
	uint32_t file_num = 0;
	FileQuene.clear();
	if (-1 == handle)
		return;
	do
	{
		std::string attribute;
		if (file_info.attrib != _A_SUBDIR)
		{
			FileQuene.push_back(strPath + "/" + file_info.name);
		}

		file_num++;

	} while (!_findnext(handle, &file_info));
	_findclose(handle);
	return;
}

void CDialogAPSImportData::Browser()
{
	QString strFileName = QFileDialog::getOpenFileName(nullptr, tr("Open APS File"), ui.lineEditDataFile->text(), tr("All Files (*.*) \n Bin File (*.bin) \n Raw File (*.raw)"));
	ui.lineEditDataFile->setText(strFileName);
}

void CDialogAPSImportData::ImportData()
{
	bool bRet = true;
	uint32_t nIndexStart = ui.lineEditIndexStart->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();
	std::string strFileName(ui.lineEditDataFile->text().toLocal8Bit());
	bool bHeaderFooter = ui.checkBoxFrameHeaderFooter->isChecked();
	ui.label_Res->setText(tr(" "));
	clock_t time = 0;
	std::ifstream infile;

	infile.open(strFileName, std::ios::binary | std::ios::in);
	if (!infile.fail())
	{
		infile.seekg(0, std::ios::end);
		uint64_t length = infile.tellg();
		infile.seekg(0, std::ios::beg);
		uint8_t* pRawData = new uint8_t[length];
		infile.read((char*)pRawData, length);
		infile.close();

		auto start = clock();
		bRet = m_pAPSAlgoInterface->ImportRawData(pRawData, length, nIndexStart, nNumber, bHeaderFooter);
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
		QString res = QString::number(time);
		ui.label_Res->setText(res);
	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));
	}
}