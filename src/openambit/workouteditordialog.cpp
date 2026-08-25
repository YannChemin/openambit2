#include "workouteditordialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QListWidget>
#include <QLineEdit>
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
#include <QFile>
#include <QTextStream>

// The community-documented entry point for the App-Zone compiler, per its own forum post -
// see movescount/workout.h's provenance comment. Not a guess: pavel.samokha (its author)
// published this thread himself as the way to use the compiler.
static const char *COMPILER_FORUM_URL = "https://forum.suunto.com/topic/7592/ambit-apps-compilation";

WorkoutEditorDialog::WorkoutEditorDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Workout Builder (experimental)"));
    resize(900, 650);

    QLabel *warning = new QLabel(tr(
        "<b>Experimental:</b> whether a freshly-compiled App Zone app actually executes its "
        "logic on real Ambit3-family firmware (as opposed to merely being accepted and "
        "displayed) has not been confirmed by this project. Treat generated apps as unverified "
        "until checked against real hardware."));
    warning->setWordWrap(true);
    warning->setStyleSheet("QLabel { background: #fff3cd; color: #664d03; padding: 6px; border: 1px solid #ffe69c; }");

    m_workoutName = new QLineEdit(tr("My workout"));

    // --- Left: step list + add/remove/reorder ---
    m_stepList = new QListWidget;
    m_addBtn    = new QPushButton(tr("Add"));
    m_removeBtn = new QPushButton(tr("Remove"));
    m_upBtn     = new QPushButton(tr("Up"));
    m_downBtn   = new QPushButton(tr("Down"));

    QHBoxLayout *listBtns = new QHBoxLayout;
    listBtns->addWidget(m_addBtn);
    listBtns->addWidget(m_removeBtn);
    listBtns->addWidget(m_upBtn);
    listBtns->addWidget(m_downBtn);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(new QLabel(tr("Steps:")));
    leftLayout->addWidget(m_stepList);
    leftLayout->addLayout(listBtns);

    QWidget *leftPanel = new QWidget;
    leftPanel->setLayout(leftLayout);

    // --- Right: step form ---
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
    m_stepText->setToolTip(tr("Optional label shown on the watch while this step runs. "
                               "Digits are stripped - the live compiler rejects string "
                               "literals containing a digit."));

    m_durationKind = new QComboBox;
    m_durationKind->addItem(tr("Time"), "time");
    m_durationKind->addItem(tr("Distance"), "distance");
    m_durationKind->addItem(tr("Ascent"), "ascent");
    m_durationKind->addItem(tr("Next Lap button press"), "lap");
    m_durationKind->addItem(tr("Energy (kcal)"), "energy");
    m_durationKind->addItem(tr("Until HR below..."), "hr_below");
    m_durationKind->addItem(tr("Until HR above..."), "hr_above");

    m_durationValueLabel = new QLabel(tr("Duration value:"));
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

    m_targetRangeLabel = new QLabel(tr("Target range:"));
    m_targetMin = new QSpinBox;
    m_targetMin->setRange(0, 100000);
    m_targetMax = new QSpinBox;
    m_targetMax->setRange(0, 100000);
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

    QFormLayout *form = new QFormLayout;
    form->addRow(tr("Step type:"), m_stepType);
    form->addRow(m_repeatCountLabel, m_repeatCount);
    form->addRow(tr("Text:"), m_stepText);
    form->addRow(tr("Duration:"), m_durationKind);
    form->addRow(m_durationValueLabel, m_durationValue);
    form->addRow(tr("Target:"), m_targetKind);
    form->addRow(m_targetRangeLabel, targetRangeWidget);
    form->addRow(tr("Notify on entry:"), notifyWidget);

    QGroupBox *stepBox = new QGroupBox(tr("Selected step"));
    stepBox->setLayout(form);

    // --- Bottom: generate / preview / compile hand-off ---
    m_generateBtn = new QPushButton(tr("Generate App Zone Source"));
    m_sourceView = new QPlainTextEdit;
    m_sourceView->setReadOnly(true);
    m_sourceView->setFont(QFont("monospace"));
    m_copyBtn = new QPushButton(tr("Copy to Clipboard"));
    m_openCompilerBtn = new QPushButton(tr("Open Compiler Website..."));

    QHBoxLayout *genBtns = new QHBoxLayout;
    genBtns->addWidget(m_generateBtn);
    genBtns->addWidget(m_copyBtn);
    genBtns->addWidget(m_openCompilerBtn);
    genBtns->addStretch();

    QLabel *pasteLabel = new QLabel(tr(
        "After compiling on the website, paste the resulting JSON here and save it - this "
        "file can later be used with openambit-cli's --app-config option."));
    pasteLabel->setWordWrap(true);
    m_compiledPaste = new QPlainTextEdit;
    m_compiledPaste->setPlaceholderText(tr("Paste the compiler's JSON output here..."));
    m_saveCompiledBtn = new QPushButton(tr("Save Compiled App to File..."));

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(stepBox);
    rightLayout->addLayout(genBtns);
    rightLayout->addWidget(new QLabel(tr("Generated source (paste this into the compiler):")));
    rightLayout->addWidget(m_sourceView, 1);
    rightLayout->addWidget(pasteLabel);
    rightLayout->addWidget(m_compiledPaste);
    rightLayout->addWidget(m_saveCompiledBtn);

    QWidget *rightPanel = new QWidget;
    rightPanel->setLayout(rightLayout);

    QSplitter *splitter = new QSplitter;
    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(warning);
    QFormLayout *nameForm = new QFormLayout;
    nameForm->addRow(tr("Workout name:"), m_workoutName);
    mainLayout->addLayout(nameForm);
    mainLayout->addWidget(splitter, 1);

    setStepFormEnabled(false);

    connect(m_stepList, &QListWidget::currentRowChanged, this, &WorkoutEditorDialog::onStepSelected);
    connect(m_addBtn, &QPushButton::clicked, this, &WorkoutEditorDialog::onAddStep);
    connect(m_removeBtn, &QPushButton::clicked, this, &WorkoutEditorDialog::onRemoveStep);
    connect(m_upBtn, &QPushButton::clicked, this, &WorkoutEditorDialog::onMoveStepUp);
    connect(m_downBtn, &QPushButton::clicked, this, &WorkoutEditorDialog::onMoveStepDown);
    connect(m_stepType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WorkoutEditorDialog::onStepTypeChanged);
    connect(m_generateBtn, &QPushButton::clicked, this, &WorkoutEditorDialog::onGenerateSource);
    connect(m_copyBtn, &QPushButton::clicked, this, &WorkoutEditorDialog::onCopySourceToClipboard);
    connect(m_openCompilerBtn, &QPushButton::clicked, this, &WorkoutEditorDialog::onOpenCompilerWebsite);
    connect(m_saveCompiledBtn, &QPushButton::clicked, this, &WorkoutEditorDialog::onSaveCompiledApp);

    onStepTypeChanged();
}

