#include "StructSerializationTest.h"

#include "TheStruct.h"

#include <QBuffer>
#include <QDataStream>
#include <QDebug>


void StructSerializationTest::testThatSerializationRestoresAStructCorrectly()
{
    // ARRANGE - prepare a complex data structure
    TheStruct const in {
            "id",
            {TheStruct::SubStruct {"tool1", {{2020, 2, 16}, {12, 34, 56}, Qt::UTC}, true},
             TheStruct::SubStruct {"tool2", {{1999, 5, 13}, {23, 45, 6}, Qt::LocalTime}, false}},
            0xDEADBEEF,
            TheStruct::ProtocolEnum::Valid,
            true};

    // ACT - serialize / deserialize
    QByteArray serialized;
    {
        QBuffer buffer(&serialized);
        buffer.open(QIODevice::WriteOnly);
        QDataStream serializer(&buffer);

        // FIXME: serializer << in;
    }

    qDebug() << "serialized to " << hex << serialized;

    TheStruct out;
    {
        QBuffer buffer(&serialized);
        buffer.open(QIODevice::ReadOnly);
        QDataStream serializer(&buffer);

        // FIXME: serializer >> out;
    }

    // ASSERT - check deserialized values
    QCOMPARE(out.id, QString("id"));
    QCOMPARE(out.multiByteValue, quint32(0xDEADBEEF));
    QCOMPARE(out.protocol, TheStruct::ProtocolEnum::Valid);
    QCOMPARE(out.active, true);

    QCOMPARE(out.subStructs.count(), 2);
    {
        QCOMPARE(out.subStructs.at(0).tool, QString("tool1"));
        QCOMPARE(out.subStructs.at(0).created, QDateTime({2020, 2, 16}, {12, 34, 56}, Qt::UTC));
        QCOMPARE(out.subStructs.at(0).flag, true);
    }
    {
        QCOMPARE(out.subStructs.at(1).tool, QString("tool2"));
        QCOMPARE(out.subStructs.at(1).created,
                 QDateTime({1999, 5, 13}, {23, 45, 6}, Qt::LocalTime));
        QCOMPARE(out.subStructs.at(1).flag, false);
    }
}


QTEST_MAIN(StructSerializationTest)