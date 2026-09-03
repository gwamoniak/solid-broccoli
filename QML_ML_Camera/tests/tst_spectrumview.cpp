#include <QtTest>

#include "SpectrometerService.h"
#include "SpectrumView.h"

class TstSpectrumView : public QObject
{
    Q_OBJECT

private slots:
    void cursorSelectsNearestPointAndCandidate()
    {
        SpectrometerService service;
        SpectrumView view;
        view.setWidth(680);
        view.setHeight(400);
        view.setSource(&service);

        service.startAcquisition();
        QTRY_VERIFY_WITH_TIMEOUT(service.peakWavelengthNm() > 500.0, 2000);

        view.inspectAtX(view.wavelengthToX(546.1));
        QVERIFY(view.cursorVisible());
        QVERIFY(qAbs(view.cursorWavelengthNm() - 546.1) < 0.6);
        QVERIFY(view.cursorValue() > 0.0);
        QCOMPARE(view.cursorCandidate(), QStringLiteral("Hg"));

        view.clearCursor();
        QVERIFY(!view.cursorVisible());
        service.stopAcquisition();
    }
};

QTEST_MAIN(TstSpectrumView)
#include "tst_spectrumview.moc"
