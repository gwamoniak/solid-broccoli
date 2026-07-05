#ifndef AIANALYST_H
#define AIANALYST_H

#include <atomic>

#include <QObject>
#include <QString>

struct llama_model;

// Thin Qt wrapper around llama.cpp: loads one GGUF model and turns a prompt
// into a streamed completion on a worker thread. Deliberately ignorant of
// the database, the prompt template, and the report workflow — those live
// in the app's ReportService. Greedy sampling: reports should be as
// deterministic as the runtime allows.
class AiAnalyst : public QObject
{
    Q_OBJECT

public:
    explicit AiAnalyst(QObject* parent = nullptr);
    ~AiAnalyst() override;

    bool isModelLoaded() const { return m_model != nullptr; }
    QString modelName() const { return m_modelName; }
    bool isBusy() const { return m_busy.load(); }

    // Both asynchronous (worker thread); results arrive via the signals.
    void loadModel(const QString& ggufPath);
    void generate(const QString& prompt, int maxNewTokens = 1024);
    void cancel() { m_cancel.store(true); }
    void unloadModel();

signals:
    void modelLoadFinished(bool ok, const QString& error);
    void tokenGenerated(const QString& token);
    void generationFinished(const QString& fullText);
    void errorOccurred(const QString& message);

private:
    void runLoad(const QString& path);
    void runGenerate(const QString& prompt, int maxNewTokens);

    llama_model* m_model = nullptr;
    QString m_modelName;
    std::atomic<bool> m_busy{false};
    std::atomic<bool> m_cancel{false};
};

#endif // AIANALYST_H
