/*
 * (C) Copyright 2026 Yann Chemin
 *
 * This file is part of Openambit.
 *
 * Openambit is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contributors:
 *
 * GUI front-end for movescount/trainingplan.h's TrainingPlanScheduler (Phase 3 of the
 * openambit2 training-plans plan). Edits one Training Plan (a named set of calendar-dated
 * workouts), packs it into App-Zone apps, and hands each pack's source to the user to
 * compile manually - same manual-hand-off design as WorkoutEditorDialog, and for the same
 * reason (see AppZoneGenerator's header comment). This dialog does NOT write anything to a
 * connected watch - see trainingplan.h's header comment for why that install step isn't
 * built yet.
 */
#ifndef TRAININGPLANDIALOG_H
#define TRAININGPLANDIALOG_H

#include <QDialog>
#include <movescount/trainingplan.h>

class QListWidget;
class QLineEdit;
class QDateEdit;
class QSpinBox;
class QComboBox;
class QCheckBox;
class QPushButton;
class QPlainTextEdit;
class QLabel;

class TrainingPlanDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TrainingPlanDialog(QWidget *parent = nullptr);

private slots:
    void onNewPlan();
    void onLoadPlan();
    void onSavePlan();

    void onEntrySelected(int row);
    void onAddEntry();
    void onRemoveEntry();

    void onStepSelected(int row);
    void onAddStep();
    void onRemoveStep();
    void onStepTypeChanged();

    void onPackAndGenerate();
    void onPackSelected(int row);
    void onCopySourceToClipboard();
    void onOpenCompilerWebsite();
    void onSaveCompiledApp();

private:
    // --- entries (date + workout) ---
    void refreshEntryList();
    void saveCurrentEntryForm();
    void loadEntryFormFrom(const TrainingPlanEntry &entry);
    void setEntryFormEnabled(bool enabled);

    // --- steps within the currently-selected entry's workout ---
    void refreshStepList();
    void saveCurrentStepForm();
    void loadStepFormFrom(const WorkoutStep &step);
    void setStepFormEnabled(bool enabled);

    QList<TrainingPlanEntry> m_entries;
    int m_currentEntryRow = -1;
    int m_currentStepRow = -1;
    bool m_loading = false;

    QList<TrainingPlanPack> m_packs;

    QLineEdit   *m_planName;
    QPushButton *m_newBtn;
    QPushButton *m_loadBtn;
    QPushButton *m_saveBtn;

    QListWidget *m_entryList;
    QPushButton *m_addEntryBtn;
    QPushButton *m_removeEntryBtn;
    QDateEdit   *m_entryDate;
    QLineEdit   *m_entryWorkoutName;

    QListWidget    *m_stepList;
    QPushButton    *m_addStepBtn;
    QPushButton    *m_removeStepBtn;
    QComboBox      *m_stepType;
    QSpinBox       *m_repeatCount;
    QLabel         *m_repeatCountLabel;
    QLineEdit      *m_stepText;
    QComboBox      *m_durationKind;
    QSpinBox       *m_durationValue;
    QComboBox      *m_targetKind;
    QSpinBox       *m_targetMin;
    QSpinBox       *m_targetMax;
    QCheckBox      *m_notifyBeep;
    QCheckBox      *m_notifyLight;

    QPushButton    *m_packBtn;
    QListWidget    *m_packList;
    QPlainTextEdit *m_sourceView;
    QPushButton    *m_copyBtn;
    QPushButton    *m_openCompilerBtn;
    QPlainTextEdit *m_compiledPaste;
    QPushButton    *m_saveCompiledBtn;
};

#endif // TRAININGPLANDIALOG_H
