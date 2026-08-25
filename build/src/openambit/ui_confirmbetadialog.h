/********************************************************************************
** Form generated from reading UI file 'confirmbetadialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.15
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONFIRMBETADIALOG_H
#define UI_CONFIRMBETADIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QTextBrowser>

QT_BEGIN_NAMESPACE

class Ui_ConfirmBetaDialog
{
public:
    QGridLayout *gridLayout;
    QDialogButtonBox *buttonBox;
    QCheckBox *skipBetaCheck;
    QTextBrowser *textBrowser;

    void setupUi(QDialog *ConfirmBetaDialog)
    {
        if (ConfirmBetaDialog->objectName().isEmpty())
            ConfirmBetaDialog->setObjectName(QString::fromUtf8("ConfirmBetaDialog"));
        ConfirmBetaDialog->resize(558, 498);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icon_disconnected"), QSize(), QIcon::Normal, QIcon::Off);
        ConfirmBetaDialog->setWindowIcon(icon);
        gridLayout = new QGridLayout(ConfirmBetaDialog);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        buttonBox = new QDialogButtonBox(ConfirmBetaDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 2, 0, 1, 1);

        skipBetaCheck = new QCheckBox(ConfirmBetaDialog);
        skipBetaCheck->setObjectName(QString::fromUtf8("skipBetaCheck"));

        gridLayout->addWidget(skipBetaCheck, 1, 0, 1, 1);

        textBrowser = new QTextBrowser(ConfirmBetaDialog);
        textBrowser->setObjectName(QString::fromUtf8("textBrowser"));
        textBrowser->setOpenExternalLinks(true);

        gridLayout->addWidget(textBrowser, 0, 0, 1, 1);


        retranslateUi(ConfirmBetaDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), ConfirmBetaDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ConfirmBetaDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(ConfirmBetaDialog);
    } // setupUi

    void retranslateUi(QDialog *ConfirmBetaDialog)
    {
        ConfirmBetaDialog->setWindowTitle(QCoreApplication::translate("ConfirmBetaDialog", "Openambit: Disclaimer", nullptr));
        skipBetaCheck->setText(QCoreApplication::translate("ConfirmBetaDialog", "Don't ask me again, I really want to use Openambit", nullptr));
        textBrowser->setHtml(QCoreApplication::translate("ConfirmBetaDialog", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Ubuntu'; font-size:11pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">EARLY BETA SOFTWARE: <a href=\"http://en.wikipedia.org/wiki/Here_be_dragons\"><span style=\" text-decoration: underline; color:#0000ff;\">here be dragons</span></a></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">This software is in early beta stage, it might upload invalid logs to movescount that cannot be overwritten, and do"
                        " other bizarre things.</p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" color:#008000;\">Openambit</span><span style=\" color:#c0c0c0;\"> </span><span style=\" color:#008000;\">is</span><span style=\" color:#c0c0c0;\"> </span><span style=\" color:#008000;\">free</span><span style=\" color:#c0c0c0;\"> </span><span style=\" color:#008000;\">software:</span><span style=\" color:#c0c0c0;\"> </span><span style=\" color:#008000;\">you</span><span style=\" color:#c0c0c0;\"> </span><span style=\" color:#008000;\">can</span><span style=\" color:#c0c0c0;\"> </span><span style=\" color:#008000;\">redistribute</span><span style=\" color:#c0c0c0;\"> </span><span style=\" color:#008000;\">it</span><span style=\" color:#c0c0c0;\"> </span><span style=\" color:#008000;\">and/"
                        "or</span><span style=\" color:#c0c0c0;\"> </span><span style=\" color:#008000;\">modify </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">it</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">under</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">the</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">terms</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">of</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">the</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" f"
                        "ont-family:'Courier New,courier'; color:#008000;\">GNU</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">General</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">Public</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">License</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">as</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">published</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">by the</span><span style=\" font-family:'Couri"
                        "er New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">Free</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">Software</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">Foundation,</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">either</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">version</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">3</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; co"
                        "lor:#008000;\">of</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">the</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">License,</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">or</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">(at</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">your</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">option)</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span"
                        " style=\" font-family:'Courier New,courier'; color:#008000;\">any</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">later</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">version.</span></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'Courier New,courier';\"><br /></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Courier New,courier'; color:#008000;\">This</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">program</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span s"
                        "tyle=\" font-family:'Courier New,courier'; color:#008000;\">is</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">distributed</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">in</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">the</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">hope</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">that</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">it</span><span style=\" font-family:'Courier "
                        "New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">will</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">be</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">useful, but</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">WITHOUT</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">ANY</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">WARRANTY;</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:"
                        "#008000;\">without</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">even</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">the</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">implied</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">warranty</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">of</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">MERCHANTABILITY</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\">"
                        " </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">or</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">FITNESS</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">FOR</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">A</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">PARTICULAR</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">PURPOSE.</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">See</span><span style=\" "
                        "font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">the GNU</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">General</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">Public</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">License</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">for</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier New,courier'; color:#008000;\">more</span><span style=\" font-family:'Courier New,courier'; color:#c0c0c0;\"> </span><span style=\" font-family:'Courier N"
                        "ew,courier'; color:#008000;\">details.</span></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Please use this software at <span style=\" text-decoration: underline;\">your own</span> risk and only if you understand the risks involved.</p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">If you decide to use Openambit please consider <a href=\"https://github.com/openambitproject/openambit/issues\"><span style=\" text-decoration: underline; color:#0000ff;\">reporting bugs</span></a> so we can make it better.</p>\n"
"<p style=\"-qt-paragraph-t"
                        "ype:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Thanks </p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'Courier New,courier'; color:#008000;\"><br /></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'Courier New,courier'; color:#008000;\"><br /></p></body></html> \n"
"      ", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ConfirmBetaDialog: public Ui_ConfirmBetaDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONFIRMBETADIALOG_H
