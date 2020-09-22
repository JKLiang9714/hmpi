dnl -*- shell-script -*-
dnl
dnl Copyright (c) 2016-2018 Inria.  All rights reserved.
dnl $COPYRIGHT$
dnl
dnl Additional copyrights may follow
dnl
dnl $HEADER$
dnl

# mca_ompi_osc_monitoring_generate_templates
#
# Overwrite $1. $1 is where the different templates are brought
# together and compose an array of components by listing component
# names in $2.
#
# $1 = filename
# $2 = osc component names
#
AC_DEFUN(
    [MCA_OMPI_OSC_MONITORING_GENERATE_TEMPLATES],
    [m4_ifval(
         [$1],
         [AC_CONFIG_COMMANDS(
              [$1],
              [filename="$1"
               components=`echo "$2" | sed -e 's/,/ /g' -e 's/monitoring//'`
               cat <<EOF >$filename
/* $filename
 *
 * This file was generated from ompi/mca/osc/monitoring/configure.m4
 *
 * DO NOT EDIT THIS FILE.
 *
 */
/*
 * Copyright (c) 2017-2018 Inria.  All rights reserved.
 * \$COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * \$HEADER$
 */

#ifndef MCA_OSC_MONITORING_GEN_TEMPLATE_H
#define MCA_OSC_MONITORING_GEN_TEMPLATE_H

#include <ompi_config.h>
#include <ompi/mca/osc/osc.h>
#include <ompi/mca/osc/monitoring/osc_monitoring_template.h>

/************************************************************/
/* Include template generating macros and inlined functions */

EOF
               # Generate each case in order to register the proper template functions
               for comp in $components
               do
                   echo "OSC_MONITORING_MODULE_TEMPLATE_GENERATE(${comp})" >>$filename
               done
               cat <<EOF >>$filename

/************************************************************/

typedef struct {
    const char * name;
    ompi_osc_base_module_t * (*fct) (ompi_osc_base_module_t *);
} osc_monitoring_components_list_t;

static const osc_monitoring_components_list_t osc_monitoring_components_list[[]] = {
EOF
               for comp in $components
               do
                   echo "    { .name = \"${comp}\", .fct = OSC_MONITORING_SET_TEMPLATE_FCT_NAME(${comp}) }," >>$filename
               done
               cat <<EOF >>$filename
    { .name = NULL, .fct = NULL }
};

#endif /* MCA_OSC_MONITORING_GEN_TEMPLATE_H */
EOF
               unset filename components
              ])
         ])dnl
    ])dnl

# MCA_ompi_osc_monitoring_CONFIG()
# ------------------------------------------------
AC_DEFUN(
    [MCA_ompi_osc_monitoring_CONFIG],
    [AC_CONFIG_FILES([ompi/mca/osc/monitoring/Makefile])

     AS_IF([test "$MCA_BUILD_ompi_common_monitoring_DSO_TRUE" = ''],
           [$1],
           [$2])

     MCA_OMPI_OSC_MONITORING_GENERATE_TEMPLATES(
         [ompi/mca/osc/monitoring/osc_monitoring_template_gen.h],
         [mca_ompi_osc_m4_config_component_list, mca_ompi_osc_no_config_component_list])dnl
    ])dnl
