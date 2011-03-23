#----------------------------------------------------------------------
# vselectfile.tcl
#
#
#----------------------------------------------------------------------
# open file dialog
# returns pathname if ok, otherwise empty string
#
proc selectfile_dialog {parent w text filename} {
    global _button _filename _pathname _search_str

    set _button 0
    if {$filename == ""} {
	if {[lsearch [info globals] _pathname] != -1} {
	    set _filename $_pathname
	} else {
	    set _filename [pwd]
	}
    } else {
	set _filename "$filename"
    }
    set _search_str "*"

    toplevel $w -class Dialog
    if {$parent != ""} {
	wm transient $w $parent
    }
    wm title $w "Select file"
    wm iconname $w "Select file"
    wm minsize $w 100 100
    wm geometry $w 450x360+500+300

    message $w.text -text $text -width 3i -padx 10 -pady 2
    pack $w.text -side top -anchor w

    frame $w.filename
    pack $w.filename -side top -fill both -padx 10 -pady 2

    frame $w.dir
    pack $w.dir -side top -fill both
    frame $w.dir.dirs
    pack $w.dir.dirs -side left -fill both -padx 5 -pady 2 -expand 1
    frame $w.dir.files
    pack $w.dir.files -side right -fill both -padx 5 -pady 2 -expand 1

    frame $w.buttons -bd 2
    pack $w.buttons -side bottom -fill both -padx 2 -pady 2

    # filename

    label $w.filename.label -text "Filename:"
    entry $w.filename.entry -textvariable _filename -relief sunken
    pack $w.filename.label -side left
    pack $w.filename.entry -fill both -expand 1

    # directories

    label $w.dir.dirs.label -text "Directories"
    pack $w.dir.dirs.label -side top
    scrollbar $w.dir.dirs.yscroll -command "$w.dir.dirs.list yview" \
	    -width 12
    pack $w.dir.dirs.yscroll -side right -fill y
    listbox $w.dir.dirs.list -relief sunken -selectmode browse\
	    -yscrollcommand "$w.dir.dirs.yscroll set"
    pack $w.dir.dirs.list -fill both -expand 1

    # files

    label $w.dir.files.label -text "Files"
    pack $w.dir.files.label -side top
    scrollbar $w.dir.files.yscroll -command "$w.dir.files.list yview" \
	    -width 12
    pack $w.dir.files.yscroll -side right -fill y
    listbox $w.dir.files.list -relief sunken \
	    -yscrollcommand "$w.dir.files.yscroll set"
    pack $w.dir.files.list -fill both -expand 1

    # buttons

    button $w.buttons.ok -text "Ok" -width 10 \
	    -command {set _button Ok}
    pack $w.buttons.ok -side left -expand 1 -padx 4 -pady 4
    button $w.buttons.cancel -text "Cancel" -width 10 \
	    -command {set _button Cancel}

    pack $w.buttons.cancel -side left -expand 1 -padx 4 -pady 4

    # bindings

    bindtags $w.filename.entry "$w.filename.entry Entry all"
    bind $w.filename.entry <Return> "selectfile_invoke $w"
    bind $w.filename.entry <Tab> ""
    bind $w.dir.files.list <Double-Button-1> "selectfile_selectfile $w %y"
    bind $w.dir.files.list <Any-Button-1> "selectfile_clickfile $w %y"
    bind $w.dir.files.list <B1-Motion> "selectfile_clickfile $w %y"
    bind $w.dir.dirs.list <Double-Button-1> "selectfile_selectdir $w %y"
    bind $w.dir.dirs.list <Any-Button-1> "selectfile_clickdir $w %y"
    bind $w.dir.dirs.list <B1-Motion> "selectfile_clickdir $w %y"


    # init file lists
    selectfile_update $w

    set oldFocus [focus]
    grab set $w
    focus $w
    
    tkwait variable _button
    destroy $w

    focus $oldFocus
    if {$_button == "Ok"} {
	return $_filename
    } else {
	return ""
    }
}

