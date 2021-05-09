#include "WebEnginePdf.h"

#include "ui_WebEnginePdf.h"

#include <QDir>
#include <QPageLayout>
#include <QPdfDocument>
#include <QPdfPageNavigation>


struct WebEnginePdf::Data
{
    Ui::WebEnginePdf ui {};
    QPdfDocument* document = new QPdfDocument;
    QString const filename = QDir::temp().filePath("WebEnginePdf.pdf");

    ~Data()
    {
        delete document;
    }

    void updatePageLabel()
    {
        auto* const nav = ui.pdfView->pageNavigation();
        ui.lbPages->setText(
                QString("Page %1 / %2").arg(nav->currentPage() + 1).arg(nav->pageCount()));
    }
};


WebEnginePdf::WebEnginePdf() : d(std::make_unique<Data>())
{
    auto* const ui = &d->ui;

    ui->setupUi(this);
    resize(384, 443);

    connect(ui->btnToBrowser, &QPushButton::clicked, [=]() {
        ui->webView->setHtml(ui->edHtmlSource->toPlainText());

        ui->tabWidget->setCurrentWidget(ui->tabWebWidget);
    });

    connect(ui->btnSavePDF, &QPushButton::clicked, [=]() {
        if (auto* const page = ui->webView->page(); page)
        {
            QPageLayout const& layout = {
                    QPageSize(QPageSize::A5), QPageLayout::Portrait, QMarginsF()};
            page->printToPdf(d->filename, layout);
        }
    });

    connect(ui->btnLoadPDF, &QPushButton::clicked, [=]() {
        d->document->load(d->filename);
        ui->pdfView->setDocument(d->document);

        ui->pdfView->setPageMode(QPdfView::SinglePage);
        ui->pdfView->pageNavigation()->setCurrentPage(0);
        ui->pdfView->setZoomMode(QPdfView::FitInView);
        d->updatePageLabel();

        ui->tabWidget->setCurrentWidget(ui->tabPdfWidget);
    });

    connect(ui->btnNext, &QPushButton::clicked, [=]() {
        ui->pdfView->pageNavigation()->goToNextPage();
    });

    connect(ui->btnPrev, &QPushButton::clicked, [=]() {
        ui->pdfView->pageNavigation()->goToPreviousPage();
    });

    connect(ui->pdfView->pageNavigation(), &QPdfPageNavigation::currentPageChanged, [=]() {
        d->updatePageLabel();
    });
}
WebEnginePdf::~WebEnginePdf() = default;
