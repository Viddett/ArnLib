// Copyright (C) 2010-2015 Michael Wiklund.
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

#ifndef ARNEVENT_HPP
#define ARNEVENT_HPP

#include "ArnInc/Arn.hpp"
#include "ArnInc/MQFlags.hpp"
#include <QEvent>
#include <QString>

class ArnLink;
class ArnLinkHandle;


namespace ArnPrivate {
class Idx {
    Q_GADGET
    Q_ENUMS(E)
public:
    enum E {
        QtEvent     = -1,
        ValueChange = 0,
        LinkCreate,
        ModeChange,
        Retired,
        ZeroRef,
        //! Max index
        N
    };
    MQ_DECLARE_ENUMTXT( Idx)
};
}


class ARNLIBSHARED_EXPORT ArnEvent : public QEvent
{
    void*  _target;
    void*  _spare;  // Can be used later as d-ptr

public:
    typedef ArnPrivate::Idx  Idx;

    ArnEvent( QEvent::Type type);

    static int  baseType( int setVal = -1);
    static bool  isArnEvent( int evType);
    static int  toIdx( QEvent::Type type);
    static QString  toString( QEvent::Type type);

    int  toIdx()  const;
    QString  toString()  const;

    virtual ArnEvent*  makeHeapClone() = 0;

    inline void*  target()  const
    { return _target;}

    inline void  setTarget( void* target)
    { _target = target;}

protected:
    ArnEvent*  copyOpt( const ArnEvent* other);
};


class ArnEvLinkCreate : public ArnEvent
{
    QString  _path;
    ArnLink*  _arnLink;
    bool  _isLastLink;

public:
    ArnEvLinkCreate( const QString& path, ArnLink* arnLink, bool isLastLink);
    static QEvent::Type  type();
    virtual ArnEvent*  makeHeapClone();

    inline const QString&  path()  const
    { return _path;}

    inline ArnLink*  arnLink()  const
    { return _arnLink;}

    inline bool  isLastLink()  const
    { return _isLastLink;}
};


class ArnEvModeChange : public ArnEvent
{
    QString  _path;
    uint  _linkId;
    Arn::ObjectMode  _mode;

public:
    ArnEvModeChange( const QString& path, uint linkId, Arn::ObjectMode mode);
    static QEvent::Type  type();
    virtual ArnEvent*  makeHeapClone();

    inline const QString&  path()  const
    { return _path;}

    inline uint  linkId()  const
    { return _linkId;}

    inline Arn::ObjectMode  mode()  const
    { return _mode;}
};


class ArnEvRetired : public ArnEvent
{
    ArnLink*  _startLink;
    bool  _isBelow;
    bool  _isGlobal;

public:
    ArnEvRetired( ArnLink* startLink, bool isBelow, bool isGlobal);
    static QEvent::Type  type();
    virtual ArnEvent*  makeHeapClone();

    inline ArnLink*  startLink()  const
    { return _startLink;}

    inline bool  isBelow()  const
    { return _isBelow;}

    inline bool  isGlobal()  const
    { return _isGlobal;}
};


class ArnEvZeroRef : public ArnEvent
{
    ArnLink*  _arnLink;

public:
    ArnEvZeroRef( ArnLink* arnLink);
    static QEvent::Type  type();
    virtual ArnEvent*  makeHeapClone();

    inline ArnLink*  arnLink()  const
    { return _arnLink;}
};


class ArnEvValueChange : public ArnEvent
{
    int  _sendId;
    const QByteArray*  _valueData;
    const ArnLinkHandle*  _handleData;

public:
    ArnEvValueChange( int sendId, const QByteArray* valueData, const ArnLinkHandle& handleData);
    virtual  ~ArnEvValueChange();
    static QEvent::Type  type();
    virtual ArnEvent*  makeHeapClone();

    inline int  sendId()  const
    { return _sendId;}

    inline const QByteArray*  valueData()  const
    { return _valueData;}

    inline const ArnLinkHandle&  handleData()  const
    { return *_handleData;}
};

#endif // ARNEVENT_HPP
