#include "DialogAPSBLC.h"
#include <qvalidator.h>
#include "AlpMPAlgoInterface.h"
#include <vector>
#include "time.h"

CDialogAPSBLC::CDialogAPSBLC(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);

	ui.lineEditIndexStart->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditNumber->setValidator(new QIntValidator(1, 100000, this));
	ui.lineEditBaseIndexStart->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditBaseNumber->setValidator(new QIntValidator(1, 100000, this));

	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(BLC()), Qt::QueuedConnection);

}

void CDialogAPSBLC::BLC()
{
	bool bRet = true;
	uint32_t nIndexStart = ui.lineEditIndexStart->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();
	uint32_t nBaseIndexStart = ui.lineEditBaseIndexStart->text().toUInt();
	uint32_t nBaseNumber = ui.lineEditBaseNumber->text().toUInt();

	ui.label_Res->setText(tr(" "));
	clock_t time = 0;
	if (ui.comboBoxBLCType->currentIndex() == 0)
	{
		std::vector<double> BaseMean(8, 0);
		auto start = clock();
		bRet = m_pAPSAlgoInterface->DataMean(nBaseIndexStart, nBaseNumber, nullptr, BaseMean)
			&& m_pAPSAlgoInterface->BLC(nIndexStart, nNumber, BaseMean);
		auto end = clock();
		time = end - start;
	}
	else
	{
		auto start = clock();
		bRet = m_pAPSAlgoInterface->BLC(nIndexStart, nNumber, nBaseIndexStart, nBaseNumber);
		auto end = clock();
		time = end - start;
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
