#include "sportmodeeditorwidget.h"
#include "sportmodestorage.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QScrollArea>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>

static const int GPS_INTERVALS[]   = {0, 1, 2, 5, 10, 15, 30, 60};
static const int GPS_INTERVAL_COUNT = 8;
static const int REC_INTERVALS[]   = {1, 5, 10, 60};
static const int REC_INTERVAL_COUNT = 4;

SportModeEditorWidget::SportModeEditorWidget(QWidget *parent)
    : QWidget(parent)
{
    // --- Left panel: list + add/remove ---
    m_list = new QListWidget;
    m_addBtn    = new QPushButton(tr("Add"));
    m_removeBtn = new QPushButton(tr("Remove"));

    QHBoxLayout *listBtns = new QHBoxLayout;
    listBtns->addWidget(m_addBtn);
    listBtns->addWidget(m_removeBtn);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(new QLabel(tr("Sport Modes:")));
    leftLayout->addWidget(m_list);
    leftLayout->addLayout(listBtns);

    QWidget *leftPanel = new QWidget;
    leftPanel->setMaximumWidth(180);
    leftPanel->setLayout(leftLayout);

    // --- Right panel: form inside scroll area ---
    m_name       = new QLineEdit;  m_name->setMaxLength(15);
    m_activityId  = new QSpinBox;  m_activityId->setRange(0, 999);
    m_sportModeId = new QSpinBox;  m_sportModeId->setRange(0, 999);

    m_gpsInterval = new QComboBox;
    m_gpsInterval->addItem(tr("Best (0)"), 0);
    m_gpsInterval->addItem("1 s", 1);
    m_gpsInterval->addItem("2 s", 2);
    m_gpsInterval->addItem("5 s", 5);
    m_gpsInterval->addItem("10 s", 10);
    m_gpsInterval->addItem("15 s", 15);
    m_gpsInterval->addItem("30 s", 30);
    m_gpsInterval->addItem("60 s", 60);

    m_recInterval = new QComboBox;
    m_recInterval->addItem("1 s", 1);
    m_recInterval->addItem("5 s", 5);
    m_recInterval->addItem("10 s", 10);
    m_recInterval->addItem("60 s", 60);

    m_altiBaroMode = new QComboBox;
    m_altiBaroMode->addItem(tr("Off"), 0);
    m_altiBaroMode->addItem(tr("Altitude"), 1);
    m_altiBaroMode->addItem(tr("Barometer"), 2);

    m_hrBelt       = new QCheckBox(tr("HR Belt"));
    m_footPod      = new QCheckBox(tr("Foot Pod"));
    m_bikePod      = new QCheckBox(tr("Bike Pod"));
    m_cadencePod   = new QCheckBox(tr("Cadence Pod"));
    m_accelerometer = new QCheckBox(tr("Accelerometer"));

    m_useHrLimits = new QCheckBox(tr("Use HR limits"));
    m_hrMax = new QSpinBox;  m_hrMax->setRange(30, 220);  m_hrMax->setSuffix(" bpm");
    m_hrMin = new QSpinBox;  m_hrMin->setRange(30, 220);  m_hrMin->setSuffix(" bpm");

    m_useAutolap = new QCheckBox(tr("Use Autolap"));
    m_autolap = new QSpinBox;  m_autolap->setRange(0, 100000);  m_autolap->setSuffix(" m");

    m_autoPause = new QDoubleSpinBox;
    m_autoPause->setRange(0.0, 10.0);
    m_autoPause->setSingleStep(0.1);
    m_autoPause->setSuffix(" m/s (0=off)");

    m_backlight = new QComboBox;
    m_backlight->addItem(tr("Auto Off"), 0);
    m_backlight->addItem(tr("Always On"), 1);
    m_backlight->addItem(tr("Invert"), 2);

    m_displayNegative = new QCheckBox(tr("Negative display"));

    QFormLayout *form = new QFormLayout;
    form->addRow(tr("Name:"), m_name);
    form->addRow(tr("Activity ID:"), m_activityId);
    form->addRow(tr("Sport Mode ID:"), m_sportModeId);
    form->addRow(tr("GPS Interval:"), m_gpsInterval);
    form->addRow(tr("Recording Interval:"), m_recInterval);
    form->addRow(tr("Alti/Baro Mode:"), m_altiBaroMode);
    form->addRow(tr("Sensors:"), m_hrBelt);
    form->addRow(QString(), m_footPod);
    form->addRow(QString(), m_bikePod);
    form->addRow(QString(), m_cadencePod);
    form->addRow(QString(), m_accelerometer);
    form->addRow(tr("HR Limits:"), m_useHrLimits);
    form->addRow(tr("HR Max:"), m_hrMax);
    form->addRow(tr("HR Min:"), m_hrMin);
    form->addRow(tr("Autolap:"), m_useAutolap);
    form->addRow(tr("Autolap distance:"), m_autolap);
    form->addRow(tr("Auto Pause speed:"), m_autoPause);
    form->addRow(tr("Backlight:"), m_backlight);
    form->addRow(QString(), m_displayNegative);

    QWidget *formWidget = new QWidget;
    formWidget->setLayout(form);

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidget(formWidget);
    scroll->setWidgetResizable(true);

    // --- Bottom buttons ---
    m_writeBtn    = new QPushButton(tr("Write to Watch"));
    m_defaultsBtn = new QPushButton(tr("Load Defaults"));
    m_saveFileBtn = new QPushButton(tr("Save to File"));
    m_loadFileBtn = new QPushButton(tr("Load from File"));

    QHBoxLayout *bottomBtns = new QHBoxLayout;
    bottomBtns->addWidget(m_writeBtn);
    bottomBtns->addStretch();
    bottomBtns->addWidget(m_defaultsBtn);
    bottomBtns->addWidget(m_saveFileBtn);
    bottomBtns->addWidget(m_loadFileBtn);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(scroll);
    rightLayout->addLayout(bottomBtns);

    // --- Top-level layout ---
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(leftPanel);
    mainLayout->addLayout(rightLayout, 1);

    setFormEnabled(false);

    connect(m_list, &QListWidget::currentRowChanged, this, &SportModeEditorWidget::onModeSelected);
    connect(m_addBtn,    &QPushButton::clicked, this, &SportModeEditorWidget::onAddMode);
    connect(m_removeBtn, &QPushButton::clicked, this, &SportModeEditorWidget::onRemoveMode);
    connect(m_writeBtn,    &QPushButton::clicked, this, &SportModeEditorWidget::onWriteToWatch);
    connect(m_defaultsBtn, &QPushButton::clicked, this, &SportModeEditorWidget::onLoadDefaults);
    connect(m_saveFileBtn, &QPushButton::clicked, this, &SportModeEditorWidget::onSaveToFile);
    connect(m_loadFileBtn, &QPushButton::clicked, this, &SportModeEditorWidget::onLoadFromFile);
}

