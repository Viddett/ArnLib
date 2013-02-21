// Copyright (C) 2010-2013 Michael Wiklund.
// All rights reserved.
// Contact: arnlib@wiklunden.se
//
// This file is part of the ArnLib - Active Registry Network.
// Parts of ArnLib depend on Qt 4 and/or other libraries that have their own
// licenses. ArnLib is independent of these licenses; however, use of these other
// libraries is subject to their respective license agreements.
//
// GNU Lesser General Public License Usage
// This file may be used under the terms of the GNU Lesser General Public
// License version 2.1 as published by the Free Software Foundation and
// appearing in the file LICENSE.LGPL included in the packaging of this file.
// In addition, as a special exception, you may use the rights described
// in the Nokia Qt LGPL Exception version 1.1, included in the file
// LGPL_EXCEPTION.txt in this package.
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public License version 3.0 as published by the Free Software Foundation
// and appearing in the file LICENSE.GPL included in the packaging of this file.
//
// Other Usage
// Alternatively, this file may be used in accordance with the terms and
// conditions contained in a signed written agreement between you and Michael Wiklund.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//

#include "ArnLink.hpp"
#include <QDebug>
// #include <QMutexLocker>
#include <limits>


QAtomicInt ArnLink::_idCount( 1);


void ArnLink::resetHave()
{
    _haveInt       = false;
    _haveDouble    = false;
    _haveString    = false;
    _haveByteArray = false;
    _haveVariant   = false;
}


//// This should in threaded: preserve order of setValue, optimize return of bytearray
void ArnLink::emitChanged( int sendId)
{
    // qDebug() << "emitChanged: isThr=" << _isThreaded << " isPipe=" << _isPipeMode <<
    //            " path=" << linkPath() << " value=" << toByteArray();
    if (_isThreaded && _isPipeMode)
        emit changed( sendId, toByteArray());
    else
        emit changed( sendId);
}


void ArnLink::setValue( int value, int sendId, bool forceKeep)
{
    if (_twin  &&  !forceKeep) {    // support for bidirectional function
        _twin->setValue( value, sendId, true);
        return;
    }

    if (_isThreaded)  _mutex.lock();
    resetHave();
    _valueInt = value;
    _type.e  = _type.Int;
    _haveInt  = true;
    if (_isThreaded)  _mutex.unlock();

    emitChanged( sendId);
}


void ArnLink::setValue( double value, int sendId, bool forceKeep)
{
    if (_twin  &&  !forceKeep) {    // support for bidirectional function
        _twin->setValue( value, sendId, true);
        return;
    }

    if (_isThreaded)  _mutex.lock();
    resetHave();
    _valueDouble = value;
    _type.e     = _type.Double;
    _haveDouble  = true;
    if (_isThreaded)  _mutex.unlock();

    emitChanged( sendId);
}


void ArnLink::setValue( const QString& value, int sendId, bool forceKeep)
{
    if (_twin  &&  !forceKeep) {    // support for bidirectional function
        _twin->setValue( value, sendId, true);
        return;
    }

    if (_isThreaded)  _mutex.lock();
    resetHave();
    _valueString.resize(0);     // Avoid heap defragmentation
    _valueString += value;
    _type.e      = _type.String;
    _haveString   = true;
    if (_isThreaded)  _mutex.unlock();

    emitChanged( sendId);
}


void ArnLink::setValue( const QByteArray& value, int sendId, bool forceKeep)
{
    if (_twin  &&  !forceKeep) {    // support for bidirectional function
        _twin->setValue( value, sendId, true);
        return;
    }

    if (_isThreaded)  _mutex.lock();
    resetHave();
    _valueByteArray.resize(0);     // Avoid heap defragmentation
    _valueByteArray += value;
    _type.e         = _type.ByteArray;
    _haveByteArray   = true;
    if (_isThreaded)  _mutex.unlock();

    emitChanged( sendId);
}


void ArnLink::setValue( const QVariant& value, int sendId, bool forceKeep)
{
    if (_twin  &&  !forceKeep) {    // support for bidirectional function
        _twin->setValue( value, sendId, true);
        return;
    }

    if (_isThreaded)  _mutex.lock();
    resetHave();
    _valueVariant = value;
    _type.e      = _type.Variant;
    _haveVariant  = true;
    if (_isThreaded)  _mutex.unlock();

    emitChanged( sendId);
}


void ArnLink::trfValue( QByteArray value, int sendId, bool forceKeep)
{
    setValue( value, sendId, forceKeep);
}


