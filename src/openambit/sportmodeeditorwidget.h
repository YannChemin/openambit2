#ifndef SPORTMODEEDITORWIDGET_H
#define SPORTMODEEDITORWIDGET_H

#include <QWidget>
#include <QVariantMap>
#include <QList>

class QListWidget;
class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QComboBox;
class QCheckBox;
class QPushButton;

class SportModeEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SportModeEditorWidget(QWidget *parent = nullptr);

    void setModes(const QList<QVariantMap> &modes);
    QList<QVariantMap> modes() const;

signals:
    void writeToWatchRequested(const QList<QVariantMap> &modes);

private slots:
    void onModeSelected(int row);
    void onAddMode();
    void onRemoveMode();
    void onWriteToWatch();
    void onLoadDefaults();
    void onSaveToFile();
    void onLoadFromFile();

private:
    void saveCurrentForm();
    void loadFormFromMap(const QVariantMap &m);
    void setFormEnabled(bool enabled);
    int gpsIntervalToIndex(int val) const;
    int indexToGpsInterval(int idx) const;
    int recIntervalToIndex(int val) const;
    int indexToRecInterval(int idx) const;

    QList<QVariantMap> m_modes;
    int m_currentRow = -1;
    bool m_loading = false;  // guard against re-entrant saves

    QListWidget   *m_list;
    QPushButton   *m_addBtn;
    QPushButton   *m_removeBtn;

    QLineEdit     *m_name;
    QSpinBox      *m_activityId;
    QSpinBox      *m_sportModeId;
    QComboBox     *m_gpsInterval;
    QComboBox     *m_recInterval;
    QComboBox     *m_altiBaroMode;
    QCheckBox     *m_hrBelt;
    QCheckBox     *m_footPod;
    QCheckBox     *m_bikePod;
    QCheckBox     *m_cadencePod;
    QCheckBox     *m_accelerometer;
    QCheckBox     *m_useHrLimits;
    QSpinBox      *m_hrMax;
    QSpinBox      *m_hrMin;
    QCheckBox     *m_useAutolap;
    QSpinBox      *m_autolap;
    QDoubleSpinBox *m_autoPause;
    QComboBox     *m_backlight;
    QCheckBox     *m_displayNegative;

    QPushButton   *m_writeBtn;
    QPushButton   *m_defaultsBtn;
    QPushButton   *m_saveFileBtn;
    QPushButton   *m_loadFileBtn;
};

#endif // SPORTMODEEDITORWIDGET_H
