/*
 * (C) Copyright 2013 Emil Ljungdahl
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
 */
#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QDateTime>
#include <QProcess>
#include <QDir>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    readSettings();

    showHideUserSettings();

    connect(this, SIGNAL(settingsError(QString)), this, SLOT(showSettingsError(QString)));
    connect(ui->listSettingGroups, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));
    connect(ui->checkBoxMovescountEnable, SIGNAL(clicked()), this, SLOT(showHideUserSettings()));
    connect(ui->comboBoxOrbitSource, SIGNAL(currentIndexChanged(int)), this, SLOT(onOrbitSourceChanged(int)));
    connect(ui->buttonBrowseOrbitFile, SIGNAL(clicked()), this, SLOT(onBrowseOrbitFile()));
    connect(ui->buttonDownloadOrbit, SIGNAL(clicked()), this, SLOT(onDownloadOrbitNow()));
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    ui->stackedWidget->setCurrentIndex(ui->listSettingGroups->row(current));
}

void SettingsDialog::accept()
{
    writeSettings();
    this->close();
}

void SettingsDialog::showHideUserSettings()
{
    if (ui->checkBoxMovescountEnable->isChecked()) {
        ui->lineEditEmail->setHidden(false);
    }
    else {
        ui->lineEditEmail->setHidden(true);
    }
}

void SettingsDialog::readSettings()
{
    settings.beginGroup("generalSettings");
    ui->checkBoxSkipBetaCheck->setChecked(settings.value("skipBetaCheck", false).toBool());
    ui->checkBoxRunningBackground->setChecked(settings.value("runningBackground", true).toBool());
    settings.endGroup();

    settings.beginGroup("syncSettings");
    ui->checkBoxSyncAutomatically->setChecked(settings.value("syncAutomatically", false).toBool());
    ui->checkBoxSyncTime->setChecked(settings.value("syncTime", true).toBool());
    ui->checkBoxSyncOrbit->setChecked(settings.value("syncOrbit", true).toBool());
    ui->checkBoxSyncSportsMode->setChecked(settings.value("syncSportMode", false).toBool());
    ui->checkBoxSyncNavigation->setChecked(settings.value("syncNavigation", false).toBool());
    ui->checkBoxSyncWatchJsonConfig->setChecked(settings.value("syncWatchJsonConfig", false).toBool());
    settings.endGroup();

    settings.beginGroup("movescountSettings");
    ui->checkBoxNewVersions->setChecked(settings.value("checkNewVersions", true).toBool());
    ui->checkBoxMovescountEnable->setChecked(settings.value("movescountEnable", false).toBool());
    ui->lineEditEmail->setText(settings.value("email", "").toString());
    ui->checkBoxDebugFiles->setChecked(settings.value("storeDebugFiles", true).toBool());
    settings.endGroup();

    settings.beginGroup("orbitSettings");
    ui->comboBoxOrbitSource->setCurrentIndex(settings.value("orbitSource", 0).toInt());
    ui->lineEditOrbitFile->setText(settings.value("orbitLocalPath", "").toString());
    settings.endGroup();
    updateOrbitUI();
}

void SettingsDialog::writeSettings()
{
    settings.beginGroup("generalSettings");
    settings.setValue("skipBetaCheck", ui->checkBoxSkipBetaCheck->isChecked());
    settings.setValue("runningBackground", ui->checkBoxRunningBackground->isChecked());
    settings.endGroup();

    settings.beginGroup("syncSettings");
    settings.setValue("syncAutomatically", ui->checkBoxSyncAutomatically->isChecked());
    settings.setValue("syncTime", ui->checkBoxSyncTime->isChecked());
    settings.setValue("syncOrbit", ui->checkBoxSyncOrbit->isChecked());
    settings.setValue("syncSportMode", ui->checkBoxSyncSportsMode->isChecked());
    settings.setValue("syncNavigation", ui->checkBoxSyncNavigation->isChecked());
    settings.setValue("syncWatchJsonConfig", ui->checkBoxSyncWatchJsonConfig->isChecked());
    settings.endGroup();

    settings.beginGroup("movescountSettings");
    settings.setValue("checkNewVersions", ui->checkBoxNewVersions->isChecked());
    settings.setValue("movescountEnable", ui->checkBoxMovescountEnable->isChecked());
    if (ui->checkBoxMovescountEnable->isChecked()) {
        settings.setValue("email", ui->lineEditEmail->text());
    }
    else {
        settings.setValue("email", "");
    }
    settings.setValue("storeDebugFiles", ui->checkBoxDebugFiles->isChecked());
    settings.endGroup();

    settings.beginGroup("orbitSettings");
    settings.setValue("orbitSource", ui->comboBoxOrbitSource->currentIndex());
    settings.setValue("orbitLocalPath", ui->lineEditOrbitFile->text());
    settings.endGroup();

    // ensure settings are written to have an up-to-date status for informing the user 
    // if settings cannot be written
    settings.sync();

    switch (settings.status()) {
        case QSettings::AccessError: {
            emit settingsError("Could not write settings-file, maybe the config-file is write-only or owned by a different user.");
            break;
        }
        case QSettings::FormatError: {
            emit settingsError("Could not read settings-file, probably the format of the file is invalid.");
            break;
        }
        case QSettings::NoError: {
            // everything fine
            emit settingsSaved();
            break;
        }
    }
}