int ArnLink::toInt()
{
    if (_isThreaded)  _mutex.lock();

    if (!_haveInt) {
        switch (_type.e) {
        case Type::Double:
            _valueInt = (int)_valueDouble;
            break;
        case Type::String:
            _valueInt = _valueString.toInt();
            break;
        case Type::ByteArray:
            _valueInt = _valueByteArray.toInt();
            break;
        case Type::Variant:
            _valueInt = _valueVariant.toInt();
            break;
        default:
            _valueInt = 0;
        }
        _haveInt = true;
    }

    if (_isThreaded) {
        int retVal = _valueInt;
        _mutex.unlock();
        return retVal;
    }
    return _valueInt;
}


double ArnLink::toDouble()
{
    if (_isThreaded)  _mutex.lock();

    if (!_haveDouble) {
        switch (_type.e) {
        case Type::Int:
            _valueDouble = (double)_valueInt;
            break;
        case Type::String:
            _valueDouble = _valueString.toDouble();
            break;
        case Type::ByteArray:
            _valueDouble = _valueByteArray.toDouble();
            break;
        case Type::Variant:
            _valueDouble = _valueVariant.toDouble();
            break;
        default:
            _valueDouble = 0.0;
        }
        _haveDouble = true;
    }

    if (_isThreaded) {
        double retVal = _valueDouble;
        _mutex.unlock();
        return retVal;
    }
    return _valueDouble;
}


QString ArnLink::toString()
{
    if (_isThreaded)  _mutex.lock();

    if (!_haveString) {
        _valueString.resize(0);     // Avoid heap defragmentation
        switch (_type.e) {
        case Type::Int:
            _valueString += QString::number(_valueInt, 10);
            break;
        case Type::Double:
            _valueString += QString::number(_valueDouble, 'g', std::numeric_limits<double>::digits10);
            break;
        case Type::ByteArray:
            _valueString += QString::fromUtf8( _valueByteArray.constData(), _valueByteArray.size());
            break;
        case Type::Variant:
            _valueString += _valueVariant.toString();
            break;
        default:;
        }
        _haveString  = true;
    }

    if (_isThreaded) {
        QString retVal = _valueString;
        _mutex.unlock();
        return retVal;
    }
    return _valueString;
}


QByteArray ArnLink::toByteArray()
{
    if (_isThreaded)  _mutex.lock();

    if (!_haveByteArray) {
        _valueByteArray.resize(0);     // Avoid heap defragmentation
        switch (_type.e) {
        case Type::Int:
            _valueByteArray += QByteArray::number( _valueInt, 10);
            break;
        case Type::Double:
            _valueByteArray += QByteArray::number( _valueDouble, 'g', std::numeric_limits<double>::digits10);
            break;
        case Type::String:
            _valueByteArray += _valueString.toUtf8();
            break;
        case Type::Variant:
            _valueByteArray += _valueVariant.toString().toUtf8();
            break;
        default:;
        }
        _haveByteArray  = true;
    }

    if (_isThreaded) {
        QByteArray retVal = _valueByteArray;
        _mutex.unlock();
        return retVal;
    }
    return _valueByteArray;
}


QVariant ArnLink::toVariant( void)
{
    if (_isThreaded)  _mutex.lock();

    if (!_haveVariant) {
        switch (_type.e) {
        case Type::Int:
            _valueVariant = _valueInt;
            break;
        case Type::Double:
            _valueVariant = _valueDouble;
            break;
        case Type::String:
            _valueVariant = _valueString;
            break;
        case Type::ByteArray:
            _valueVariant = _valueByteArray;
            break;
        default:
            _valueVariant = QVariant();
        }
        _haveVariant = true;
    }

    if (_isThreaded) {
        QVariant retVal = _valueVariant;
        _mutex.unlock();
        return retVal;
    }
    return _valueVariant;
}


ArnLink::Type  ArnLink::type()
{
    if (_isThreaded)  _mutex.lock();
    Type  retVal = Type( _type.e);
    if (_isThreaded)  _mutex.unlock();

    return retVal;
}


QString ArnLink::linkPath( ArnLink::NameF nameF)
{
    nameF.set( nameF.NoFolderMark, false);  // Foldermark '/' must be ...
    QString  path;
    QString  linkName;
    ArnLink*  link = this;

    while (link) {  // Backwards until root
        ArnLink*  parentLink = qobject_cast<ArnLink*>( link->parent());
        if (!parentLink) {
            if (nameF.is( nameF.Relative))  break;  // Skip Root
            nameF.set( nameF.EmptyOk, true);  // Root is Empty node name
        }
        linkName = link->linkName( nameF);
        path.insert(0, linkName);
        link  = parentLink;
    }

    return path;
}