void SportModeEditorWidget::setModes(const QList<QVariantMap> &modes)
{
    m_loading = true;
    m_modes = modes;
    m_list->clear();
    for (const QVariantMap &m : m_modes)
        m_list->addItem(m.value("Name", tr("(unnamed)")).toString());
    m_loading = false;
    m_currentRow = -1;
    if (!m_modes.isEmpty())
        m_list->setCurrentRow(0);
}

QList<QVariantMap> SportModeEditorWidget::modes() const
{
    return m_modes;
}

void SportModeEditorWidget::onModeSelected(int row)
{
    if (m_loading) return;
    if (m_currentRow >= 0 && m_currentRow < m_modes.size())
        saveCurrentForm();
    m_currentRow = row;
    if (row < 0 || row >= m_modes.size()) {
        setFormEnabled(false);
        return;
    }
    m_loading = true;
    loadFormFromMap(m_modes[row]);
    m_loading = false;
    setFormEnabled(true);
}

void SportModeEditorWidget::saveCurrentForm()
{
    if (m_currentRow < 0 || m_currentRow >= m_modes.size()) return;
    QVariantMap &m = m_modes[m_currentRow];
    m["Name"]              = m_name->text();
    m["ActivityID"]        = m_activityId->value();
    m["CustomModeID"]      = m_sportModeId->value();
    m["GPSInterval"]       = m_gpsInterval->currentData().toInt();
    m["RecordingInterval"] = m_recInterval->currentData().toInt();
    m["AltiBaroMode"]      = m_altiBaroMode->currentData().toInt();
    m["UseHRBelt"]         = m_hrBelt->isChecked();
    m["UseFootPOD"]        = m_footPod->isChecked();
    m["UseBikePOD"]        = m_bikePod->isChecked();
    m["UseCadencePOD"]     = m_cadencePod->isChecked();
    m["UseAccelerometer"]  = m_accelerometer->isChecked();
    m["UseHRLimits"]       = m_useHrLimits->isChecked();
    m["HRLimitHigh"]       = m_hrMax->value();
    m["HRLimitLow"]        = m_hrMin->value();
    m["UseAutolap"]        = m_useAutolap->isChecked();
    m["AutolapDistance"]   = m_autolap->value();
    m["AutoPauseSpeed"]    = m_autoPause->value();
    m["BacklightMode"]     = m_backlight->currentData().toInt();
    m["DisplayIsNegative"] = m_displayNegative->isChecked() ? 1 : 0;

    // Update list item label if name changed
    if (m_list->item(m_currentRow))
        m_list->item(m_currentRow)->setText(m["Name"].toString());
}

