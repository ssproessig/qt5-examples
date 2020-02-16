#include "TheStruct.h"

#include <QDataStream>


QDataStream& operator<<(QDataStream& ds, TheStruct const& s)
{
    ds << s.id;
    ds << s.multiByteValue;
    {
        ds << static_cast<quint16>(s.protocol);
    }
    ds << s.active;
    ds << s.subStructs;

    return ds;
}

QDataStream& operator>>(QDataStream& ds, TheStruct& s)
{
    ds >> s.id;
    ds >> s.multiByteValue;
    {
        quint16 protocol;
        ds >> protocol;
        s.protocol = static_cast<TheStruct::ProtocolEnum>(protocol);
    }
    ds >> s.active;
    ds >> s.subStructs;

    return ds;
}


QDataStream& operator<<(QDataStream& ds, TheStruct::SubStruct const& s)
{
    ds << s.tool;
    ds << s.created;
    ds << s.flag;

    return ds;
}

QDataStream& operator>>(QDataStream& ds, TheStruct::SubStruct& s)
{
    ds >> s.tool;
    ds >> s.created;
    ds >> s.flag;

    return ds;
}
