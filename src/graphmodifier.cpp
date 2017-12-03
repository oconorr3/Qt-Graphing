/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Data Visualization module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "../include/graphmodifier.h"
#include <QtDataVisualization/qcategory3daxis.h>
#include <QtDataVisualization/qvalue3daxis.h>
#include <QtDataVisualization/qbardataproxy.h>
#include <QtDataVisualization/q3dscene.h>
#include <QtDataVisualization/q3dcamera.h>
#include <QtDataVisualization/qbar3dseries.h>
#include <QtDataVisualization/q3dtheme.h>
#include <QtCore/QTime>
#include <QtWidgets/QComboBox>
#include <QtCore/qmath.h>

#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <sstream>

using namespace QtDataVisualization;

//! [0]
GraphModifier::GraphModifier(Q3DBars *bargraph, std::vector<std::vector<std::string>> data,
                             std::vector<std::vector<std::string>> averages)
    : m_graph(bargraph),
      m_xRotation(0.0f),
      m_yRotation(0.0f),
      m_fontSize(12),
      m_segments(10),
      m_subSegments(4),
      m_minval(-10000.0f),
      m_maxval(10000.0f),
      m_loadAxis(new QValue3DAxis),
      m_timeAxis(new QCategory3DAxis),
      m_nodeAxis(new QCategory3DAxis),
      m_primarySeries(new QBar3DSeries),
      m_secondarySeries(new QBar3DSeries),
      m_barMesh(QAbstract3DSeries::MeshBar),
      m_smooth(true)
{

    //Convert data for graphs
    for (int i = 0; i < (int)data.size(); i++) {
        std::vector<std::string> dataRow = data[i];
        QVector<double> QDataRow;
        for (int k = 0; k < (int)dataRow.size(); k++) {
            QDataRow.push_back(atof(dataRow[k].c_str()));   //convert the std::string to double
        }
        this->data.push_back(QDataRow);
    }

    for (int i = 0; i < (int)averages.size(); i++) {
        std::vector<std::string> averageRow = averages[i];
        for (int k = 0; k < (int)averageRow.size(); k++) {
            this->averages.push_back(atof(averageRow[k].c_str()));   //convert the std::string to double
        }
    }

    for (int i = 0; i < (int)data.size(); i++) {
        this->node_nums.append(QString::fromStdString(std::to_string(i)));
    }
    for (int i = 0; i < (int)data[0].size(); i++) {
        this->times.append(QString::fromStdString(std::to_string(i)));
    }

    //configure graph options
    m_graph->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
    m_graph->activeTheme()->setBackgroundEnabled(false);
    m_graph->activeTheme()->setFont(QFont("Times New Roman", m_fontSize));
    m_graph->activeTheme()->setLabelBackgroundEnabled(true);
    m_graph->setMultiSeriesUniform(false);

    m_loadAxis->setTitle("Load On Node n At Time t");
    m_loadAxis->setSegmentCount(m_segments);
    m_loadAxis->setSubSegmentCount(m_subSegments);

    //m_loadAxis->setLabelFormat(QString());
    m_loadAxis->setLabelAutoRotation(30.0f);
    m_loadAxis->setTitleVisible(true);

    m_timeAxis->setTitle("Time");
    m_timeAxis->setLabelAutoRotation(30.0f);
    m_timeAxis->setTitleVisible(true);
    m_nodeAxis->setTitle("Nodes");
    m_nodeAxis->setLabelAutoRotation(30.0f);
    m_nodeAxis->setTitleVisible(true);

    m_graph->setValueAxis(m_loadAxis);
    m_graph->setRowAxis(m_nodeAxis);
    m_graph->setColumnAxis(m_timeAxis);

    m_primarySeries->setItemLabelFormat(QStringLiteral("@colLabel Node @rowLabel: Load @valueLabel"));
    m_primarySeries->setMesh(QAbstract3DSeries::MeshBar);
    m_primarySeries->setMeshSmooth(false);
    m_primarySeries->setBaseColor(QColor(0,0, 255));

    m_secondarySeries->setItemLabelFormat(QStringLiteral("@colLabel @rowLabel: @valueLabel"));
    m_secondarySeries->setMesh(QAbstract3DSeries::MeshBevelBar);
    m_secondarySeries->setMeshSmooth(false);
    m_secondarySeries->setVisible(false);
    m_secondarySeries->setBaseColor(QColor(255, 0, 0));

    m_graph->addSeries(m_primarySeries);
    m_graph->addSeries(m_secondarySeries);

    changePresetCamera();

    setLoadData();

    // Set up property animations for zooming to the selected bar
    Q3DCamera *camera = m_graph->scene()->activeCamera();
    m_defaultAngleX = camera->xRotation();
    m_defaultAngleY = camera->yRotation();
    m_defaultZoom = camera->zoomLevel();
    m_defaultTarget = camera->target();

    m_animationCameraX.setTargetObject(camera);
    m_animationCameraY.setTargetObject(camera);
    m_animationCameraZoom.setTargetObject(camera);
    m_animationCameraTarget.setTargetObject(camera);

    m_animationCameraX.setPropertyName("xRotation");
    m_animationCameraY.setPropertyName("yRotation");
    m_animationCameraZoom.setPropertyName("zoomLevel");
    m_animationCameraTarget.setPropertyName("target");

    int duration = 1700;
    m_animationCameraX.setDuration(duration);
    m_animationCameraY.setDuration(duration);
    m_animationCameraZoom.setDuration(duration);
    m_animationCameraTarget.setDuration(duration);

    // The zoom always first zooms out above the graph and then zooms in
    qreal zoomOutFraction = 0.3;
    m_animationCameraX.setKeyValueAt(zoomOutFraction, QVariant::fromValue(0.0f));
    m_animationCameraY.setKeyValueAt(zoomOutFraction, QVariant::fromValue(90.0f));
    m_animationCameraZoom.setKeyValueAt(zoomOutFraction, QVariant::fromValue(50.0f));
    m_animationCameraTarget.setKeyValueAt(zoomOutFraction,
                                          QVariant::fromValue(QVector3D(0.0f, 0.0f, 0.0f)));
}

