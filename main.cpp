#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>
#include <QInputDialog>
#include <QCheckBox>
#include <QDialog>
#include <QListWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStringList>
#include <QTextStream>

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
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Qt Zenity replacement");
    parser.addHelpOption();
    parser.addVersionOption();

    // Dialog type options
    QCommandLineOption infoOption(QStringList() << "info", "Show info message box.", "text");
    QCommandLineOption warningOption(QStringList() << "warning", "Show warning message box.", "text");
    QCommandLineOption errorOption(QStringList() << "error", "Show error message box.", "text");
    QCommandLineOption questionOption(QStringList() << "question", "Show question message box.", "text");

    QCommandLineOption entryOption(QStringList() << "entry", "Show text entry dialog.", "text");
    QCommandLineOption checkboxOption(QStringList() << "checkbox", "Show checkbox dialog.", "text");
    QCommandLineOption checkedOption(QStringList() << "checked", "Checkbox default checked.");

    QCommandLineOption listOption(QStringList() << "list", "Show list selection dialog.", "text");
    QCommandLineOption itemsOption(QStringList() << "items", "Comma separated list items", "items");

    parser.addOption(infoOption);
    parser.addOption(warningOption);
    parser.addOption(errorOption);
    parser.addOption(questionOption);

    parser.addOption(entryOption);
    parser.addOption(checkboxOption);
    parser.addOption(checkedOption);

    parser.addOption(listOption);
    parser.addOption(itemsOption);

    parser.process(app);

    QTextStream out(stdout);
    QTextStream err(stderr);

    if (parser.isSet(infoOption)) {
        QString text = parser.value(infoOption);
        QMessageBox::information(nullptr, "Information", text);
        out << "OK\n";
        return 0;
    }

    if (parser.isSet(warningOption)) {
        QString text = parser.value(warningOption);
        QMessageBox::warning(nullptr, "Warning", text);
        out << "OK\n";
        return 0;
    }

    if (parser.isSet(errorOption)) {
        QString text = parser.value(errorOption);
        QMessageBox::critical(nullptr, "Error", text);
        out << "OK\n";
        return 0;
    }

    if (parser.isSet(questionOption)) {
        QString text = parser.value(questionOption);
        QMessageBox::StandardButton reply = QMessageBox::question(nullptr, "Question", text, QMessageBox::Yes | QMessageBox::No);
        out << (reply == QMessageBox::Yes ? "Yes" : "No") << "\n";
        return (reply == QMessageBox::Yes ? 0 : 1);
    }

    if (parser.isSet(entryOption)) {
        QString text = parser.value(entryOption);
        bool ok;
        QString input = QInputDialog::getText(nullptr, "Input", text, QLineEdit::Normal, "", &ok);
        if (!ok) return 1;
        out << csvEscape(input) << "\n";
        return 0;
    }

    if (parser.isSet(checkboxOption)) {
        QString text = parser.value(checkboxOption);
        QDialog dialog;
        dialog.setWindowTitle("Checkbox");
        QVBoxLayout layout(&dialog);
        QCheckBox checkbox(text);
        if (parser.isSet(checkedOption)) checkbox.setChecked(true);
        layout.addWidget(&checkbox);

        QPushButton okButton("OK");
        QPushButton cancelButton("Cancel");
        layout.addWidget(&okButton);
        layout.addWidget(&cancelButton);

        QObject::connect(&okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
        QObject::connect(&cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

        int ret = dialog.exec();
        if (ret == QDialog::Accepted) {
            out << (checkbox.isChecked() ? "true" : "false") << "\n";
            return 0;
        }
        return 1;
    }

    if (parser.isSet(listOption)) {
        QString title = parser.value(listOption);
        if (!parser.isSet(itemsOption)) {
            err << "Error: --items required for --list\n";
            return 2;
        }
        QStringList items = parser.value(itemsOption).split(',');
        for (QString &item : items) item = item.trimmed();

        QDialog dialog;
        dialog.setWindowTitle(title);
        QVBoxLayout layout(&dialog);
        QListWidget listWidget;
        listWidget.addItems(items);
        listWidget.setSelectionMode(QAbstractItemView::SingleSelection);
        layout.addWidget(&listWidget);

        QPushButton okButton("OK");
        QPushButton cancelButton("Cancel");
        layout.addWidget(&okButton);
        layout.addWidget(&cancelButton);

        QObject::connect(&okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
        QObject::connect(&cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

        int ret = dialog.exec();
        if (ret == QDialog::Accepted && !listWidget.selectedItems().isEmpty()) {
            QString selected = listWidget.selectedItems().first()->text();
            out << csvEscape(selected) << "\n";
            return 0;
        }
        return 1;
    }

    err << "Error: No valid dialog type specified. Use --help.\n";
    return 2;
}
