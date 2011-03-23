#----------------------------------------------------------------------
# vdialog.tcl
#
#----------------------------------------------------------------------

#----------------------------------------------------------------------
# vdialog
#
# This is modified from tk_dialog
#
# This procedure displays a dialog box, waits for a button in the dialog
# to be invoked, then returns the index of the selected button.
#
# Arguments:
# w -		Window to use for dialog top-level.
# title -	Title to display in dialog's decorative frame.
# text -	Message to display in dialog.
# bitmap -	Bitmap to display in dialog (empty string means none).
# default -	Index of button that is to display the default ring
#		(-1 means none).
# args -	One or more strings to display in buttons across the
#		bottom of the dialog box.
proc vdialog {w title text bitmap default args} {

    global visual
    set bg $visual(DIALOG_BACKGROUND)

    # 1. Create the top-level window and divide it into top
    # and bottom parts.

    catch {destroy $w}
    toplevel $w -class Dialog

    wm title $w $title
    wm iconname $w Dialog
    frame $w.top -relief raised -bd 1 -background $bg
    pack $w.top -side top -fill both -expand 1
    frame $w.bot -relief raised -bd 1 -background $bg
    pack $w.bot -side bottom -fill both

    # 2. Fill the top part with bitmap and message.

    message $w.msg -width 5i -text $text \
	    -font $visual(DIALOG_FONT) \
	    -background $bg
    pack $w.msg -in $w.top -side right -expand 1 -fill both -padx 3m -pady 3m
    if {$bitmap != ""} {
	label $w.bitmap -bitmap $bitmap
	pack $w.bitmap -in $w.top -side left -padx 3m -pady 3m
    }

    # 3. Create a row of buttons at the bottom of the dialog.
    set visual(button) -1

    set i 0
    foreach but $args {
	button $w.button$i -text $but -command "set visual(button) $i" \
		-background $bg
	if {$i == $default} {
	    frame $w.default -relief sunken -bd 1 \
		    -background $bg
	    raise $w.button$i $w.default
	    pack $w.default -in $w.bot -side left -expand 1 -padx 2 -pady 2
	    pack $w.button$i -in $w.default -padx 2 -pady 2 \
		    -ipadx 2 -ipady 2
	    bind $w <Return> "$w.button$i flash; set visual(button) $i"
	} else {
	    pack $w.button$i -in $w.bot -side left -expand 1 \
		    -padx 2 -pady 2 -ipadx 2 -ipady 2
	}
	incr i
    }

    # 4. Withdraw the window, then update all the geometry information
    # so we know how big it wants to be, then center the window in the
    # display and de-iconify it.

    wm withdraw $w
    update idletasks
    set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 \
	    - [winfo vrootx [winfo parent $w]]]
    set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 \
	    - [winfo vrooty [winfo parent $w]]]
    wm geom $w +$x+$y
    wm deiconify $w

    # 5. Set a grab and claim the focus too.

    set oldFocus [focus]
    set oldGrab [grab current $w]
    if {$oldGrab != ""} {
	set grabStatus [grab status $oldGrab]
    }
    catch {grab $w}

    tkwait visibility $w
    if {$default >= 0} {
	focus $w.button$default
    } else {
	focus $w
    }

    # 6. Wait for the user to respond, then restore the focus and
    # return the index of the selected button.  Restore the focus
    # before deleting the window, since otherwise the window manager
    # may take the focus away so we can't redirect it.  Finally,
    # restore any grab that was in effect.

    tkwait variable visual(button)
    catch {focus $oldFocus}
    destroy $w
    if {$oldGrab != ""} {
	if {$grabStatus == "global"} {
	    grab -global $oldGrab
	} else {
	    grab $oldGrab
	}
    }
    update
    return $visual(button)
}

#----------------------------------------------------------------------
# vquery
#
proc vquery {w title text bitmap} {

    global visual
    set bg $visual(DIALOG_BACKGROUND)

    # 1. Create the top-level window and divide it into top
    # and bottom parts.

    catch {destroy $w}
    toplevel $w -class Dialog

    wm title $w $title
    wm iconname $w Dialog
    frame $w.top -relief raised -bd 1 -background $bg
    pack $w.top -side top -fill both -expand 1
    frame $w.bot -relief raised -bd 1 -background $bg
    pack $w.bot -side bottom -fill both

    # 2. Fill the top part with bitmap and message.

    message $w.msg -width 5i -text $text \
	    -font $visual(DIALOG_FONT) \
	    -background $bg
    pack $w.msg -in $w.top -side right -expand 1 -fill both -padx 3m -pady 3m
    if {$bitmap != ""} {
	label $w.bitmap -bitmap $bitmap
	pack $w.bitmap -in $w.top -side left -padx 3m -pady 3m
    }

    # 3. Create a entry
    entry $w.entry
    pack $w.entry -fill both -padx 5 -pady 5
    bind $w.entry <Key-Return> "global visual; set visual(enter) 1"
    bind $w.entry <Key-Escape> "global visual; set visual(enter) 0"

    # 4. Withdraw the window, then update all the geometry information
    # so we know how big it wants to be, then center the window in the
    # display and de-iconify it.

    wm withdraw $w
    update idletasks
    set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 \
	    - [winfo vrootx [winfo parent $w]]]
    set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 \
	    - [winfo vrooty [winfo parent $w]]]
    wm geom $w +$x+$y
    wm deiconify $w

    # 5. Set a grab and claim the focus too.

    set oldFocus [focus]
    set oldGrab [grab current $w]
    if {$oldGrab != ""} {
	set grabStatus [grab status $oldGrab]
    }
    catch {grab $w}

    tkwait visibility $w
    focus $w.entry

    # 6. Wait for the user to respond, then restore the focus

    tkwait variable visual(enter)
    set result [$w.entry get]

    catch {focus $oldFocus}
    destroy $w
    if {$oldGrab != ""} {
	if {$grabStatus == "global"} {
	    grab -global $oldGrab
	} else {
	    grab $oldGrab
	}
    }
    update
    return $result
}
