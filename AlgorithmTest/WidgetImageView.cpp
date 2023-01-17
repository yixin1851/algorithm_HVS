#include "WidgetImageView.h"

CWidgetImageView::CWidgetImageView(QWidget* parent)
	:QWidget(parent)
{
	ui.setupUi(this);
	ui.labelImageView->setScaledContents(true);
}

void CWidgetImageView::SetGrayData(const uchar * pImage, int32_t nWidth, int32_t nHeight)
{
	QImage Img(pImage, nWidth, nHeight, QImage::Format_Indexed8);
	QPixmap pixImg = QPixmap::fromImage(Img);
	pixImg.scaled(ui.labelImageView->size());
	ui.labelImageView->setPixmap(pixImg);
}

void CWidgetImageView::SetColorData(const uchar* pImage, int32_t nWidth, int32_t nHeight)
{
	QImage Img(pImage, nWidth, nHeight, QImage::Format_RGB888);
	QPixmap pixImg = QPixmap::fromImage(Img);
	pixImg.scaled(ui.labelImageView->size());
	ui.labelImageView->setPixmap(pixImg);
}

void CWidgetImageView::Clear()
{
	ui.labelImageView->clear();
}