proc selectfile_clickfile {w y} {
    global _filename _pathname

    set index [$w.dir.files.list nearest $y]
    if {$index != ""} {
	set _filename "$_pathname[$w.dir.files.list get $index]"
	$w.filename.entry xview end
	# $w.dir.files.list select set $index $index
    }
}

#----------------------------------------------------------------------
proc selectfile_clickdir {w y} {
    global _filename _pathname

    set index [$w.dir.dirs.list nearest $y]
    if {$index != ""} {
	set _filename "$_pathname[$w.dir.dirs.list get $index]"
	$w.filename.entry xview end
	$w.dir.dirs.list select set $index
    }
}

#----------------------------------------------------------------------
proc selectfile_selectfile {w y} {
    global _filename _pathname _button

    set index [$w.dir.files.list nearest $y]
    set _filename "$_pathname[$w.dir.files.list get $index]"
    set _button Ok
}

#----------------------------------------------------------------------
proc selectfile_selectdir {w y} {
    global _filename _search_str _pathname

    set index [$w.dir.dirs.list nearest $y]
    set selection "[$w.dir.dirs.list get $index]"
    if { $selection == "./" } {
      set _filename "$_pathname"
    } elseif { $selection == "../" } {
      set _filename  [string trimright $_pathname /]
      set last "[string last / $_filename ]"
      if { $last != "-1" }  {
        set _filename [string range $_filename 0 [expr $last - 1 ] ]
      } 
    } else {
      set _filename "$_pathname[$w.dir.dirs.list get $index]"
    }
    selectfile_update $w
}

proc selectfile_invoke {w} {
    global _filename _button _search_str

    set t [file tail $_filename]
    if {[string first "*" $t] != -1} {
	set _search_str $t
	selectfile_update $w
    } else {
	if {[file isdirectory $_filename] == 1} {
	    selectfile_update $w
	} else {
	    set _button Ok
	}
    }
}

#----------------------------------------------------------------------
proc selectfile_update {w} {

    global _filename _pathname _search_str
    if {[file isdirectory $_filename] == 1} {
	set _pathname $_filename
    } else {
	set _pathname "[file dirname $_filename]"
    }
    if {[regexp {^.*/$} $_pathname] != 1} {
	set _pathname $_pathname/
    }
    set _filename "$_pathname$_search_str"
    $w.filename.entry xview end

    get_dirs_files $_pathname $_search_str d f
    $w.dir.dirs.list delete 0 end
    $w.dir.files.list delete 0 end
    
    foreach i $d {
	$w.dir.dirs.list insert end "$i/"
    }
    foreach i $f {
	$w.dir.files.list insert end $i
    }
   
}

#----------------------------------------------------------------------
# get the list of subdirectories and files
proc get_dirs_files {path search_str dirs files} {
    upvar $dirs d
    upvar $files f
    set d {. ..}
    set f {}
    if {[catch {set flist [glob $path$search_str]}] == 1} {
	return
    }
    set flist [lsort $flist]
    foreach filename $flist {
	# [split $flist "\n"]
	# set filename $path/$i
	set i [file tail $filename]
	if {[file readable $filename] == 1} {
	    if 0 {
		set type [file type $path/$i]
		if {$type == "directory"} {
		    lappend d "$i/"
		} elseif {$type == "file"} {
		    lappend f $i
		} elseif {$type == "link"} {
		    lappend f $i
		}
	    }
	    if {[file isdirectory $filename] == 1} {
		lappend d $i
	    } elseif {[file isfile $filename] == 1} {
		lappend f $i
	    }
	}
    }
}

