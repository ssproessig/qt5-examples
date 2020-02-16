#pragma once

#include <QTest>


class StructSerializationTest final : public QObject
{
    Q_OBJECT

private slots:
    static void testThatSerializationRestoresAStructCorrectly();
};
