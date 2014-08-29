// Copyright (C) 2010-2014 Michael Wiklund.
// All rights reserved.
// Contact: arnlib@wiklunden.se
//
// This file is part of the ArnLib - Active Registry Network.
// Parts of ArnLib depend on Qt and/or other libraries that have their own
// licenses. ArnLib is independent of these licenses; however, use of these
// other libraries is subject to their respective license agreements.
//
// GNU Lesser General Public License Usage
// This file may be used under the terms of the GNU Lesser General Public
// License version 2.1 as published by the Free Software Foundation and
// appearing in the file LICENSE_LGPL.txt included in the packaging of this
// file. In addition, as a special exception, you may use the rights described
// in the Nokia Qt LGPL Exception version 1.1, included in the file
// LGPL_EXCEPTION.txt in this package.
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General Public
// License version 3.0 as published by the Free Software Foundation and appearing
// in the file LICENSE_GPL.txt included in the packaging of this file.
//
// Other Usage
// Alternatively, this file may be used in accordance with the terms and conditions
// contained in a signed written agreement between you and Michael Wiklund.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
//

#include "ArnInc/ArnQml.hpp"
#include "ArnInc/ArnQmlMSystem.hpp"
#include "ArnInc/ArnM.hpp"
#include <QtQml>

using namespace Arn;


ArnQml::ArnQml()
    : QObject(0)
{
    _arnRootPath = "/";
    _arnNetworkAccessManagerFactory = new ArnNetworkAccessManagerFactory;
}


QString  ArnQml::arnRootPath()
{
    return instance()._arnRootPath;
}


void  ArnQml::setArnRootPath( const QString& path)
{
    instance()._arnRootPath = path;
}


QByteArray  ArnQml::arnCachedValue( const QString path)
{
    ArnQml&  in = ArnQml::instance();
    int  s = path.size();
    if (in._arnCache.contains( path))
        return in._arnCache.value( path);

    QByteArray  retVal;
    if (path.endsWith("/qmldir"))
        retVal = "";
    else
        retVal = ArnM::valueByteArray( path);

    in._arnCache.insert( path, retVal);
    return retVal;
}


void  ArnQml::setup( QQmlEngine* qmlEngine, ArnQml::UseFlags flags)
{
    ArnQml&  in = ArnQml::instance();

    if (flags.is( flags.ArnLib) && !in._regedUse.is( flags.ArnLib)) {
        in._regedUse.set( flags.ArnLib);
        qmlRegisterType<ArnItemQml>("ArnLib", 1, 0, "ArnItem");
    }
    if (flags.is( flags.MSystem) && !in._regedUse.is( flags.MSystem)) {
        in._regedUse.set( flags.MSystem);
        qmlRegisterType<QmlMFileIO>("MSystem", 1, 0, "MFileIO");
    }

    if (qmlEngine) {
        qmlEngine->setNetworkAccessManagerFactory( in._arnNetworkAccessManagerFactory);
    }
}


ArnQml&  ArnQml::instance()
{
    static ArnQml  ins;

    return ins;
}


///////// ArnItemQml

ArnItemQml::ArnItemQml( QObject* parent)
    : ArnItem( parent)
{
    _isCompleted = false;
}


QString  ArnItemQml::variantType()  const
{
    if (_valueType == QMetaType::Void)  return QString();

    const char*  typeName = QMetaType::typeName(_valueType);
    if (!typeName)  return QString();

    return typeName;
}


void  ArnItemQml::setVariantType( const QString& typeName)
{
    if (typeName.isEmpty()) {
        _valueType = QMetaType::Void;
    }
    else {
        int  type = QMetaType::type( typeName.toLatin1().constData());
/*
        if (!type) {
            context()->throwError( QScriptContext::TypeError,
                                   "Setting unknown defaultType=" + typeName);
            return;
        }
        else
*/
            _valueType = type;
    }

    emit variantTypeChanged();
}


QString  ArnItemQml::path()  const
{
    return _path;
}


void  ArnItemQml::setPath( const QString& path)
{
    _path = path;
    if (_isCompleted) {
        QString  arnPath = Arn::changeBasePath("/", ArnQml::arnRootPath(), path);
        open( arnPath);
    }

    emit pathChanged();
}


