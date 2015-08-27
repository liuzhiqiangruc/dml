INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/util)

#lib kmeans
set(CMAKE_C_FLAGS "-std=c99 -O3 -pipe -Wall -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function -Wextra")
SET(LIBKMEANS_SRC kmeans.c)
ADD_LIBRARY(kmeans_static STATIC ${LIBKMEANS_SRC})
SET_TARGET_PROPERTIES(kmeans_static PROPERTIES OUTPUT_NAME "kmeans")
SET_TARGET_PROPERTIES(kmeans_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
INSTALL(TARGETS kmeans_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES kmeans.h DESTINATION include)

#lib modl
SET(LIBMODL_SRC modl.c)
ADD_LIBRARY(modl_static STATIC ${LIBMODL_SRC})
SET_TARGET_PROPERTIES(modl_static PROPERTIES OUTPUT_NAME "modl")
SET_TARGET_PROPERTIES(modl_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
INSTALL(TARGETS modl_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES modl.h DESTINATION include)

#lib jlda
SET(LIBjlda_SRC joint_lda.c)
ADD_LIBRARY(jlda_static STATIC ${LIBjlda_SRC})
SET_TARGET_PROPERTIES(jlda_static PROPERTIES OUTPUT_NAME "jlda")
SET_TARGET_PROPERTIES(jlda_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
INSTALL(TARGETS jlda_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES joint_lda.h DESTINATION include)
TARGET_LINK_LIBRARIES(jlda_static str_static)
TARGET_LINK_LIBRARIES(jlda_static idmap_static)
TARGET_LINK_LIBRARIES(jlda_static m)

#lib lda
SET(LIBlda_SRC lda.c)
ADD_LIBRARY(lda_static STATIC ${LIBlda_SRC})
SET_TARGET_PROPERTIES(lda_static PROPERTIES OUTPUT_NAME "lda")
SET_TARGET_PROPERTIES(lda_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
INSTALL(TARGETS lda_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES lda.h DESTINATION include)
TARGET_LINK_LIBRARIES(lda_static str_static)
TARGET_LINK_LIBRARIES(lda_static idmap_static)
TARGET_LINK_LIBRARIES(lda_static m)

#bridge lda
SET(LIBblda_SRC bri_lda.c)
ADD_LIBRARY(blda_static STATIC ${LIBblda_SRC})
SET_TARGET_PROPERTIES(blda_static PROPERTIES OUTPUT_NAME "blda")
SET_TARGET_PROPERTIES(blda_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
INSTALL(TARGETS blda_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES bri_lda.h DESTINATION include)
TARGET_LINK_LIBRARIES(blda_static str_static)
TARGET_LINK_LIBRARIES(blda_static idmap_static)
TARGET_LINK_LIBRARIES(blda_static m)

#author topic model
SET(LIBalda_SRC author_lda.c)
ADD_LIBRARY(alda_static STATIC ${LIBalda_SRC})
SET_TARGET_PROPERTIES(alda_static PROPERTIES OUTPUT_NAME "alda")
SET_TARGET_PROPERTIES(alda_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
INSTALL(TARGETS alda_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES author_lda.h DESTINATION include)
target_compile_definitions(alda_static PRIVATE -DPHI)
TARGET_LINK_LIBRARIES(alda_static str_static)
TARGET_LINK_LIBRARIES(alda_static idmap_static)
TARGET_LINK_LIBRARIES(alda_static m)

#lib tot
SET(LIBtot_SRC tot.c)
ADD_LIBRARY(tot_static STATIC ${LIBtot_SRC})
SET_TARGET_PROPERTIES(tot_static PROPERTIES OUTPUT_NAME "tot")
SET_TARGET_PROPERTIES(tot_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
INSTALL(TARGETS tot_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES tot.h DESTINATION include)
target_compile_definitions(tot_static PRIVATE -DBETA)
target_link_libraries(tot_static str_static)
target_link_libraries(tot_static idmap_static)
target_link_libraries(tot_static m)

#lib tot_gaussian
SET(LIBtotg_SRC tot.c)
ADD_LIBRARY(totg_static STATIC ${LIBtotg_SRC})
SET_TARGET_PROPERTIES(totg_static PROPERTIES OUTPUT_NAME "totg")
SET_TARGET_PROPERTIES(totg_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
INSTALL(TARGETS totg_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES tot.h DESTINATION include)
target_compile_definitions(totg_static PRIVATE -DGAUSSIAN)
target_link_libraries(totg_static str_static)
target_link_libraries(totg_static idmap_static)
target_link_libraries(totg_static m)

#lib newton
SET(LIBnewton_opt_SRC newton_opt.c)
ADD_LIBRARY(newton_opt_static STATIC ${LIBnewton_opt_SRC})
SET_TARGET_PROPERTIES(newton_opt_static PROPERTIES OUTPUT_NAME "newton_opt")
SET_TARGET_PROPERTIES(newton_opt_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
target_link_libraries(newton_opt_static m)
INSTALL(TARGETS newton_opt_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES newton_opt.h DESTINATION include)

#lib mf
SET(LIBmf_SRC mf.c)
ADD_LIBRARY(mf_static STATIC ${LIBmf_SRC})
SET_TARGET_PROPERTIES(mf_static PROPERTIES OUTPUT_NAME "mf")
SET_TARGET_PROPERTIES(mf_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
INSTALL(TARGETS mf_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES mf.h DESTINATION include)

#lib pmf
SET(LIBpmf_SRC pmf.c)
ADD_LIBRARY(pmf_static STATIC ${LIBpmf_SRC})
SET_TARGET_PROPERTIES(pmf_static PROPERTIES OUTPUT_NAME "pmf")
SET_TARGET_PROPERTIES(pmf_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
INSTALL(TARGETS pmf_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES mf.h DESTINATION include)

#lib bmf
SET(LIBbmf_SRC bmf.c)
ADD_LIBRARY(bmf_static STATIC ${LIBbmf_SRC})
SET_TARGET_PROPERTIES(bmf_static PROPERTIES OUTPUT_NAME "bmf")
SET_TARGET_PROPERTIES(bmf_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
INSTALL(TARGETS bmf_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES mf.h DESTINATION include)

#lib regression
SET(LIBregression_SRC regression.c)
ADD_LIBRARY(regression_static STATIC ${LIBregression_SRC})
SET_TARGET_PROPERTIES(regression_static PROPERTIES OUTPUT_NAME "regression")
SET_TARGET_PROPERTIES(regression_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
INSTALL(TARGETS regression_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES regression.h DESTINATION include)
target_link_libraries(regression_static str_static)
target_link_libraries(regression_static idmap_static)

#lib lr
SET(LIBlr_SRC lr.c)
ADD_LIBRARY(lr_static STATIC ${LIBlr_SRC})
SET_TARGET_PROPERTIES(lr_static PROPERTIES OUTPUT_NAME "lr")
SET_TARGET_PROPERTIES(lr_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
INSTALL(TARGETS lr_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES regression.h DESTINATION include)
target_link_libraries(lr_static newton_opt_static)
target_link_libraries(lr_static auc_static)

#lib pr
SET(LIBpr_SRC pr.c)
ADD_LIBRARY(pr_static STATIC ${LIBpr_SRC})
SET_TARGET_PROPERTIES(pr_static PROPERTIES OUTPUT_NAME "pr")
SET_TARGET_PROPERTIES(pr_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
INSTALL(TARGETS pr_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES regression.h DESTINATION include)
target_link_libraries(pr_static newton_opt_static)

#lib gr
SET(LIBgr_SRC gr.c)
ADD_LIBRARY(gr_static STATIC ${LIBgr_SRC})
SET_TARGET_PROPERTIES(gr_static PROPERTIES OUTPUT_NAME "gr")
SET_TARGET_PROPERTIES(gr_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
INSTALL(TARGETS gr_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES regression.h DESTINATION include)
target_link_libraries(gr_static newton_opt_static)

#lib er
SET(LIBer_SRC er.c)
ADD_LIBRARY(er_static STATIC ${LIBer_SRC})
SET_TARGET_PROPERTIES(er_static PROPERTIES OUTPUT_NAME "er")
SET_TARGET_PROPERTIES(er_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
INSTALL(TARGETS er_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES regression.h DESTINATION include)
target_link_libraries(er_static newton_opt_static)




#lib auc
SET(LIBauc_SRC auc.c)
ADD_LIBRARY(auc_static STATIC ${LIBauc_SRC})
SET_TARGET_PROPERTIES(auc_static PROPERTIES OUTPUT_NAME "auc")
SET_TARGET_PROPERTIES(auc_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
INSTALL(TARGETS auc_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES auc.h DESTINATION include)
target_link_libraries(auc_static newton_opt_static)
