#----------------------------------------------------------------------
# vtext.tcl
#
# Internals:
# Tags:
#   bold   - for displaying boldface text
#   italic - for displaying italic text
#   selected - currently selected text
#   search - text found by a textual search
#
# variables
#   $w.text(ind_count) - number of indicator windows
#
#----------------------------------------------------------------------

#----------------------------------------------------------------------
# vtext_create
#
# create a text widget
#
proc vtext_create {w obj} {
    # create text widget
    global visual
    scrollbar $w.xscroll -orient horizontal -command "$w.text xview" \
	    -width 12
    pack $w.xscroll -fill x -side bottom
    scrollbar $w.yscroll -command "vtext_scrollbar_view $w" \
	    -width 12
    pack $w.yscroll -side right -fill y
    text $w.text -relief raised -borderwidth 2 -width $visual(TEXT_COLUMNS) \
	    -height $visual(TEXT_ROWS) -wrap none\
	    -yscrollcommand "$w.yscroll set" \
	    -xscrollcommand "$w.xscroll set" \
	    -cursor {arrow} \
	    -state disabled -font $visual(TEXT_FONT)
    pack $w.text -fill both -expand 1
    bindtags $w.text "$w.text all"

    # bindings
    bind $w.text <Enter> "focus %W; vtext_do_preselect $w $obj"
    bind $w.text <Leave> "vtext_preselect_clear $w"
    bind $w.text <Destroy> "vtext_cleanup $w; vtext destroy $obj"
    bind $w.text <Key-Prior> "vtext_pageup $w"
    bind $w.text <Key-Next> "vtext_pagedown $w"
    bind $w.text <Key-Up> "vtext_up $w"
    bind $w.text <Key-Down> "vtext_down $w"
    bind $w.text <Control-g> "vtext_goto_line $w"
    bind $w.text <Control-s> "vtext_search $w"
    bind $w.text <Any-Key> { }
    bind $w.text <B1-Motion> { }
    bind $w.text <B2-Motion> { }
    bind $w.text <Any-Button-1> "vtext_doselect $w $obj {%x} {%y}"
    bind $w.text <Any-Button-2> "vtext_expand $w $obj {%x} {%y}"
    bind $w.text <Double-Button-1> "vtext invoke $obj"
    bind $w.text <Motion> "vtext_do_preselect $w $obj"

    # tags

    $w.text tag configure selected -background $visual(TEXT_SEL_BACKGROUND) \
	    -relief raised -borderwidth 1
    $w.text tag configure preselected -relief raised -borderwidth 1
    $w.text tag configure bold -font $visual(TEXT_BOLD_FONT)
    $w.text tag configure italic -font $visual(TEXT_ITALIC_FONT)
    $w.text tag configure search -background yellow -relief raised
    $w.text tag raise search selected

    # columns
    global $w.text
    set $w.text(cols) 0

    # indicator
    global $w.text
    set $w.text(ind_count) 0

    # popup menu

    menu $w.text.menu -tearoff 0
    $w.text.menu add separator
    $w.text.menu add command -label "Goto line..  " -accelerator "Ctrl+g"\
	    -command "vtext_goto_line $w"
    $w.text.menu add command -label "Search.." -accelerator \
	    "Ctrl+s" \
	    -command "vtext_search $w"
    $w.text.menu add separator
    $w.text.menu add command -label "Help" \
	    -command "vtext_help"
    popup_init $w.text $w.text.menu 3
}

proc vtext_destroy {w} {
    if {[winfo exist $w.text] == 1} {
	destroy $w.text $w.yscroll
	global $w.text
	set columns [set $w.text(cols)]
	for {set i 0} {$i < $columns} {incr i} {
	    destroy $w.col$i
	}
    }
}

proc vtext_cleanup {w} {
    catch {global $w.text; unset $w.text}
}

#----------------------------------------------------------------------
# config
#
#
proc vtext_configure {w config} {
    eval $w.text configure $config
}

#----------------------------------------------------------------------
# indicator
#
# Add an indicator
#
proc vtext_add_ind {w obj row col on} {
    global $w.text
    set indpath "$w.ind[set $w.text(ind_count)]"
    incr $w.text(ind_count)

    canvas $indpath -width 14 -height 14 -borderwidth 0
    bind $indpath <Button-1> "vtext_toggle $w $obj $indpath"
    bindtags $indpath "$indpath all"
    if {$on == 1} {
	$indpath create polygon 1 5 7 11 13 5 -fill red -outline black
    } else {
	$indpath create polygon 5 1 11 7 5 13 -fill red -outline black
    }
    $w.text window create $row.$col -window $indpath -padx 0 -pady 0
}

proc vtext_toggle {w obj indicator} {
    scan [$w.text index $indicator] "%d.%d" row col
    vtext select $obj $row $col 0 
    vtext toggle $obj
}

