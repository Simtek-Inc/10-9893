# Created by Microsemi Libero Software 11.9.1.0
# Fri Apr 05 08:41:40 2019

# (OPEN DESIGN)

open_design "tl755129.adb"

# set default back-annotation base-name
set_defvar "BA_NAME" "tl755129_ba"
set_defvar "IDE_DESIGNERVIEW_NAME" {Impl1}
set_defvar "IDE_DESIGNERVIEW_COUNT" "1"
set_defvar "IDE_DESIGNERVIEW_REV0" {Impl1}
set_defvar "IDE_DESIGNERVIEW_REVNUM0" "1"
set_defvar "IDE_DESIGNERVIEW_ROOTDIR" {W:\W218000\W218234\INSTRUMENTS\8665\Electrical\Engineering\Source_Code\FPGA\i75_5129\designer}
set_defvar "IDE_DESIGNERVIEW_LASTREV" "1"


# import of input files
import_source  \
-format "edif" -edif_flavor "GENERIC" -netlist_naming "VHDL" {../../synthesis/tl755129.edn} -merge_physical "yes" -merge_timing "yes"
compile
report -type "status" {tl755129_compile_report.txt}
report -type "pin" -listby "name" {tl755129_report_pin_byname.txt}
report -type "pin" -listby "number" {tl755129_report_pin_bynumber.txt}

save_design