GraphModifier::~GraphModifier()
{
    delete m_graph;
}
QStringList GraphModifier::getNodeAxisList() {
    return this->node_nums;
}
QStringList GraphModifier::getTimeAxisList() {
    return this->times;
}

void GraphModifier::setLoadData()
{
    // Create data arrays
    QBarDataArray *dataSet = new QBarDataArray;
    QBarDataArray *dataSet2 = new QBarDataArray;
    QBarDataRow *dataRow;
    QBarDataRow *dataRow2;

    dataSet->reserve(data.size() * times.size());
    //dataSet2->reserve(data.size() * times.size());

    for (int iNode = 0; iNode < data.size(); iNode++) {
        // Create a data row
        dataRow = new QBarDataRow(data.at(iNode).size());
        dataRow2 = new QBarDataRow(data.at(iNode).size());

        QVector<double> iNodeVectorData = data.at(iNode);

        for (int iNodeVectorIndex = 0; iNodeVectorIndex < (int)iNodeVectorData.size(); iNodeVectorIndex++) {
            // Add data to the row
            (*dataRow)[iNodeVectorIndex].setValue(iNodeVectorData.at(iNodeVectorIndex));
            (*dataRow2)[iNodeVectorIndex].setValue(averages.at(iNodeVectorIndex));
        }
        // Add the row to the set
        dataSet->append(dataRow);
        dataSet2->append(dataRow2);
    }

    // Add data to the data proxy (the data proxy assumes ownership of it)
    m_primarySeries->dataProxy()->resetArray(dataSet, node_nums, times);
    m_secondarySeries->dataProxy()->resetArray(dataSet2, node_nums, times);
    //! [5]
}

void GraphModifier::setLoadRange(int min, int max) {
    m_loadAxis->setRange(min, max);
}

void GraphModifier::changeRange(int range)
{
    if (range >= times.count())
        m_timeAxis->setRange(0, times.count() - 1);
    else
        m_timeAxis->setRange(range, range);
}