#----------------------------------------------------------------------
# additional columns
#
proc vtext_col_create {w width} {
    global $w.text
    set num [set $w.text(cols)]
    incr $w.text(cols)

    set col_path $w.col$num
    text $col_path -relief groove -borderwidth 1 -width $width \
	    -cursor {arrow} -state disabled
    pack $col_path -side left -fill y -before $w.text
    bindtags $col_path "$col_path all"

    return $num
}

proc vtext_col_destroy {w num} {
    pack forget $w.col$num
}

proc vtext_col_config {w num config} {
    set col_path $w.col$num
    eval $col_path configure $config
}

proc vtext_col_set_line {w num line text} {
    set col_path $w.col$num
    $col_path configure -state normal
    $col_path delete $line.0 "$line.0 lineend"
    $col_path insert $line.0 $text
    $col_path configure -state disabled
}

proc vtext_col_setminrows {w min_rows} {
    global $w.text
    set columns [set $w.text(cols)]

    set linefeeds "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
    for {set i 0} {$i < $columns} {incr i} {
	scan [$w.col$i index end] "%d.%d" rows dummy

	$w.col$i configure -state normal
	while {$rows < $min_rows} {
	    $w.col$i insert end $linefeeds
	    incr rows 25
	}
	$w.col$i configure -state disabled
    }
}

#----------------------------------------------------------------------
# select object at (x,y) position
#
proc vtext_doselect {w obj x y} {
    scan [$w.text index @$x,$y] "%d.%d" row col
    vtext select $obj $row $col 0
}

#----------------------------------------------------------------------
# select text from $begin to $end
#
proc vtext_select {w beginrow begincol endrow endcol} {
    $w.text tag remove selected 1.0 end
    $w.text tag add selected $beginrow.$begincol $endrow.$endcol
}

#----------------------------------------------------------------------
# select additional text
#
proc vtext_select_add {w beginrow begincol endrow endcol} {
    $w.text tag add selected $beginrow.$begincol $endrow.$endcol
}

proc vtext_select_clear {w} {
    $w.text tag remove selected 1.0 end

    global $w.text
}

#----------------------------------------------------------------------
# expand scope of selection
#
proc vtext_expand {w obj x y} {
    scan [$w.text index @$x,$y] "%d.%d" row col
    vtext select $obj $row $col 1
}

#----------------------------------------------------------------------
# append text
#

proc vtext_insert_file {w file} {
    if {[catch {set fd [open $file r]} error] == 1} {
	puts stdout "Error: $error"
	return -1
    }
    $w.text configure -state normal
    while {![eof $fd]} {
	$w.text insert end [read $fd 1024]
    }
    $w.text configure -state disabled
    close $fd

    scan [$w.text index end] "%d.%d" rows dummy
    vtext_col_setminrows $w $rows

    return 0
}

proc vtext_insert_text {w row col text} {

    $w.text mark set tmp $row.$col
    $w.text configure -state normal
    $w.text insert $row.$col $text
    $w.text configure -state disabled
    set end_pos [$w.text index tmp]

    scan $end_pos "%d.%d" last_row last_col
    set newlines [expr $last_row - $row]
    set linefeeds "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"

    global $w.text; set columns [set $w.text(cols)]
    set index "$row.0"
    for {set i 0} {$i < $columns} {incr i} {
	set col_path $w.col$i
	$col_path configure -state normal

	set n $newlines
	while {$n > 0} {
	    if {$n >= 15} {
		$col_path insert $index $linefeeds
		incr n -15
	    } else {
		$col_path insert $index "\n"
		incr n -1
	    }
	}
	$col_path configure -state disabled
    }

    # return the end of the insertion
    return $end_pos
}

#----------------------------------------------------------------------
# read text
#
proc vtext_read {w begrow begcol endrow endcol} {
    return [$w.text get $begrow.$begcol $endrow.$endcol]
}

#----------------------------------------------------------------------
# delete text
#
proc vtext_delete_text {w begrow begcol endrow endcol} {
    $w.text configure -state normal
    $w.text delete $begrow.$begcol $endrow.$endcol
    $w.text configure -state disabled

    global $w.text; set columns [set $w.text(cols)]
    for {set i 0} {$i < $columns} {incr i} {
	set col_path $w.col$i
	$col_path configure -state normal
	$col_path delete $begrow.0 $endrow.0
	$col_path configure -state disabled
    }
}


#----------------------------------------------------------------------
# clear the text buffer
#
proc vtext_clear {w} {
    $w.text configure -state normal
    $w.text delete 1.0 end
    $w.text configure -state disabled
    global $w.text
    set columns [set $w.text(cols)]
    for {set i 0} {$i < $columns} {incr i} {
	$w.col$i configure -state normal
	$w.col$i delete 1.0 end
	$w.col$i configure -state disabled
    }
    set $w.text(ind_count) 0
}

