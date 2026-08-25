#include "trainingplandialog.h"
#include "trainingplanstorage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QDateEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QLabel>
#include <QGroupBox>
#include <QSplitter>
#include <QClipboard>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDate>

// See workouteditordialog.cpp for provenance of this URL.
static const char *COMPILER_FORUM_URL = "https://forum.suunto.com/topic/7592/ambit-apps-compilation";

TrainingPlanDialog::TrainingPlanDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Training Plan Builder (experimental)"));
    resize(1100, 750);

    QLabel *warning = new QLabel(tr(
        "<b>Experimental:</b> this schedules workouts as date-gated App Zone apps (the watch "
        "shows a countdown until each planned date, then runs that day's workout). Whether a "
        "compiled app actually executes on real Ambit3-family firmware is unverified - and "
        "writing a compiled app onto the watch is not implemented in this dialog at all yet; "
        "it only generates sources for you to compile and save."));
    warning->setWordWrap(true);
    warning->setStyleSheet("QLabel { background: #fff3cd; color: #664d03; padding: 6px; border: 1px solid #ffe69c; }");

    // --- Top: plan name + new/load/save ---
    m_planName = new QLineEdit(tr("My training plan"));
    m_newBtn = new QPushButton(tr("New"));
    m_loadBtn = new QPushButton(tr("Load..."));
    m_saveBtn = new QPushButton(tr("Save"));

    QFormLayout *nameForm = new QFormLayout;
    QHBoxLayout *planBtns = new QHBoxLayout;
    planBtns->addWidget(m_newBtn);
    planBtns->addWidget(m_loadBtn);
    planBtns->addWidget(m_saveBtn);
    QWidget *planNameRow = new QWidget;
    QHBoxLayout *planNameLayout = new QHBoxLayout(planNameRow);
    planNameLayout->setContentsMargins(0, 0, 0, 0);
    planNameLayout->addWidget(m_planName, 1);
    planNameLayout->addLayout(planBtns);
    nameForm->addRow(tr("Plan name:"), planNameRow);

    // --- Left: entries (date + workout name) ---
    m_entryList = new QListWidget;
    m_addEntryBtn = new QPushButton(tr("Add Entry"));
    m_removeEntryBtn = new QPushButton(tr("Remove Entry"));
    QHBoxLayout *entryBtns = new QHBoxLayout;
    entryBtns->addWidget(m_addEntryBtn);
    entryBtns->addWidget(m_removeEntryBtn);

    m_entryDate = new QDateEdit(QDate::currentDate());
    m_entryDate->setCalendarPopup(true);
    m_entryDate->setDisplayFormat("yyyy-MM-dd");
    m_entryWorkoutName = new QLineEdit;

    QFormLayout *entryForm = new QFormLayout;
    entryForm->addRow(tr("Date:"), m_entryDate);
    entryForm->addRow(tr("Workout name:"), m_entryWorkoutName);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(new QLabel(tr("Plan entries:")));
    leftLayout->addWidget(m_entryList);
    leftLayout->addLayout(entryBtns);
    leftLayout->addLayout(entryForm);
    QWidget *leftPanel = new QWidget;
    leftPanel->setLayout(leftLayout);

    // --- Middle: steps of the selected entry's workout ---
    m_stepList = new QListWidget;
    m_addStepBtn = new QPushButton(tr("Add Step"));
    m_removeStepBtn = new QPushButton(tr("Remove Step"));
    QHBoxLayout *stepBtns = new QHBoxLayout;
    stepBtns->addWidget(m_addStepBtn);
    stepBtns->addWidget(m_removeStepBtn);

    m_stepType = new QComboBox;
    m_stepType->addItem(tr("Warm-up"), "warmup");
    m_stepType->addItem(tr("Interval"), "interval");
    m_stepType->addItem(tr("Recovery"), "recovery");
    m_stepType->addItem(tr("Cool-down"), "cooldown");
    m_stepType->addItem(tr("Repeat start"), "repeatStart");
    m_stepType->addItem(tr("Repeat end"), "repeatEnd");

    m_repeatCountLabel = new QLabel(tr("Repeat count:"));
    m_repeatCount = new QSpinBox;
    m_repeatCount->setRange(2, 99);
    m_repeatCount->setValue(2);

    m_stepText = new QLineEdit;

    m_durationKind = new QComboBox;
    m_durationKind->addItem(tr("Time"), "time");
    m_durationKind->addItem(tr("Distance"), "distance");
    m_durationKind->addItem(tr("Ascent"), "ascent");
    m_durationKind->addItem(tr("Next Lap button press"), "lap");
    m_durationKind->addItem(tr("Energy (kcal)"), "energy");
    m_durationKind->addItem(tr("Until HR below..."), "hr_below");
    m_durationKind->addItem(tr("Until HR above..."), "hr_above");

    m_durationValue = new QSpinBox;
    m_durationValue->setRange(0, 100000);

    m_targetKind = new QComboBox;
    m_targetKind->addItem(tr("None"), "none");
    m_targetKind->addItem(tr("Heart rate"), "hr");
    m_targetKind->addItem(tr("Pace"), "pace");
    m_targetKind->addItem(tr("Speed"), "speed");
    m_targetKind->addItem(tr("Vertical speed"), "vertical_speed");
    m_targetKind->addItem(tr("Power"), "power");
    m_targetKind->addItem(tr("Cadence"), "cadence");

    m_targetMin = new QSpinBox; m_targetMin->setRange(0, 100000);
    m_targetMax = new QSpinBox; m_targetMax->setRange(0, 100000);
    QHBoxLayout *targetRangeLayout = new QHBoxLayout;
    targetRangeLayout->addWidget(m_targetMin);
    targetRangeLayout->addWidget(new QLabel(tr("to")));
    targetRangeLayout->addWidget(m_targetMax);
    QWidget *targetRangeWidget = new QWidget;
    targetRangeWidget->setLayout(targetRangeLayout);

    m_notifyBeep = new QCheckBox(tr("Beep"));
    m_notifyBeep->setChecked(true);
    m_notifyLight = new QCheckBox(tr("Light"));
    m_notifyLight->setChecked(true);
    QHBoxLayout *notifyLayout = new QHBoxLayout;
    notifyLayout->addWidget(m_notifyBeep);
    notifyLayout->addWidget(m_notifyLight);
    QWidget *notifyWidget = new QWidget;
    notifyWidget->setLayout(notifyLayout);

    QFormLayout *stepForm = new QFormLayout;
    stepForm->addRow(tr("Step type:"), m_stepType);
    stepForm->addRow(m_repeatCountLabel, m_repeatCount);
    stepForm->addRow(tr("Text:"), m_stepText);
    stepForm->addRow(tr("Duration:"), m_durationKind);
    stepForm->addRow(tr("Duration value:"), m_durationValue);
    stepForm->addRow(tr("Target:"), m_targetKind);
    stepForm->addRow(tr("Target range:"), targetRangeWidget);
    stepForm->addRow(tr("Notify on entry:"), notifyWidget);

    QGroupBox *stepBox = new QGroupBox(tr("Selected step"));
    stepBox->setLayout(stepForm);

    QVBoxLayout *middleLayout = new QVBoxLayout;
    middleLayout->addWidget(new QLabel(tr("Steps of selected entry's workout:")));
    middleLayout->addWidget(m_stepList);
    middleLayout->addLayout(stepBtns);
    middleLayout->addWidget(stepBox);
    QWidget *middlePanel = new QWidget;
    middlePanel->setLayout(middleLayout);

    // --- Right: pack & compile hand-off ---
    m_packBtn = new QPushButton(tr("Pack && Generate Sources"));
    m_packList = new QListWidget;
    m_sourceView = new QPlainTextEdit;
    m_sourceView->setReadOnly(true);
    m_sourceView->setFont(QFont("monospace"));
    m_copyBtn = new QPushButton(tr("Copy to Clipboard"));
    m_openCompilerBtn = new QPushButton(tr("Open Compiler Website..."));
    QHBoxLayout *genBtns = new QHBoxLayout;
    genBtns->addWidget(m_copyBtn);
    genBtns->addWidget(m_openCompilerBtn);

    QLabel *pasteLabel = new QLabel(tr(
        "After compiling the SELECTED pack above on the website, paste its JSON output here "
        "and save it."));
    pasteLabel->setWordWrap(true);
    m_compiledPaste = new QPlainTextEdit;
    m_compiledPaste->setPlaceholderText(tr("Paste the compiler's JSON output here..."));
    m_saveCompiledBtn = new QPushButton(tr("Save Compiled App..."));

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(m_packBtn);
    rightLayout->addWidget(new QLabel(tr("Packs (one App Zone app each):")));
    rightLayout->addWidget(m_packList);
    rightLayout->addWidget(new QLabel(tr("Selected pack's source (paste into the compiler):")));
    rightLayout->addWidget(m_sourceView, 1);
    rightLayout->addLayout(genBtns);
    rightLayout->addWidget(pasteLabel);
    rightLayout->addWidget(m_compiledPaste);
    rightLayout->addWidget(m_saveCompiledBtn);
    QWidget *rightPanel = new QWidget;
    rightPanel->setLayout(rightLayout);

    QSplitter *splitter = new QSplitter;
    splitter->addWidget(leftPanel);
    splitter->addWidget(middlePanel);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);
    splitter->setStretchFactor(2, 1);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(warning);
    mainLayout->addLayout(nameForm);
    mainLayout->addWidget(splitter, 1);

    setEntryFormEnabled(false);
    setStepFormEnabled(false);

    connect(m_newBtn, &QPushButton::clicked, this, &TrainingPlanDialog::onNewPlan);
    connect(m_loadBtn, &QPushButton::clicked, this, &TrainingPlanDialog::onLoadPlan);
    connect(m_saveBtn, &QPushButton::clicked, this, &TrainingPlanDialog::onSavePlan);

    connect(m_entryList, &QListWidget::currentRowChanged, this, &TrainingPlanDialog::onEntrySelected);
    connect(m_addEntryBtn, &QPushButton::clicked, this, &TrainingPlanDialog::onAddEntry);
    connect(m_removeEntryBtn, &QPushButton::clicked, this, &TrainingPlanDialog::onRemoveEntry);

    connect(m_stepList, &QListWidget::currentRowChanged, this, &TrainingPlanDialog::onStepSelected);
    connect(m_addStepBtn, &QPushButton::clicked, this, &TrainingPlanDialog::onAddStep);
    connect(m_removeStepBtn, &QPushButton::clicked, this, &TrainingPlanDialog::onRemoveStep);
    connect(m_stepType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TrainingPlanDialog::onStepTypeChanged);

    connect(m_packBtn, &QPushButton::clicked, this, &TrainingPlanDialog::onPackAndGenerate);
    connect(m_packList, &QListWidget::currentRowChanged, this, &TrainingPlanDialog::onPackSelected);
    connect(m_copyBtn, &QPushButton::clicked, this, &TrainingPlanDialog::onCopySourceToClipboard);
    connect(m_openCompilerBtn, &QPushButton::clicked, this, &TrainingPlanDialog::onOpenCompilerWebsite);
    connect(m_saveCompiledBtn, &QPushButton::clicked, this, &TrainingPlanDialog::onSaveCompiledApp);
}

