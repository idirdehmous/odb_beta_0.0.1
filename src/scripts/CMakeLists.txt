#configure_file(use_odb.csh.in ${CMAKE_BINARY_DIR}/bin/use_odb.csh @ONLY)
#configure_file(use_odb.sh.in ${CMAKE_BINARY_DIR}/bin/use_odb.sh @ONLY)

#install(PROGRAMS ${CMAKE_BINARY_DIR}/bin/use_odb.csh DESTINATION bin)
#install(PROGRAMS ${CMAKE_BINARY_DIR}/bin/use_odb.sh DESTINATION bin)

# WE NEED ONLY dcgan SCRIPT 
list(APPEND _odb_scripts

    #check_links
    #create_index
    #create_ioassign
    #create_odbglue
    #create_static_stubb
    #dcafix
    dcagen
    #dcaquick
    #latlon_rad
    #latlonrad
    #makegp.ksh
    #newodb
    #odb1to4
    #odb4to1
    #odb_1to4
    #odb_4to1
    #odb_cat
    #odb_compress
    #odb_diff
    #odb_dup
    #odb_gnuplot
    #odb_less
    #odb_merge
    #odb_more
    #odb_prune
    #odbc++
    #odbcat
    #odbcc
    #odbclean
    #odbcomp
    #odbcompress
    #odbdiff
    #odbdup
    #odbf90
    #odbf90mp
    #odbgnuplot
    #odbless
    #odbmerge
    #odbmore
    #odbprune
    #odbsql
    #test_arch

    # DEPRECATED: wrapper scripts for odb-api tools

    #    .odb_deprecated.sh
    # odb_count
    #odb_dump
    #odb_header
    #odb_set
    #odb_split
    #odb_to_request
    #odbcount
    #odbdump
    #odbheader
    #odbset
)

# DON't NEED ODB SERVER 
#if(ODB_SERVER_CLIENT)
#  list(APPEND _odb_scripts
#    askodb
#    odbi_proxy
#    odbi_show_server
#    odbi_start_server
#    odbi_stop_server)
#endif()

install(PROGRAMS ${_odb_scripts} DESTINATION bin)

foreach(script ${_odb_scripts})
    configure_file(${script} ${CMAKE_BINARY_DIR}/bin COPYONLY)
endforeach()
