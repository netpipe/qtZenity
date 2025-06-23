// qtzenity_extended.cpp
// Compile with: g++ qtzenity_extended.cpp -o qtzenity `pkg-config --cflags --libs Qt5Widgets`

#include <QApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QCalendarWidget>
#include <QSlider>
#include <QDial>
#include <QRadioButton>
#include <QButtonGroup>
#include <QDateTimeEdit>
#include <QMovie>
#include <QPixmap>
#include <QImageReader>
#include <QCommandLineParser>
#include <QLCDNumber>
#include <QTimer>
#include <QFileInfo>
#include <iostream>

using namespace std;
QMap<QString, QButtonGroup*> radioGroups;


QString csvEscape(const QString &str) {
    QString result = str;
    if (result.contains(',') || result.contains('"')) {
        result.replace("\"", "\"\"");
        result = "\"" + result + "\"";
    }
    return result;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("QtZenity Extended");
    parser.addHelpOption();
    parser.addOptions({
        {"entry", "Input box", "label"},
        {"multi-checkbox", "Comma-separated checkboxes", "list"},
        {"calendar", "Calendar with label", "label"},
        {"slider", "Slider: label,min,max,default", "spec"},
        {"dial", "Dial: label,min,max,default", "spec"},
        {"datetime", "DateTimeEdit with label", "label"},
        {"radio", "Radio: label,opt1,opt2,...", "spec"},
        {"image", "Image: path[,w,h]", "spec"},
        {"movie", "Movie/GIF: path[,w,h]", "spec"},
        {"clock", "Show clock"},
        {"window-size", "Window size: w,h", "spec"},
        {"debug", "Debug output with widget names"}
    });

    parser.process(app);
    bool debug = parser.isSet("debug");

    QDialog dialog;
    dialog.setWindowTitle("QtZenity Extended");
    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    if (parser.isSet("window-size")) {
        auto parts = parser.value("window-size").split(',');
        if (parts.size() == 2)
            dialog.resize(parts[0].toInt(), parts[1].toInt());
      //  dialog.setFixedSize(parts[0].toInt(), parts[1].toInt());

        dialog.setMinimumSize(50, 50); // Example: basic layout protection
     //   dialog.resize(width, height);    // Starting size
    }

    QMap<QString, QWidget*> widgetMap;

    if (parser.isSet("image")) {
        auto specs = parser.values("image");
        for (auto spec : specs) {
            auto parts = spec.split(',');
            if (parts.size() >= 1) {
                QLabel *imgLabel = new QLabel;
                QPixmap pix(parts[0]);
                if (parts.size() >= 3)
                    pix = pix.scaled(parts[1].toInt(), parts[2].toInt(), Qt::KeepAspectRatio);
                imgLabel->setPixmap(pix);
                layout->addWidget(imgLabel);
            }
        }
    }

    if (parser.isSet("movie")) {
        auto specs = parser.values("movie");
        for (auto spec : specs) {
            auto parts = spec.split(',');
            if (parts.size() >= 1) {
                QLabel *movLabel = new QLabel;
                QMovie *movie = new QMovie(parts[0]);
                movLabel->setMovie(movie);
                if (parts.size() >= 3)
                    movLabel->setFixedSize(parts[1].toInt(), parts[2].toInt());
                movie->start();
                layout->addWidget(movLabel);
            }
        }
    }

    QLineEdit *entryWidget = nullptr;
    if (parser.isSet("entry")) {
        layout->addWidget(new QLabel(parser.value("entry")));
        entryWidget = new QLineEdit;
        layout->addWidget(entryWidget);
    }

    QList<QCheckBox*> checkboxes;
    if (parser.isSet("multi-checkbox")) {
        for (QString label : parser.value("multi-checkbox").split(',')) {
            QCheckBox *cb = new QCheckBox(label.trimmed());
            checkboxes << cb;
            layout->addWidget(cb);
        }
    }

    QCalendarWidget *calendar = nullptr;
    if (parser.isSet("calendar")) {
        layout->addWidget(new QLabel(parser.value("calendar")));
        calendar = new QCalendarWidget;
        layout->addWidget(calendar);
    }

    if (parser.isSet("slider")) {
        auto parts = parser.value("slider").split(',');
        if (parts.size() == 4) {
            layout->addWidget(new QLabel(parts[0]));
            QSlider *slider = new QSlider(Qt::Horizontal);
            slider->setMinimum(parts[1].toInt());
            slider->setMaximum(parts[2].toInt());
            slider->setValue(parts[3].toInt());
            layout->addWidget(slider);
            widgetMap["slider"] = slider;
        }
    }

    if (parser.isSet("dial")) {
        auto parts = parser.value("dial").split(',');
        if (parts.size() == 4) {
            layout->addWidget(new QLabel(parts[0]));
            QDial *dial = new QDial;
            dial->setMinimum(parts[1].toInt());
            dial->setMaximum(parts[2].toInt());
            dial->setValue(parts[3].toInt());
            layout->addWidget(dial);
            widgetMap["dial"] = dial;
        }
    }

    if (parser.isSet("datetime")) {
        layout->addWidget(new QLabel(parser.value("datetime")));
        QDateTimeEdit *dtEdit = new QDateTimeEdit(QDateTime::currentDateTime());
        layout->addWidget(dtEdit);
        widgetMap["datetime"] = dtEdit;
    }

    if (parser.isSet("radio")) {
        auto parts = parser.value("radio").split(',');
        if (parts.size() >= 2) {
            layout->addWidget(new QLabel(parts[0]));
            QButtonGroup *group = new QButtonGroup(&dialog);
            for (int i = 1; i < parts.size(); ++i) {
                QRadioButton *rb = new QRadioButton(parts[i]);
                group->addButton(rb);
                layout->addWidget(rb);
                if (i == 1) rb->setChecked(true);
            }
            radioGroups.insert(parts[0], group);  // Store group by label or name
        }
    }


    QLCDNumber *clockDisplay = nullptr;
    if (parser.isSet("clock")) {
        clockDisplay = new QLCDNumber;
        layout->addWidget(clockDisplay);
        QTimer *timer = new QTimer(&dialog);
        QObject::connect(timer, &QTimer::timeout, [&]() {
            clockDisplay->display(QTime::currentTime().toString("hh:mm:ss"));
        });
        timer->start(1000);
    }

    QHBoxLayout *btnLayout = new QHBoxLayout;
    QPushButton *ok = new QPushButton("OK"), *cancel = new QPushButton("Cancel");
    QObject::connect(ok, &QPushButton::clicked, &dialog, &QDialog::accept);
    QObject::connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    btnLayout->addWidget(ok);
    btnLayout->addWidget(cancel);
    layout->addLayout(btnLayout);

    if (dialog.exec() != QDialog::Accepted)
        return 1;

    QStringList results;
    if (entryWidget)
        results << QString(debug ? "input=%1" : "%1").arg(csvEscape(entryWidget->text()));

    if (calendar)
        results << QString(debug ? "calendar=%1" : "%1").arg(calendar->selectedDate().toString(Qt::ISODate));

    for (QCheckBox *cb : checkboxes) {
        QString label = cb->text().toLower().replace(" ", "_");
        QString value = cb->isChecked() ? "true" : "false";
        results << QString(debug ? "%1=%2" : "%2").arg(label).arg(value);
    }

    if (widgetMap.contains("slider")) {
        auto s = qobject_cast<QSlider*>(widgetMap["slider"]);
        results << QString(debug ? "slider=%1" : "%1").arg(s->value());
    }

    if (widgetMap.contains("dial")) {
        auto d = qobject_cast<QDial*>(widgetMap["dial"]);
        results << QString(debug ? "dial=%1" : "%1").arg(d->value());
    }

    if (widgetMap.contains("datetime")) {
        auto dt = qobject_cast<QDateTimeEdit*>(widgetMap["datetime"]);
        results << QString(debug ? "datetime=%1" : "%1").arg(dt->dateTime().toString(Qt::ISODate));
    }

    for (auto it = radioGroups.begin(); it != radioGroups.end(); ++it) {
        QString groupName = it.key();
        QButtonGroup *group = it.value();
        if (group->checkedButton()) {
            results << QString("%1=%2").arg(groupName, group->checkedButton()->text());
        }
    }


    if (clockDisplay)
        results << QString(debug ? "clock=%1" : "%1").arg(QTime::currentTime().toString("hh:mm:ss"));

    cout << results.join(",").toStdString() << endl;
    return 0;
}
