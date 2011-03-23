#-------------------------------------------------------------------------
# vform.tcl
#
#-------------------------------------------------------------------------

#-------------------------------------------------------------------------
#
proc vform_create {w obj} {
    global visual

    # create form
    scrollbar $w.xscroll -orient horizontal -command "$w.form xview" \
	    -width 12
    pack $w.xscroll -fill x -side bottom

    scrollbar $w.yscroll -command "$w.form yview" -width 12
    pack $w.yscroll -side right -fill y

    text $w.form -relief groove -borderwidth 2 -wrap none \
	    -yscrollcommand "$w.yscroll set" \
	    -xscrollcommand "$w.xscroll set" \
	    -cursor {arrow} \
	    -state disabled -font $visual(TEXT_FONT)
    pack $w.form -fill both -expand 1
    bind $w.form <Destroy> "vform_cleanup $w; vform destroy $obj"
    bindtags $w.form "$w.form all"

    # local variables
    global $w.form
    set $w.form(field_count) 0
    set $w.form(entry_count) 0
    set $w.form(current_entry) 0
}

#----------------------------------------------------------------------
proc vform_destroy {w} {
    if {[winfo exist $w.form] == 1} {
	destroy $w.f $w.xscroll $w.yscroll
    }
}

#----------------------------------------------------------------------
proc vform_cleanup {w} {
    global $w.form
    catch {unset $w.form}
}

#----------------------------------------------------------------------
# clear the form
#
proc vform_clear {w} {
    $w.form configure -state normal
    $w.form delete 1.0 end
    $w.form configure -state disabled

    global $w.form
    set $w.form(field_count) 0
    set $w.form(entry_count) 0
}

#----------------------------------------------------------------------
# add a form entry
#
proc vform_add_entry {w formobj field_name type val} {
    global $w.form
    set field_num [set $w.form(field_count)]
    vform_insert_entry $w $formobj $field_name $type $val $field_num
}

#----------------------------------------------------------------------
# get pos of a field
#
proc vform_get_field_pos {w field_num} {
    global $w.form
    set n [set $w.form(field_count)]
    if {$field_num >= $n} {
	set pos [$w.form index end]
    } else {
	set entry_num [set $w.form(entry$field_num)]
	set pos [$w.form index "$w.form.entry$entry_num"]
	set pos [$w.form index "$pos linestart"]
    }
    return $pos
}

#----------------------------------------------------------------------
# insert a form entry
#
proc vform_insert_entry {w formobj field_name type val field_num} {

    if {$field_num == 0} {
	set pos 1.0
    } else {
	set pos [vform_get_field_pos $w $field_num]
    }

    # create entry window
    global $w.form
    set e_num [set $w.form(entry_count)]

    set entrypath $w.form.entry$e_num
    set entryvar $w.form(val$e_num)
    set $entryvar $val
    set num_fields [set $w.form(field_count)]

    for {set i [expr $num_fields-1]} {$i > $field_num} {incr i -1} {
	set $w.form(entry[expr $i+1]) [set $w.form(entry$i)]
    }
    set $w.form(entry$field_num) $e_num

    $w.form configure -state normal
    $w.form insert $pos "$field_name ($type): \n"

    entry $entrypath -relief sunken -textvariable $entryvar \
	    -width 50
    bindtags $entrypath "$entrypath Entry all"
    $w.form window create "$pos lineend -1c" -window $entrypath \
	    -padx 2 -pady 2
    incr $w.form(entry_count)
    incr $w.form(field_count)

    $w.form configure -state disabled

    # bindings
    bind $entrypath <FocusOut> "vform filter $formobj $type $entryvar"
    bind $entrypath <FocusIn> "vform_focus_entry $w $e_num"
}

proc vform_focus_entry {w entry_num} {
    global $w.form
    set $w.form(current_entry) $entry_num
}

#----------------------------------------------------------------------
# remove a form entry
#
proc vform_remove_entry {w formobj field_num} {

    set pos [vform_get_field_pos $w $field_num]
    set posend [$w.form index "$pos + 1 lines linestart"]

    $w.form configure -state normal
    $w.form delete $pos $posend
    $w.form configure -state disabled

    global $w.form
    set n [set $w.form(field_count)]
    for {set i [expr $field_num+1]} {$i < $n} {incr i} {
	set $w.form(entry[expr $i-1]) [set $w.form(entry$i)]
    }
    incr $w.form(field_count) -1
}

#----------------------------------------------------------------------
# get the form data
#
proc vform_get_data {w field_num} {
    global $w.form
    set entry_num [set $w.form(entry$field_num)]
    set entryvar $w.form(val$entry_num)

    set data ""
    catch {set data [set $entryvar]}
    return $data
}

#----------------------------------------------------------------------
# set current field
#
proc vform_set_current_field {w formobj field_num} {
    global $w.form
    catch {set entry_num [set $w.form(entry$field_num)]} error
    if {$error == ""} {
	focus $w.form.entry$entry_num
    }
}

#----------------------------------------------------------------------
# get current field
#
proc vform_get_current_field {w formobj} {
    global $w.form
    set n [set $w.form(field_count)]
    set entry [set $w.form(current_entry)]

    for {set i 0} {$i < $n} {incr i} {
	if {[set $w.form(entry$i)] == $entry} {
	    return $i
	}
    }
    return 0
}

#----------------------------------------------------------------------
# set focus on a field
#
proc vform_set_focus {w formobj field_num} {
    global $w.form
    set n [set $w.form(field_count)]
    if {$field_num < $n} {
	set e_num [set $w.form(entry$field_num)]
	focus $w.form.entry[set $w.form(entry$e_num)]
	vform_focus_entry $w $e_num

	set pos [vform_get_field_pos $w $field_num]
	$w.form yview -pickplace $pos
    }
}

#-------------------------------------------------------------------------
#source vdefaults.tcl
#toplevel .t
#vform_create .t 0x0 
#vform_add_entry .t 0x0 a int 3

