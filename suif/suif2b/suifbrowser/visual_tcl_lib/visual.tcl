#----------------------------------------------------------------------
# visual.tcl
#
# Visual system initialization script
#
#----------------------------------------------------------------------
# Window classes:
#   Dialog - dialog windows, e.g. file select
#
#
#----------------------------------------------------------------------

set visual_tcl_lib_interface_version 1.0

#----------------------------------------------------------------------
# Check version

# Check tcl version
catch {scan $tcl_version "%d.%d" major minor} error
if {$error == 1 || $major < 7 || ($major == 7 && $minor < 4)} {
    puts stderr "Error: tcl version 7.4 is required to run this program."
    return "Error"
}

# Check tk version
catch {scan $tk_version "%d.%d" major minor} error
if {$error == 1 || $major < 4} {
    puts stderr "Error: tk version 4.0 is required to run this program."
    return "Error"
}

#----------------------------------------------------------------------
set initfiles {
    vpopup.tcl
    vselectfile.tcl
    vdefaults.tcl
    vmisc.tcl
    vtoplevel.tcl
    vtext.tcl
    vform.tcl
    vgraph.tcl
    vframe.tcl
    vmenu.tcl
    vmessage.tcl
    vdialog.tcl
    vlistbox.tcl
    vhtml.tcl
    vbuttonbar.tcl
}

foreach file $initfiles {
    set filename "$env(VISUAL_TCL)/$file"
    if {[file exists $filename] == 0} {
	puts stdout "Error: cannot find '$file'"
    } else {
	if {[catch {source $filename} error] == 1} {
	    puts stdout "Error: reading tcl/tk file '$file': $error"
	}
    }
}

#----------------------------------------------------------------------
# Remove the "." main window
#
wm withdraw .

#----------------------------------------------------------------------
# Done with tcl/tk initilization.
# Return the library version

return $visual_tcl_lib_interface_version
