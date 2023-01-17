#include "WidgetWelcome.h"
#include "WidgetDVSInit.h"
#include "DialogDVSFunction.h"
#include "DialogAPSInit.h"
#include "DialogAPSFunction.h"

CWidgetWelcome::CWidgetWelcome(QWidget* parent) : QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.pushButtonDVS, SIGNAL(clicked()), this, SLOT(OpenDVSFunctionWidget()));
	connect(ui.pushButtonAPS, SIGNAL(clicked()), this, SLOT(OpenAPSFunctionWidget()));
}

CWidgetWelcome::~CWidgetWelcome()
{
}

void CWidgetWelcome::OpenDVSFunctionWidget()
{
	CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr;
	CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr;

	CWidgetDVSInit init(nullptr, pAPSAlgoInterface, pDVSAlgoInterface);
	if (QDialog::Accepted == init.exec())
	{
		pDVSAlgoInterface = init.GetDVSAlgoInterface();
		CDialogDVSFunction DVSFunc(nullptr, pAPSAlgoInterface, pDVSAlgoInterface);
		DVSFunc.exec();
	}
}

void CWidgetWelcome::OpenAPSFunctionWidget()
{
	CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr;
	CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr;

	CDialogAPSInit init(nullptr, pAPSAlgoInterface, pDVSAlgoInterface);
	if (QDialog::Accepted == init.exec())
	{
		pAPSAlgoInterface = init.GetAPSAlgoInterface();
		CDialogAPSFunction APSFunc(nullptr, pAPSAlgoInterface, pDVSAlgoInterface);
		APSFunc.exec();
	}
}