/* ============================================================ plans ============================================================ */

void TrainingPlanDialog::onNewPlan()
{
    saveCurrentEntryForm();
    m_entries.clear();
    m_currentEntryRow = -1;
    m_planName->setText(tr("My training plan"));
    refreshEntryList();
    setEntryFormEnabled(false);
    setStepFormEnabled(false);
    m_packs.clear();
    m_packList->clear();
    m_sourceView->clear();
}

void TrainingPlanDialog::onLoadPlan()
{
    QList<TrainingPlan> plans = TrainingPlanStorage::load();
    if (plans.isEmpty()) {
        QMessageBox::information(this, tr("No saved plans"),
            tr("No training plans have been saved yet (%1).").arg(TrainingPlanStorage::defaultPath()));
        return;
    }

    QStringList names;
    for (const TrainingPlan &p : plans) {
        names << p.name;
    }
    bool ok = false;
    QString chosen = QInputDialog::getItem(this, tr("Load Training Plan"), tr("Plan:"), names, 0, false, &ok);
    if (!ok) {
        return;
    }

    for (const TrainingPlan &p : plans) {
        if (p.name == chosen) {
            saveCurrentEntryForm();
            m_planName->setText(p.name);
            m_entries = p.entries;
            m_currentEntryRow = -1;
            refreshEntryList();
            setEntryFormEnabled(false);
            setStepFormEnabled(false);
            m_packs.clear();
            m_packList->clear();
            m_sourceView->clear();
            break;
        }
    }
}

