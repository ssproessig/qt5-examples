#pragma once

#include <QObject>

struct PasswordDigestorTests final : QObject
{
    Q_OBJECT
private slots:

    static void test_PBKDF2_samples();
};
