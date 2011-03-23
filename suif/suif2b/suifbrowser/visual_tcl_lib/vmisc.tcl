#----------------------------------------------------------------------
# vmisc.tcl
#
#----------------------------------------------------------------------

#----------------------------------------------------------------------
# display all visualsuif commands

proc vs_help {} {
    foreach i [info commands] {
	if {[string match vs_* $i]} {
	    lappend l $i
	}
    }
    return $l
}

#----------------------------------------------------------------------
# select_file

proc select_file {parent_path text pathname} {
    return [selectfile_dialog $parent_path .selectfile $text $pathname]
}

#----------------------------------------------------------------------
# select_fileset

proc select_fileset {parent_path text fileset} {
    return [selectfileset_dialog $parent_path .selectfileset $text $fileset]
}

#----------------------------------------------------------------------
# display_message

proc display_message {parent mesg} {
    vdialog .vdialog "" $mesg "" 0 Ok
}

#----------------------------------------------------------------------
# display_dialog

proc display_dialog {parent mesg options default} {
    set command "[list vdialog .vdialog {} $mesg {} $default] $options"
    eval $command
}

#----------------------------------------------------------------------
# display_query

proc display_query {parent mesg} {
    return [vquery .vquery {} $mesg {}]
}

#----------------------------------------------------------------------
# post_progress

proc post_progress {parent mesg percent_completed} {
    if {[winfo exist .vprogress] == 0} {
	toplevel .vprogress -class Dialog
	catch {wm transient .vprogress $parent}
	wm title .vprogress "Progress ..."
	wm geometry .vprogress +400+300
	canvas .vprogress.c -width 250 -height 100 ;# -cursor {watch black}
	.vprogress.c create text 125 40 -text "Please wait:\n$mesg"
	.vprogress.c create rectangle 50 70 200 90 -fill Bisque
	set p [expr $percent_completed * 1.5 + 50]
	.vprogress.c create rectangle 50 70 $p 90 -fill red
	pack .vprogress.c -expand 1 -fill both
    } else {
	raise .vprogress
	.vprogress.c itemconfigure 1 -text "Please wait:\n$mesg"
	set p [expr $percent_completed * 1.5 + 50]
	.vprogress.c coords 3 50 70 $p 90
    }
}

#----------------------------------------------------------------------
# unpost_progress

proc unpost_progress {parent} {
    catch {
	# make sure the "parent" is the correct one
	if {[wm transient .vprogress] == $parent} {
	    destroy .vprogress
	}
    }
}