void SportModeEditorWidget::loadFormFromMap(const QVariantMap &m)
{
    m_name->setText(m.value("Name").toString());
    m_activityId->setValue(m.value("ActivityID", 0).toInt());
    m_sportModeId->setValue(m.value("CustomModeID", 0).toInt());

    int gpsVal = m.value("GPSInterval", 1).toInt();
    int gpsIdx = m_gpsInterval->findData(gpsVal);
    m_gpsInterval->setCurrentIndex(gpsIdx >= 0 ? gpsIdx : 1);

    int recVal = m.value("RecordingInterval", 1).toInt();
    int recIdx = m_recInterval->findData(recVal);
    m_recInterval->setCurrentIndex(recIdx >= 0 ? recIdx : 0);

    int altiVal = m.value("AltiBaroMode", 0).toInt();
    int altiIdx = m_altiBaroMode->findData(altiVal);
    m_altiBaroMode->setCurrentIndex(altiIdx >= 0 ? altiIdx : 0);

    m_hrBelt->setChecked(m.value("UseHRBelt", false).toBool());
    m_footPod->setChecked(m.value("UseFootPOD", false).toBool());
    m_bikePod->setChecked(m.value("UseBikePOD", false).toBool());
    m_cadencePod->setChecked(m.value("UseCadencePOD", false).toBool());
    m_accelerometer->setChecked(m.value("UseAccelerometer", false).toBool());
    m_useHrLimits->setChecked(m.value("UseHRLimits", false).toBool());
    m_hrMax->setValue(m.value("HRLimitHigh", 180).toInt());
    m_hrMin->setValue(m.value("HRLimitLow", 120).toInt());
    m_useAutolap->setChecked(m.value("UseAutolap", false).toBool());
    m_autolap->setValue(m.value("AutolapDistance", 0).toInt());
    m_autoPause->setValue(m.value("AutoPauseSpeed", 0.0).toDouble());

    int blVal = m.value("BacklightMode", 0).toInt();
    int blIdx = m_backlight->findData(blVal);
    m_backlight->setCurrentIndex(blIdx >= 0 ? blIdx : 0);

    m_displayNegative->setChecked(m.value("DisplayIsNegative", 0).toInt() != 0);
}

void SportModeEditorWidget::setFormEnabled(bool enabled)
{
    m_name->setEnabled(enabled);
    m_activityId->setEnabled(enabled);
    m_sportModeId->setEnabled(enabled);
    m_gpsInterval->setEnabled(enabled);
    m_recInterval->setEnabled(enabled);
    m_altiBaroMode->setEnabled(enabled);
    m_hrBelt->setEnabled(enabled);
    m_footPod->setEnabled(enabled);
    m_bikePod->setEnabled(enabled);
    m_cadencePod->setEnabled(enabled);
    m_accelerometer->setEnabled(enabled);
    m_useHrLimits->setEnabled(enabled);
    m_hrMax->setEnabled(enabled);
    m_hrMin->setEnabled(enabled);
    m_useAutolap->setEnabled(enabled);
    m_autolap->setEnabled(enabled);
    m_autoPause->setEnabled(enabled);
    m_backlight->setEnabled(enabled);
    m_displayNegative->setEnabled(enabled);
}

void SportModeEditorWidget::onAddMode()
{
    if (m_currentRow >= 0 && m_currentRow < m_modes.size())
        saveCurrentForm();

    QVariantMap nm = SportModeStorage::factoryDefaults().first();
    nm["Name"]         = tr("New Mode");
    nm["CustomModeID"] = m_modes.size();
    m_modes.append(nm);

    m_loading = true;
    m_list->addItem(nm["Name"].toString());
    m_loading = false;
    m_list->setCurrentRow(m_modes.size() - 1);
}

void SportModeEditorWidget::onRemoveMode()
{
    int row = m_list->currentRow();
    if (row < 0 || row >= m_modes.size()) return;
    m_loading = true;
    m_modes.removeAt(row);
    delete m_list->takeItem(row);
    m_loading = false;
    m_currentRow = -1;
    if (!m_modes.isEmpty())
        m_list->setCurrentRow(qMin(row, m_modes.size() - 1));
    else
        setFormEnabled(false);
}

void SportModeEditorWidget::onWriteToWatch()
{
    if (m_currentRow >= 0 && m_currentRow < m_modes.size())
        saveCurrentForm();
    emit writeToWatchRequested(m_modes);
}

void SportModeEditorWidget::onLoadDefaults()
{
    if (QMessageBox::question(this, tr("Load Defaults"),
            tr("Replace current sport modes with factory defaults?"),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        setModes(SportModeStorage::factoryDefaults());
}

void SportModeEditorWidget::onSaveToFile()
{
    if (m_currentRow >= 0 && m_currentRow < m_modes.size())
        saveCurrentForm();
    QString path = QFileDialog::getSaveFileName(this, tr("Save sport modes"),
        SportModeStorage::defaultPath(), tr("JSON files (*.json)"));
    if (path.isEmpty()) return;
    if (SportModeStorage::save(m_modes, path))
        QMessageBox::information(this, tr("Saved"), tr("Sport modes saved to:\n") + path);
    else
        QMessageBox::critical(this, tr("Error"), tr("Could not save file."));
}

void SportModeEditorWidget::onLoadFromFile()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Load sport modes"),
        SportModeStorage::defaultPath(), tr("JSON files (*.json)"));
    if (path.isEmpty()) return;
    QList<QVariantMap> loaded = SportModeStorage::load(path);
    if (loaded.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Could not load file or file is empty."));
        return;
    }
    setModes(loaded);
}
