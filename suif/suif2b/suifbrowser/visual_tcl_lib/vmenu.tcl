# vmenu.tcl
#----------------------------------------------------------------------
# Internals:
#
# Menu names:
#   Menu "a/b/c" has widget path "$w.mbar.menu.a.b.c"
#   where $w is the base path
#
# Global variables:
#   varcount  - variable count
#   var       - variable array
#
#----------------------------------------------------------------------
proc vmenu_create {w obj} {
    frame $w.mbar -relief raised -bd 3
    pack $w.mbar -side top -fill x

    bind $w.mbar <Destroy> "vmenu_cleanup $w; vmenu destroy $obj"

    global $w.m
    set $w.m(varcount) 0
}

proc vmenu_destroy {w} {
    if {[winfo exist $w.mbar] == 1} {
	destroy $w.mbar
    }
}

# clean up global variables

proc vmenu_cleanup {w} {
    global $w.m
    unset $w.m
}

#----------------------------------------------------------------------
# vmenu_add_menu_int [INTERNAL FUNCTION]
#
# Add a menu, format of menu_name is "a/b/c.", e.g. "File/Close"
# return the menu path
#
proc vmenu_add_menu_int {w obj menu_str} {
    set menupath $w.mbar
    set level 0
    while {$menu_str != ""} {
	set i [string first / $menu_str]
	if {$i == -1} {
	    set name $menu_str
	    set menu_str ""
	} else {
	    set name [string range $menu_str 0 [expr $i - 1]]
	    set menu_str [string range $menu_str [expr $i + 1] end]
	}
	if {$name == ""} {
	    continue
	}

	# Construct a path name for the menu
	# replace " " with "+"s
	#
	regsub -all " " $name "+" p
	set menupath "$menupath.[string tolower $p]"
	if {[winfo exists $menupath] == 0} {
	    if {$level == 0} {
		menubutton $menupath -text "$name" -menu $menupath.menu
		bindtags $menupath "$menupath Menubutton all"
		pack $menupath -side left -ipadx 5
	    } else {
		menu $menupath
		$parent add cascade -label "$name" -menu $menupath
	    }
	}

	if {$level == 0} {
	    set menupath $menupath.menu
	    if {[winfo exists $menupath] == 0} {
		menu $menupath
	    }
	}
	set parent $menupath
	incr level
    }
    return $menupath
}

#----------------------------------------------------------------------
# vmenu_add_item [INTERNAL FUNCTION]
#
# Add an item to a menu
#
# w - path of menubar
# item - type of item, e.g. command, radio
# obj - the C++ vmenu object shadow
# menu - menu path, e.g. "File/Options"
# accel_key - acceleration key, e.g. "n" for control-n
# client_data - data to be passed when a command is invoked
#
proc vmenu_add_item {w item obj menu accel_key title client_data} {

    set menu_path [vmenu_add_menu_int $w $obj $menu]
    if {$menu_path != ""} {
	$menu_path add $item -label "$title "\
		-command "vmenu invoke $obj $client_data"
	if {$accel_key != ""} {
	    set accel_string "Ctrl+$accel_key"
	    $menu_path entryconfigure last -accelerator $accel_string
	    set window [winfo toplevel $w]
	    bind $window.frame0.text <Control-$accel_key> \
		    "vmenu invoke $obj $client_data"
	    bind $menu_path <Destroy> \
		    "+vmenu destroy_item $client_data"
	}
    }
    return $menu_path
}

#----------------------------------------------------------------------
# vmenu_add_menu
#
proc vmenu_add_menu {w obj menu client_data} {
    set menupath [vmenu_add_menu_int $w $obj $menu]
    if {$menupath != ""} {
	$menupath configure -postcommand "vmenu invoke $obj $client_data"
	bind $menupath <Destroy> "+vmenu destroy_item $client_data"
    }
}

#----------------------------------------------------------------------
# vmenu_add_separator
#
proc vmenu_add_separator {w obj menu} {
    set menu_path [vmenu_add_menu_int $w $obj $menu]
    if {$menu_path != ""} {
	$menu_path add separator
    }
}

#----------------------------------------------------------------------
# vmenu_add_command
#
# Add a command entry
#
proc vmenu_add_command {w obj menu accel_key title client_data} {
    vmenu_add_item $w command $obj $menu $accel_key $title $client_data
}

#----------------------------------------------------------------------
# vmenu_add_check
#
# Add a check entry
#
proc vmenu_add_check {w obj menu accel_key title client_data on} {
    set menu_path [vmenu_add_item $w check $obj $menu $accel_key $title \
	    $client_data]
    global $w.m
    set v "$w.m(var([set $w.m(varcount)]))"
    incr $w.m(varcount)
    set $v $on
    $menu_path entryconfigure last -variable $v
}

#----------------------------------------------------------------------
# vmenu_add_radio
#
# Add a radiobutton entry
#
proc vmenu_add_radio {w obj menu accel_key title client_data on} {
    set menu_path [vmenu_add_item $w radio $obj $menu $accel_key $title \
	    $client_data]

    # set id to be the item number
    # set variable v to be "$menu_path(radio_var)", so all radio items
    # on the same menu have the same variable

    set id [$menu_path index last]
    set v ${menu_path}(radio_var)
    global ${menu_path}

    $menu_path entryconfigure last -variable $v -value $id
    if {$on == 1} {
	set $v $id
    }
}

#----------------------------------------------------------------------
# vmenu_get_menu_path
#
proc vmenu_get_menu_path {w menu_str} {
    set menupath $w.mbar
    set level 0
    while {$menu_str != ""} {
	set i [string first / $menu_str]
	if {$i == -1} {
	    set s $menu_str
	    set menu_str ""
	} else {
	    set s [string range $menu_str 0 [expr $i - 1]]
	    set menu_str [string range $menu_str [expr $i + 1] end]
	}
	set menupath "$menupath.[string tolower $s]"
	if {$level == 0} {
	    set menupath "$menupath.menu"
	}
	incr level
    }
    if {[winfo exist $menupath] == 1} {
	return $menupath
    } else {
	return ""
    }
}

#----------------------------------------------------------------------
# vmenu_invoke
#
# invoke an item
#
proc vmenu_invoke {w menu item} {
    set menupath [vmenu_get_menu_path $w $menu]
    if {$menupath != ""} {
    	$menupath invoke "${item}*"
    }
}

#----------------------------------------------------------------------
# vmenu_clear
#
# clear all entries in the specified menu
#
proc vmenu_clear {w menu} {
    set menupath [vmenu_get_menu_path $w $menu]

    if {$menupath != ""} {
	catch {$menupath delete 0 end}
	foreach child [winfo children $menupath] {
	    destroy $child
	}
    }
}

#----------------------------------------------------------------------
# vmenu_remove
#
# Remove the specified submenu, if the menu specified is "", then all
# all menu items are removed.
#
proc vmenu_remove {w menu} {
    if {$menu != ""} {
	set menupath [vmenu_get_menu_path $w $menu]
	if {$menupath != ""} {
	    destroy $menupath
	}
    } else {
	# remove all
	foreach child [winfo children $w.mbar] {
	    destroy $child
	}
    }
}
