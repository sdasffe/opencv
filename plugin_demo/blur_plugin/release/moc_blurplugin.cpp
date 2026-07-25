/****************************************************************************
** Meta object code from reading C++ file 'blurplugin.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../blurplugin.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qplugin.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'blurplugin.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSBlurPluginENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSBlurPluginENDCLASS = QtMocHelpers::stringData(
    "BlurPlugin"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSBlurPluginENDCLASS[] = {

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

Q_CONSTINIT const QMetaObject BlurPlugin::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSBlurPluginENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSBlurPluginENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSBlurPluginENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<BlurPlugin, std::true_type>
    >,
    nullptr
} };

void BlurPlugin::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *BlurPlugin::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BlurPlugin::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSBlurPluginENDCLASS.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "IBlockPlugin"))
        return static_cast< IBlockPlugin*>(this);
    if (!strcmp(_clname, "com.plugin_demo.IBlockPlugin/1.0"))
        return static_cast< IBlockPlugin*>(this);
    return QObject::qt_metacast(_clname);
}

int BlurPlugin::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    return _id;
}

#ifdef QT_MOC_EXPORT_PLUGIN_V2
static constexpr unsigned char qt_pluginMetaDataV2_BlurPlugin[] = {
    0xbf, 
    // "IID"
    0x02,  0x78,  0x20,  'c',  'o',  'm',  '.',  'p', 
    'l',  'u',  'g',  'i',  'n',  '_',  'd',  'e', 
    'm',  'o',  '.',  'I',  'B',  'l',  'o',  'c', 
    'k',  'P',  'l',  'u',  'g',  'i',  'n',  '/', 
    '1',  '.',  '0', 
    // "className"
    0x03,  0x6a,  'B',  'l',  'u',  'r',  'P',  'l', 
    'u',  'g',  'i',  'n', 
    0xff, 
};
QT_MOC_EXPORT_PLUGIN_V2(BlurPlugin, BlurPlugin, qt_pluginMetaDataV2_BlurPlugin)
#else
QT_PLUGIN_METADATA_SECTION
Q_CONSTINIT static constexpr unsigned char qt_pluginMetaData_BlurPlugin[] = {
    'Q', 'T', 'M', 'E', 'T', 'A', 'D', 'A', 'T', 'A', ' ', '!',
    // metadata version, Qt version, architectural requirements
    0, QT_VERSION_MAJOR, QT_VERSION_MINOR, qPluginArchRequirements(),
    0xbf, 
    // "IID"
    0x02,  0x78,  0x20,  'c',  'o',  'm',  '.',  'p', 
    'l',  'u',  'g',  'i',  'n',  '_',  'd',  'e', 
    'm',  'o',  '.',  'I',  'B',  'l',  'o',  'c', 
    'k',  'P',  'l',  'u',  'g',  'i',  'n',  '/', 
    '1',  '.',  '0', 
    // "className"
    0x03,  0x6a,  'B',  'l',  'u',  'r',  'P',  'l', 
    'u',  'g',  'i',  'n', 
    0xff, 
};
QT_MOC_EXPORT_PLUGIN(BlurPlugin, BlurPlugin)
#endif  // QT_MOC_EXPORT_PLUGIN_V2

QT_WARNING_POP
