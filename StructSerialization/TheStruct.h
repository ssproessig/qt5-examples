#pragma once

#include <QDateTime>
#include <QList>
#include <QString>


struct TheStruct
{
    struct SubStruct
    {
        QString tool;
        QDateTime created;

        bool flag = false;
    };

    enum class ProtocolEnum : quint16 { Valid = 0x0000, ValidOld = 0x0001, Invalid = 0xFFFF };

    QString id;

    QList<SubStruct> subStructs;

    quint32 multiByteValue = 0;

    ProtocolEnum protocol = ProtocolEnum::Invalid;

    bool active = false;
};


QDataStream& operator<<(QDataStream&, TheStruct const&);
QDataStream& operator>>(QDataStream&, TheStruct&);

QDataStream& operator<<(QDataStream&, TheStruct::SubStruct const&);
QDataStream& operator>>(QDataStream&, TheStruct::SubStruct&);
