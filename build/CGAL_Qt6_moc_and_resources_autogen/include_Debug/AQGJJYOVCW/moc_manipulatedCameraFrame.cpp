/****************************************************************************
** Meta object code from reading C++ file 'manipulatedCameraFrame.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../../../tools/vcpkg/installed/x64-windows/include/CGAL/Qt/manipulatedCameraFrame.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'manipulatedCameraFrame.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSCGALSCOPEqglviewerSCOPEManipulatedCameraFrameENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSCGALSCOPEqglviewerSCOPEManipulatedCameraFrameENDCLASS = QtMocHelpers::stringData(
    "CGAL::qglviewer::ManipulatedCameraFrame",
    "setFlySpeed",
    "",
    "speed",
    "setSceneUpVector",
    "Vec",
    "up",
    "spin",
    "flyUpdate"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSCGALSCOPEqglviewerSCOPEManipulatedCameraFrameENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   38,    2, 0x0a,    1 /* Public */,
       4,    1,   41,    2, 0x0a,    3 /* Public */,
       7,    0,   44,    2, 0x09,    5 /* Protected */,
       8,    0,   45,    2, 0x08,    6 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::QReal,    3,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject CGAL::qglviewer::ManipulatedCameraFrame::staticMetaObject = { {
    QMetaObject::SuperData::link<ManipulatedFrame::staticMetaObject>(),
    qt_meta_stringdata_CLASSCGALSCOPEqglviewerSCOPEManipulatedCameraFrameENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSCGALSCOPEqglviewerSCOPEManipulatedCameraFrameENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSCGALSCOPEqglviewerSCOPEManipulatedCameraFrameENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ManipulatedCameraFrame, std::true_type>,
        // method 'setFlySpeed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'setSceneUpVector'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const Vec &, std::false_type>,
        // method 'spin'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'flyUpdate'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void CGAL::qglviewer::ManipulatedCameraFrame::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ManipulatedCameraFrame *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->setFlySpeed((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1]))); break;
        case 1: _t->setSceneUpVector((*reinterpret_cast< std::add_pointer_t<Vec>>(_a[1]))); break;
        case 2: _t->spin(); break;
        case 3: _t->flyUpdate(); break;
        default: ;
        }
    }
}

const QMetaObject *CGAL::qglviewer::ManipulatedCameraFrame::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CGAL::qglviewer::ManipulatedCameraFrame::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSCGALSCOPEqglviewerSCOPEManipulatedCameraFrameENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return ManipulatedFrame::qt_metacast(_clname);
}

int CGAL::qglviewer::ManipulatedCameraFrame::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ManipulatedFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}
QT_WARNING_POP
