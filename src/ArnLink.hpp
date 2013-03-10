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

#ifndef ARNLINK_HPP
#define ARNLINK_HPP

#include "ArnLib.hpp"
#include "ArnLib_global.hpp"
#include "MQFlags.hpp"
#include <QObject>
#include <QString>
#include <QVariant>
#include <QAtomicInt>
#include <QMutex>


class ArnLink : public QObject
{
    Q_OBJECT

    friend class ArnM;

public:
    struct Type {
        enum E {
            Null      = 0,
            Int       = 1,
            Double    = 2,
            ByteArray = 3,
            String    = 4,
            Variant   = 5
        };
        MQ_DECLARE_ENUM( Type)
    };
    struct Flags {
        enum E {
            Folder        = 0x01,
            CreateAllowed = 0x02,
            SilentError   = 0x04,
            Threaded      = 0x08
        };
        MQ_DECLARE_FLAGS( Flags)
    };
    struct NameF {
        //! Selects a format for path or item name
        enum E {
            //! Only on discrete names, no effect on path. "test/" ==> "test"
            NoFolderMark = 0x01,
            //! Path: "/@/test" ==> "//test", Item: "@" ==> ""
            EmptyOk      = 0x02,
            //! Only on path, no effect on discrete names. "/test/value" ==> "test/value"
            Relative     = 0x04
        };
        MQ_DECLARE_FLAGS( NameF)
    };

    void  setValue( int value, int sendId = 0, bool forceKeep = 0);
    void  setValue( double value, int sendId = 0, bool forceKeep = 0);
    void  setValue( const QString& value, int sendId = 0, bool forceKeep = 0);
    void  setValue( const QByteArray& value, int sendId = 0, bool forceKeep = 0);
    void  setValue( const QVariant& value, int sendId = 0, bool forceKeep = 0);

    int  toInt();
    double  toDouble();
    QString  toString();
    QByteArray  toByteArray();
    QVariant  toVariant();

    Type  type();

    QString  linkPath( ArnLink::NameF nameF = ArnLink::NameF::EmptyOk);
    QString  linkName( NameF nameF = NameF());
    uint  linkId()  const { return _id;}
    bool  isFolder();

    void  addSyncMode( int syncMode);
    int   syncMode();
    bool  isBiDirMode();
    void  setPipeMode( bool isPipeMode, bool alsoSetTwin = true);
    bool  isPipeMode();
    void  setSaveMode( bool isSaveMode);
    bool  isSaveMode();
    bool  isProvider()  const { return _isProvider;}
    bool  isThreaded()  const { return _isThreaded;}
    bool  isRetired();
    ArnLink*  twinLink();
    ArnLink*  valueLink();
    ArnLink*  providerLink();
    ArnLink*  holderLink( bool forceKeep);
    QString  twinName();
    void  deref();
    ~ArnLink();

    static QString  convertName( const QString& name, NameF nameF = NameF());
    static QString  convertBaseName( const QString& name, NameF nameF);

public slots:
    void  trfValue( QByteArray value, int sendId, bool forceKeep);

signals:
    void  changed( uint sendId);
    void  changed( uint sendId, QByteArray value);
    void  modeChanged( QString path, uint linkId);
    void  modeChangedBelow( QString path, uint linkId);
    void  linkCreatedBelow( ArnLink* link);
    void  zeroRef();
    void  retired();

protected:
    ArnLink( ArnLink *parent, const QString& name, Flags flags);
    void  setupEnd( int syncMode);
    ArnLink*  findLink(const QString& name);
    void  ref();
    int  refCount();
    void  setRefCount( int count);
    void  setRetired();

private:
    void  resetHave();
    void  emitChanged( int sendId);

    /// Source for unique id to all ArnLink ..
    static QAtomicInt  _idCount;

    uint  _id;
    bool  _isFolder;
    bool  _isProvider;
    bool  _isThreaded;
    QMutex  _mutex;
    QAtomicInt  _refCount;
    volatile bool  _isRetired;

    volatile int  _valueInt;
    volatile double  _valueDouble;
    QString  _valueString;
    QByteArray  _valueByteArray;
    QVariant  _valueVariant;

    volatile bool  _haveInt;
    volatile bool  _haveDouble;
    volatile bool  _haveString;
    volatile bool  _haveByteArray;
    volatile bool  _haveVariant;

    volatile Type  _type;

    ArnLink*  _twin;   // Used for bidirectional functionality
    volatile int   _syncMode;
    volatile bool  _isPipeMode;
    volatile bool  _isSaveMode;
    bool  _hasBeenSetup;
};

MQ_DECLARE_OPERATORS_FOR_FLAGS( ArnLink::Flags)
MQ_DECLARE_OPERATORS_FOR_FLAGS( ArnLink::NameF)


#endif // ARNLINK_HPP
