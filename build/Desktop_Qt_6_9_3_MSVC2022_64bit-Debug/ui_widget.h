/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
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
    QSpacerItem *horizontalSpacer;
    QPushButton *btnApply;
    QPushButton *btnCompare;
    QPushButton *btnSave;
    QPushButton *btnClearChain;
    QLabel *labelInfo;
    QLabel *label_2;
    QLabel *label_3;
    QGraphicsView *graphicsView;
    QWidget *widget_2;
    QVBoxLayout *verticalLayout_3;
    QLabel *algoTitleLabel;
    MyListWidget *listWidget;
    QSpacerItem *verticalSpacer;
    QWidget *widget_3;
    QVBoxLayout *blockPanelOuterLayout;
    QLabel *chainTitleLabel;
    QLabel *chainHintLabel;
    QSpacerItem *blockPanelSpacer;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName("Widget");
        Widget->resize(1200, 720);
        Widget->setMinimumSize(QSize(960, 560));
        horizontalLayout = new QHBoxLayout(Widget);
        horizontalLayout->setSpacing(10);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(10, 10, 10, 10);
        widget = new QWidget(Widget);
        widget->setObjectName("widget");
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setSpacing(8);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(8);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        pushButton = new QPushButton(widget);
        pushButton->setObjectName("pushButton");
        pushButton->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        horizontalLayout_2->addWidget(pushButton);

        pushButton_2 = new QPushButton(widget);
        pushButton_2->setObjectName("pushButton_2");
        pushButton_2->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        horizontalLayout_2->addWidget(pushButton_2);

        label = new QLabel(widget);
        label->setObjectName("label");

        horizontalLayout_2->addWidget(label);

        comboBox = new QComboBox(widget);
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->setObjectName("comboBox");
        comboBox->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        horizontalLayout_2->addWidget(comboBox);

        pushButton_3 = new QPushButton(widget);
        pushButton_3->setObjectName("pushButton_3");
        pushButton_3->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        horizontalLayout_2->addWidget(pushButton_3);

        deltete = new QPushButton(widget);
        deltete->setObjectName("deltete");
        deltete->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        horizontalLayout_2->addWidget(deltete);

        horizontalSpacer = new QSpacerItem(20, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        btnApply = new QPushButton(widget);
        btnApply->setObjectName("btnApply");
        btnApply->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        horizontalLayout_2->addWidget(btnApply);

        btnCompare = new QPushButton(widget);
        btnCompare->setObjectName("btnCompare");
        btnCompare->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnCompare->setCheckable(true);

        horizontalLayout_2->addWidget(btnCompare);

        btnSave = new QPushButton(widget);
        btnSave->setObjectName("btnSave");
        btnSave->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        horizontalLayout_2->addWidget(btnSave);

        btnClearChain = new QPushButton(widget);
        btnClearChain->setObjectName("btnClearChain");
        btnClearChain->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        horizontalLayout_2->addWidget(btnClearChain);

        labelInfo = new QLabel(widget);
        labelInfo->setObjectName("labelInfo");

        horizontalLayout_2->addWidget(labelInfo);

        label_2 = new QLabel(widget);
        label_2->setObjectName("label_2");

        horizontalLayout_2->addWidget(label_2);

        label_3 = new QLabel(widget);
        label_3->setObjectName("label_3");

        horizontalLayout_2->addWidget(label_3);


        verticalLayout->addLayout(horizontalLayout_2);

        graphicsView = new QGraphicsView(widget);
        graphicsView->setObjectName("graphicsView");

        verticalLayout->addWidget(graphicsView);


        horizontalLayout->addWidget(widget);

        widget_2 = new QWidget(Widget);
        widget_2->setObjectName("widget_2");
        widget_2->setMinimumSize(QSize(160, 0));
        verticalLayout_3 = new QVBoxLayout(widget_2);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setContentsMargins(10, 10, 10, 10);
        algoTitleLabel = new QLabel(widget_2);
        algoTitleLabel->setObjectName("algoTitleLabel");

        verticalLayout_3->addWidget(algoTitleLabel);

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

        verticalLayout_3->addWidget(listWidget);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_3->addItem(verticalSpacer);


        horizontalLayout->addWidget(widget_2);

        widget_3 = new QWidget(Widget);
        widget_3->setObjectName("widget_3");
        widget_3->setMinimumSize(QSize(220, 0));
        blockPanelOuterLayout = new QVBoxLayout(widget_3);
        blockPanelOuterLayout->setSpacing(6);
        blockPanelOuterLayout->setObjectName("blockPanelOuterLayout");
        blockPanelOuterLayout->setContentsMargins(10, 10, 10, 10);
        chainTitleLabel = new QLabel(widget_3);
        chainTitleLabel->setObjectName("chainTitleLabel");

        blockPanelOuterLayout->addWidget(chainTitleLabel);

        chainHintLabel = new QLabel(widget_3);
        chainHintLabel->setObjectName("chainHintLabel");
        chainHintLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        blockPanelOuterLayout->addWidget(chainHintLabel);

        blockPanelSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        blockPanelOuterLayout->addItem(blockPanelSpacer);


        horizontalLayout->addWidget(widget_3);

        horizontalLayout->setStretch(0, 12);
        horizontalLayout->setStretch(1, 3);
        horizontalLayout->setStretch(2, 4);

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "\345\233\276\345\203\217\345\244\204\347\220\206\345\267\245\345\205\267", nullptr));
#if QT_CONFIG(tooltip)
        pushButton->setToolTip(QCoreApplication::translate("Widget", "\346\211\223\345\274\200\345\233\276\347\211\207", nullptr));