QString ArnLink::linkName( NameF nameF)
{
    QString  retVal = convertBaseName( objectName(), nameF);
    if (this->isFolder() && !nameF.is(( nameF.NoFolderMark)))
        retVal += '/';

    return retVal;
}


QString ArnLink::convertName( const QString& name, NameF nameF)
{
    bool  isFolderMarked = name.endsWith('/');
    QString  baseName = name.left( name.size() - int( isFolderMarked));  // remove any foldermark

    QString  retVal = convertBaseName( baseName, nameF);
    if (isFolderMarked && !nameF.is(( nameF.NoFolderMark)))
        retVal += '/';  // Restore previous foldermark

    return retVal;
}


QString ArnLink::convertBaseName( const QString& name, NameF nameF)
{
    QString  retVal("");

    if (name.isEmpty() && !nameF.is( nameF.EmptyOk))
        retVal = '@';
    else if ((name != "@") || !nameF.is( nameF.EmptyOk))
        retVal = name;

    return retVal;
}


ArnLink::ArnLink( ArnLink *parent, const QString& name, Flags flags)
        : QObject(0)
{
    QString  name_ = convertBaseName( name, NameF());

    QObject::setParent((QObject*) parent);
    QObject::setObjectName( name_);
    _isFolder       = flags.is( flags.Folder);
    _isProvider     = name_.endsWith('!');
    _isThreaded     = false;  // The correct value is set by Arn::link
    _valueInt       = 0;
    _valueDouble    = 0.0;
    _valueString    = "";
    _valueByteArray = "";
    _valueVariant   = QVariant();
    _type.e         = Type::Null;
    _twin           = 0;
    _isPipeMode     = false;
    _isSaveMode     = false;
    _hasBeenSetup   = false;
    _syncMode       = 0;
    _id             = _idCount.fetchAndAddRelaxed(1);
    _refCount       = -1;  // Mark no reference, Ok to delete
    _isRetired      = false;

    if (parent) {
        if (_isFolder) {
            connect( this, SIGNAL(linkCreatedBelow(ArnLink*)), parent, SIGNAL(linkCreatedBelow(ArnLink*)));
            connect( this, SIGNAL(modeChangedBelow(QString,uint)), parent, SIGNAL(modeChangedBelow(QString,uint)));
        }
        else {
            connect( this, SIGNAL(modeChanged(QString,uint)), parent, SIGNAL(modeChangedBelow(QString,uint)));
        }
    }
}


void  ArnLink::setupEnd( int syncMode)
{
    if (!_hasBeenSetup) {
        _hasBeenSetup = true;
        addSyncMode( syncMode);
        ArnLink*  parent = qobject_cast<ArnLink*>( this->parent());
        emit parent->linkCreatedBelow( this);
    }
}


ArnLink::~ArnLink()
{
    qDebug() << "Destructor link: path=" << linkPath();
    if (_twin) {
        _twin->_twin = 0;  // points to this object
        delete _twin;
        _twin = 0;
    }
}


ArnLink*  ArnLink::findLink( const QString& name)
{
    QString  name_ = convertBaseName( name, NameF());
    const QList<QObject*>  children = this->children();

    for (int i = 0; i < children.size(); i++) {
        QObject*  child = children.at(i);

        if (child->objectName() == name_) {
            return qobject_cast<ArnLink*>( child);
        }
    }

    return 0;
}


bool ArnLink::isFolder( void)
{
    return _isFolder;
}


void  ArnLink::addSyncMode( int syncMode)
{
    if (_isThreaded)  _mutex.lock();
    _syncMode |= syncMode;
    if (_isThreaded)  _mutex.unlock();
}


int   ArnLink::syncMode()
{
    if (_isThreaded)  _mutex.lock();
    int  retVal = _syncMode;
    if (_isThreaded)  _mutex.unlock();
    return retVal;
}


bool  ArnLink::isBiDirMode()
{
    return _twin != 0;   // Having a twin is bidirectional mode
}


