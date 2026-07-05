#include "CodecDevice.h"

#include <utility>

#include "loggingcategories.h"

CodecDevice::CodecDevice(QString name,
                         std::unique_ptr<SensorTransport> transport,
                         std::unique_ptr<ProtocolCodec> codec,
                         QObject* parent)
    : SensorDevice(parent)
    , m_name(std::move(name))
    , m_transport(std::move(transport))
    , m_codec(std::move(codec))
{
    m_transport->setParent(nullptr);  // owned by unique_ptr, not the QObject tree

    m_codec->setSink([this](SensorReading reading) {
        if (std::holds_alternative<Spectrum>(reading))
            emit spectrumReady(std::get<Spectrum>(std::move(reading)));
        else
            emit readingReady(std::get<GeigerReading>(reading));
    });

    connect(m_transport.get(), &SensorTransport::bytesReceived,
            this, [this](const QByteArray& bytes) { m_codec->feed(bytes); });
    connect(m_transport.get(), &SensorTransport::transportError,
            this, [this](const QString& message) {
                qWarning(logCritical()) << "CodecDevice" << m_name
                                        << "transport error:" << message;
                emit errorOccurred(message);
            });
    connect(m_transport.get(), &SensorTransport::closed,
            this, [this]() { setConnected(false); });
}

CodecDevice::~CodecDevice()
{
    if (isConnected())
        m_transport->close();
}

bool CodecDevice::connectDevice(QString* errorMessage)
{
    if (isConnected())
        return true;

    QString message;
    if (!m_transport->open(&message)) {
        qWarning(logCritical()) << "CodecDevice" << m_name << "open failed:" << message;
        if (errorMessage)
            *errorMessage = message;
        emit errorOccurred(message);
        return false;
    }
    qDebug(logInfo()) << "CodecDevice" << m_name << "connected.";
    setConnected(true);
    return true;
}

void CodecDevice::disconnectDevice()
{
    if (!isConnected())
        return;
    m_transport->close();
    qDebug(logInfo()) << "CodecDevice" << m_name << "disconnected.";
    setConnected(false);
}

void CodecDevice::start()
{
    if (!isConnected())
        return;
    m_transport->writeBytes(m_codec->encodeStart());
}

void CodecDevice::stop()
{
    if (!isConnected())
        return;
    m_transport->writeBytes(m_codec->encodeStop());
}

void CodecDevice::setParams(const AcquisitionParams& params)
{
    m_params = params;
    if (isConnected())
        m_transport->writeBytes(m_codec->encodeParams(params));
}

QString CodecDevice::name() const
{
    return m_name;
}