void GraphModifier::changeStyle(int style)
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(sender());
    if (comboBox) {
        m_barMesh = QAbstract3DSeries::Mesh(comboBox->itemData(style).toInt());
        m_primarySeries->setMesh(m_barMesh);
        //m_secondarySeries->setMesh(m_barMesh);
    }
}

void GraphModifier::changePresetCamera()
{
    m_animationCameraX.stop();
    m_animationCameraY.stop();
    m_animationCameraZoom.stop();
    m_animationCameraTarget.stop();

    // Restore camera target in case animation has changed it
    m_graph->scene()->activeCamera()->setTarget(QVector3D(0.0f, 0.0f, 0.0f));

    //! [10]
    static int preset = Q3DCamera::CameraPresetFront;

    m_graph->scene()->activeCamera()->setCameraPreset((Q3DCamera::CameraPreset)preset);

    if (++preset > Q3DCamera::CameraPresetDirectlyBelow)
        preset = Q3DCamera::CameraPresetFrontLow;
    //! [10]
}

void GraphModifier::changeTheme(int theme)
{
    Q3DTheme *currentTheme = m_graph->activeTheme();
    currentTheme->setType(Q3DTheme::Theme(theme));
    emit backgroundEnabledChanged(currentTheme->isBackgroundEnabled());
    emit gridEnabledChanged(currentTheme->isGridEnabled());
    emit fontChanged(currentTheme->font());
    emit fontSizeChanged(currentTheme->font().pointSize());
}

void GraphModifier::changeLabelBackground()
{
   //m_graph->activeTheme()->setLabelBackgroundEnabled(!m_graph->activeTheme()->isLabelBackgroundEnabled());
    if (m_graph->activeTheme()->isLabelBorderEnabled()) {
         m_graph->activeTheme()->setLabelBorderEnabled(false);
    } else {
        m_graph->activeTheme()->setLabelBorderEnabled(true);
    }
}

void GraphModifier::changeSelectionMode(int selectionMode)
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(sender());
    if (comboBox) {
        int flags = comboBox->itemData(selectionMode).toInt();
        m_graph->setSelectionMode(QAbstract3DGraph::SelectionFlags(flags));
    }
}

void GraphModifier::changeFont(const QFont &font)
{
    QFont newFont = font;
    m_graph->activeTheme()->setFont(newFont);
}

void GraphModifier::changeFontSize(int fontsize)
{
    m_fontSize = fontsize;
    QFont font = m_graph->activeTheme()->font();
    font.setPointSize(m_fontSize);
    m_graph->activeTheme()->setFont(font);
}

void GraphModifier::shadowQualityUpdatedByVisual(QAbstract3DGraph::ShadowQuality sq)
{
    int quality = int(sq);
    // Updates the UI component to show correct shadow quality
    emit shadowQualityChanged(quality);
}

void GraphModifier::changeLabelRotation(int rotation)
{
    m_loadAxis->setLabelAutoRotation(float(rotation));
    m_nodeAxis->setLabelAutoRotation(float(rotation));
    m_timeAxis->setLabelAutoRotation(float(rotation));
}

void GraphModifier::setAxisTitleVisibility(bool enabled)
{
    m_loadAxis->setTitleVisible(enabled);
    m_nodeAxis->setTitleVisible(enabled);
    m_timeAxis->setTitleVisible(enabled);
}

void GraphModifier::setAxisTitleFixed(bool enabled)
{
    m_loadAxis->setTitleFixed(enabled);
    m_nodeAxis->setTitleFixed(enabled);
    m_timeAxis->setTitleFixed(enabled);
}