#----------------------------------------------------------------------
# add a node link
#
proc vtext_tag {w begrow begcol endrow endcol} {
    $w.text tag add link $begrow.$begcol "$endrow.$endcol-1c"
}

#----------------------------------------------------------------------
# get current position
#
proc vtext_get_mark {w} {
    return [$w.text index end]
}


#----------------------------------------------------------------------
# get current position
#
proc vtext_get_top_row {w} {
    set index [$w.text index @0,0]
    scan $index "%d.%d" row col
    return $row
}

#----------------------------------------------------------------------
# view a position
#
proc vtext_view {w row col pickplace} {
    if {$pickplace == 1} {
	$w.text yview -pickplace $row.$col
    } else {
	$w.text yview $row.$col
    }

    global $w.text
    set columns [set $w.text(cols)]
    for {set i 0} {$i < $columns} {incr i} {
	if {$pickplace == 1} {
	    $w.col$i yview -pickplace $row.0
	} else {
	    $w.col$i yview $row.0
	}
    }
}

proc vtext_scrollbar_view args {
    set w [lindex $args 0]
    set params [lrange $args 1 end]
    eval $w.text yview $params
    scan [$w.text index @0,0] "%d.%d" row col

    global $w.text; set columns [set $w.text(cols)]
    for {set i 0} {$i < $columns} {incr i} {
	$w.col$i yview $row.$col
    }
}

#----------------------------------------------------------------------
# cursor movements: page up and down, up and down
#
proc vtext_pageup {w} {
    scan [$w.text index @0,0] "%d.%d" top dummy
    set rows [expr [winfo height $w.text] / 20]
    vtext_view $w [expr $top - $rows + 1] 0 ""
}

proc vtext_pagedown {w} {
    scan [$w.text index @0,0] "%d.%d" top dummy
    set rows [expr [winfo height $w.text] / 20]
    vtext_view $w [expr $top + $rows - 1] 0 ""
}

proc vtext_up {w} {
    scan [$w.text index @0,0] "%d.%d" top dummy
    vtext_view $w [expr $top - 1] 0 ""
}

proc vtext_down {w} {
    scan [$w.text index @0,0] "%d.%d" top dummy
    vtext_view $w [expr $top + 1] 0 ""
}

#----------------------------------------------------------------------
# tags
#
proc vtext_tag_create {w name} {
}

proc vtext_tag_configure {w name s} {
    eval $w.text tag configure $name $s
    $w.text tag raise $name selected
}

proc vtext_tag_clear {w name} {
    catch {$w.text tag delete $name}
}

proc vtext_tag_add {w name beginrow begincol endrow endcol} {
    $w.text tag add $name $beginrow.$begincol $endrow.$endcol
}

#----------------------------------------------------------------------
# attributes
#
proc vtext_set_bold {w begrow begcol endrow endcol} {
    if {$endcol == 0} {
	incr endrow -1
	$w.text tag add bold $begrow.$begcol "$endrow.0 lineend"
    } else {
	$w.text tag add bold $begrow.$begcol $endrow.$endcol
    }
}

proc vtext_set_italic {w begrow begcol endrow endcol} {
    if {$endcol == 0} {
	incr endrow -1
	$w.text tag add italic $begrow.$begcol "$endrow.0 lineend"
    } else {
	$w.text tag add italic $begrow.$begcol $endrow.$endcol
    }
}

#----------------------------------------------------------------------
# popup menu functions
#
proc vtext_help {} {
    set mesg \
"Key Bindings:
<Page Up>. <Up>: scroll up
<Page Down>, <Down>: scroll down

Mouse Bindings:
<Button 1>: Select node
<Button 2>: Expand scope of selection
<Button 3>: Pop-up menu"

    vdialog .vdialog Help $mesg "" 0 Ok 
}

