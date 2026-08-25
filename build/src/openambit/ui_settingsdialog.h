/********************************************************************************
** Form generated from reading UI file 'settingsdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.15
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTINGSDIALOG_H
#define UI_SETTINGSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SettingsDialog
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QListWidget *listSettingGroups;
    QVBoxLayout *verticalLayout_3;
    QStackedWidget *stackedWidget;
    QWidget *generalSettingsPage;
    QVBoxLayout *verticalLayout_5;
    QGroupBox *groupBox;
    QFormLayout *formLayout_3;
    QCheckBox *checkBoxSkipBetaCheck;
    QCheckBox *checkBoxRunningBackground;
    QWidget *deviceSyncPage;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupDeviceSync;
    QFormLayout *formLayout;
    QCheckBox *checkBoxSyncTime;
    QCheckBox *checkBoxSyncOrbit;
    QCheckBox *checkBoxSyncAutomatically;
    QCheckBox *checkBoxSyncSportsMode;
    QCheckBox *checkBoxSyncNavigation;
    QCheckBox *checkBoxSyncWatchJsonConfig;
    QWidget *movesCountPage;
    QVBoxLayout *verticalLayout_4;
    QGroupBox *groupMovescount;
    QFormLayout *formLayout_2;
    QCheckBox *checkBoxNewVersions;
    QCheckBox *checkBoxDebugFiles;
    QCheckBox *checkBoxMovescountEnable;
    QLineEdit *lineEditEmail;
    QWidget *gpsOrbitPage;
    QVBoxLayout *verticalLayout_6;
    QGroupBox *groupGpsOrbit;
    QFormLayout *formLayout_4;
    QLabel *labelOrbitSource;
    QComboBox *comboBoxOrbitSource;
    QLabel *labelLocalOrbitFile;
    QHBoxLayout *hLayoutOrbitFile;
    QLineEdit *lineEditOrbitFile;
    QPushButton *buttonBrowseOrbitFile;
    QLabel *labelOrbitCachePath;
    QHBoxLayout *hLayoutOrbitActions;
    QPushButton *buttonDownloadOrbit;
    QLabel *labelOrbitStatus;
    QSpacerItem *verticalSpacerOrbit;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *SettingsDialog)
    {
        if (SettingsDialog->objectName().isEmpty())
            SettingsDialog->setObjectName(QString::fromUtf8("SettingsDialog"));
        SettingsDialog->resize(734, 438);
        verticalLayout = new QVBoxLayout(SettingsDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        listSettingGroups = new QListWidget(SettingsDialog);
        QIcon icon;
        QString iconThemeName = QString::fromUtf8("preferences-system");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon = QIcon::fromTheme(iconThemeName);
        } else {
            icon.addFile(QString::fromUtf8(""), QSize(), QIcon::Normal, QIcon::Off);
        }
        QListWidgetItem *__qlistwidgetitem = new QListWidgetItem(listSettingGroups);
        __qlistwidgetitem->setIcon(icon);
        QIcon icon1;
        iconThemeName = QString::fromUtf8("clock");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon1 = QIcon::fromTheme(iconThemeName);
        } else {
            icon1.addFile(QString::fromUtf8(""), QSize(), QIcon::Normal, QIcon::Off);
        }
        QListWidgetItem *__qlistwidgetitem1 = new QListWidgetItem(listSettingGroups);
        __qlistwidgetitem1->setIcon(icon1);
        QIcon icon2;
        iconThemeName = QString::fromUtf8("internet-web-browser");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon2 = QIcon::fromTheme(iconThemeName);
        } else {
            icon2.addFile(QString::fromUtf8(""), QSize(), QIcon::Normal, QIcon::Off);
        }
        QListWidgetItem *__qlistwidgetitem2 = new QListWidgetItem(listSettingGroups);
        __qlistwidgetitem2->setIcon(icon2);
        QIcon icon3;
        iconThemeName = QString::fromUtf8("applications-science");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon3 = QIcon::fromTheme(iconThemeName);
        } else {
            icon3.addFile(QString::fromUtf8(""), QSize(), QIcon::Normal, QIcon::Off);
        }
        QListWidgetItem *__qlistwidgetitem3 = new QListWidgetItem(listSettingGroups);
        __qlistwidgetitem3->setIcon(icon3);
        listSettingGroups->setObjectName(QString::fromUtf8("listSettingGroups"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(listSettingGroups->sizePolicy().hasHeightForWidth());
        listSettingGroups->setSizePolicy(sizePolicy);
        listSettingGroups->setMinimumSize(QSize(180, 0));
        listSettingGroups->setMaximumSize(QSize(180, 16777215));
        listSettingGroups->setBaseSize(QSize(180, 0));
        listSettingGroups->setResizeMode(QListView::Fixed);

        horizontalLayout->addWidget(listSettingGroups);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        stackedWidget = new QStackedWidget(SettingsDialog);
        stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(stackedWidget->sizePolicy().hasHeightForWidth());
        stackedWidget->setSizePolicy(sizePolicy1);
        generalSettingsPage = new QWidget();
        generalSettingsPage->setObjectName(QString::fromUtf8("generalSettingsPage"));
        verticalLayout_5 = new QVBoxLayout(generalSettingsPage);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        groupBox = new QGroupBox(generalSettingsPage);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        formLayout_3 = new QFormLayout(groupBox);
        formLayout_3->setObjectName(QString::fromUtf8("formLayout_3"));
        checkBoxSkipBetaCheck = new QCheckBox(groupBox);
        checkBoxSkipBetaCheck->setObjectName(QString::fromUtf8("checkBoxSkipBetaCheck"));

        formLayout_3->setWidget(0, QFormLayout::LabelRole, checkBoxSkipBetaCheck);

        checkBoxRunningBackground = new QCheckBox(groupBox);
        checkBoxRunningBackground->setObjectName(QString::fromUtf8("checkBoxRunningBackground"));

        formLayout_3->setWidget(1, QFormLayout::LabelRole, checkBoxRunningBackground);


        verticalLayout_5->addWidget(groupBox);

        stackedWidget->addWidget(generalSettingsPage);
        deviceSyncPage = new QWidget();
        deviceSyncPage->setObjectName(QString::fromUtf8("deviceSyncPage"));
        sizePolicy1.setHeightForWidth(deviceSyncPage->sizePolicy().hasHeightForWidth());
        deviceSyncPage->setSizePolicy(sizePolicy1);
        verticalLayout_2 = new QVBoxLayout(deviceSyncPage);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        groupDeviceSync = new QGroupBox(deviceSyncPage);
        groupDeviceSync->setObjectName(QString::fromUtf8("groupDeviceSync"));
        sizePolicy1.setHeightForWidth(groupDeviceSync->sizePolicy().hasHeightForWidth());
        groupDeviceSync->setSizePolicy(sizePolicy1);
        formLayout = new QFormLayout(groupDeviceSync);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        checkBoxSyncTime = new QCheckBox(groupDeviceSync);
        checkBoxSyncTime->setObjectName(QString::fromUtf8("checkBoxSyncTime"));

        formLayout->setWidget(1, QFormLayout::LabelRole, checkBoxSyncTime);

        checkBoxSyncOrbit = new QCheckBox(groupDeviceSync);
        checkBoxSyncOrbit->setObjectName(QString::fromUtf8("checkBoxSyncOrbit"));

        formLayout->setWidget(3, QFormLayout::LabelRole, checkBoxSyncOrbit);

        checkBoxSyncAutomatically = new QCheckBox(groupDeviceSync);
        checkBoxSyncAutomatically->setObjectName(QString::fromUtf8("checkBoxSyncAutomatically"));

        formLayout->setWidget(0, QFormLayout::LabelRole, checkBoxSyncAutomatically);

        checkBoxSyncSportsMode = new QCheckBox(groupDeviceSync);
        checkBoxSyncSportsMode->setObjectName(QString::fromUtf8("checkBoxSyncSportsMode"));

        formLayout->setWidget(4, QFormLayout::LabelRole, checkBoxSyncSportsMode);

        checkBoxSyncNavigation = new QCheckBox(groupDeviceSync);
        checkBoxSyncNavigation->setObjectName(QString::fromUtf8("checkBoxSyncNavigation"));

        formLayout->setWidget(5, QFormLayout::LabelRole, checkBoxSyncNavigation);

        checkBoxSyncWatchJsonConfig = new QCheckBox(groupDeviceSync);
        checkBoxSyncWatchJsonConfig->setObjectName(QString::fromUtf8("checkBoxSyncWatchJsonConfig"));

        formLayout->setWidget(6, QFormLayout::LabelRole, checkBoxSyncWatchJsonConfig);


        verticalLayout_2->addWidget(groupDeviceSync);

        stackedWidget->addWidget(deviceSyncPage);
        movesCountPage = new QWidget();
        movesCountPage->setObjectName(QString::fromUtf8("movesCountPage"));
        verticalLayout_4 = new QVBoxLayout(movesCountPage);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        groupMovescount = new QGroupBox(movesCountPage);
        groupMovescount->setObjectName(QString::fromUtf8("groupMovescount"));
        formLayout_2 = new QFormLayout(groupMovescount);
        formLayout_2->setObjectName(QString::fromUtf8("formLayout_2"));
        formLayout_2->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        checkBoxNewVersions = new QCheckBox(groupMovescount);
        checkBoxNewVersions->setObjectName(QString::fromUtf8("checkBoxNewVersions"));

        formLayout_2->setWidget(0, QFormLayout::LabelRole, checkBoxNewVersions);

        checkBoxDebugFiles = new QCheckBox(groupMovescount);
        checkBoxDebugFiles->setObjectName(QString::fromUtf8("checkBoxDebugFiles"));

        formLayout_2->setWidget(1, QFormLayout::LabelRole, checkBoxDebugFiles);

        checkBoxMovescountEnable = new QCheckBox(groupMovescount);
        checkBoxMovescountEnable->setObjectName(QString::fromUtf8("checkBoxMovescountEnable"));

        formLayout_2->setWidget(2, QFormLayout::LabelRole, checkBoxMovescountEnable);

        lineEditEmail = new QLineEdit(groupMovescount);
        lineEditEmail->setObjectName(QString::fromUtf8("lineEditEmail"));

        formLayout_2->setWidget(3, QFormLayout::SpanningRole, lineEditEmail);


        verticalLayout_4->addWidget(groupMovescount);

        stackedWidget->addWidget(movesCountPage);
        gpsOrbitPage = new QWidget();
        gpsOrbitPage->setObjectName(QString::fromUtf8("gpsOrbitPage"));
        verticalLayout_6 = new QVBoxLayout(gpsOrbitPage);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        groupGpsOrbit = new QGroupBox(gpsOrbitPage);
        groupGpsOrbit->setObjectName(QString::fromUtf8("groupGpsOrbit"));
        formLayout_4 = new QFormLayout(groupGpsOrbit);
        formLayout_4->setObjectName(QString::fromUtf8("formLayout_4"));
        labelOrbitSource = new QLabel(groupGpsOrbit);
        labelOrbitSource->setObjectName(QString::fromUtf8("labelOrbitSource"));

        formLayout_4->setWidget(0, QFormLayout::LabelRole, labelOrbitSource);

        comboBoxOrbitSource = new QComboBox(groupGpsOrbit);
        comboBoxOrbitSource->addItem(QString());
        comboBoxOrbitSource->addItem(QString());
        comboBoxOrbitSource->addItem(QString());
        comboBoxOrbitSource->setObjectName(QString::fromUtf8("comboBoxOrbitSource"));

        formLayout_4->setWidget(0, QFormLayout::FieldRole, comboBoxOrbitSource);

        labelLocalOrbitFile = new QLabel(groupGpsOrbit);
        labelLocalOrbitFile->setObjectName(QString::fromUtf8("labelLocalOrbitFile"));

        formLayout_4->setWidget(1, QFormLayout::LabelRole, labelLocalOrbitFile);

        hLayoutOrbitFile = new QHBoxLayout();
        hLayoutOrbitFile->setObjectName(QString::fromUtf8("hLayoutOrbitFile"));
        lineEditOrbitFile = new QLineEdit(groupGpsOrbit);
        lineEditOrbitFile->setObjectName(QString::fromUtf8("lineEditOrbitFile"));

        hLayoutOrbitFile->addWidget(lineEditOrbitFile);

        buttonBrowseOrbitFile = new QPushButton(groupGpsOrbit);
        buttonBrowseOrbitFile->setObjectName(QString::fromUtf8("buttonBrowseOrbitFile"));

        hLayoutOrbitFile->addWidget(buttonBrowseOrbitFile);


        formLayout_4->setLayout(1, QFormLayout::FieldRole, hLayoutOrbitFile);

        labelOrbitCachePath = new QLabel(groupGpsOrbit);
        labelOrbitCachePath->setObjectName(QString::fromUtf8("labelOrbitCachePath"));
        labelOrbitCachePath->setWordWrap(true);

        formLayout_4->setWidget(2, QFormLayout::SpanningRole, labelOrbitCachePath);

        hLayoutOrbitActions = new QHBoxLayout();
        hLayoutOrbitActions->setObjectName(QString::fromUtf8("hLayoutOrbitActions"));
        buttonDownloadOrbit = new QPushButton(groupGpsOrbit);
        buttonDownloadOrbit->setObjectName(QString::fromUtf8("buttonDownloadOrbit"));

        hLayoutOrbitActions->addWidget(buttonDownloadOrbit);

        labelOrbitStatus = new QLabel(groupGpsOrbit);
        labelOrbitStatus->setObjectName(QString::fromUtf8("labelOrbitStatus"));
        labelOrbitStatus->setWordWrap(true);

        hLayoutOrbitActions->addWidget(labelOrbitStatus);


        formLayout_4->setLayout(3, QFormLayout::SpanningRole, hLayoutOrbitActions);


        verticalLayout_6->addWidget(groupGpsOrbit);

        verticalSpacerOrbit = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_6->addItem(verticalSpacerOrbit);

        stackedWidget->addWidget(gpsOrbitPage);

        verticalLayout_3->addWidget(stackedWidget);


        horizontalLayout->addLayout(verticalLayout_3);


        verticalLayout->addLayout(horizontalLayout);

        buttonBox = new QDialogButtonBox(SettingsDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Apply|QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(SettingsDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), SettingsDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), SettingsDialog, SLOT(reject()));

        stackedWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(SettingsDialog);
    } // setupUi

    void retranslateUi(QDialog *SettingsDialog)
    {
        SettingsDialog->setWindowTitle(QCoreApplication::translate("SettingsDialog", "Dialog", nullptr));

        const bool __sortingEnabled = listSettingGroups->isSortingEnabled();
        listSettingGroups->setSortingEnabled(false);
        QListWidgetItem *___qlistwidgetitem = listSettingGroups->item(0);
        ___qlistwidgetitem->setText(QCoreApplication::translate("SettingsDialog", "General", nullptr));
        QListWidgetItem *___qlistwidgetitem1 = listSettingGroups->item(1);
        ___qlistwidgetitem1->setText(QCoreApplication::translate("SettingsDialog", "Device sync", nullptr));
        QListWidgetItem *___qlistwidgetitem2 = listSettingGroups->item(2);
        ___qlistwidgetitem2->setText(QCoreApplication::translate("SettingsDialog", "Movescount", nullptr));
        QListWidgetItem *___qlistwidgetitem3 = listSettingGroups->item(3);
        ___qlistwidgetitem3->setText(QCoreApplication::translate("SettingsDialog", "GPS Orbit", nullptr));
        listSettingGroups->setSortingEnabled(__sortingEnabled);

        groupBox->setTitle(QCoreApplication::translate("SettingsDialog", "General settings", nullptr));
        checkBoxSkipBetaCheck->setText(QCoreApplication::translate("SettingsDialog", "Skip Beta check at startup", nullptr));
        checkBoxRunningBackground->setText(QCoreApplication::translate("SettingsDialog", "Continue running in background when Openambit main window is closed", nullptr));
        groupDeviceSync->setTitle(QCoreApplication::translate("SettingsDialog", "Device synchronization settings", nullptr));
        checkBoxSyncTime->setText(QCoreApplication::translate("SettingsDialog", "Sync time from computer", nullptr));
        checkBoxSyncOrbit->setText(QCoreApplication::translate("SettingsDialog", "Sync orbital data (from Movescount)", nullptr));
        checkBoxSyncAutomatically->setText(QCoreApplication::translate("SettingsDialog", "Start sync automatically when device connected", nullptr));
        checkBoxSyncSportsMode->setText(QCoreApplication::translate("SettingsDialog", "Sync sport modes (from Movescount)", nullptr));
        checkBoxSyncNavigation->setText(QCoreApplication::translate("SettingsDialog", "Sync navigation (from Movescount)", nullptr));
        checkBoxSyncWatchJsonConfig->setText(QCoreApplication::translate("SettingsDialog", "Save configuration for the watch (JSON file)", nullptr));
        groupMovescount->setTitle(QCoreApplication::translate("SettingsDialog", "Movescount connectivity", nullptr));
        checkBoxNewVersions->setText(QCoreApplication::translate("SettingsDialog", "Check Movescount for new versions", nullptr));
        checkBoxDebugFiles->setText(QCoreApplication::translate("SettingsDialog", "Generate debug files (XMLs stored in ~/.openambit/movescount)", nullptr));
        checkBoxMovescountEnable->setText(QCoreApplication::translate("SettingsDialog", "Sync logs with Movescount", nullptr));
        lineEditEmail->setPlaceholderText(QCoreApplication::translate("SettingsDialog", "Email (Movescount account)", nullptr));
        groupGpsOrbit->setTitle(QCoreApplication::translate("SettingsDialog", "GPS orbit source", nullptr));
        labelOrbitSource->setText(QCoreApplication::translate("SettingsDialog", "Source:", nullptr));
        comboBoxOrbitSource->setItemText(0, QCoreApplication::translate("SettingsDialog", "Open IGS / Geoscience Australia (free, no login)", nullptr));
        comboBoxOrbitSource->setItemText(1, QCoreApplication::translate("SettingsDialog", "Local file", nullptr));
        comboBoxOrbitSource->setItemText(2, QCoreApplication::translate("SettingsDialog", "Movescount (legacy \342\200\224 offline)", nullptr));

        labelLocalOrbitFile->setText(QCoreApplication::translate("SettingsDialog", "File path:", nullptr));
        lineEditOrbitFile->setPlaceholderText(QCoreApplication::translate("SettingsDialog", "/path/to/orbit.uo", nullptr));
        buttonBrowseOrbitFile->setText(QCoreApplication::translate("SettingsDialog", "Browse\342\200\246", nullptr));
        labelOrbitCachePath->setText(QCoreApplication::translate("SettingsDialog", "Cache: ~/.openambit/orbit_cache.uo", nullptr));
        buttonDownloadOrbit->setText(QCoreApplication::translate("SettingsDialog", "Download / refresh now", nullptr));
        labelOrbitStatus->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class SettingsDialog: public Ui_SettingsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTINGSDIALOG_H
