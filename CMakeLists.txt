PROJECT(pipicross)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

##-------- external libraries
#FIND_PACKAGE(Qt4 REQUIRED)
#INCLUDE(${QT_USE_FILE})

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})
INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIRS})
SET(VTK_LIBRARIES vtkRendering vtkCommon vtkFiltering vtkGraphics vtkIO vtkImaging vtkRendering vtkHybrid)

ADD_EXECUTABLE(pipicross pipicross.cpp helper.cpp)
TARGET_LINK_LIBRARIES(pipicross ${VTK_LIBRARIES})

