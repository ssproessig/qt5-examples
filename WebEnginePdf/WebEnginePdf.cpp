#include "WebEnginePdf.h"

#include "ui_WebEnginePdf.h"

#include <QDir>
#include <QDirIterator>
#include <QLineEdit>
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

    // populate all embedded resources into the combobox
    QRegularExpression const re(":/embedded/[a-zA-Z-_/]+((\\.htm[l]?)|(\\.xml))");
    QStringList resources {""};
    QDirIterator it(":", QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        auto const& url = it.next();
        qDebug() << url;
        if (re.match(url).hasMatch())
        {
            resources.append("qrc://" + url.mid(1));
        }
    }
    ui->edUrl->addItems(resources);

    // open URL by selecting from ComboBox or entering an arbitrary URL
    auto const openUrl = [=]() {
        if (auto const url = ui->edUrl->currentText().trimmed(); !url.isEmpty())
        {
            ui->webView->setUrl(QUrl(url));

            ui->tabWidget->setCurrentWidget(ui->tabWebWidget);
        }
    };
    connect(ui->edUrl->lineEdit(), &QLineEdit::returnPressed, openUrl);
    connect(ui->edUrl, QOverload<int>::of(&QComboBox::currentIndexChanged), openUrl);

    // or just write your own HTML and send it directly to the browser
    connect(ui->btnToBrowser, &QPushButton::clicked, [=]() {
        ui->webView->setHtml(ui->edHtmlSource->toPlainText());
        ui->tabWidget->setCurrentWidget(ui->tabWebWidget);
    });


    // block saving to PDF while the HTML is still being loaded
    connect(ui->webView, &QWebEngineView::loadStarted, [=]() {
        ui->btnSavePDF->setEnabled(false);
    });
    connect(ui->webView, &QWebEngineView::loadFinished, [=]() {
        ui->btnSavePDF->setEnabled(true);
    });

    // saving to PDF
    connect(ui->btnSavePDF, &QPushButton::clicked, [=]() {
        if (auto* const page = ui->webView->page(); page)
        {
            QPageLayout const& layout = {
                    QPageSize(QPageSize::A5), QPageLayout::Portrait, QMarginsF()};
            page->printToPdf(d->filename, layout);
        }
    });


    // PDF loading and navigation
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
