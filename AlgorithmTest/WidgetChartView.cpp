#include "WidgetChartView.h"
#include <QVector>

CWidgetChartView::CWidgetChartView(QWidget* parent)
	:QWidget(parent)
{
	ui.setupUi(this);
	ui.widgetChartView->legend->setVisible(true);
	ui.widgetChartView->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
        QCP::iSelectLegend | QCP::iSelectPlottables);

    ui.widgetChartView->xAxis2->setVisible(true);
    ui.widgetChartView->xAxis2->setTickLabels(false);
    ui.widgetChartView->yAxis2->setVisible(true);
    ui.widgetChartView->yAxis2->setTickLabels(false);
    connect(ui.widgetChartView->xAxis, SIGNAL(rangeChanged(QCPRange)), ui.widgetChartView->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui.widgetChartView->yAxis, SIGNAL(rangeChanged(QCPRange)), ui.widgetChartView->yAxis2, SLOT(setRange(QCPRange)));
    connect(ui.widgetChartView, SIGNAL(plottableClick(QCPAbstractPlottable*, int, QMouseEvent*)), this, SLOT(AddToolTips(QCPAbstractPlottable * , int , QMouseEvent * )), Qt::UniqueConnection);
    m_nLineNum = 0;
    ToolTips = nullptr;
}

void CWidgetChartView::SetLine(std::string strLineName, QVector<double> &XData, QVector<double> &YData)
{
    ui.widgetChartView->addGraph();
    ui.widgetChartView->graph(m_nLineNum)->setData(XData, YData);
    ui.widgetChartView->graph(m_nLineNum)->rescaleAxes(true);
    ui.widgetChartView->graph(m_nLineNum)->setName(QString::fromStdString(strLineName));
    ui.widgetChartView->graph(m_nLineNum)->setSelectable(QCP::SelectionType::stSingleData);

    QPen graphPen;
    graphPen.setColor(QColor(rand() % 245 + 10, rand() % 245 + 10, rand() % 245 + 10));

    ui.widgetChartView->graph(m_nLineNum)->setPen(graphPen);

    ui.widgetChartView->graph(m_nLineNum)->setLineStyle(QCPGraph::lsLine);
    ui.widgetChartView->graph(m_nLineNum)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 2));
    ui.widgetChartView->graph(m_nLineNum)->setVisible(false);
    QCheckBoxControl.resize(m_nLineNum + 1);
    QCheckBox* LineChose = new QCheckBox(QString::fromStdString(strLineName), this);
    QCheckBoxControl[m_nLineNum] = LineChose;
    LineChose->setChecked(false);
    ui.verticalLayoutLineChose->addWidget(LineChose);
    connect(LineChose, SIGNAL(stateChanged(int)), this, SLOT(LineVisibleCheck()), Qt::UniqueConnection);
    ui.widgetChartView->replot();
    m_nLineNum++;
}

void CWidgetChartView::SetScatter(std::string strLineName, QVector<double>& XData, QVector<double>& YData)
{
    ui.widgetChartView->addGraph();
    ui.widgetChartView->graph(m_nLineNum)->setData(XData, YData);
    ui.widgetChartView->graph(m_nLineNum)->rescaleAxes(true);
    ui.widgetChartView->graph(m_nLineNum)->setName(QString::fromStdString(strLineName));
    ui.widgetChartView->graph(m_nLineNum)->setSelectable(QCP::SelectionType::stSingleData);
    QPen graphPen;
    graphPen.setColor(QColor(rand() % 245 + 10, rand() % 245 + 10, rand() % 245 + 10));

    ui.widgetChartView->graph(m_nLineNum)->setPen(graphPen);

    ui.widgetChartView->graph(m_nLineNum)->setLineStyle(QCPGraph::lsNone);
    ui.widgetChartView->graph(m_nLineNum)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 8));
    ui.widgetChartView->graph(m_nLineNum)->setVisible(false);
    QCheckBoxControl.resize(m_nLineNum + 1);
    QCheckBox* LineChose = new QCheckBox(QString::fromStdString(strLineName), this);
    QCheckBoxControl[m_nLineNum] = LineChose;
    LineChose->setChecked(false);
    ui.verticalLayoutLineChose->addWidget(LineChose);
    connect(LineChose, SIGNAL(stateChanged(int)), this, SLOT(LineVisibleCheck()), Qt::UniqueConnection);
    ui.widgetChartView->replot();
    m_nLineNum++;
}