void TrainingPlanDialog::onSavePlan()
{
    saveCurrentEntryForm();

    TrainingPlan plan;
    plan.name = m_planName->text().trimmed().isEmpty() ? tr("My training plan") : m_planName->text().trimmed();
    plan.entries = m_entries;

    QList<TrainingPlan> plans = TrainingPlanStorage::load();
    bool replaced = false;
    for (TrainingPlan &p : plans) {
        if (p.name == plan.name) {
            p = plan;
            replaced = true;
            break;
        }
    }
    if (!replaced) {
        plans.append(plan);
    }

    if (TrainingPlanStorage::save(plans)) {
        QMessageBox::information(this, tr("Saved"), tr("Plan saved to %1").arg(TrainingPlanStorage::defaultPath()));
    } else {
        QMessageBox::warning(this, tr("Save failed"), tr("Could not write to %1").arg(TrainingPlanStorage::defaultPath()));
    }
}

/* ============================================================ entries ============================================================ */

void TrainingPlanDialog::refreshEntryList()
{
    m_loading = true;
    m_entryList->clear();
    for (const TrainingPlanEntry &entry : m_entries) {
        QString label = entry.date;
        if (!entry.workout.name.isEmpty()) {
            label += " - " + entry.workout.name;
        }
        m_entryList->addItem(label);
    }
    m_loading = false;
}

