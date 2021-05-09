#pragma once

#include <QMainWindow>

#include <memory>


class WebEnginePdf final : public QMainWindow
{
    Q_OBJECT

public:
    WebEnginePdf();
    ~WebEnginePdf() override;

private:
    struct Data;
    std::unique_ptr<Data> d;
};
