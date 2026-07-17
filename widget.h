#ifndef WIDGET_H
#define WIDGET_H

#include"resizablerectitem.h"
#include"resizableellipseitem.h"
// 核心必须的
#include <QApplication>
#include <QWidget>
#include <QDebug>

// 文件操作和对话框
#include <QFileDialog>
#include <QMessageBox>

// 图形相关
#include <QPainter>
#include <QPen>
#include <QPixmap>

// 布局
#include <QVBoxLayout>
#include <QHBoxLayout>

// 控件
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QListWidget>
#include <QSpinBox>
#include <QCheckBox>

// 事件
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QMimeData>

// Graphics View
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>


QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

// ========== 自定义 ListWidget（支持拖出文本数据） ==========
class MyListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit MyListWidget(QWidget *parent = nullptr) {
        setDragEnabled(true);
        setDragDropMode(QAbstractItemView::DragOnly);
    }

protected:
    QMimeData *mimeData(const QList<QListWidgetItem *> &items) const override {
        if (items.isEmpty()) return nullptr;
        QMimeData *mimeData = new QMimeData;
        mimeData->setText(items.first()->text());
        return mimeData;
    }
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

protected:
    void wheelEvent(QWheelEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void addRectItem(qreal x,qreal y,qreal w,qreal h);
    void addEllipseItem(qreal x,qreal y,qreal w,qreal h);
private slots:
    void on_pushButton_clicked();
    void on_comboBox_currentIndexChanged(int index);

    void on_pushButton_3_clicked();

    void on_deltete_clicked();

private:
    void updateImage();

    Ui::Widget *ui;

    // 图片相关
    QPixmap currentPixmap;
    QGraphicsScene *scene;
    QGraphicsPixmapItem *pixmapItem;
    ResizableRectItem *rectItem;
    ResizableEllipseItem *ellipseItem;
    QPixmap originalPixmap;  // 保存原始图片（用于恢复）

    double scaleFactor = 1.0;

    bool isDragInside = false;

    void updatePixmapItem(const QPixmap &pixmap);//显示更新的图片
    // ========== 处理块列表 ==========
    QVBoxLayout *blockLayout;        // widget_3 的布局（用于排列处理块）
    QList<QWidget*> blockList;       // 所有处理块的列表
    //========二值化处理模块============
    // ========== 创建处理块 ==========
    void createBinarizationBlock();  // 创建二值化处理块界面
    int calculateOtsuThreshold();//otsu计算阈值
    QPixmap applyBinaryThreshold(int lower, int upper);//二值化处理
    // ========== 处理块的子控件 ==========
    QWidget *currentBlock;           // 当前显示的处理块
    QCheckBox *enableCheckBox;       // 使能复选框
    QSpinBox *lowerSpinBox;          // 下限值
    QSpinBox *upperSpinBox;          // 上限值
    QPushButton *autoBtn;            // 自动设置按钮
};

#endif // WIDGET_H