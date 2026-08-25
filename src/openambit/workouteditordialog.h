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
 * GUI front-end for movescount/workout.h's AppZoneGenerator (Phase 2 of the openambit2
 * training-plans plan). Builds a structured workout, generates App Zone source from it, and
 * hands that source to the user to compile manually via the community compiler website -
 * this dialog deliberately does NOT call the compiler service itself, matching the plan's
 * (and the sibling sommet project's own) reasoning for keeping that step a human action.
 * See AppZoneGenerator's header comment for why on-watch execution is still unverified.
 */
#ifndef WORKOUTEDITORDIALOG_H
#define WORKOUTEDITORDIALOG_H

#include <QDialog>
#include <movescount/workout.h>

class QListWidget;
class QLineEdit;
class QSpinBox;
class QComboBox;
class QCheckBox;
class QPushButton;
class QPlainTextEdit;
class QLabel;

class WorkoutEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WorkoutEditorDialog(QWidget *parent = nullptr);

private slots:
    void onStepSelected(int row);
    void onAddStep();
    void onRemoveStep();
    void onMoveStepUp();
    void onMoveStepDown();
    void onStepTypeChanged();
    void onGenerateSource();
    void onCopySourceToClipboard();
    void onOpenCompilerWebsite();
    void onSaveCompiledApp();

private:
    void refreshStepList();
    void saveCurrentStepForm();
    void loadStepFormFrom(const WorkoutStep &step);
    void setStepFormEnabled(bool enabled);
    QVariantMap stepToMap(const WorkoutStep &step) const;

    QList<WorkoutStep> m_steps;
    int m_currentRow = -1;
    bool m_loading = false;

    QLineEdit      *m_workoutName;

    QListWidget    *m_stepList;
    QPushButton    *m_addBtn;
    QPushButton    *m_removeBtn;
    QPushButton    *m_upBtn;
    QPushButton    *m_downBtn;

    QComboBox      *m_stepType;
    QSpinBox       *m_repeatCount;
    QLabel         *m_repeatCountLabel;
    QLineEdit      *m_stepText;

    QComboBox      *m_durationKind;
    QSpinBox       *m_durationValue;
    QLabel         *m_durationValueLabel;

    QComboBox      *m_targetKind;
    QSpinBox       *m_targetMin;
    QSpinBox       *m_targetMax;
    QLabel         *m_targetRangeLabel;

    QCheckBox      *m_notifyBeep;
    QCheckBox      *m_notifyLight;

    QPushButton    *m_generateBtn;
    QPlainTextEdit *m_sourceView;
    QPushButton    *m_copyBtn;
    QPushButton    *m_openCompilerBtn;

    QPlainTextEdit *m_compiledPaste;
    QPushButton    *m_saveCompiledBtn;
};

#endif // WORKOUTEDITORDIALOG_H
