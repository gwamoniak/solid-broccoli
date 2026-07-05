#include "AiAnalyst.h"

#include <vector>

#include <QFileInfo>
#include <QThread>

#include <llama.h>

#include "loggingcategories.h"

AiAnalyst::AiAnalyst(QObject* parent)
    : QObject(parent)
{
    llama_backend_init();
}

AiAnalyst::~AiAnalyst()
{
    m_cancel.store(true);
    while (m_busy.load())
        QThread::msleep(10);
    unloadModel();
    llama_backend_free();
}

void AiAnalyst::unloadModel()
{
    if (m_model) {
        llama_model_free(m_model);
        m_model = nullptr;
        m_modelName.clear();
    }
}

void AiAnalyst::loadModel(const QString& ggufPath)
{
    if (m_busy.exchange(true)) {
        emit errorOccurred(tr("The analyst is busy."));
        return;
    }
    QThread* thread = QThread::create([this, ggufPath]() { runLoad(ggufPath); });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
}

void AiAnalyst::runLoad(const QString& path)
{
    unloadModel();

    llama_model_params params = llama_model_default_params();
    params.n_gpu_layers = 99;  // Metal on macOS; harmless on CPU-only builds

    qDebug(logInfo()) << "AiAnalyst: loading model" << path;
    m_model = llama_model_load_from_file(path.toUtf8().constData(), params);
    m_busy.store(false);
    if (!m_model) {
        const QString error = tr("Could not load model %1").arg(path);
        qWarning(logCritical()) << "AiAnalyst:" << error;
        emit modelLoadFinished(false, error);
        return;
    }
    m_modelName = QFileInfo(path).fileName();
    qDebug(logInfo()) << "AiAnalyst: model ready:" << m_modelName;
    emit modelLoadFinished(true, QString());
}

void AiAnalyst::generate(const QString& prompt, int maxNewTokens)
{
    if (!m_model) {
        emit errorOccurred(tr("No model loaded."));
        return;
    }
    if (m_busy.exchange(true)) {
        emit errorOccurred(tr("The analyst is busy."));
        return;
    }
    m_cancel.store(false);
    QThread* thread = QThread::create(
        [this, prompt, maxNewTokens]() { runGenerate(prompt, maxNewTokens); });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
}

void AiAnalyst::runGenerate(const QString& prompt, int maxNewTokens)
{
    const llama_vocab* vocab = llama_model_get_vocab(m_model);

    // Wrap the prompt in the model's own chat template (Gemma instruct
    // models require their turn markers); fall back to the raw prompt.
    QByteArray promptUtf8 = prompt.toUtf8();
    if (const char* tmpl = llama_model_chat_template(m_model, nullptr)) {
        const llama_chat_message message{"user", promptUtf8.constData()};
        std::vector<char> buffer(size_t(promptUtf8.size()) * 2 + 1024);
        int written = llama_chat_apply_template(tmpl, &message, 1, true,
                                                buffer.data(), int(buffer.size()));
        if (written > int(buffer.size())) {
            buffer.resize(size_t(written));
            written = llama_chat_apply_template(tmpl, &message, 1, true,
                                                buffer.data(), int(buffer.size()));
        }
        if (written > 0)
            promptUtf8 = QByteArray(buffer.data(), written);
    }

    // Tokenize (first call sized, second call fills).
    const int needed = -llama_tokenize(vocab, promptUtf8.constData(),
                                       promptUtf8.size(), nullptr, 0, true, true);
    std::vector<llama_token> tokens(size_t(std::max(needed, 0)));
    if (needed <= 0
        || llama_tokenize(vocab, promptUtf8.constData(), promptUtf8.size(),
                          tokens.data(), int(tokens.size()), true, true) < 0) {
        m_busy.store(false);
        emit errorOccurred(tr("Could not tokenize the report context."));
        return;
    }

    llama_context_params ctxParams = llama_context_default_params();
    ctxParams.n_ctx = 8192;
    ctxParams.n_batch = 2048;
    llama_context* ctx = llama_init_from_model(m_model, ctxParams);
    if (!ctx) {
        m_busy.store(false);
        emit errorOccurred(tr("Could not create an inference context."));
        return;
    }
    if (int(tokens.size()) + maxNewTokens >= int(ctxParams.n_ctx)) {
        llama_free(ctx);
        m_busy.store(false);
        emit errorOccurred(tr("Session context is too large for the model window."));
        return;
    }

    llama_sampler* sampler =
        llama_sampler_chain_init(llama_sampler_chain_default_params());
    llama_sampler_chain_add(sampler, llama_sampler_init_greedy());

    qDebug(logInfo()) << "AiAnalyst: generating," << int(tokens.size())
                      << "prompt tokens.";
    QString fullText;
    llama_batch batch = llama_batch_get_one(tokens.data(), int(tokens.size()));
    bool failed = false;
    for (int produced = 0; produced < maxNewTokens && !m_cancel.load(); ++produced) {
        if (llama_decode(ctx, batch) != 0) {
            failed = true;
            emit errorOccurred(tr("Inference failed mid-generation."));
            break;
        }
        llama_token token = llama_sampler_sample(sampler, ctx, -1);
        if (llama_vocab_is_eog(vocab, token))
            break;

        char piece[256];
        const int length =
            llama_token_to_piece(vocab, token, piece, int(sizeof(piece)), 0, true);
        if (length > 0) {
            const QString text = QString::fromUtf8(piece, length);
            fullText += text;
            emit tokenGenerated(text);
        }
        batch = llama_batch_get_one(&token, 1);
    }

    llama_sampler_free(sampler);
    llama_free(ctx);
    m_busy.store(false);
    if (!failed) {
        qDebug(logInfo()) << "AiAnalyst: generation finished,"
                          << fullText.size() << "chars.";
        emit generationFinished(fullText);
    }
}
