/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <widget.h>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QHBoxLayout *horizontalLayout;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QLabel *label;
    QComboBox *comboBox;
    QPushButton *pushButton_3;
    QPushButton *deltete;
    QLabel *label_2;
    QLabel *label_3;
    QGraphicsView *graphicsView;
    QWidget *widget_2;
    QVBoxLayout *verticalLayout_3;
    QVBoxLayout *verticalLayout_2;
    MyListWidget *listWidget;
    QSpacerItem *verticalSpacer;
    QWidget *widget_3;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName("Widget");
        Widget->resize(1093, 600);
        QFont font;
        font.setPointSize(12);
        Widget->setFont(font);
        horizontalLayout = new QHBoxLayout(Widget);
        horizontalLayout->setObjectName("horizontalLayout");
        widget = new QWidget(Widget);
        widget->setObjectName("widget");
        widget->setStyleSheet(QString::fromUtf8("background: qlineargradient(x1:0, y1:0, x2:0, y2:1,\n"
"    stop:0 #ffffff,\n"
"    stop:1 #f0f0f0);"));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        pushButton = new QPushButton(widget);
        pushButton->setObjectName("pushButton");
        pushButton->setStyleSheet(QString::fromUtf8("/* \346\224\271\346\214\211\351\222\256\347\232\204\345\255\227\344\275\223 */\n"
"QPushButton {\n"
"    font-family: \"\345\276\256\350\275\257\351\233\205\351\273\221\";\n"
"    font-size: 14px;\n"
"    font-weight: bold;\n"
"    color: #ffffff;\n"
"    background-color: #4CAF50;\n"
"    border-radius: 5px;\n"
"    padding: 8px 16px;\n"
"}\n"
"\n"
"/* \351\274\240\346\240\207\346\202\254\345\201\234\346\225\210\346\236\234 */\n"
"QPushButton:hover {\n"
"    background-color: #45a049;\n"
"}"));

        horizontalLayout_2->addWidget(pushButton);

        pushButton_2 = new QPushButton(widget);
        pushButton_2->setObjectName("pushButton_2");
        pushButton_2->setStyleSheet(QString::fromUtf8("/* \346\224\271\346\214\211\351\222\256\347\232\204\345\255\227\344\275\223 */\n"
"QPushButton {\n"
"    font-family: \"\345\276\256\350\275\257\351\233\205\351\273\221\";\n"
"    font-size: 14px;\n"
"    font-weight: bold;\n"
"    color: #ffffff;\n"
"    background-color: #4CAF50;\n"
"    border-radius: 5px;\n"
"    padding: 8px 16px;\n"
"}\n"
"\n"
"/* \351\274\240\346\240\207\346\202\254\345\201\234\346\225\210\346\236\234 */\n"
"QPushButton:hover {\n"
"    background-color: #45a049;\n"
"}"));

        horizontalLayout_2->addWidget(pushButton_2);

        label = new QLabel(widget);
        label->setObjectName("label");
        QFont font1;
        font1.setPointSize(13);
        font1.setBold(true);
        label->setFont(font1);

        horizontalLayout_2->addWidget(label);

        comboBox = new QComboBox(widget);
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->setObjectName("comboBox");

        horizontalLayout_2->addWidget(comboBox);

        pushButton_3 = new QPushButton(widget);
        pushButton_3->setObjectName("pushButton_3");
        pushButton_3->setStyleSheet(QString::fromUtf8("/* \346\224\271\346\214\211\351\222\256\347\232\204\345\255\227\344\275\223 */\n"
"QPushButton {\n"
"    font-family: \"\345\276\256\350\275\257\351\233\205\351\273\221\";\n"
"    font-size: 14px;\n"
"    font-weight: bold;\n"
"    color: #ffffff;\n"
"    background-color: #4CAF50;\n"
"    border-radius: 5px;\n"
"    padding: 8px 16px;\n"
"}\n"
"\n"
"/* \351\274\240\346\240\207\346\202\254\345\201\234\346\225\210\346\236\234 */\n"
"QPushButton:hover {\n"
"    background-color: #45a049;\n"
"}"));

        horizontalLayout_2->addWidget(pushButton_3);

        deltete = new QPushButton(widget);
        deltete->setObjectName("deltete");
        deltete->setStyleSheet(QString::fromUtf8("/* \346\224\271\346\214\211\351\222\256\347\232\204\345\255\227\344\275\223 */\n"
"QPushButton {\n"
"    font-family: \"\345\276\256\350\275\257\351\233\205\351\273\221\";\n"
"    font-size: 14px;\n"
"    font-weight: bold;\n"
"    color: #ffffff;\n"
"    background-color: #4CAF50;\n"
"    border-radius: 5px;\n"
"    padding: 8px 16px;\n"
"}\n"
"\n"
"/* \351\274\240\346\240\207\346\202\254\345\201\234\346\225\210\346\236\234 */\n"
"QPushButton:hover {\n"
"    background-color: #45a049;\n"
"}"));

        horizontalLayout_2->addWidget(deltete);

        label_2 = new QLabel(widget);
        label_2->setObjectName("label_2");
        label_2->setFont(font1);

        horizontalLayout_2->addWidget(label_2);

        label_3 = new QLabel(widget);
        label_3->setObjectName("label_3");
        label_3->setFont(font1);

        horizontalLayout_2->addWidget(label_3);


        verticalLayout->addLayout(horizontalLayout_2);

        graphicsView = new QGraphicsView(widget);
        graphicsView->setObjectName("graphicsView");

        verticalLayout->addWidget(graphicsView);


        horizontalLayout->addWidget(widget);

        widget_2 = new QWidget(Widget);
        widget_2->setObjectName("widget_2");
        widget_2->setStyleSheet(QString::fromUtf8("background: qlineargradient(x1:0, y1:0, x2:0, y2:1,\n"
"    stop:0 #f2f2f2,\n"
"    stop:1 #e0e0e0);"));
        verticalLayout_3 = new QVBoxLayout(widget_2);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName("verticalLayout_2");
        listWidget = new MyListWidget(widget_2);
        QListWidgetItem *__qlistwidgetitem = new QListWidgetItem(listWidget);
        __qlistwidgetitem->setTextAlignment(Qt::AlignCenter);
        QListWidgetItem *__qlistwidgetitem1 = new QListWidgetItem(listWidget);
        __qlistwidgetitem1->setTextAlignment(Qt::AlignCenter);
        QListWidgetItem *__qlistwidgetitem2 = new QListWidgetItem(listWidget);
        __qlistwidgetitem2->setTextAlignment(Qt::AlignCenter);
        QListWidgetItem *__qlistwidgetitem3 = new QListWidgetItem(listWidget);
        __qlistwidgetitem3->setTextAlignment(Qt::AlignCenter);
        QListWidgetItem *__qlistwidgetitem4 = new QListWidgetItem(listWidget);
        __qlistwidgetitem4->setTextAlignment(Qt::AlignCenter);
        listWidget->setObjectName("listWidget");
        listWidget->setMinimumSize(QSize(0, 250));
        QFont font2;
        font2.setPointSize(16);
        font2.setWeight(QFont::Black);
        font2.setKerning(true);
        listWidget->setFont(font2);
        listWidget->setStyleSheet(QString::fromUtf8("QListWidget::item {\n"
"    text-align: center;\n"
"    }"));

        verticalLayout_2->addWidget(listWidget);

        verticalSpacer = new QSpacerItem(20, 2000, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);


        verticalLayout_3->addLayout(verticalLayout_2);


        horizontalLayout->addWidget(widget_2);

        widget_3 = new QWidget(Widget);
        widget_3->setObjectName("widget_3");
        widget_3->setStyleSheet(QString::fromUtf8("background: qlineargradient(x1:0, y1:0, x2:0, y2:1,\n"
"    stop:0 #e8e8e8,\n"
"    stop:1 #d0d0d0);"));

        horizontalLayout->addWidget(widget_3);

        horizontalLayout->setStretch(0, 10);
        horizontalLayout->setStretch(1, 3);
        horizontalLayout->setStretch(2, 4);

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "Widget", nullptr));
#if QT_CONFIG(whatsthis)
        pushButton->setWhatsThis(QCoreApplication::translate("Widget", "<html><head/><body><p><br/></p></body></html>", nullptr));
