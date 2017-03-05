include (FindPkgConfig)

set (apricosfsman
    main.c
    )

add_executable (${PROJECT_NAME} ${apricosfsman})

install (TARGETS apricosfsman DESTINATION bin)