void  ArnItemQml::setVariant( const QVariant& value)
{
    if (_valueType == QMetaType::Void)  // No valueType, no conversion
        ArnItem::setValue( value);
    else {  // Use valueType
        QVariant  val = value;
        if (val.convert( QVariant::Type( _valueType))) {
            ArnItem::setValue( val);
        }
        else {
            // context()->throwError( QScriptContext::TypeError,
            //                       "Can't convert to defaultType=" + defaultType());
        }
    }
}


void  ArnItemQml::setPipeMode( bool isPipeMode)
{
    if (isPipeMode)
        ArnItem::setPipeMode();
}


void  ArnItemQml::setMaster( bool isMaster)
{
    if (isMaster)
        ArnItem::setMaster();
}


void  ArnItemQml::setAutoDestroy( bool isAutoDestroy)
{
    if (isAutoDestroy)
        ArnItem::setAutoDestroy();
}


void  ArnItemQml::setSaveMode( bool isSaveMode)
{
    if (isSaveMode)
        ArnItem::setSaveMode();
}


void  ArnItemQml::classBegin()
{
}


void  ArnItemQml::componentComplete()
{
    _isCompleted = true;
    if (!_path.isEmpty())
        setPath( _path);
}


void ArnItemQml::itemUpdated(const ArnLinkHandle& handleData, const QByteArray* value)
{
    ArnItem::itemUpdated( handleData, value);

    emit valueChanged();
}


///////// ArnNetworkReply

ArnNetworkReply::ArnNetworkReply( QObject* parent)
    : QNetworkReply( parent)
{
    _readPos = 0;
}


bool  ArnNetworkReply::isFinished()  const
{
    return true;
}


qint64  ArnNetworkReply::bytesAvailable()  const
{
    return QNetworkReply::bytesAvailable() + _data.size() - _readPos;
}


bool  ArnNetworkReply::isSequential()  const
{
    return true;
}


qint64 ArnNetworkReply::size() const
{
    return _data.size();
}


qint64  ArnNetworkReply::readData( char* data, qint64 maxlen)
{
    int  len = qMin( _data.size() - _readPos, int(maxlen));
    if (len > 0) {
        memcpy( data, _data.constData() + _readPos, len);
        _readPos += len;
    }

    if ((len == 0) && (bytesAvailable() == 0)) {
        return -1; // everything had been read
    }

    return len;
}


void  ArnNetworkReply::abort()
{
    close();
}


QByteArray  ArnNetworkReply::data()  const
{
    return _data;
}


void  ArnNetworkReply::setData( const QByteArray& data)
{
    _data = data;
    _readPos = 0;
    setUrl( request().url());
    open( ReadOnly);

    QMetaObject::invokeMethod (this, "downloadProgress", Qt::QueuedConnection,
                               Q_ARG( qint64, _data.size()),
                               Q_ARG( qint64, _data.size()));
    QMetaObject::invokeMethod( this, "readyRead", Qt::QueuedConnection);
    QMetaObject::invokeMethod( this, "finished", Qt::QueuedConnection);
}



///////// ArnNetworkAccessManager

ArnNetworkAccessManager::ArnNetworkAccessManager( QObject* parent)
    : QNetworkAccessManager( parent)
{
}


QNetworkReply*  ArnNetworkAccessManager::createRequest( QNetworkAccessManager::Operation op,
                                                        const QNetworkRequest& request,
                                                        QIODevice* outgoingData)
{
    QUrl  url = request.url();
    if (url.scheme() != "arn")
        return QNetworkAccessManager::createRequest( op, request, outgoingData);

    ArnNetworkReply*  reply = new ArnNetworkReply;
    reply->setRequest( request);
    reply->setOperation( op);

    switch (op) {
    case GetOperation:
    {
        QString  path = url.path();
        QString  arnPath = Arn::changeBasePath("/", ArnQml::arnRootPath(), path);
        arnPath = Arn::convertPath( arnPath, Arn::NameF::EmptyOk);
        reply->setData( ArnQml::arnCachedValue( arnPath));
        break;
    }
    default:
        qDebug() << "ArnNetworkAccessManager: Operation not supported op=" << op;
        break;
    }

    return reply;
}


///////// ArnNetworkAccessManagerFactory

QNetworkAccessManager*  ArnNetworkAccessManagerFactory::create( QObject* parent)
{
    return new ArnNetworkAccessManager( parent);
}