#endif // QT_CONFIG(whatsthis)
        pushButton->setText(QCoreApplication::translate("Widget", "\346\211\223\345\274\200\345\233\276\347\211\207", nullptr));
#if QT_CONFIG(whatsthis)
        pushButton_2->setWhatsThis(QCoreApplication::translate("Widget", "<html><head/><body><p><br/></p></body></html>", nullptr));
#endif // QT_CONFIG(whatsthis)
        pushButton_2->setText(QCoreApplication::translate("Widget", "\346\211\223\345\274\200\346\226\207\344\273\266\345\244\271", nullptr));
        label->setText(QCoreApplication::translate("Widget", "ROI\357\274\232", nullptr));
        comboBox->setItemText(0, QCoreApplication::translate("Widget", "\347\237\251\345\275\242", nullptr));
        comboBox->setItemText(1, QCoreApplication::translate("Widget", "\345\234\206\345\275\242", nullptr));
        comboBox->setItemText(2, QCoreApplication::translate("Widget", "\346\227\213\350\275\254\347\237\251\345\275\242", nullptr));

        pushButton_3->setText(QCoreApplication::translate("Widget", "\346\267\273\345\212\240", nullptr));
        deltete->setText(QCoreApplication::translate("Widget", "\345\210\240\351\231\244", nullptr));
        label_2->setText(QCoreApplication::translate("Widget", "\350\200\227\346\227\266\357\274\232", nullptr));
        label_3->setText(QCoreApplication::translate("Widget", "0ms", nullptr));

        const bool __sortingEnabled = listWidget->isSortingEnabled();
        listWidget->setSortingEnabled(false);
        QListWidgetItem *___qlistwidgetitem = listWidget->item(0);
        ___qlistwidgetitem->setText(QCoreApplication::translate("Widget", "\344\272\214\345\200\274\345\214\226\345\244\204\347\220\206", nullptr));
        QListWidgetItem *___qlistwidgetitem1 = listWidget->item(1);
        ___qlistwidgetitem1->setText(QCoreApplication::translate("Widget", "\345\275\242\346\200\201\345\255\246\345\244\204\347\220\206", nullptr));
        QListWidgetItem *___qlistwidgetitem2 = listWidget->item(2);
        ___qlistwidgetitem2->setText(QCoreApplication::translate("Widget", "\346\273\244\346\263\242\345\244\204\347\220\206", nullptr));
        QListWidgetItem *___qlistwidgetitem3 = listWidget->item(3);
        ___qlistwidgetitem3->setText(QCoreApplication::translate("Widget", "\347\201\260\345\272\246\345\217\230\346\215\242", nullptr));
        QListWidgetItem *___qlistwidgetitem4 = listWidget->item(4);
        ___qlistwidgetitem4->setText(QCoreApplication::translate("Widget", "\344\274\252\345\275\251\350\211\262\345\244\204\347\220\206", nullptr));
        listWidget->setSortingEnabled(__sortingEnabled);

    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