void  ArnLink::setPipeMode( bool isPipeMode, bool alsoSetTwin)
{
    if (_isThreaded)  _mutex.lock();
    if (isPipeMode != _isPipeMode) {
        _isPipeMode = isPipeMode;
        if (_isThreaded)  _mutex.unlock();
        emit modeChanged( linkPath(), linkId());
        if (_isThreaded)  _mutex.lock();
    }
    if (_twin  &&  alsoSetTwin) {
        if (_isThreaded)  _mutex.unlock();
        _twin->setPipeMode( isPipeMode, false);
        if (_isThreaded)  _mutex.lock();
    }
    if (_isThreaded)  _mutex.unlock();
}


bool  ArnLink::isPipeMode()
{
    if (_isThreaded)  _mutex.lock();
    bool  retVal = _isPipeMode;
    if (_isThreaded)  _mutex.unlock();
    return retVal;
}


void  ArnLink::setSaveMode( bool isSaveMode)
{
    ArnLink*  link = valueLink();  // SaveMode is always stored in ValueLink
    if (this != link)  return link->setSaveMode( isSaveMode);

    if (_isThreaded)  _mutex.lock();
    if (isSaveMode != _isSaveMode) {
        _isSaveMode = isSaveMode;
        if (_isThreaded)  _mutex.unlock();
        emit modeChanged( linkPath(), linkId());
        if (_isThreaded)  _mutex.lock();
    }
    if (_isThreaded)  _mutex.unlock();
}


bool  ArnLink::isSaveMode()
{
    ArnLink*  link = valueLink();  // SaveMode is always stored in ValueLink
    if (this != link)  return link->isSaveMode();

    if (_isThreaded)  _mutex.lock();
    bool  retVal = _isSaveMode;
    if (_isThreaded)  _mutex.unlock();
    return retVal;
}


bool  ArnLink::isRetired()
{
    if (_isThreaded)  _mutex.lock();
    bool  retVal = _isRetired;
    if (_isThreaded)  _mutex.unlock();
    return retVal;
}


void  ArnLink::setRetired()
{
    if (_isThreaded)  _mutex.lock();
    qDebug() << "setRetired: path=" << this->linkPath();
    bool  wasRetired = _isRetired;
    _isRetired = true;
    if (_isThreaded)  _mutex.unlock();

    if (!wasRetired)  emit retired();
}


ArnLink*  ArnLink::twinLink()
{
    if (_isThreaded)  _mutex.lock();
    ArnLink*  retVal = _twin;
    if (_isThreaded)  _mutex.unlock();
    return retVal;
}


ArnLink*  ArnLink::valueLink()
{
    if (_isThreaded)  _mutex.lock();  // MW: not needed (?)
    ArnLink*  retVal = _isProvider ? _twin : this;
    if (_isThreaded)  _mutex.unlock();
    return retVal;
}


ArnLink*  ArnLink::providerLink()
{
    if (_isThreaded)  _mutex.lock();
    ArnLink*  retVal = _isProvider ? this : _twin;
    if (_isThreaded)  _mutex.unlock();
    return retVal;
}


ArnLink*  ArnLink::holderLink( bool forceKeep)
{
    if (_isThreaded)  _mutex.lock();
    ArnLink*  retVal = (_twin && !forceKeep) ? _twin : this;
    if (_isThreaded)  _mutex.unlock();
    return retVal;
}


QString  ArnLink::twinName()
{
    QString linkName = objectName();

    if (isProvider()) {  // Link is provider, twin is value
        return linkName.left( linkName.size() - 1);  // Remove '!' at end
    }
    else {  // Link is value, twin is provider
        return linkName + "!";
    }
}


/// Can only be called from main-thread
void  ArnLink::setRefCount( int count)
{
    valueLink()->_refCount = count;
}


/// Can only be called from main-thread
void  ArnLink::ref()
{
    if (valueLink()->_refCount <= 0)   // First reference, no other thread involved
        valueLink()->_refCount = 1;
    else
        valueLink()->_refCount.ref();  // Increase reference atomicly
    if (gDebugLinkRef)  qDebug() << "link-ref: path=" << this->linkPath() << " count=" << refCount();
}


/// Can be called from any thread any time
void  ArnLink::deref()
{
    bool  moreRefs = valueLink()->_refCount.deref();
    if (gDebugLinkRef)  qDebug() << "link-deref: path=" << this->linkPath() << " count=" << refCount();
    if (!moreRefs) {  // This is last reference
        emit zeroRef();  // Will be received in main-thread
    }
}


int  ArnLink::refCount()
{
    return valueLink()->_refCount;
}

