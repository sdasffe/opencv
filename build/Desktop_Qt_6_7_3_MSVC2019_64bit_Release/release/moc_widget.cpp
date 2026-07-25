/****************************************************************************
** Meta object code from reading C++ file 'widget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../core/widget.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'widget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.7.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSMyListWidgetENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSMyListWidgetENDCLASS = QtMocHelpers::stringData(
    "MyListWidget"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSMyListWidgetENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

Q_CONSTINIT const QMetaObject MyListWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QListWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSMyListWidgetENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSMyListWidgetENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSMyListWidgetENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MyListWidget, std::true_type>
    >,
    nullptr
} };

void MyListWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *MyListWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MyListWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSMyListWidgetENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QListWidget::qt_metacast(_clname);
}

int MyListWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QListWidget::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSWidgetENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSWidgetENDCLASS = QtMocHelpers::stringData(
    "Widget",
    "actOpenImage",
    "",
    "actOpenFolder",
    "AddRoi",
    "DelteteRoi",
    "onExitApp",
    "onAboutApp",
    "onHelpShortcuts",
    "onLanguageChinese",
    "onLanguageEnglish",
    "onThemeLight",
    "onThemeDark",
    "onProcessingFinished",
    "elapsedMs",
    "onApplyProcessing",
    "onRoiGeometryChanged",
    "on_btnApply_clicked",
    "on_btnCompare_pressed",
    "on_btnCompare_released",
    "on_btnSave_clicked",
    "on_btnClearChain_clicked",
    "on_btnExportChain_clicked",
    "on_btnImportChain_clicked",
    "on_folderImageList_itemClicked",
    "QListWidgetItem*",
    "item",
    "onUndo"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSWidgetENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      23,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  152,    2, 0x08,    1 /* Private */,
       3,    0,  153,    2, 0x08,    2 /* Private */,
       4,    0,  154,    2, 0x08,    3 /* Private */,
       5,    0,  155,    2, 0x08,    4 /* Private */,
       6,    0,  156,    2, 0x08,    5 /* Private */,
       7,    0,  157,    2, 0x08,    6 /* Private */,
       8,    0,  158,    2, 0x08,    7 /* Private */,
       9,    0,  159,    2, 0x08,    8 /* Private */,
      10,    0,  160,    2, 0x08,    9 /* Private */,
      11,    0,  161,    2, 0x08,   10 /* Private */,
      12,    0,  162,    2, 0x08,   11 /* Private */,
      13,    1,  163,    2, 0x08,   12 /* Private */,
      15,    0,  166,    2, 0x08,   14 /* Private */,
      16,    0,  167,    2, 0x08,   15 /* Private */,
      17,    0,  168,    2, 0x08,   16 /* Private */,
      18,    0,  169,    2, 0x08,   17 /* Private */,
      19,    0,  170,    2, 0x08,   18 /* Private */,
      20,    0,  171,    2, 0x08,   19 /* Private */,
      21,    0,  172,    2, 0x08,   20 /* Private */,
      22,    0,  173,    2, 0x08,   21 /* Private */,
      23,    0,  174,    2, 0x08,   22 /* Private */,
      24,    1,  175,    2, 0x08,   23 /* Private */,
      27,    0,  178,    2, 0x08,   25 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::LongLong,   14,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 25,   26,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject Widget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSWidgetENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSWidgetENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSWidgetENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<Widget, std::true_type>,
        // method 'actOpenImage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'actOpenFolder'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'AddRoi'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'DelteteRoi'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onExitApp'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onAboutApp'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onHelpShortcuts'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onLanguageChinese'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onLanguageEnglish'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onThemeLight'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onThemeDark'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onProcessingFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        // method 'onApplyProcessing'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRoiGeometryChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnApply_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnCompare_pressed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnCompare_released'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnSave_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnClearChain_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnExportChain_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnImportChain_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_folderImageList_itemClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QListWidgetItem *, std::false_type>,
        // method 'onUndo'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void Widget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Widget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->actOpenImage(); break;
        case 1: _t->actOpenFolder(); break;
        case 2: _t->AddRoi(); break;
        case 3: _t->DelteteRoi(); break;
        case 4: _t->onExitApp(); break;
        case 5: _t->onAboutApp(); break;
        case 6: _t->onHelpShortcuts(); break;
        case 7: _t->onLanguageChinese(); break;
        case 8: _t->onLanguageEnglish(); break;
        case 9: _t->onThemeLight(); break;
        case 10: _t->onThemeDark(); break;
        case 11: _t->onProcessingFinished((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1]))); break;
        case 12: _t->onApplyProcessing(); break;
        case 13: _t->onRoiGeometryChanged(); break;
        case 14: _t->on_btnApply_clicked(); break;
        case 15: _t->on_btnCompare_pressed(); break;
        case 16: _t->on_btnCompare_released(); break;
        case 17: _t->on_btnSave_clicked(); break;
        case 18: _t->on_btnClearChain_clicked(); break;
        case 19: _t->on_btnExportChain_clicked(); break;
        case 20: _t->on_btnImportChain_clicked(); break;
        case 21: _t->on_folderImageList_itemClicked((*reinterpret_cast< std::add_pointer_t<QListWidgetItem*>>(_a[1]))); break;
        case 22: _t->onUndo(); break;
        default: ;
        }
    }
}

const QMetaObject *Widget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Widget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSWidgetENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Widget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 23)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 23;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 23)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 23;
    }
    return _id;
}
QT_WARNING_POP
