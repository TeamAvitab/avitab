# This file is part of the Avitab project. See the README and LICENSE for details.

# This file is a basic CMake wrapper for the proj external library. The provided
# CMake support is not sufficiently configurable to be easily used directly.

project(proj
    VERSION 5.2.0
    DESCRIPTION "A library for map projection algorithms"
    LANGUAGES C
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
find_package(Threads REQUIRED)
include(Prefixer)

# Configure the proj library
include (CheckIncludeFiles)
include (CheckLibraryExists) 
include (CheckFunctionExists)

# check needed include file
check_include_files (dlfcn.h HAVE_DLFCN_H)
check_include_files (inttypes.h HAVE_INTTYPES_H)
check_include_files (jni.h HAVE_JNI_H)
check_include_files (memory.h HAVE_MEMORY_H)
check_include_files (stdint.h HAVE_STDINT_H)
check_include_files (stdlib.h HAVE_STDLIB_H)
check_include_files (string.h HAVE_STRING_H)
check_include_files (sys/stat.h HAVE_SYS_STAT_H)
check_include_files (sys/types.h HAVE_SYS_TYPES_H)
check_include_files (unistd.h HAVE_UNISTD_H)
check_include_files("stdlib.h;stdarg.h;string.h;float.h" STDC_HEADERS)

CHECK_FUNCTION_EXISTS(localeconv HAVE_LOCALECONV)

# check libm need on unix 
check_library_exists(m ceil "" HAVE_LIBM) 

configure_file("${AVITAB_proj_SOURCE_DIR}/cmake/proj_config.cmake.in" "${AVITAB_proj_BINARY_DIR}/proj_config.h")

# The proj library
set(proj_SOURCES
    aasincos.c
    adjlon.c
    bch2bps.c
    bchgen.c
    biveval.c
    cct.c
    cs2cs.c
    dmstor.c
    emess.c
    emess.h
    gen_cheb.c
    geocent.c
    geocent.h
    geod_interface.c
    geod_interface.h
    geod_set.c
    geod.c
    geodesic.c
    geodesic.h
    geodtest.c
    gie.c
    mk_cheby.c
    multistresstest.c
    nad_cvt.c
    nad_init.c
    nad_intr.c
    nad_list.h
    nad2bin.c
    optargpm.h
    org_proj4_PJ.h
    p_series.c
    PJ_aea.c
    PJ_aeqd.c
    PJ_airy.c
    PJ_aitoff.c
    pj_apply_gridshift.c
    pj_apply_vgridshift.c
    PJ_august.c
    pj_auth.c
    PJ_axisswap.c
    PJ_bacon.c
    PJ_bipc.c
    PJ_boggs.c
    PJ_bonne.c
    PJ_calcofi.c
    PJ_cart.c
    PJ_cass.c
    PJ_cc.c
    PJ_ccon.c
    PJ_cea.c
    PJ_chamb.c
    PJ_collg.c
    PJ_comill.c
    PJ_crast.c
    pj_ctx.c
    pj_datum_set.c
    pj_datums.c
    PJ_deformation.c
    PJ_denoy.c
    pj_deriv.c
    PJ_eck1.c
    PJ_eck2.c
    PJ_eck3.c
    PJ_eck4.c
    PJ_eck5.c
    pj_ell_set.c
    pj_ellps.c
    PJ_eqc.c
    PJ_eqdc.c
    PJ_eqearth.c
    pj_errno.c
    pj_factors.c
    PJ_fahey.c
    pj_fileapi.c
    PJ_fouc_s.c
    pj_fwd.c
    PJ_gall.c
    pj_gauss.c
    pj_gc_reader.c
    PJ_geoc.c
    pj_geocent.c
    PJ_geos.c
    PJ_gins8.c
    PJ_gn_sinu.c
    PJ_gnom.c
    PJ_goode.c
    pj_gridcatalog.c
    pj_gridinfo.c
    pj_gridlist.c
    PJ_gstmerc.c
    PJ_hammer.c
    PJ_hatano.c
    PJ_healpix.c
    PJ_helmert.c
    PJ_hgridshift.c
    PJ_horner.c
    PJ_igh.c
    PJ_imw_p.c
    pj_init.c
    pj_initcache.c
    pj_internal.c
    pj_inv.c
    PJ_isea.c
    PJ_krovak.c
    PJ_labrd.c
    PJ_laea.c
    PJ_lagrng.c
    PJ_larr.c
    PJ_lask.c
    PJ_latlong.c
    PJ_lcc.c
    PJ_lcca.c
    pj_list.c
    pj_list.h
    pj_log.c
    PJ_loxim.c
    PJ_lsat.c
    pj_malloc.c
    pj_math.c
    PJ_mbt_fps.c
    PJ_mbtfpp.c
    PJ_mbtfpq.c
    PJ_merc.c
    PJ_mill.c
    PJ_misrsom.c
    pj_mlfn.c
    PJ_mod_ster.c
    PJ_moll.c
    PJ_molodensky.c
    pj_msfn.c
    pj_mutex.c
    PJ_natearth.c
    PJ_natearth2.c
    PJ_nell_h.c
    PJ_nell.c
    PJ_nocol.c
    PJ_nsper.c
    PJ_nzmg.c
    PJ_ob_tran.c
    PJ_ocea.c
    PJ_oea.c
    PJ_omerc.c
    pj_open_lib.c
    PJ_ortho.c
    pj_param.c
    PJ_patterson.c
    pj_phi2.c
    PJ_pipeline.c
    PJ_poly.c
    pj_pr_list.c
    PJ_putp2.c
    PJ_putp3.c
    PJ_putp4p.c
    PJ_putp5.c
    PJ_putp6.c
    PJ_qsc.c
    pj_qsfn.c
    pj_release.c
    PJ_robin.c
    PJ_rpoly.c
    PJ_sch.c
    PJ_sconics.c
    PJ_somerc.c
    PJ_stere.c
    PJ_sterea.c
    pj_strerrno.c
    pj_strtod.c
    PJ_sts.c
    PJ_tcc.c
    PJ_tcea.c
    PJ_times.c
    PJ_tmerc.c
    PJ_tpeqd.c
    pj_transform.c
    pj_tsfn.c
    PJ_unitconvert.c
    pj_units.c
    PJ_urm5.c
    PJ_urmfps.c
    pj_utils.c
    PJ_vandg.c
    PJ_vandg2.c
    PJ_vandg4.c
    PJ_vgridshift.c
    PJ_wag2.c
    PJ_wag3.c
    PJ_wag7.c
    PJ_wink1.c
    PJ_wink2.c
    pj_zpoly1.c
    proj_4D_api.c
    proj_api.h
    proj_etmerc.c
    proj_internal.h
    proj_math.h
    proj_mdist.c
    proj_rouss.c
    proj_strtod.c
    proj_strtod.h
    proj.c
    proj.h
    projects.h
    rtodms.c
    vector1.c
)
PrefixSources(proj_SOURCES "${AVITAB_proj_SOURCE_DIR}/src")

add_library(proj STATIC ${proj_SOURCES})
target_include_directories(proj
    PRIVATE "${AVITAB_proj_BINARY_DIR}"
    PUBLIC "${AVITAB_proj_SOURCE_DIR}"
)

# Cache settings to be used when AVITAB_FORCE_BUILD_THIRDPARTY is OFF
set(AVITAB_proj_PKGLIBS proj CACHE INTERNAL "")
if(MSVC)
    set(AVITAB_proj_LOCATION "${AVITAB_LIBS3RD_DIR}/proj.lib" CACHE INTERNAL "")
else()
    set(AVITAB_proj_LOCATION "${AVITAB_LIBS3RD_DIR}/libproj.a" CACHE INTERNAL "")
endif()
set(AVITAB_proj_INCDIRS "${AVITAB_proj_SOURCE_DIR}/src" CACHE INTERNAL "")
set(AVITAB_proj_LINKLIBS sqlite3 CACHE INTERNAL "")
