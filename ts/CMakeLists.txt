INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/util ${PROJECT_SOURCE_DIR}/dm)
set(CMAKE_C_FLAGS "-std=c99 -O3 -pipe -Wall -Wextra")

#lib edm
SET(LIBEDM_SRC edm.c)
ADD_LIBRARY(edm_static STATIC ${LIBEDM_SRC})
SET_TARGET_PROPERTIES(edm_static PROPERTIES OUTPUT_NAME "edm")
SET_TARGET_PROPERTIES(edm_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
INSTALL(TARGETS edm_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES edm.h DESTINATION include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/util ${PROJECT_SOURCE_DIR}/dm)
TARGET_LINK_LIBRARIES(edm_static mtrace_static)

#lib viterbi
SET(LIBviterbi_SRC viterbi.c)
ADD_LIBRARY(viterbi_static STATIC ${LIBviterbi_SRC})
SET_TARGET_PROPERTIES(viterbi_static PROPERTIES OUTPUT_NAME "viterbi")
SET_TARGET_PROPERTIES(viterbi_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
INSTALL(TARGETS viterbi_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES viterbi.h DESTINATION include)

#lib hash_pattern
SET(LIBhash_pattern_SRC hash_pattern.c)
ADD_LIBRARY(hash_pattern_static STATIC ${LIBhash_pattern_SRC})
SET_TARGET_PROPERTIES(hash_pattern_static PROPERTIES OUTPUT_NAME "hash_pattern")
SET_TARGET_PROPERTIES(hash_pattern_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
INSTALL(TARGETS hash_pattern_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES hash_pattern.h DESTINATION include)
TARGET_LINK_LIBRARIES(hash_pattern_static m)

#lib bayesian_changepoint
SET(LIBbayesian_changepoint_SRC bayesian_changepoint.c)
ADD_LIBRARY(bayesian_changepoint_static STATIC ${LIBbayesian_changepoint_SRC})
SET_TARGET_PROPERTIES(bayesian_changepoint_static PROPERTIES OUTPUT_NAME "bayesian_changepoint")
SET_TARGET_PROPERTIES(bayesian_changepoint_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
INSTALL(TARGETS bayesian_changepoint_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES bayesian_changepoint.h DESTINATION include)
TARGET_LINK_LIBRARIES(bayesian_changepoint_static m)

#lib loess
SET(LIBloess_SRC loess.c)
ADD_LIBRARY(loess_static STATIC ${LIBloess_SRC})
SET_TARGET_PROPERTIES(loess_static PROPERTIES OUTPUT_NAME "loess")
SET_TARGET_PROPERTIES(loess_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
INSTALL(TARGETS loess_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES loess.h DESTINATION include)
TARGET_LINK_LIBRARIES(loess_static mtrace_static)
TARGET_LINK_LIBRARIES(loess_static m)

#lib fit_newton
SET(LIBfit_newton_SRC fit_newton.c)
ADD_LIBRARY(fit_newton_static STATIC ${LIBfit_newton_SRC})
SET_TARGET_PROPERTIES(fit_newton_static PROPERTIES OUTPUT_NAME "fit_newton")
SET_TARGET_PROPERTIES(fit_newton_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
INSTALL(TARGETS fit_newton_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES fit_newton.h DESTINATION include)
TARGET_LINK_LIBRARIES(fit_newton_static newton_opt_static)
