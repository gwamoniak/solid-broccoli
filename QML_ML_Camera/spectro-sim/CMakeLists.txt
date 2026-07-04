qt_add_library(spectro-sim SHARED
    FaultPolicy.h
    InstrumentModel.cpp
    InstrumentModel.h
    ReplayTransport.cpp
    ReplayTransport.h
    ScriptedTransport.cpp
    ScriptedTransport.h
    SimScene.h
    SimulatedBridgeTransport.cpp
    SimulatedBridgeTransport.h
    SimulatedSpectrometer.cpp
    SimulatedSpectrometer.h
    SpectrumSynthesizer.cpp
    SpectrumSynthesizer.h
    TraceRecorder.cpp
    TraceRecorder.h
    spectro-sim_global.h
)

target_link_libraries(spectro-sim
    PUBLIC
        Qt6::Core
        spectro-core
)

target_compile_definitions(spectro-sim
    PRIVATE SPECTROSIM_LIBRARY
)

target_include_directories(spectro-sim
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
