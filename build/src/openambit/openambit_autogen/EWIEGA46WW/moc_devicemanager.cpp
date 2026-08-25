/****************************************************************************
** Meta object code from reading C++ file 'devicemanager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.15)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../src/openambit/devicemanager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'devicemanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.15. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DeviceManager_t {
    QByteArrayData data[27];
    char stringdata0[301];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DeviceManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DeviceManager_t qt_meta_stringdata_DeviceManager = {
    {
QT_MOC_LITERAL(0, 0, 13), // "DeviceManager"
QT_MOC_LITERAL(1, 14, 14), // "deviceDetected"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 10), // "DeviceInfo"
QT_MOC_LITERAL(4, 41, 10), // "deviceInfo"
QT_MOC_LITERAL(5, 52, 13), // "deviceRemoved"
QT_MOC_LITERAL(6, 66, 12), // "deviceCharge"
QT_MOC_LITERAL(7, 79, 7), // "percent"
QT_MOC_LITERAL(8, 87, 12), // "syncFinished"
QT_MOC_LITERAL(9, 100, 7), // "success"
QT_MOC_LITERAL(10, 108, 18), // "syncProgressInform"
QT_MOC_LITERAL(11, 127, 7), // "message"
QT_MOC_LITERAL(12, 135, 5), // "error"
QT_MOC_LITERAL(13, 141, 6), // "newRow"
QT_MOC_LITERAL(14, 148, 11), // "percentDone"
QT_MOC_LITERAL(15, 160, 20), // "sportModeWriteResult"
QT_MOC_LITERAL(16, 181, 6), // "detect"
QT_MOC_LITERAL(17, 188, 9), // "startSync"
QT_MOC_LITERAL(18, 198, 11), // "readAllLogs"
QT_MOC_LITERAL(19, 210, 15), // "writeSportModes"
QT_MOC_LITERAL(20, 226, 18), // "QList<QVariantMap>"
QT_MOC_LITERAL(21, 245, 5), // "modes"
QT_MOC_LITERAL(22, 251, 14), // "chargeTimerHit"
QT_MOC_LITERAL(23, 266, 15), // "logMovescountID"
QT_MOC_LITERAL(24, 282, 6), // "device"
QT_MOC_LITERAL(25, 289, 4), // "time"
QT_MOC_LITERAL(26, 294, 6) // "moveID"

    },
    "DeviceManager\0deviceDetected\0\0DeviceInfo\0"
    "deviceInfo\0deviceRemoved\0deviceCharge\0"
    "percent\0syncFinished\0success\0"
    "syncProgressInform\0message\0error\0"
    "newRow\0percentDone\0sportModeWriteResult\0"
    "detect\0startSync\0readAllLogs\0"
    "writeSportModes\0QList<QVariantMap>\0"
    "modes\0chargeTimerHit\0logMovescountID\0"
    "device\0time\0moveID"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DeviceManager[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   69,    2, 0x06 /* Public */,
       5,    0,   72,    2, 0x06 /* Public */,
       6,    1,   73,    2, 0x06 /* Public */,
       8,    1,   76,    2, 0x06 /* Public */,
      10,    4,   79,    2, 0x06 /* Public */,
      15,    1,   88,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      16,    0,   91,    2, 0x0a /* Public */,
      17,    1,   92,    2, 0x0a /* Public */,
      19,    1,   95,    2, 0x0a /* Public */,
      22,    0,   98,    2, 0x08 /* Private */,
      23,    3,   99,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,
    QMetaType::Void, QMetaType::UChar,    7,
    QMetaType::Void, QMetaType::Bool,    9,
    QMetaType::Void, QMetaType::QString, QMetaType::Bool, QMetaType::Bool, QMetaType::UChar,   11,   12,   13,   14,
    QMetaType::Void, QMetaType::Bool,    9,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   18,
    QMetaType::Void, 0x80000000 | 20,   21,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::QDateTime, QMetaType::QString,   24,   25,   26,

       0        // eod
};

void DeviceManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DeviceManager *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->deviceDetected((*reinterpret_cast< const DeviceInfo(*)>(_a[1]))); break;
        case 1: _t->deviceRemoved(); break;
        case 2: _t->deviceCharge((*reinterpret_cast< quint8(*)>(_a[1]))); break;
        case 3: _t->syncFinished((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->syncProgressInform((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3])),(*reinterpret_cast< quint8(*)>(_a[4]))); break;
        case 5: _t->sportModeWriteResult((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->detect(); break;
        case 7: _t->startSync((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->writeSportModes((*reinterpret_cast< const QList<QVariantMap>(*)>(_a[1]))); break;
        case 9: _t->chargeTimerHit(); break;
        case 10: _t->logMovescountID((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QDateTime(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 8:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QList<QVariantMap> >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DeviceManager::*)(const DeviceInfo & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeviceManager::deviceDetected)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DeviceManager::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeviceManager::deviceRemoved)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DeviceManager::*)(quint8 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeviceManager::deviceCharge)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DeviceManager::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeviceManager::syncFinished)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (DeviceManager::*)(QString , bool , bool , quint8 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeviceManager::syncProgressInform)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (DeviceManager::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeviceManager::sportModeWriteResult)) {
                *result = 5;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DeviceManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_DeviceManager.data,
    qt_meta_data_DeviceManager,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DeviceManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DeviceManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DeviceManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DeviceManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void DeviceManager::deviceDetected(const DeviceInfo & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DeviceManager::deviceRemoved()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void DeviceManager::deviceCharge(quint8 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void DeviceManager::syncFinished(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void DeviceManager::syncProgressInform(QString _t1, bool _t2, bool _t3, quint8 _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void DeviceManager::sportModeWriteResult(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