void SettingsDialog::showSettingsError(QString msg)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(msg);
    msgBox.setInformativeText(QString(getenv("HOME")) + "/.openambit/openambit.conf");
    msgBox.exec();
}

void SettingsDialog::updateOrbitUI()
{
    // 0 = Open IGS, 1 = Local file, 2 = Movescount (legacy)
    bool localFile = (ui->comboBoxOrbitSource->currentIndex() == 1);
    ui->labelLocalOrbitFile->setVisible(localFile);
    ui->lineEditOrbitFile->setVisible(localFile);
    ui->buttonBrowseOrbitFile->setVisible(localFile);

    // "Download now" only makes sense for Open IGS
    bool isOpenIGS = (ui->comboBoxOrbitSource->currentIndex() == 0);
    ui->buttonDownloadOrbit->setEnabled(isOpenIGS || localFile);
    ui->buttonDownloadOrbit->setText(isOpenIGS ? "Download / refresh now" : "Verify file");

    // Show cache path
    QString cache = QString(getenv("HOME")) + "/.openambit/orbit_cache.uo";
    QFileInfo fi(cache);
    if (fi.exists()) {
        ui->labelOrbitCachePath->setText(QString("Cache: %1  (%2 bytes, %3)")
            .arg(cache)
            .arg(fi.size())
            .arg(fi.lastModified().toString("yyyy-MM-dd hh:mm")));
    } else {
        ui->labelOrbitCachePath->setText(QString("Cache: %1  (not yet downloaded)").arg(cache));
    }
}

void SettingsDialog::onOrbitSourceChanged(int)
{
    updateOrbitUI();
    ui->labelOrbitStatus->clear();
}

void SettingsDialog::onBrowseOrbitFile()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Select orbit file"),
        ui->lineEditOrbitFile->text(),
        tr("Orbit files (*.uo *.bin);;All files (*)"));
    if (!path.isEmpty())
        ui->lineEditOrbitFile->setText(path);
}

void SettingsDialog::onDownloadOrbitNow()
{
    int src = ui->comboBoxOrbitSource->currentIndex();

    if (src == 1) {
        // Local file: just verify it exists and report size
        QFileInfo fi(ui->lineEditOrbitFile->text());
        if (fi.exists())
            ui->labelOrbitStatus->setText(QString("OK — %1 bytes").arg(fi.size()));
        else
            ui->labelOrbitStatus->setText("File not found.");
        return;
    }

    // Open IGS: run the Python converter script
    ui->buttonDownloadOrbit->setEnabled(false);
    ui->labelOrbitStatus->setText("Downloading…");

    // Find the script relative to the executable or source tree
    QStringList candidates = {
        QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../tools/rinex2ubx_ambit.py"),
        QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../tools/rinex2ubx_ambit.py"),
        QString(getenv("HOME")) + "/dev/openambit/tools/rinex2ubx_ambit.py"
    };
    QString script;
    for (const QString &c : candidates) {
        if (QFileInfo::exists(c)) { script = c; break; }
    }

    if (script.isEmpty()) {
        ui->labelOrbitStatus->setText("Script not found: tools/rinex2ubx_ambit.py");
        ui->buttonDownloadOrbit->setEnabled(true);
        return;
    }

    QProcess *proc = new QProcess(this);
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, proc](int code, QProcess::ExitStatus) {
        QString out = proc->readAllStandardOutput() + proc->readAllStandardError();
        if (code == 0) {
            updateOrbitUI();
            ui->labelOrbitStatus->setText("Done. " + out.split('\n').filter("bytes").value(0));
        } else {
            ui->labelOrbitStatus->setText("Failed: " + out.left(200));
        }
        ui->buttonDownloadOrbit->setEnabled(true);
        proc->deleteLater();
    });
    proc->start("python3", QStringList() << script);
}