void CWidgetChartView::Clear()
{
    ui.widgetChartView->clearGraphs();
    for (uint32_t nIndex = 0; nIndex < m_nLineNum; nIndex++)
    {
        QCheckBoxControl[nIndex]->setParent(nullptr);
        ui.verticalLayoutLineChose->removeWidget(QCheckBoxControl[nIndex]);
    }
    m_nLineNum = 0;
    ui.widgetChartView->replot();
    QCheckBoxControl.resize(0);
}

void CWidgetChartView::AddToolTips(QCPAbstractPlottable* plottable, int dataIndex, QMouseEvent* event)
{
    if(ToolTips != nullptr)
    {
        delete ToolTips;
    }
    QCPGraph* graph = (QCPGraph * )plottable;
    double X = graph->dataMainKey(dataIndex);
    double Y = graph->dataMainValue(dataIndex);
    ToolTips = new TextLabel(ui.widgetChartView, X, Y);
}

void CWidgetChartView::LineVisibleCheck()
{
    for (uint32_t nIndex = 0; nIndex < m_nLineNum; nIndex++)
    {
        ui.widgetChartView->graph(nIndex)->setVisible(QCheckBoxControl[nIndex]->isChecked());
    }
    ui.widgetChartView->replot();
}

TextLabel::TextLabel(QCustomPlot* parent, double dXValue, double dYValue)
{
    m_parent = parent;
    textLabel = new QCPItemText(parent);
    textLabel->setPositionAlignment(Qt::AlignTop | Qt::AlignHCenter);
    textLabel->position->setType(QCPItemPosition::ptPlotCoords);
    textLabel->position->setCoords(dXValue, dYValue); // place position at center/top of axis rect
    auto pos = textLabel->position->pixelPosition();
    pos.setY(pos.y() - 100);
    textLabel->position->setPixelPosition(pos);
    textLabel->setText(QString("X: %1\nY: %2").arg(dXValue).arg(dYValue));
    textLabel->setFont(QFont(parent->font().family(), 12)); // make font a bit larger
    textLabel->setPen(QPen(Qt::black)); // show black border around text

    // add the arrow:
    arrow = new QCPItemLine(parent);
    arrow->start->setParentAnchor(textLabel->bottom);
    arrow->end->setCoords(dXValue, dYValue); // point to (4, 1.6) in x-y-plot coordinates
    pos = arrow->end->pixelPosition();
    pos.setY(pos.y() - 5);
    arrow->end->setPixelPosition(pos);

    arrow->setHead(QCPLineEnding::esSpikeArrow);

    m_dXValue = dXValue;
    m_dYValue = dYValue;

    connect(parent, SIGNAL(afterReplot()), this, SLOT(Replot()));
}

TextLabel::~TextLabel()
{
    textLabel->setVisible(false);
    arrow->setVisible(false);
    m_parent->removeItem(textLabel);
    m_parent->removeItem(arrow);
}

void TextLabel::GetCoords(double& X, double& Y)
{
    X = m_dXValue;
    Y = m_dYValue;
}

void TextLabel::Replot()
{
    textLabel->position->setCoords(m_dXValue, m_dYValue);
    auto pos = textLabel->position->pixelPosition();
    pos.setY(pos.y() - 100);
    textLabel->position->setPixelPosition(pos);

    arrow->end->setCoords(m_dXValue, m_dYValue);
    pos = arrow->end->pixelPosition();
    pos.setY(pos.y() - 5);
    arrow->end->setPixelPosition(pos);

}