void TrainingPlanDialog::onEntrySelected(int row)
{
    if (m_loading) {
        return;
    }
    saveCurrentEntryForm();
    m_currentEntryRow = row;
    m_currentStepRow = -1;
    if (row < 0 || row >= m_entries.size()) {
        setEntryFormEnabled(false);
        setStepFormEnabled(false);
        m_stepList->clear();
        return;
    }
    setEntryFormEnabled(true);
    loadEntryFormFrom(m_entries[row]);
}

void TrainingPlanDialog::onAddEntry()
{
    saveCurrentEntryForm();
    m_entries.append(TrainingPlanEntry());
    m_entries.last().date = QDate::currentDate().toString("yyyy-MM-dd");
    refreshEntryList();
    m_entryList->setCurrentRow(m_entries.size() - 1);
}

void TrainingPlanDialog::onRemoveEntry()
{
    if (m_currentEntryRow < 0 || m_currentEntryRow >= m_entries.size()) {
        return;
    }
    m_entries.removeAt(m_currentEntryRow);
    m_currentEntryRow = -1;
    refreshEntryList();
    if (!m_entries.isEmpty()) {
        m_entryList->setCurrentRow(qMin(m_currentEntryRow, m_entries.size() - 1));
    } else {
        setEntryFormEnabled(false);
        setStepFormEnabled(false);
        m_stepList->clear();
    }
}

void TrainingPlanDialog::setEntryFormEnabled(bool enabled)
{
    m_entryDate->setEnabled(enabled);
    m_entryWorkoutName->setEnabled(enabled);
    m_addStepBtn->setEnabled(enabled);
}

void TrainingPlanDialog::loadEntryFormFrom(const TrainingPlanEntry &entry)
{
    m_loading = true;
    QDate d = QDate::fromString(entry.date, "yyyy-MM-dd");
    m_entryDate->setDate(d.isValid() ? d : QDate::currentDate());
    m_entryWorkoutName->setText(entry.workout.name);
    m_loading = false;

    refreshStepList();
    if (!entry.workout.steps.isEmpty()) {
        m_stepList->setCurrentRow(0);
    } else {
        setStepFormEnabled(false);
    }
}

void TrainingPlanDialog::saveCurrentEntryForm()
{
    saveCurrentStepForm();
    if (m_loading || m_currentEntryRow < 0 || m_currentEntryRow >= m_entries.size()) {
        return;
    }
    TrainingPlanEntry &entry = m_entries[m_currentEntryRow];
    entry.date = m_entryDate->date().toString("yyyy-MM-dd");
    entry.workout.name = m_entryWorkoutName->text();

    refreshEntryList();
    m_loading = true;
    m_entryList->setCurrentRow(m_currentEntryRow);
    m_loading = false;
}

