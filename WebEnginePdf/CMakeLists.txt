add_executable(WebEnginePdf WIN32
    WebEnginePdf.h
    WebEnginePdf.cpp
    WebEnginePdf.qrc
    WebEnginePdf.ui
    main.cpp
)
target_compile_options(WebEnginePdf PRIVATE ${COMPILE_OPTIONS})
target_link_libraries(
    WebEnginePdf
    Qt::Core
    Qt::Gui Qt::Widgets
    Qt::WebEngine Qt::WebEngineWidgets
    Qt::Pdf Qt::PdfWidgets
)
