project(LabelMaker)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
find_package(OpenCV)
find_package(Qt4 REQUIRED)
set(QT_USE_QTMAIN true)
set(QT_USE_QTGUI true)
set(QT_USE_QTNETWORK true)
set(QT_USE_QTOPENGL true)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})
include(labelmaker.cmake)
QT4_WRAP_UI(LABELMAKER_UIS_H ${LABELMAKER_UIS})
QT4_WRAP_CPP(LABELMAKER_MOC_SRCS ${LABELMAKER_MOC_HDRS})

include_directories(${CMAKE_CURRENT_BINARY_DIR} src)
add_executable(LabelMaker
	${LABELMAKER_SRCS}
	${LABELMAKER_UIS_H}
	${LABELMAKER_MOC_SRCS}
)

target_link_libraries(LabelMaker
	${OpenCV_LIBS}
	${QT_LIBRARIES}
	)