#----------------------------------------------------------------------
proc selectfileset_dialog {parent w text fileset} {
    global _button _filename _pathname _search_str _fileset

    set _button 0
    if {[lsearch [info globals] _pathname] != -1} {
	set _filename $_pathname
    } else {
	set _filename [pwd]
    }
    set _search_str "*"
    set _fileset $fileset

    toplevel $w -class Dialog
    if {$parent != ""} {
	wm transient $w $parent
    }
    wm title $w "Select file set"
    wm iconname $w "Select file"
    wm minsize $w 100 100
    wm geometry $w 450x500+500+200

    message $w.text -text $text -width 3i -padx 10 -pady 2
    pack $w.text -side top -anchor w
    frame $w.filename
    pack $w.filename -side top -fill both -padx 10 -pady 2

    frame $w.dir
    pack $w.dir -side top -fill both
    frame $w.dir.dirs
    pack $w.dir.dirs -side left -fill both -padx 5 -pady 2 -expand 1
    frame $w.dir.files
    pack $w.dir.files -side right -fill both -padx 5 -pady 2 -expand 1

    label $w.fse_label -text "File Set Entries:"
    pack $w.fse_label -anchor w

    frame $w.fs
    pack $w.fs -fill both -padx 2 -pady 2
    frame $w.fs.fse
    pack $w.fs.fse -side left -expand 1 -fill both

    #frame $w.buttons -bd 2
    #pack $w.buttons -fill both -padx 2 -pady 2

    # filename

    label $w.filename.label -text "Filename:"
    entry $w.filename.entry -textvariable _filename -relief sunken
    pack $w.filename.label -side left
    pack $w.filename.entry -fill both -expand 1

    # directories

    label $w.dir.dirs.label -text "Directories"
    pack $w.dir.dirs.label -side top
    scrollbar $w.dir.dirs.yscroll -command "$w.dir.dirs.list yview" \
	    -width 12
    pack $w.dir.dirs.yscroll -side right -fill y
    listbox $w.dir.dirs.list -relief sunken -selectmode browse\
	    -yscrollcommand "$w.dir.dirs.yscroll set"
    pack $w.dir.dirs.list -fill both -expand 1

    # files

    label $w.dir.files.label -text "Files"
    pack $w.dir.files.label -side top
    scrollbar $w.dir.files.yscroll -command "$w.dir.files.list yview" \
	    -width 12
    pack $w.dir.files.yscroll -side right -fill y
    listbox $w.dir.files.list -relief sunken -selectmode extended\
	    -yscrollcommand "$w.dir.files.yscroll set"
    pack $w.dir.files.list -fill both -expand 1

    # fileset entries
    scrollbar $w.fs.fse.yscroll -command "$w.fs.fse.l yview" \
	    -width 12
    pack $w.fs.fse.yscroll -side right -fill y
    listbox $w.fs.fse.l -relief sunken -selectmode extended\
	    -yscrollcommand "$w.fs.fse.yscroll set"
    pack $w.fs.fse.l -fill both -expand 1
    button $w.fs.add -text "Add" -state disabled -command \
	    "selectfileset_click_add $w"
    pack $w.fs.add -fill x
    button $w.fs.remove -text "Remove" -state disabled -command \
	    "selectfileset_click_remove $w"
    pack $w.fs.remove -fill x

    foreach file $_fileset {
	$w.fs.fse.l insert end $file
    }

    # buttons

    button $w.fs.ok -text "Ok" -width 10 -state disabled\
	    -command {set _button Ok}
    pack $w.fs.ok -fill x
    button $w.fs.cancel -text "Cancel" -width 10 \
	    -command {set _button Cancel}
    pack $w.fs.cancel -fill x

    # bindings

    bindtags $w.filename.entry "$w.filename.entry Entry all"

    bind $w.filename.entry <Return> "selectfileset_invoke $w"
    bind $w.filename.entry <Tab> ""
    bind $w.dir.files.list <Any-Button-1> \
	    "selectfile_clickfile $w %y"
    bind $w.dir.files.list <Any-B1-Motion> \
	    "selectfile_clickfile $w %y;"
    bind $w.dir.dirs.list <Double-Button-1> \
	    "selectfile_selectdir $w %y"
    bind $w.dir.dirs.list <Any-Button-1> \
	    "selectfile_clickdir $w %y"
    bind $w.dir.files.list <Double-Button-1> \
	    "selectfileset_selectfile $w %y"
    bind $w.dir.dirs.list <Any-B1-Motion> "selectfile_clickdir $w %y"

    bind $w.dir.files.list <Any-Button-1> "+selectfileset_toadd $w"
    bind $w.dir.dirs.list <Any-Button-1> "+selectfileset_toadd $w"
    bind $w.fs.fse.l <Any-Button-1> "+selectfileset_toremove $w"
    bind $w.filename.entry <Any-Key> "+selectfileset_toadd $w"

    # init file lists
    selectfile_update $w

    set oldFocus [focus]
    grab set $w
    focus $w
    
    tkwait variable _button
    destroy $w

    focus $oldFocus
    update
    if {$_button == "Ok"} {
	return $_fileset
    } else {
	return ""
    }
}

