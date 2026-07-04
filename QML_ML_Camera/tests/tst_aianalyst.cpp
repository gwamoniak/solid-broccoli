#include <QSignalSpy>
#include <QtTest>

#include "AiAnalyst.h"

// Gated generation smoke test: needs a real GGUF. Set SOLIDBROCCOLI_TEST_GGUF
// to a model path to run; otherwise the test QSKIPs (never fails) — same
// gating philosophy as the hardware acceptances. Asserts structure, not
// wording: sampling is not bit-stable across llama.cpp versions.
class TstAiAnalyst : public QObject
{
    Q_OBJECT

private slots:
    void generatesFromATinyContext()
    {
        const QString ggufPath =
            qEnvironmentVariable("SOLIDBROCCOLI_TEST_GGUF");
        if (ggufPath.isEmpty())
            QSKIP("SOLIDBROCCOLI_TEST_GGUF not set — generation smoke skipped.");

        AiAnalyst analyst;
        QSignalSpy loaded(&analyst, &AiAnalyst::modelLoadFinished);
        analyst.loadModel(ggufPath);
        QTRY_VERIFY_WITH_TIMEOUT(loaded.count() == 1, 120000);
        QVERIFY2(loaded.first().first().toBool(), "model failed to load");

        QSignalSpy tokens(&analyst, &AiAnalyst::tokenGenerated);
        QSignalSpy finished(&analyst, &AiAnalyst::generationFinished);
        analyst.generate(QStringLiteral(
            "Reply with the single word READY and nothing else."), 16);
        QTRY_VERIFY_WITH_TIMEOUT(finished.count() == 1, 300000);

        const QString text = finished.first().first().toString();
        QVERIFY(!text.isEmpty());
        QVERIFY(tokens.count() > 0);
    }
};

QTEST_GUILESS_MAIN(TstAiAnalyst)
#include "tst_aianalyst.moc"
