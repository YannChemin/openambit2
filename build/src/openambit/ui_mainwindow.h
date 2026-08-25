/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.15
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "logview.h"
#include "mapview.h"
#include "sportmodeeditorwidget.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionReport_bug;
    QAction *actionAbout_openambit;
    QAction *actionE_xit;
    QAction *actionSettings;
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout;
    QSplitter *splitter;
    QWidget *leftPanel;
    QVBoxLayout *verticalLayoutLeft;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_3;
    QLabel *labelDeviceDetected;
    QToolButton *buttonDeviceReload;
    QLabel *labelSerial;
    QLabel *labelFirmwareVersion;
    QLabel *labelOrbitAge;
    QHBoxLayout *horizontalLayoutNotSupported;
    QLabel *labelNotSupported;
    QLabel *labelNotSupportedIcon;
    QHBoxLayout *horizontalLayoutMovescountAuth;
    QLabel *labelMovescountAuth;
    QLabel *labelMovescountAuthIcon;
    QHBoxLayout *horizontalLayoutNewFirmware;
    QLabel *labelNewFirmware;
    QLabel *labelNewFirmwareIcon;
    QHBoxLayout *horizontalLayoutCharge;
    QLabel *labelCharge;
    QProgressBar *chargeIndicator;
    QFrame *line;
    QVBoxLayout *verticalLayoutLogMessages;
    QProgressBar *syncProgressBar;
    QCheckBox *checkBoxResyncAll;
    QPushButton *buttonSyncNow;
    QGroupBox *groupBoxLayers;
    QVBoxLayout *verticalLayoutLayers;
    QTreeWidget *layerTree;
    QPushButton *buttonFitAll;
    QListWidget *logsList;
    QTabWidget *centerTabs;
    QWidget *mapTab;
    QVBoxLayout *mapTabLayout;
    MapView *mapView;
    QWidget *sportModeTab;
    QVBoxLayout *sportModeTabLayout;
    SportModeEditorWidget *sportModeEditor;
    LogView *logDetail;
    QMenuBar *menuBar;
    QMenu *menuOpenambit;
    QMenu *menu_Help;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1100, 700);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icon_disconnected"), QSize(), QIcon::Normal, QIcon::Off);
        MainWindow->setWindowIcon(icon);
        actionReport_bug = new QAction(MainWindow);
        actionReport_bug->setObjectName(QString::fromUtf8("actionReport_bug"));
        QIcon icon1;
        QString iconThemeName = QString::fromUtf8("help-faq");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon1 = QIcon::fromTheme(iconThemeName);
        } else {
            icon1.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionReport_bug->setIcon(icon1);
        actionAbout_openambit = new QAction(MainWindow);
        actionAbout_openambit->setObjectName(QString::fromUtf8("actionAbout_openambit"));
        QIcon icon2;
        iconThemeName = QString::fromUtf8("help-about");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon2 = QIcon::fromTheme(iconThemeName);
        } else {
            icon2.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionAbout_openambit->setIcon(icon2);
        actionE_xit = new QAction(MainWindow);
        actionE_xit->setObjectName(QString::fromUtf8("actionE_xit"));
        QIcon icon3;
        iconThemeName = QString::fromUtf8("application-exit");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon3 = QIcon::fromTheme(iconThemeName);
        } else {
            icon3.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionE_xit->setIcon(icon3);
        actionSettings = new QAction(MainWindow);
        actionSettings->setObjectName(QString::fromUtf8("actionSettings"));
        QIcon icon4;
        iconThemeName = QString::fromUtf8("preferences-system");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon4 = QIcon::fromTheme(iconThemeName);
        } else {
            icon4.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        actionSettings->setIcon(icon4);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        horizontalLayout = new QHBoxLayout(centralWidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        splitter = new QSplitter(centralWidget);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        splitter->setChildrenCollapsible(false);
        leftPanel = new QWidget(splitter);
        leftPanel->setObjectName(QString::fromUtf8("leftPanel"));
        leftPanel->setMinimumSize(QSize(230, 0));
        leftPanel->setMaximumSize(QSize(320, 16777215));
        verticalLayoutLeft = new QVBoxLayout(leftPanel);
        verticalLayoutLeft->setSpacing(4);
        verticalLayoutLeft->setContentsMargins(6, 6, 6, 6);
        verticalLayoutLeft->setObjectName(QString::fromUtf8("verticalLayoutLeft"));
        groupBox = new QGroupBox(leftPanel);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setSpacing(4);
        verticalLayout_2->setContentsMargins(6, 6, 6, 6);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        labelDeviceDetected = new QLabel(groupBox);
        labelDeviceDetected->setObjectName(QString::fromUtf8("labelDeviceDetected"));
        labelDeviceDetected->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        horizontalLayout_3->addWidget(labelDeviceDetected);

        buttonDeviceReload = new QToolButton(groupBox);
        buttonDeviceReload->setObjectName(QString::fromUtf8("buttonDeviceReload"));
        QIcon icon5;
        iconThemeName = QString::fromUtf8("view-refresh");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon5 = QIcon::fromTheme(iconThemeName);
        } else {
            icon5.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        buttonDeviceReload->setIcon(icon5);

        horizontalLayout_3->addWidget(buttonDeviceReload);


        verticalLayout_2->addLayout(horizontalLayout_3);

        labelSerial = new QLabel(groupBox);
        labelSerial->setObjectName(QString::fromUtf8("labelSerial"));

        verticalLayout_2->addWidget(labelSerial);

        labelFirmwareVersion = new QLabel(groupBox);
        labelFirmwareVersion->setObjectName(QString::fromUtf8("labelFirmwareVersion"));

        verticalLayout_2->addWidget(labelFirmwareVersion);

        labelOrbitAge = new QLabel(groupBox);
        labelOrbitAge->setObjectName(QString::fromUtf8("labelOrbitAge"));

        verticalLayout_2->addWidget(labelOrbitAge);

        horizontalLayoutNotSupported = new QHBoxLayout();
        horizontalLayoutNotSupported->setSpacing(6);
        horizontalLayoutNotSupported->setObjectName(QString::fromUtf8("horizontalLayoutNotSupported"));
        labelNotSupported = new QLabel(groupBox);
        labelNotSupported->setObjectName(QString::fromUtf8("labelNotSupported"));

        horizontalLayoutNotSupported->addWidget(labelNotSupported);

        labelNotSupportedIcon = new QLabel(groupBox);
        labelNotSupportedIcon->setObjectName(QString::fromUtf8("labelNotSupportedIcon"));
        labelNotSupportedIcon->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayoutNotSupported->addWidget(labelNotSupportedIcon);


        verticalLayout_2->addLayout(horizontalLayoutNotSupported);

        horizontalLayoutMovescountAuth = new QHBoxLayout();
        horizontalLayoutMovescountAuth->setSpacing(6);
        horizontalLayoutMovescountAuth->setObjectName(QString::fromUtf8("horizontalLayoutMovescountAuth"));
        labelMovescountAuth = new QLabel(groupBox);
        labelMovescountAuth->setObjectName(QString::fromUtf8("labelMovescountAuth"));
        labelMovescountAuth->setOpenExternalLinks(true);

        horizontalLayoutMovescountAuth->addWidget(labelMovescountAuth);

        labelMovescountAuthIcon = new QLabel(groupBox);
        labelMovescountAuthIcon->setObjectName(QString::fromUtf8("labelMovescountAuthIcon"));
        labelMovescountAuthIcon->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayoutMovescountAuth->addWidget(labelMovescountAuthIcon);


        verticalLayout_2->addLayout(horizontalLayoutMovescountAuth);

        horizontalLayoutNewFirmware = new QHBoxLayout();
        horizontalLayoutNewFirmware->setSpacing(6);
        horizontalLayoutNewFirmware->setObjectName(QString::fromUtf8("horizontalLayoutNewFirmware"));
        labelNewFirmware = new QLabel(groupBox);
        labelNewFirmware->setObjectName(QString::fromUtf8("labelNewFirmware"));

        horizontalLayoutNewFirmware->addWidget(labelNewFirmware);

        labelNewFirmwareIcon = new QLabel(groupBox);
        labelNewFirmwareIcon->setObjectName(QString::fromUtf8("labelNewFirmwareIcon"));
        labelNewFirmwareIcon->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayoutNewFirmware->addWidget(labelNewFirmwareIcon);


        verticalLayout_2->addLayout(horizontalLayoutNewFirmware);

        horizontalLayoutCharge = new QHBoxLayout();
        horizontalLayoutCharge->setSpacing(6);
        horizontalLayoutCharge->setObjectName(QString::fromUtf8("horizontalLayoutCharge"));
        labelCharge = new QLabel(groupBox);
        labelCharge->setObjectName(QString::fromUtf8("labelCharge"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(labelCharge->sizePolicy().hasHeightForWidth());
        labelCharge->setSizePolicy(sizePolicy);

        horizontalLayoutCharge->addWidget(labelCharge);

        chargeIndicator = new QProgressBar(groupBox);
        chargeIndicator->setObjectName(QString::fromUtf8("chargeIndicator"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(chargeIndicator->sizePolicy().hasHeightForWidth());
        chargeIndicator->setSizePolicy(sizePolicy1);
        chargeIndicator->setMaximumSize(QSize(16777215, 16));
        chargeIndicator->setValue(0);

        horizontalLayoutCharge->addWidget(chargeIndicator);


        verticalLayout_2->addLayout(horizontalLayoutCharge);

        line = new QFrame(groupBox);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout_2->addWidget(line);

        verticalLayoutLogMessages = new QVBoxLayout();
        verticalLayoutLogMessages->setSpacing(6);
        verticalLayoutLogMessages->setObjectName(QString::fromUtf8("verticalLayoutLogMessages"));

        verticalLayout_2->addLayout(verticalLayoutLogMessages);

        syncProgressBar = new QProgressBar(groupBox);
        syncProgressBar->setObjectName(QString::fromUtf8("syncProgressBar"));
        sizePolicy1.setHeightForWidth(syncProgressBar->sizePolicy().hasHeightForWidth());
        syncProgressBar->setSizePolicy(sizePolicy1);
        syncProgressBar->setValue(0);

        verticalLayout_2->addWidget(syncProgressBar);

        checkBoxResyncAll = new QCheckBox(groupBox);
        checkBoxResyncAll->setObjectName(QString::fromUtf8("checkBoxResyncAll"));

        verticalLayout_2->addWidget(checkBoxResyncAll);

        buttonSyncNow = new QPushButton(groupBox);
        buttonSyncNow->setObjectName(QString::fromUtf8("buttonSyncNow"));

        verticalLayout_2->addWidget(buttonSyncNow);


        verticalLayoutLeft->addWidget(groupBox);

        groupBoxLayers = new QGroupBox(leftPanel);
        groupBoxLayers->setObjectName(QString::fromUtf8("groupBoxLayers"));
        verticalLayoutLayers = new QVBoxLayout(groupBoxLayers);
        verticalLayoutLayers->setSpacing(4);
        verticalLayoutLayers->setContentsMargins(6, 6, 6, 6);
        verticalLayoutLayers->setObjectName(QString::fromUtf8("verticalLayoutLayers"));
        layerTree = new QTreeWidget(groupBoxLayers);
        layerTree->setObjectName(QString::fromUtf8("layerTree"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(layerTree->sizePolicy().hasHeightForWidth());
        layerTree->setSizePolicy(sizePolicy2);
        layerTree->setAlternatingRowColors(true);
        layerTree->setHeaderHidden(true);

        verticalLayoutLayers->addWidget(layerTree);

        buttonFitAll = new QPushButton(groupBoxLayers);
        buttonFitAll->setObjectName(QString::fromUtf8("buttonFitAll"));

        verticalLayoutLayers->addWidget(buttonFitAll);


        verticalLayoutLeft->addWidget(groupBoxLayers);

        logsList = new QListWidget(leftPanel);
        logsList->setObjectName(QString::fromUtf8("logsList"));
        sizePolicy2.setHeightForWidth(logsList->sizePolicy().hasHeightForWidth());
        logsList->setSizePolicy(sizePolicy2);

        verticalLayoutLeft->addWidget(logsList);

        splitter->addWidget(leftPanel);
        centerTabs = new QTabWidget(splitter);
        centerTabs->setObjectName(QString::fromUtf8("centerTabs"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy3.setHorizontalStretch(3);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(centerTabs->sizePolicy().hasHeightForWidth());
        centerTabs->setSizePolicy(sizePolicy3);
        centerTabs->setDocumentMode(true);
        mapTab = new QWidget();
        mapTab->setObjectName(QString::fromUtf8("mapTab"));
        mapTabLayout = new QVBoxLayout(mapTab);
        mapTabLayout->setSpacing(0);
        mapTabLayout->setContentsMargins(0, 0, 0, 0);
        mapTabLayout->setObjectName(QString::fromUtf8("mapTabLayout"));
        mapView = new MapView(mapTab);
        mapView->setObjectName(QString::fromUtf8("mapView"));
        QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(mapView->sizePolicy().hasHeightForWidth());
        mapView->setSizePolicy(sizePolicy4);

        mapTabLayout->addWidget(mapView);

        centerTabs->addTab(mapTab, QString());
        sportModeTab = new QWidget();
        sportModeTab->setObjectName(QString::fromUtf8("sportModeTab"));
        sportModeTabLayout = new QVBoxLayout(sportModeTab);
        sportModeTabLayout->setSpacing(0);
        sportModeTabLayout->setContentsMargins(0, 0, 0, 0);
        sportModeTabLayout->setObjectName(QString::fromUtf8("sportModeTabLayout"));
        sportModeEditor = new SportModeEditorWidget(sportModeTab);
        sportModeEditor->setObjectName(QString::fromUtf8("sportModeEditor"));
        sizePolicy4.setHeightForWidth(sportModeEditor->sizePolicy().hasHeightForWidth());
        sportModeEditor->setSizePolicy(sizePolicy4);

        sportModeTabLayout->addWidget(sportModeEditor);

        centerTabs->addTab(sportModeTab, QString());
        splitter->addWidget(centerTabs);
        logDetail = new LogView(splitter);
        logDetail->setObjectName(QString::fromUtf8("logDetail"));
        QSizePolicy sizePolicy5(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy5.setHorizontalStretch(1);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(logDetail->sizePolicy().hasHeightForWidth());
        logDetail->setSizePolicy(sizePolicy5);
        logDetail->setProperty("openExternalLinks", QVariant(true));
        splitter->addWidget(logDetail);

        horizontalLayout->addWidget(splitter);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1100, 20));
        menuOpenambit = new QMenu(menuBar);
        menuOpenambit->setObjectName(QString::fromUtf8("menuOpenambit"));
        menu_Help = new QMenu(menuBar);
        menu_Help->setObjectName(QString::fromUtf8("menu_Help"));
        MainWindow->setMenuBar(menuBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuOpenambit->menuAction());
        menuBar->addAction(menu_Help->menuAction());
        menuOpenambit->addAction(actionSettings);
        menuOpenambit->addSeparator();
        menuOpenambit->addAction(actionE_xit);
        menu_Help->addAction(actionReport_bug);
        menu_Help->addAction(actionAbout_openambit);

        retranslateUi(MainWindow);
        QObject::connect(actionE_xit, SIGNAL(triggered()), MainWindow, SLOT(closeRequested()));
        QObject::connect(actionSettings, SIGNAL(triggered()), MainWindow, SLOT(showSettings()));
        QObject::connect(actionReport_bug, SIGNAL(triggered()), MainWindow, SLOT(showReportBug()));
        QObject::connect(actionAbout_openambit, SIGNAL(triggered()), MainWindow, SLOT(showAbout()));

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Openambit", nullptr));
        actionReport_bug->setText(QCoreApplication::translate("MainWindow", "Report bug...", nullptr));
#if QT_CONFIG(tooltip)
        actionReport_bug->setToolTip(QCoreApplication::translate("MainWindow", "Report bug...", nullptr));
#endif // QT_CONFIG(tooltip)
        actionAbout_openambit->setText(QCoreApplication::translate("MainWindow", "About Openambit...", nullptr));
#if QT_CONFIG(tooltip)
        actionAbout_openambit->setToolTip(QCoreApplication::translate("MainWindow", "About Openambit...", nullptr));
#endif // QT_CONFIG(tooltip)
        actionE_xit->setText(QCoreApplication::translate("MainWindow", "E&xit", nullptr));
        actionSettings->setText(QCoreApplication::translate("MainWindow", "&Settings", nullptr));
        groupBox->setTitle(QCoreApplication::translate("MainWindow", "Device", nullptr));
        labelDeviceDetected->setText(QCoreApplication::translate("MainWindow", "No device detected", nullptr));
        buttonDeviceReload->setText(QString());
        labelSerial->setText(QString());
        labelFirmwareVersion->setText(QString());
        labelOrbitAge->setText(QString());
        labelNotSupported->setText(QCoreApplication::translate("MainWindow", "Device not supported yet!", nullptr));
        labelNotSupportedIcon->setText(QString());
        labelMovescountAuth->setText(QCoreApplication::translate("MainWindow", "Auth on <a href=\"http://www.movescount.com\"><span style=\" text-decoration: underline; color:#0057ae;\">movescount.com</span></a>!", nullptr));
        labelMovescountAuthIcon->setText(QString());
        labelNewFirmware->setText(QString());
        labelNewFirmwareIcon->setText(QString());
        labelCharge->setText(QCoreApplication::translate("MainWindow", "Charge:", nullptr));
        checkBoxResyncAll->setText(QCoreApplication::translate("MainWindow", "Resync all", nullptr));
        buttonSyncNow->setText(QCoreApplication::translate("MainWindow", "Sync now", nullptr));
        groupBoxLayers->setTitle(QCoreApplication::translate("MainWindow", "Map Layers", nullptr));
        QTreeWidgetItem *___qtreewidgetitem = layerTree->headerItem();
        ___qtreewidgetitem->setText(0, QCoreApplication::translate("MainWindow", "Layer", nullptr));
        buttonFitAll->setText(QCoreApplication::translate("MainWindow", "Fit All", nullptr));
        centerTabs->setTabText(centerTabs->indexOf(mapTab), QCoreApplication::translate("MainWindow", "Map", nullptr));
        centerTabs->setTabText(centerTabs->indexOf(sportModeTab), QCoreApplication::translate("MainWindow", "Sport Modes", nullptr));
        menuOpenambit->setTitle(QCoreApplication::translate("MainWindow", "&File", nullptr));
        menu_Help->setTitle(QCoreApplication::translate("MainWindow", "&Help", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