proc selectfileset_toadd {w} {
    $w.fs.add configure -state normal
    $w.fs.remove configure -state disabled
    $w.fs.fse.l selection clear 0 end
}

proc selectfileset_toremove {w} {
    $w.fs.add configure -state disabled
    $w.fs.remove configure -state normal
    $w.dir.dirs.list selection clear 0 end
    $w.dir.files.list selection clear 0 end
}

proc selectfileset_selectfile {w y} {
    global _pathname _fileset

    set index [$w.dir.files.list nearest $y]
    set filename "$_pathname[$w.dir.files.list get $index]"
    selectfileset_addfile $w $filename
}

proc selectfileset_addfile {w filename} {
    global _fileset
    set index [lsearch $_fileset $filename]
    if {$index < 0} {
	$w.fs.fse.l insert end $filename
	lappend _fileset $filename
	$w.fs.ok configure -state normal
	$w.fs.fse.l select set end
    } else {
	$w.fs.fse.l select set $index
    }
}

proc selectfileset_removefile {w filename} {
    global _fileset
    set index [lsearch $_fileset $filename]
    if {$index >= 0} {
	set _fileset "[lrange $_fileset 0 [expr $index-1]] [lrange \
		$_fileset [expr $index+1] end]"
	set entries [$w.fs.fse.l get 0 end]
	set i [lsearch $entries $filename]
	if {$i >= 0} {
	    $w.fs.fse.l delete $i
	    if {[llength $_fileset] < 1} {
		$w.fs.ok configure -state disabled
	    }
	}
    }
}

proc selectfileset_click_add {w} {
    global _pathname
    foreach index [$w.dir.files.list curselection] {
	selectfileset_addfile $w "$_pathname[$w.dir.files.list get $index]"
    }
    $w.dir.files.list select clear 0 end
    selectfileset_toremove $w
}

proc selectfileset_click_remove {w} {
    set indices [$w.fs.fse.l curselection]
    set files {}
    foreach i $indices {
	lappend files [$w.fs.fse.l get $i]
    }

    foreach f $files {
	selectfileset_removefile $w $f
    }   
}

proc selectfileset_invoke {w} {
    global _filename _button _search_str

    set t [file tail $_filename]
    if {[string first "*" $t] != -1} {
	set _search_str $t
	selectfile_update $w
    } else {
	if {[file isdirectory $_filename] == 1} {
	    selectfile_update $w
	} elseif {[file isfile $_filename] == 1} {
	    selectfileset_addfile $w $_filename
	}
    }
}

#----------------------------------------------------------------------
# Test code
#
#set a [selectfile_dialog .t "Open file:" [pwd]]
#puts stdout "file = $a"

#set a [selectfileset_dialog .t "Open.." {a b c}]
#puts stdout "fileset = $a"