/* ============================================================ steps ============================================================ */

void TrainingPlanDialog::refreshStepList()
{
    if (m_currentEntryRow < 0 || m_currentEntryRow >= m_entries.size()) {
        m_stepList->clear();
        return;
    }
    m_loading = true;
    m_stepList->clear();
    for (const WorkoutStep &step : m_entries[m_currentEntryRow].workout.steps) {
        QString label = step.type.typeName;
        if (step.type.typeName == "repeatStart") {
            label = tr("Repeat start (x%1)").arg(step.type.value);
        } else if (!step.text.isEmpty()) {
            label = QString("%1 - %2").arg(step.type.typeName, step.text);
        }
        m_stepList->addItem(label);
    }
    m_loading = false;
}

void TrainingPlanDialog::onStepSelected(int row)
{
    if (m_loading) {
        return;
    }
    saveCurrentStepForm();
    m_currentStepRow = row;
    if (m_currentEntryRow < 0 || row < 0 || row >= m_entries[m_currentEntryRow].workout.steps.size()) {
        setStepFormEnabled(false);
        return;
    }
    setStepFormEnabled(true);
    loadStepFormFrom(m_entries[m_currentEntryRow].workout.steps[row]);
}

void TrainingPlanDialog::onAddStep()
{
    if (m_currentEntryRow < 0 || m_currentEntryRow >= m_entries.size()) {
        return;
    }
    saveCurrentStepForm();
    m_entries[m_currentEntryRow].workout.steps.append(WorkoutStep());
    refreshStepList();
    m_stepList->setCurrentRow(m_entries[m_currentEntryRow].workout.steps.size() - 1);
}

void TrainingPlanDialog::onRemoveStep()
{
    if (m_currentEntryRow < 0 || m_currentStepRow < 0 ||
        m_currentStepRow >= m_entries[m_currentEntryRow].workout.steps.size()) {
        return;
    }
    m_entries[m_currentEntryRow].workout.steps.removeAt(m_currentStepRow);
    m_currentStepRow = -1;
    refreshStepList();
    setStepFormEnabled(false);
}

void TrainingPlanDialog::onStepTypeChanged()
{
    QString typeName = m_stepType->currentData().toString();
    bool isRepeatStart = (typeName == "repeatStart");
    bool isRepeatBoundary = isRepeatStart || (typeName == "repeatEnd");

    m_repeatCountLabel->setVisible(isRepeatStart);
    m_repeatCount->setVisible(isRepeatStart);

    m_durationKind->setEnabled(!isRepeatBoundary);
    m_durationValue->setEnabled(!isRepeatBoundary);
    m_targetKind->setEnabled(!isRepeatBoundary);
    m_targetMin->setEnabled(!isRepeatBoundary);
    m_targetMax->setEnabled(!isRepeatBoundary);
}

void TrainingPlanDialog::setStepFormEnabled(bool enabled)
{
    m_stepType->setEnabled(enabled);
    m_repeatCount->setEnabled(enabled);
    m_stepText->setEnabled(enabled);
    m_durationKind->setEnabled(enabled);
    m_durationValue->setEnabled(enabled);
    m_targetKind->setEnabled(enabled);
    m_targetMin->setEnabled(enabled);
    m_targetMax->setEnabled(enabled);
    m_notifyBeep->setEnabled(enabled);
    m_notifyLight->setEnabled(enabled);
}

void TrainingPlanDialog::loadStepFormFrom(const WorkoutStep &step)
{
    m_loading = true;

    int typeIdx = m_stepType->findData(step.type.typeName);
    m_stepType->setCurrentIndex(typeIdx >= 0 ? typeIdx : 0);
    m_repeatCount->setValue(step.type.value > 0 ? step.type.value : 2);
    m_stepText->setText(step.text);

    int durIdx = m_durationKind->findData(step.duration.durationName);
    m_durationKind->setCurrentIndex(durIdx >= 0 ? durIdx : 0);
    m_durationValue->setValue(step.duration.value);

    int tgtIdx = m_targetKind->findData(step.target.targetName);
    m_targetKind->setCurrentIndex(tgtIdx >= 0 ? tgtIdx : 0);
    m_targetMin->setValue(step.target.rangeMin);
    m_targetMax->setValue(step.target.rangeMax);

    m_notifyBeep->setChecked(step.notify.beep);
    m_notifyLight->setChecked(step.notify.light);

    m_loading = false;
    onStepTypeChanged();
}

