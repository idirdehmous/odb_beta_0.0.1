
#ecbuild_add_library(TARGET odbmain
#    TYPE STATIC # to allow extraction of objects using ar -x
#    DEPENDS odbifsaux odb_lib # parkind1.mod, odb_mod.mod
#    SOURCES Create_index.F90
#            Odbcompress.F90
#            Odbdiff.F90
#            Odbgnuplot.F90
#            Odbless.F90)

set(libs ${ODB_LIBRARIES} -Wl,--whole-archive odbmpiserial -Wl,--no-whole-archive)
include_directories(${ODB_INCLUDE_DIRS})

#ecbuild_add_executable(TARGET odbsql.x SOURCES Fodbsql.F90
#    LINKER_LANGUAGE Fortran LIBS ${libs})

#ecbuild_add_executable(TARGET fscheduler.x SOURCES Fscheduler.F90
#    LINKER_LANGUAGE Fortran LIBS ${libs})

ecbuild_add_executable(TARGET dcagen.x SOURCES dcagen.c LIBS ${libs})
#ecbuild_add_executable(TARGET hcat     SOURCES hcat.c   LIBS ${libs})
#ecbuild_add_executable(TARGET ioassign SOURCES ioassign.c LIBS ${libs})
#ecbuild_add_executable(TARGET odb2rgg SOURCES odb2rgg.c LIBS ${libs})
#ecbuild_add_executable(TARGET odbversion SOURCES odbversion.c LIBS ${libs})
#ecbuild_add_executable(TARGET odbcksum SOURCES odbcksum.c LIBS ${libs})
ecbuild_add_executable(TARGET odbdump.x SOURCES odbdump_main.c LIBS ${libs})
#ecbuild_add_executable(TARGET odbmd5sum SOURCES odbmd5sum.c LIBS ${libs})
#ecbuild_add_executable(TARGET pcma SOURCES pcma_main.c LIBS ${libs})
#ecbuild_add_executable(TARGET qtar SOURCES qtar.c LIBS ${libs})


# Client/server tools


#ecbuild_add_executable(TARGET b4.x DEFINITIONS HAS_XMOTIF SOURCES b4.c
#    LIBS ${libs} ${MOTIF_LIBRARIES}
#    CONDITION MOTIF_FOUND AND ODB_DEPRECATED)