#endif // QT_CONFIG(tooltip)
        pushButton->setText(QCoreApplication::translate("Widget", "\346\211\223\345\274\200", nullptr));
#if QT_CONFIG(tooltip)
        pushButton_2->setToolTip(QCoreApplication::translate("Widget", "\346\211\223\345\274\200\346\226\207\344\273\266\345\244\271", nullptr));
#endif // QT_CONFIG(tooltip)
        pushButton_2->setText(QCoreApplication::translate("Widget", "\346\226\207\344\273\266\345\244\271", nullptr));
        label->setText(QCoreApplication::translate("Widget", "ROI", nullptr));
        comboBox->setItemText(0, QCoreApplication::translate("Widget", "\347\237\251\345\275\242", nullptr));
        comboBox->setItemText(1, QCoreApplication::translate("Widget", "\345\234\206\345\275\242", nullptr));
        comboBox->setItemText(2, QCoreApplication::translate("Widget", "\346\227\213\350\275\254\347\237\251\345\275\242", nullptr));

        pushButton_3->setText(QCoreApplication::translate("Widget", "\346\267\273\345\212\240", nullptr));
#if QT_CONFIG(tooltip)
        deltete->setToolTip(QCoreApplication::translate("Widget", "\345\210\240\351\231\244\351\200\211\344\270\255 ROI", nullptr));
#endif // QT_CONFIG(tooltip)
        deltete->setText(QCoreApplication::translate("Widget", "\345\210\240\351\231\244", nullptr));
#if QT_CONFIG(tooltip)
        btnApply->setToolTip(QCoreApplication::translate("Widget", "\346\214\211\345\275\223\345\211\215 ROI \344\270\216\345\244\204\347\220\206\351\223\276\351\207\215\346\226\260\350\256\241\347\256\227", nullptr));
#endif // QT_CONFIG(tooltip)
        btnApply->setText(QCoreApplication::translate("Widget", "\345\272\224\347\224\250", nullptr));
#if QT_CONFIG(tooltip)
        btnCompare->setToolTip(QCoreApplication::translate("Widget", "\345\216\237\345\233\276 / \347\273\223\346\236\234\345\210\207\346\215\242", nullptr));
#endif // QT_CONFIG(tooltip)
        btnCompare->setText(QCoreApplication::translate("Widget", "\345\257\271\346\257\224", nullptr));
#if QT_CONFIG(tooltip)
        btnSave->setToolTip(QCoreApplication::translate("Widget", "\344\277\235\345\255\230\345\244\204\347\220\206\347\273\223\346\236\234", nullptr));
#endif // QT_CONFIG(tooltip)
        btnSave->setText(QCoreApplication::translate("Widget", "\344\277\235\345\255\230", nullptr));
#if QT_CONFIG(tooltip)
        btnClearChain->setToolTip(QCoreApplication::translate("Widget", "\346\270\205\347\251\272\345\244\204\347\220\206\351\223\276", nullptr));
#endif // QT_CONFIG(tooltip)
        btnClearChain->setText(QCoreApplication::translate("Widget", "\346\270\205\347\251\272", nullptr));
#if QT_CONFIG(tooltip)
        labelInfo->setToolTip(QCoreApplication::translate("Widget", "\345\233\276\347\211\207\344\277\241\346\201\257", nullptr));
#endif // QT_CONFIG(tooltip)
        labelInfo->setText(QCoreApplication::translate("Widget", "-", nullptr));
        label_2->setText(QCoreApplication::translate("Widget", "\350\200\227\346\227\266", nullptr));
        label_3->setText(QCoreApplication::translate("Widget", "0 ms", nullptr));
        algoTitleLabel->setText(QCoreApplication::translate("Widget", "\347\256\227\346\263\225", nullptr));

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

        chainTitleLabel->setText(QCoreApplication::translate("Widget", "\345\244\204\347\220\206\351\223\276", nullptr));
        chainHintLabel->setText(QCoreApplication::translate("Widget", "\346\213\226\345\205\245\347\256\227\346\263\225", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
