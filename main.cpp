#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>
#include <QInputDialog>
#include <QCheckBox>
#include <QDialog>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QCalendarWidget>
#include <QLineEdit>
#include <QStringList>
#include <QTextStream>
#include <QDate>

QString csvEscape(const QString &text) {
    QString escaped = text;
    if (escaped.contains('"'))
        escaped.replace("\"", "\"\"");
    if (escaped.contains(',') || escaped.contains('"') || escaped.contains('\n') || escaped.contains('\r'))
        escaped = "\"" + escaped + "\"";
    return escaped;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("qtzenity");
    QCoreApplication::setApplicationVersion("1.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("Qt Zenity replacement with multi widgets");
    parser.addHelpOption();
    parser.addVersionOption();

    // Existing dialog options
    parser.addOption(QCommandLineOption("info", "Show info message box.", "text"));
    parser.addOption(QCommandLineOption("warning", "Show warning message box.", "text"));
    parser.addOption(QCommandLineOption("error", "Show error message box.", "text"));
    parser.addOption(QCommandLineOption("question", "Show question message box.", "text"));
    parser.addOption(QCommandLineOption("entry", "Show text entry dialog.", "text"));
    parser.addOption(QCommandLineOption("checkbox", "Show checkbox dialog.", "text"));
    parser.addOption(QCommandLineOption("checked", "Checkbox default checked."));
    parser.addOption(QCommandLineOption("list", "Show list selection dialog.", "text"));
    parser.addOption(QCommandLineOption("items", "Comma separated list items", "items"));

    // New combined widgets options
    parser.addOption(QCommandLineOption("multi-checkbox", "Comma separated checkbox labels (multiple checkboxes)", "labels"));
    parser.addOption(QCommandLineOption("calendar", "Show calendar widget with prompt label", "text"));
    parser.addOption(QCommandLineOption("debug", "Output debug info with widget names"));

    parser.process(app);

    QTextStream out(stdout);
    QTextStream err(stderr);

    // Handle simple dialogs first (like before)
    if (parser.isSet("info")) {
        QMessageBox::information(nullptr, "Information", parser.value("info"));
        out << "OK\n";
        return 0;
    }
    if (parser.isSet("warning")) {
        QMessageBox::warning(nullptr, "Warning", parser.value("warning"));
        out << "OK\n";
        return 0;
    }
    if (parser.isSet("error")) {
        QMessageBox::critical(nullptr, "Error", parser.value("error"));
        out << "OK\n";
        return 0;
    }
    if (parser.isSet("question")) {
        QMessageBox::StandardButton reply = QMessageBox::question(nullptr, "Question", parser.value("question"), QMessageBox::Yes | QMessageBox::No);
        out << (reply == QMessageBox::Yes ? "Yes" : "No") << "\n";
        return (reply == QMessageBox::Yes ? 0 : 1);
    }

    if (parser.isSet("checkbox")) {
        QDialog dialog;
        dialog.setWindowTitle("Checkbox");
        QVBoxLayout layout(&dialog);
        QCheckBox checkbox(parser.value("checkbox"));
        if (parser.isSet("checked")) checkbox.setChecked(true);
        layout.addWidget(&checkbox);
        QPushButton okButton("OK");
        QPushButton cancelButton("Cancel");
        QHBoxLayout btnLayout;
        btnLayout.addWidget(&okButton);
        btnLayout.addWidget(&cancelButton);
        layout.addLayout(&btnLayout);
        QObject::connect(&okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
        QObject::connect(&cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
        if (dialog.exec() == QDialog::Accepted) {
            out << (checkbox.isChecked() ? "true" : "false") << "\n";
            return 0;
        }
        return 1;
    }
    if (parser.isSet("list")) {
        if (!parser.isSet("items")) {
            err << "Error: --items required for --list\n";
            return 2;
        }
        QStringList items = parser.value("items").split(',' );
        for (QString &item : items) item = item.trimmed();
        QDialog dialog;
        dialog.setWindowTitle(parser.value("list"));
        QVBoxLayout layout(&dialog);
        QListWidget listWidget;
        listWidget.addItems(items);
        listWidget.setSelectionMode(QAbstractItemView::SingleSelection);
        layout.addWidget(&listWidget);
        QPushButton okButton("OK");
        QPushButton cancelButton("Cancel");
        QHBoxLayout btnLayout;
        btnLayout.addWidget(&okButton);
        btnLayout.addWidget(&cancelButton);
        layout.addLayout(&btnLayout);
        QObject::connect(&okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
        QObject::connect(&cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
        if (dialog.exec() == QDialog::Accepted && !listWidget.selectedItems().isEmpty()) {
            out << csvEscape(listWidget.selectedItems().first()->text()) << "\n";
            return 0;
        }
        return 1;
    }

    // Now handle combined dialog with multi widgets
    bool hasMultiCheckbox = parser.isSet("multi-checkbox");
    bool hasCalendar = parser.isSet("calendar");
    bool hasEntry = parser.isSet("entry");
    bool debug = parser.isSet("debug");

    if (hasMultiCheckbox || hasCalendar || hasEntry) {
        QDialog dialog;
        dialog.setWindowTitle("Input Dialog");
        QVBoxLayout mainLayout(&dialog);

        // Store pointers to widgets for output
        QList<QCheckBox*> checkboxes;
        QLineEdit *lineEdit = nullptr;
        QCalendarWidget *calendar = nullptr;

        // Add entry widget if requested
        if (hasEntry) {
            QLabel *entryLabel = new QLabel(parser.value("entry"));
            mainLayout.addWidget(entryLabel);
            lineEdit = new QLineEdit;
            mainLayout.addWidget(lineEdit);
        }

        // Add calendar widget if requested
        if (hasCalendar) {
            QLabel *calendarLabel = new QLabel(parser.value("calendar"));
            mainLayout.addWidget(calendarLabel);
            calendar = new QCalendarWidget;
            calendar->setGridVisible(true);
            mainLayout.addWidget(calendar);
        }

        // Add multi-checkbox widgets
        if (hasMultiCheckbox) {
            QStringList labels = parser.value("multi-checkbox").split(',');
            for (QString &label : labels) {
                label = label.trimmed();
                QCheckBox *cb = new QCheckBox(label);
                checkboxes.append(cb);
                mainLayout.addWidget(cb);
            }
        }

        // Buttons
        QHBoxLayout *btnLayout = new QHBoxLayout;
        QPushButton *okButton = new QPushButton("OK");
        QPushButton *cancelButton = new QPushButton("Cancel");
        btnLayout->addWidget(okButton);
        btnLayout->addWidget(cancelButton);
        mainLayout.addLayout(btnLayout);

        QObject::connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
        QObject::connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

        if (dialog.exec() != QDialog::Accepted) {
            return 1; // cancel
        }

        // Compose output string
        QStringList parts;

        if (hasCalendar && calendar) {
            QDate date = calendar->selectedDate();
            if (debug)
                parts << QString("calendar=%1").arg(csvEscape(date.toString(Qt::ISODate)));
            else
                parts << QString("calendar=%1").arg(date.toString(Qt::ISODate));
        }
        if (hasEntry && lineEdit) {
            QString val = lineEdit->text();
            if (debug)
                parts << QString("input=%1").arg(csvEscape(val));
            else
                parts << QString("input=%1").arg(val);
        }
        if (hasMultiCheckbox) {
            for (QCheckBox *cb : checkboxes) {
                QString key = cb->text();
                bool checked = cb->isChecked();
                if (debug)
                    parts << QString("%1=%2").arg(csvEscape(key)).arg(checked ? "true" : "false");
                else {
                    // For no debug, key with spaces replaced by underscores, all lowercase (optional)
                    QString simpleKey = key.toLower().replace(" ", "_");
                    parts << QString("%1=%2").arg(simpleKey).arg(checked ? "true" : "false");
                }
            }
        }

        out << parts.join(",") << "\n";
        return 0;
    }

    err << "Error: No valid dialog type specified. Use --help.\n";
    return 2;
}
