#ifndef SIMULATEDSPECTROMETER_H
#define SIMULATEDSPECTROMETER_H

#include <QElapsedTimer>
#include <QTimer>

#include "InstrumentModel.h"
#include "SensorDevice.h"
#include "SimScene.h"
#include "spectro-sim_global.h"

// A spectrometer that needs no hardware and no transport: a scene evaluated
// through the synthesizer and the instrument-noise model on a timer. The two
// canonical instruments the app lists are the named constructors below.
class SPECTROSIM_EXPORT SimulatedSpectrometer : public SpectrometerDevice
{
    Q_OBJECT

public:
    SimulatedSpectrometer(QString name, SimScene scene,
                          InstrumentProfile profile = {},
                          QObject* parent = nullptr);

    // Emission source with the five mercury lines — for peak work.
    static SimulatedSpectrometer* mercuryLamp(QObject* parent = nullptr);
    // Blackbody through a methylene-blue-like 664 nm absorber — for
    // absorbance work. Concentration 0 (setSceneConcentration) is the blank.
    static SimulatedSpectrometer* dyeSample(QObject* parent = nullptr);

    bool connectDevice(QString* errorMessage) override;
    void disconnectDevice() override;
    void start() override;
    void stop() override;
    void setParams(const AcquisitionParams& params) override;
    QString name() const override;

    // Scene controls used by the service for dark/reference capture.
    void setLampEnabled(bool on);
    void setSceneConcentration(double concentration);
    const SimScene& scene() const { return m_scene; }

private:
    void rebuildFlux();
    void tick();

    QString m_name;
    SimScene m_scene;
    InstrumentModel m_model;
    AcquisitionParams m_params;
    QVector<double> m_grid;
    QVector<double> m_flux;      // scene evaluated on m_grid, drift excluded
    QTimer m_timer;
    QElapsedTimer m_elapsed;
};

#endif // SIMULATEDSPECTROMETER_H