void TrainingPlanDialog::saveCurrentStepForm()
{
    if (m_loading || m_currentEntryRow < 0 || m_currentEntryRow >= m_entries.size()) {
        return;
    }
    QList<WorkoutStep> &steps = m_entries[m_currentEntryRow].workout.steps;
    if (m_currentStepRow < 0 || m_currentStepRow >= steps.size()) {
        return;
    }
    WorkoutStep &step = steps[m_currentStepRow];
    step.type.typeName = m_stepType->currentData().toString();
    step.type.value = (step.type.typeName == "repeatStart") ? m_repeatCount->value() : 0;
    step.text = m_stepText->text();
    step.duration.durationName = m_durationKind->currentData().toString();
    step.duration.value = m_durationValue->value();
    step.target.targetName = m_targetKind->currentData().toString();
    step.target.rangeMin = m_targetMin->value();
    step.target.rangeMax = m_targetMax->value();
    step.notify.beep = m_notifyBeep->isChecked();
    step.notify.light = m_notifyLight->isChecked();

    refreshStepList();
    m_loading = true;
    m_stepList->setCurrentRow(m_currentStepRow);
    m_loading = false;
}

/* ============================================================ packing / compile hand-off ============================================================ */

void TrainingPlanDialog::onPackAndGenerate()
{
    saveCurrentEntryForm();

    TrainingPlan plan;
    plan.name = m_planName->text().trimmed().isEmpty() ? tr("My training plan") : m_planName->text().trimmed();
    plan.entries = m_entries;

    QString error;
    m_packs = TrainingPlanScheduler::packPlan(plan, error);
    m_packList->clear();
    m_sourceView->clear();

    if (m_packs.isEmpty()) {
        QMessageBox::warning(this, tr("Could not pack plan"), error);
        return;
    }

    for (const TrainingPlanPack &pack : m_packs) {
        m_packList->addItem(QString("%1 (%2)").arg(pack.name, pack.dates.join(", ")));
    }
    m_packList->setCurrentRow(0);
}

void TrainingPlanDialog::onPackSelected(int row)
{
    if (row < 0 || row >= m_packs.size()) {
        m_sourceView->clear();
        return;
    }
    m_sourceView->setPlainText(m_packs[row].compileRequest);
}

void TrainingPlanDialog::onCopySourceToClipboard()
{
    QApplication::clipboard()->setText(m_sourceView->toPlainText());
}

void TrainingPlanDialog::onOpenCompilerWebsite()
{
    QDesktopServices::openUrl(QUrl(COMPILER_FORUM_URL));
}

void TrainingPlanDialog::onSaveCompiledApp()
{
    int row = m_packList->currentRow();
    if (row < 0 || row >= m_packs.size()) {
        QMessageBox::information(this, tr("No pack selected"), tr("Select a pack above first."));
        return;
    }

    QString text = m_compiledPaste->toPlainText();
    if (text.trimmed().isEmpty()) {
        QMessageBox::information(this, tr("Nothing to save"),
            tr("Paste the compiler's JSON output above first."));
        return;
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        QMessageBox::warning(this, tr("Not valid JSON"),
            tr("The pasted text doesn't look like the compiler's JSON output: %1").arg(err.errorString()));
        return;
    }

    QJsonObject obj = doc.object();
    obj["name"] = m_packs[row].name;
    QJsonArray dates;
    for (const QString &d : m_packs[row].dates) {
        dates.append(d);
    }
    obj["dates"] = dates;

    QString planName = m_planName->text().trimmed().isEmpty() ? tr("My training plan") : m_planName->text().trimmed();
    QString dir = TrainingPlanStorage::compiledAppsDir(planName);
    QDir().mkpath(dir);
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Compiled App"),
        dir + "/" + QString("app_%1.json").arg(row + 1), tr("JSON files (*.json)"));
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("Save failed"), tr("Could not write to %1").arg(fileName));
        return;
    }
    file.write(QJsonDocument(obj).toJson());
    file.close();
}