proc vtext_goto_line {w} {
    global visual

    toplevel .gotoline
    wm transient .gotoline [winfo toplevel $w]
    set xpos [expr [winfo rootx $w]+40]
    set ypos [expr [winfo rooty $w]+[winfo height $w]/2]
    wm geometry .gotoline "+$xpos+$ypos"
    wm title .gotoline "Goto line"

    frame .gotoline.line -relief groove
    pack .gotoline.line -fill both -side top
    label .gotoline.line.label -text "Line number: "
    pack .gotoline.line.label -side left -padx 10 -pady 10

    set visual(line_entered) ""
    entry .gotoline.line.entry -relief sunken -bd 2 \
	    -textvariable visual(line_num)
    pack .gotoline.line.entry -side right -padx 10 -pady 10

    button .gotoline.ok -text "Ok" -command \
	    {set visual(line_entered) Ok"}
    pack .gotoline.ok

    bind .gotoline.line.entry <Key-Return> "set visual(line_entered) Ok"
    bind .gotoline.line.entry <Key-Escape> "set visual(line_entered) Cancel"

    set oldFocus [focus]
    grab set .gotoline
    focus .gotoline.line.entry

    tkwait variable visual(line_entered)
    grab release .gotoline
    focus $oldFocus
    destroy .gotoline

    if {$visual(line_entered) == "Ok" && $visual(line_num) != ""} {
	catch {vtext_view $w $visual(line_num) 0 ""}
    }
}

#----------------------------------------------------------------------
# Textual Search
#
proc vtext_search {w} {
    if {[winfo exist $w.search]} {
	raise $w.search
	return;
    }

    toplevel $w.search
    wm transient $w.search [winfo toplevel $w]
    set xpos [expr [winfo rootx $w]+200]
    set ypos [expr [winfo rooty $w]+[winfo height $w]/2]
    wm geometry $w.search "+$xpos+$ypos"
    wm title $w.search "Search"

    global $w.search
    set $w.search(search_text) ""
    set $w.search(search_backwards) 0
    bind $w.search <Destroy> "catch {unset $w.search}"

    # options
    frame $w.search.options -relief groove -bd 3
    pack $w.search.options -side top -fill both -ipadx 5 -ipady 5
    frame $w.search.options.backwards -width 12 -height 12 \
	    -bd 3
    pack $w.search.options.backwards -side left -padx 5 -pady 5
    label $w.search.options.blabel -text "Search backwards"
    pack $w.search.options.blabel -side left

    bind $w.search.options.backwards <Button-1> "vtext_toggle_dir $w"
    vtext_search_update_options $w

    # search text
    frame $w.search.text
    pack $w.search.text -fill both
    label $w.search.text.label -text "Search text: "
    pack $w.search.text.label -side left -padx 10 -pady 10

    entry $w.search.text.entry -relief sunken -bd 2 \
	    -textvariable $w.search(search_text)
    pack $w.search.text.entry -side right -padx 10 -pady 10
    
    bind $w.search.text.entry <Key-Return> "vtext_search_it $w"
    bind $w.search.text.entry <Key-Escape> "destroy $w.search"
    bind $w.search <Control-s> "vtext_search_it $w"

    # buttons
    frame $w.search.but
    pack $w.search.but -fill both
    button $w.search.but.find -text "Find" \
	    -command "vtext_search_it $w"
    pack $w.search.but.find -side left -padx 10 -pady 5
    button $w.search.but.close -text "Close" \
	    -command "destroy $w.search"
    pack $w.search.but.close -side right -padx 10 -pady 5

}

proc vtext_toggle_dir {w} {
    global $w.search
    if {[set $w.search(search_backwards)] == 1} {
	set $w.search(search_backwards) 0
    } else {
	set $w.search(search_backwards) 1
    }
    vtext_search_update_options $w
}

proc vtext_search_update_options {w} {
    global $w.search
    if {[set $w.search(search_backwards)] == 1} {
	$w.search.options.backwards configure -relief sunken
    } else {
	$w.search.options.backwards configure -relief raised
    }
}

proc vtext_search_it {w} {
    global $w.search
    set search_text [set $w.search(search_text)]

    if {[set $w.search(search_backwards)] != 1} {
	if {[$w.text tag ranges search] == ""} {
	    set start_index [$w.text index @0,0]
	} else {
	    set start_index [$w.text index search.last]
	}
	set index [$w.text search -nocase $search_text $start_index]
    } else {
	if {[$w.text tag ranges search] == ""} {
	    set start_index [$w.text index @0,[winfo height $w.text]]
	} else {
	    set start_index [$w.text index search.first]
	}
	set index [$w.text search -nocase -backwards \
		$search_text $start_index]
    }
    if {$index != ""} {
	$w.text yview -pickplace $index
	$w.text tag remove search 1.0 end
	$w.text tag add search $index \
		"$index + [string length $search_text] char"
    } else {
	display_message $w.search "Cannot find text `$search_text'"
    }
}

#----------------------------------------------------------------------
# preselect
#
proc vtext_do_preselect {w obj} {
    scan [$w.text index current] "%d.%d" row col
    vtext preselect $obj $row $col
}

#----------------------------------------------------------------------
proc vtext_preselect {w beginrow begincol endrow endcol} {
    $w.text tag remove preselected 1.0 end
    $w.text tag add preselected $beginrow.$begincol $endrow.$endcol
}

proc vtext_preselect_clear {w} {
    $w.text tag remove preselected 1.0 end
}

#----------------------------------------------------------------------
#source vdefaults.tcl
#source vpopup.tcl
#toplevel .t
#vtext_create .t 0x0