void WorkoutEditorDialog::refreshStepList()
{
    m_loading = true;
    m_stepList->clear();
    for (const WorkoutStep &step : m_steps) {
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

void WorkoutEditorDialog::onStepSelected(int row)
{
    if (m_loading) {
        return;
    }
    saveCurrentStepForm();
    m_currentRow = row;
    if (row < 0 || row >= m_steps.size()) {
        setStepFormEnabled(false);
        return;
    }
    setStepFormEnabled(true);
    loadStepFormFrom(m_steps[row]);
}

void WorkoutEditorDialog::onAddStep()
{
    saveCurrentStepForm();
    m_steps.append(WorkoutStep());
    refreshStepList();
    m_stepList->setCurrentRow(m_steps.size() - 1);
}

void WorkoutEditorDialog::onRemoveStep()
{
    if (m_currentRow < 0 || m_currentRow >= m_steps.size()) {
        return;
    }
    m_steps.removeAt(m_currentRow);
    m_currentRow = -1;
    refreshStepList();
    if (!m_steps.isEmpty()) {
        m_stepList->setCurrentRow(qMin(m_currentRow, m_steps.size() - 1));
    } else {
        setStepFormEnabled(false);
    }
}

void WorkoutEditorDialog::onMoveStepUp()
{
    if (m_currentRow <= 0 || m_currentRow >= m_steps.size()) {
        return;
    }
    saveCurrentStepForm();
    m_steps.swapItemsAt(m_currentRow, m_currentRow - 1);
    refreshStepList();
    m_stepList->setCurrentRow(m_currentRow - 1);
}

void WorkoutEditorDialog::onMoveStepDown()
{
    if (m_currentRow < 0 || m_currentRow >= m_steps.size() - 1) {
        return;
    }
    saveCurrentStepForm();
    m_steps.swapItemsAt(m_currentRow, m_currentRow + 1);
    refreshStepList();
    m_stepList->setCurrentRow(m_currentRow + 1);
}

void WorkoutEditorDialog::onStepTypeChanged()
{
    QString typeName = m_stepType->currentData().toString();
    bool isRepeatStart = (typeName == "repeatStart");
    bool isRepeatBoundary = isRepeatStart || (typeName == "repeatEnd");

    m_repeatCountLabel->setVisible(isRepeatStart);
    m_repeatCount->setVisible(isRepeatStart);

    m_durationKind->setEnabled(!isRepeatBoundary);
    m_durationValue->setEnabled(!isRepeatBoundary);
    m_durationValueLabel->setVisible(!isRepeatBoundary);
    m_targetKind->setEnabled(!isRepeatBoundary);
    m_targetMin->setEnabled(!isRepeatBoundary);
    m_targetMax->setEnabled(!isRepeatBoundary);
}

void WorkoutEditorDialog::setStepFormEnabled(bool enabled)
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

void WorkoutEditorDialog::loadStepFormFrom(const WorkoutStep &step)
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

void WorkoutEditorDialog::saveCurrentStepForm()
{
    if (m_loading || m_currentRow < 0 || m_currentRow >= m_steps.size()) {
        return;
    }
    WorkoutStep &step = m_steps[m_currentRow];
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
    m_stepList->setCurrentRow(m_currentRow);
    m_loading = false;
}

void WorkoutEditorDialog::onGenerateSource()
{
    saveCurrentStepForm();

    QVariantMap map;
    map.insert(Workout::NAME, m_workoutName->text());
    QVariantList stepMaps;
    for (const WorkoutStep &step : m_steps) {
        stepMaps.append(step.toMap());
    }
    map.insert(Workout::STEPS, stepMaps);
    Workout workout(map);

    QString source, error;
    QStringList ownVars;
    if (!AppZoneGenerator::generateSource(workout, source, ownVars, error)) {
        m_sourceView->setPlainText(tr("Could not generate source: %1").arg(error));
        return;
    }

    QString request = AppZoneGenerator::buildCompileRequest(source, ownVars);
    m_sourceView->setPlainText(request);
}

void WorkoutEditorDialog::onCopySourceToClipboard()
{
    QApplication::clipboard()->setText(m_sourceView->toPlainText());
}

void WorkoutEditorDialog::onOpenCompilerWebsite()
{
    QDesktopServices::openUrl(QUrl(COMPILER_FORUM_URL));
}

void WorkoutEditorDialog::onSaveCompiledApp()
{
    QString text = m_compiledPaste->toPlainText();
    if (text.trimmed().isEmpty()) {
        QMessageBox::information(this, tr("Nothing to save"),
            tr("Paste the compiler's JSON output above first."));
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Compiled App"),
        QString(), tr("JSON files (*.json)"));
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Save failed"), tr("Could not write to %1").arg(fileName));
        return;
    }
    QTextStream out(&file);
    out << text;
    file.close();
}

