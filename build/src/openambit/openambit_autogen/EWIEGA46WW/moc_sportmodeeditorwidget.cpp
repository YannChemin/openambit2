/****************************************************************************
** Meta object code from reading C++ file 'sportmodeeditorwidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.15)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../src/openambit/sportmodeeditorwidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sportmodeeditorwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.15. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SportModeEditorWidget_t {
    QByteArrayData data[13];
    char stringdata0[170];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SportModeEditorWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SportModeEditorWidget_t qt_meta_stringdata_SportModeEditorWidget = {
    {
QT_MOC_LITERAL(0, 0, 21), // "SportModeEditorWidget"
QT_MOC_LITERAL(1, 22, 21), // "writeToWatchRequested"
QT_MOC_LITERAL(2, 44, 0), // ""
QT_MOC_LITERAL(3, 45, 18), // "QList<QVariantMap>"
QT_MOC_LITERAL(4, 64, 5), // "modes"
QT_MOC_LITERAL(5, 70, 14), // "onModeSelected"
QT_MOC_LITERAL(6, 85, 3), // "row"
QT_MOC_LITERAL(7, 89, 9), // "onAddMode"
QT_MOC_LITERAL(8, 99, 12), // "onRemoveMode"
QT_MOC_LITERAL(9, 112, 14), // "onWriteToWatch"
QT_MOC_LITERAL(10, 127, 14), // "onLoadDefaults"
QT_MOC_LITERAL(11, 142, 12), // "onSaveToFile"
QT_MOC_LITERAL(12, 155, 14) // "onLoadFromFile"

    },
    "SportModeEditorWidget\0writeToWatchRequested\0"
    "\0QList<QVariantMap>\0modes\0onModeSelected\0"
    "row\0onAddMode\0onRemoveMode\0onWriteToWatch\0"
    "onLoadDefaults\0onSaveToFile\0onLoadFromFile"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SportModeEditorWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   54,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,   57,    2, 0x08 /* Private */,
       7,    0,   60,    2, 0x08 /* Private */,
       8,    0,   61,    2, 0x08 /* Private */,
       9,    0,   62,    2, 0x08 /* Private */,
      10,    0,   63,    2, 0x08 /* Private */,
      11,    0,   64,    2, 0x08 /* Private */,
      12,    0,   65,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void SportModeEditorWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SportModeEditorWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->writeToWatchRequested((*reinterpret_cast< const QList<QVariantMap>(*)>(_a[1]))); break;
        case 1: _t->onModeSelected((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->onAddMode(); break;
        case 3: _t->onRemoveMode(); break;
        case 4: _t->onWriteToWatch(); break;
        case 5: _t->onLoadDefaults(); break;
        case 6: _t->onSaveToFile(); break;
        case 7: _t->onLoadFromFile(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
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
            using _t = void (SportModeEditorWidget::*)(const QList<QVariantMap> & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SportModeEditorWidget::writeToWatchRequested)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SportModeEditorWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_SportModeEditorWidget.data,
    qt_meta_data_SportModeEditorWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *SportModeEditorWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SportModeEditorWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SportModeEditorWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int SportModeEditorWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void SportModeEditorWidget::writeToWatchRequested(const QList<QVariantMap> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
