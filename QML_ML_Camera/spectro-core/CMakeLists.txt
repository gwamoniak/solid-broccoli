qt_add_library(spectro-core SHARED
    As7265xCodec.cpp
    As7265xCodec.h
    BleTransport.cpp
    BleTransport.h
    BridgeContract.h
    BridgeFrameParser.cpp
    BridgeFrameParser.h
    CodecDevice.cpp
    GeigerCodec.cpp
    GeigerCodec.h
    GeigerMath.cpp
    GeigerMath.h
    PeakMatcher.cpp
    PeakMatcher.h
    SpectralLineLibrary.cpp
    SpectralLineLibrary.h
    CodecDevice.h
    ProtocolCodec.h
    SensorDevice.cpp
    SensorDevice.h
    SensorTransport.h
    SpectroAnalysis.cpp
    SpectroAnalysis.h
    SpectrumExporter.cpp
    SpectrumExporter.h
    Spectrum.cpp
    Spectrum.h
    spectro-core_global.h
)

target_link_libraries(spectro-core
    PUBLIC
        Qt6::Core
        Qt6::Bluetooth
        logger-core
)

target_compile_definitions(spectro-core
    PRIVATE SPECTROCORE_LIBRARY
)

target_include_directories(spectro-core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