//! [11]
void GraphModifier::zoomToSelectedBar()
{
    m_animationCameraX.stop();
    m_animationCameraY.stop();
    m_animationCameraZoom.stop();
    m_animationCameraTarget.stop();

    Q3DCamera *camera = m_graph->scene()->activeCamera();
    float currentX = camera->xRotation();
    float currentY = camera->yRotation();
    float currentZoom = camera->zoomLevel();
    QVector3D currentTarget = camera->target();

    m_animationCameraX.setStartValue(QVariant::fromValue(currentX));
    m_animationCameraY.setStartValue(QVariant::fromValue(currentY));
    m_animationCameraZoom.setStartValue(QVariant::fromValue(currentZoom));
    m_animationCameraTarget.setStartValue(QVariant::fromValue(currentTarget));

    QPoint selectedBar = m_graph->selectedSeries()
            ? m_graph->selectedSeries()->selectedBar()
            : QBar3DSeries::invalidSelectionPosition();

    if (selectedBar != QBar3DSeries::invalidSelectionPosition()) {
        // Normalize selected bar position within axis range to determine target coordinates
        //! [13]
        QVector3D endTarget;
        float xMin = m_graph->columnAxis()->min();
        float xRange = m_graph->columnAxis()->max() - xMin;
        float zMin = m_graph->rowAxis()->min();
        float zRange = m_graph->rowAxis()->max() - zMin;
        endTarget.setX((selectedBar.y() - xMin) / xRange * 2.0f - 1.0f);
        endTarget.setZ((selectedBar.x() - zMin) / zRange * 2.0f - 1.0f);
        //! [13]

        // Rotate the camera so that it always points approximately to the graph center
        //! [15]
        qreal endAngleX = qAtan(qreal(endTarget.z() / endTarget.x())) / M_PI * -180.0 + 90.0;
        if (endTarget.x() > 0.0f)
            endAngleX -= 180.0f;
        float barValue = m_graph->selectedSeries()->dataProxy()->itemAt(selectedBar.x(),
                                                                        selectedBar.y())->value();
        float endAngleY = barValue >= 0.0f ? 30.0f : -30.0f;
        if (m_graph->valueAxis()->reversed())
            endAngleY *= -1.0f;
        //! [15]

        m_animationCameraX.setEndValue(QVariant::fromValue(float(endAngleX)));
        m_animationCameraY.setEndValue(QVariant::fromValue(endAngleY));
        m_animationCameraZoom.setEndValue(QVariant::fromValue(250));
        //! [14]
        m_animationCameraTarget.setEndValue(QVariant::fromValue(endTarget));
        //! [14]
    } else {
        // No selected bar, so return to the default view
        m_animationCameraX.setEndValue(QVariant::fromValue(m_defaultAngleX));
        m_animationCameraY.setEndValue(QVariant::fromValue(m_defaultAngleY));
        m_animationCameraZoom.setEndValue(QVariant::fromValue(m_defaultZoom));
        m_animationCameraTarget.setEndValue(QVariant::fromValue(m_defaultTarget));
    }

    m_animationCameraX.start();
    m_animationCameraY.start();
    m_animationCameraZoom.start();
    m_animationCameraTarget.start();
}
//! [11]

void GraphModifier::changeShadowQuality(int quality)
{
    QAbstract3DGraph::ShadowQuality sq = QAbstract3DGraph::ShadowQuality(quality);
    m_graph->setShadowQuality(sq);
    emit shadowQualityChanged(quality);
}

//! [7]
void GraphModifier::rotateX(int rotation)
{
    m_xRotation = rotation;
    m_graph->scene()->activeCamera()->setCameraPosition(m_xRotation, m_yRotation);
}

void GraphModifier::rotateY(int rotation)
{
    m_yRotation = rotation;
    m_graph->scene()->activeCamera()->setCameraPosition(m_xRotation, m_yRotation);
}
//! [7]

void GraphModifier::setBackgroundEnabled(int enabled)
{
    m_graph->activeTheme()->setBackgroundEnabled(bool(enabled));
}

void GraphModifier::setGridEnabled(int enabled)
{
    m_graph->activeTheme()->setGridEnabled(bool(enabled));
}

void GraphModifier::setSmoothBars(int smooth)
{
    m_smooth = bool(smooth);
    m_primarySeries->setMeshSmooth(m_smooth);
    //m_secondarySeries->setMeshSmooth(m_smooth);
}

void GraphModifier::setSeriesVisibility(int enabled)
{
    m_secondarySeries->setVisible(bool(enabled));
}

void GraphModifier::setReverseValueAxis(int enabled)
{
    m_graph->valueAxis()->setReversed(enabled);
}

void GraphModifier::setReflection(bool enabled)
{
    m_graph->setReflection(enabled);
}
