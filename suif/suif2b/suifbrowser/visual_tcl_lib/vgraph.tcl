#-------------------------------------------------------------------------
# vgraph.tcl
#
# This file implements a simple graph widget using a canvas.
#
# Internals:
# canvas tags:
#   boxes - all rectangle items of the nodes
#   texts - all text items
#   nodes - all nodes
#   edges - all line items of the edges
#   selected - selected item
#   band - selection rubber band (a single rectangle item)
#
# local variables:
#   x, y - origin position for dragging items & popup menu
#   textid(id) - maps from box id of a node, to its text id
#   boxid(id) - maps from text id of a node, to its box id
#   node1(id) - maps from a edge id, to its first node
#   node2(id) - maps from a edge id, to its second node
#   edge1(id) - maps from the box id of a node, to a list of edge ids that
#               has the node as its first node
#   edge2(id) - maps from the box id of a node, to a list of edge ids that
#               has the node as its second node
#   cur_node  - current node that the cursor is pointing to
#   cur_edge  - current edge that the cursor is pointing to
#
#----------------------------------------------------------------------

proc vgraph_create {w obj} {
    message $w.m -relief sunken -bd 2 -width 500
    pack $w.m -side bottom -fill x
   
    scrollbar $w.xscroll -orient horizontal\
	    -command "$w.c xview" -width 12
    pack $w.xscroll -side bottom -fill x
    scrollbar $w.yscroll -command "$w.c yview" -width 12
    pack $w.yscroll -side right -fill y

    global visual $w.c
    canvas $w.c -height $visual(GRAPH_WIDTH) -width $visual(GRAPH_HEIGHT)\
	    -relief raised -bd 2 -scrollregion "-2000 -2000 2000 2000"\
	    -xscrollcommand "$w.xscroll set" -yscrollcommand "$w.yscroll set"
    pack $w.c -fill both -expand 1

    # bindings
    bind $w.c <Enter> "focus %W"

    # rubber band
    bind $w.c <Button-1> "vgraph_band_start $w %x %y 1"
    bind $w.c <Shift-Button-1> "vgraph_band_start $w %x %y 0"
    bind $w.c <Any-B1-Motion> "vgraph_band_drag $w %x %y"
    bind $w.c <Any-ButtonRelease-1> "vgraph_band_drag_end $w %x %y"

    # node bindings
    
    $w.c bind nodes <Button-1> "vgraph_click_select $w $obj"
    $w.c bind nodes <Button-2> "vgraph_drag_start $w %x %y"
    $w.c bind nodes <B2-Motion> "vgraph_drag $w %x %y"
    $w.c bind nodes <ButtonRelease-2> "vgraph_drag_end $w %x %y"
    $w.c bind nodes <Double-Button-1> \
	    "global $w.c; set $w.c(state) disabled;\
	    vgraph invoke $obj"
    $w.c bind nodes <Shift-Button-1> \
	    "vgraph_toggle_select $w"
    $w.c bind texts <Enter> "vgraph_enter_node $w %x %y"
    $w.c bind texts <Leave> "vgraph_leave_node $w %x %y"

    # edge bindings

    $w.c bind edges <Enter> "vgraph_enter_edge $w %x %y"
    $w.c bind edges <Leave> "vgraph_leave_edge $w %x %y"

    # popup menu

    menu $w.c.menu -tearoff 0
    $w.c.menu add separator
    $w.c.menu add command -label "Zoom In " \
	    -command "vgraph_zoomin $w"
    $w.c.menu add command -label "Zoom Out" \
	    -command "vgraph_zoomout $w"
    $w.c.menu add command -label "Zoom to Fit" \
	    -command "vgraph_zoomtofit $w"
    #$w.c.menu add command -label "Center" -command "vgraph_center $w"
    $w.c.menu add cascade -label "Layout" \
	    -menu $w.c.menu.layout
    $w.c.menu add separator
    $w.c.menu add command -label "Print" \
	    -command "vgraph_print $w"
    $w.c.menu add command -label "Print to file.." \
	    -command "vgraph_print_file $w"
    $w.c.menu add command -label "Export to .dot file.." \
	    -command "vgraph export_dot $obj"

    # help menu
    $w.c.menu add command -label "Help" \
	    -command "vgraph_help"

    menu $w.c.menu.layout
    foreach i {Tree Dot} {
	$w.c.menu.layout add command -label $i \
		-command "vgraph_layout $w $obj [string tolower $i]"
    }

    popup_init $w.c $w.c.menu 3
    
    bind $w.c <ButtonPress-3> "+global $w.c; set $w.c(x) %x; set $w.c(y) %y"
    
    # misc
    set $w.c(state) ""
    bind $w.c <Destroy> "vgraph_cleanup $w; vgraph destroy $obj"

    set $w.c(xscale) 1
    set $w.c(yscale) 1
}

proc vgraph_destroy {w} {
    if {[winfo exist $w.c] == 1} {
	foreach i {$w.c $w.xscroll $w.yscroll $w.m} {
	    catch {destroy $i}
	}
    }
}


# clean up global variables
proc vgraph_cleanup {w} {
    catch {global $w.c; unset $w.c}
}


# clear
proc vgraph_clear {w} {
    $w.c delete all
}

#----------------------------------------------------------------------
# add a node to the graph
#

proc vgraph_add_node {w text x y} {

    global visual $w.c
    set j [$w.c create text $x $y -text "$text" -font $visual(GRAPH_FONT)]
    set bbox [$w.c bbox $j]
    set i [$w.c create rectangle [expr [lindex $bbox 0] - 1]\
	    [expr [lindex $bbox 1] - 1] [expr [lindex $bbox 2] + 1]\
	    [expr [lindex $bbox 3] + 1] -fill grey]
    $w.c raise $j $i

    $w.c addtag boxes withtag $i
    $w.c addtag texts withtag $j
    $w.c addtag nodes withtag $i
    $w.c addtag nodes withtag $j

    set $w.c(textid($i)) $j
    set $w.c(boxid($j)) $i
    return $i
}

#----------------------------------------------------------------------
# add an edge to the graph
#
proc vgraph_add_edge {w node1 node2 arrow} {
    global visual $w.c

    set bbox1 [$w.c bbox $node1]
    set bbox2 [$w.c bbox $node2]

    set smooth [expr ($node1 == $node2) ? 1 : 0]
    set id [$w.c create line 0 0 0 0 -arrow $arrow -arrowshape {-4 8 6}\
	    -fill $visual(GRAPH_EDGE_COLOR) -smooth $smooth -width 2]

    set $w.c(node1($id)) $node1
    set $w.c(node2($id)) $node2
    lappend $w.c(edge1($node1)) $id
    lappend $w.c(edge2($node2)) $id
    $w.c addtag edges withtag $id

    return $id
}

#----------------------------------------------------------------------
# selecting a node
#

proc vgraph_select {w tagOrid} {
    vgraph_unselect $w selected
    vgraph_select_add $w $tagOrid
}

proc vgraph_select_add {w tagOrid} {
    global $w.c
    foreach id [$w.c find withtag $tagOrid] {
	set box [vgraph_get_boxid $w $id]
	$w.c addtag selected withtag $box
	$w.c addtag selected withtag [set $w.c(textid($box))]

	$w.c itemconfigure $box -fill yellow
    }
}

proc vgraph_select_clear {w} {
    vgraph_unselect $w selected
}

proc vgraph_unselect {w tagOrid} {
    global $w.c
    foreach id [$w.c find withtag $tagOrid] {
	set box [vgraph_get_boxid $w $id]
	$w.c itemconfigure $box -fill grey

	$w.c dtag $box selected
	$w.c dtag [set $w.c(textid($box))] selected
    }
}

proc vgraph_click_select {w obj} {
    global $w.c
    set $w.c(state) disabled

    $w.c delete band
    vgraph_select $w current

    set id [$w.c find withtag current]
    set node [vgraph_get_boxid $w $id]
    vgraph select $obj $node
}

proc vgraph_toggle_select {w} {
    global $w.c
    set $w.c(state) disabled

    if {[lsearch [$w.c gettags current] selected] == -1} {
	vgraph_select_add $w current
    } else {
	vgraph_unselect $w current
    }
}

#----------------------------------------------------------------------
# helper functions
#

#
# return the box id given the node id, or the box id itself
#
proc vgraph_get_boxid {w id} {
    if {[$w.c type $id] == "text"} {
	# id is the text
	global $w.c
	return [set $w.c(boxid($id))]
    } else {
	# id is the box
	return $id
    }
}

#
# returns the currently selected node id, -1 if none
#
proc vgraph_getsel {w} {
    set sel [$w.c find withtag selected]
    if {$sel != ""} {
	set sel_id [lindex $sel 0]
	set node [vgraph_get_boxid $w $sel_id]
	return $node
    } else {
	return -1
    }
}

#
# Convert from window coordinates to canvas coordinates
# 

proc vgraph_coords {w x y cx_name cy_name} {
    upvar $cx_name cx
    upvar $cy_name cy
    set cx [$w.c canvasx $x]
    set cy [$w.c canvasy $y]
}


#----------------------------------------------------------------------
# Layout - Set node size
#
# w = canvas path
# id = canvas id of node (the rectangle item)
#

proc vgraph_set_node_size {w node width height update_edges} {
    set orig_coords [$w.c coords $node]

    set x1 [expr ([lindex $orig_coords 2] + [lindex $orig_coords 0]\
	    - $width)/2]
    set y1 [expr ([lindex $orig_coords 1] + [lindex $orig_coords 3]\
	    - $height)/2]
    set x2 [expr $x1 + $width]
    set y2 [expr $y1 + $height]
    $w.c coords $node $x1 $y1 $x2 $y2

    # update edges
    if {$update_edges == 1} {
	vgraph_update_edge $w $node
    }
}

#----------------------------------------------------------------------
# Layout - Place node
#
# w = canvas path
# id = canvas id of node (the rectangle item)
# (x, y) = center of node
#

proc vgraph_place_node {w node x y update_edges} {

    global $w.c
    set textid [set $w.c(textid($node))]

    # initialize node size
    set bbox [$w.c bbox $textid]
    $w.c coords $node [expr [lindex $bbox 0] - 1]\
	    [expr [lindex $bbox 1] - 1] [expr [lindex $bbox 2] + 1]\
	    [expr [lindex $bbox 3] + 1]

    # move node
    set orig_coords [$w.c coords $node]
    set dx [expr $x - ([lindex $orig_coords 0] + [lindex $orig_coords 2])/2]
    set dy [expr $y - ([lindex $orig_coords 1] + [lindex $orig_coords 3])/2]

    $w.c move $node $dx $dy
    $w.c move $textid $dx $dy

    if {$update_edges == 1} {
	vgraph_update_edge $w $node
    }
}

#----------------------------------------------------------------------
# Layout - place edge
# 

proc vgraph_place_edge {w id x1 y1 x2 y2} {
    $w.c coords $id $x1 $y1 $x2 $y2
}

proc vgraph_place_edge_spline {w id args} {
    $w.c itemconfigure $id -smooth 1
    eval $w.c coords $id $args
}


#----------------------------------------------------------------------
# vgraph_auto_place_edge
#
# w = canvas path
# id = canvas id of edge
# node1, node2 = canvas ids of the nodes
#

proc vgraph_auto_place_edge {w id node1 node2} {

    set bbox1 [$w.c bbox $node1]
    set bbox2 [$w.c bbox $node2]
    if {$node1 != $node2} {

	set c1x [expr ([lindex $bbox1 0] + [lindex $bbox1 2]) /2 ]
	set c1y [expr ([lindex $bbox1 1] + [lindex $bbox1 3]) /2 ]
	set c2x [expr ([lindex $bbox2 0] + [lindex $bbox2 2]) /2 ]
	set c2y [expr ([lindex $bbox2 1] + [lindex $bbox2 3]) /2 ]

	clip_line c1x c1y $c2x $c2y $bbox1

	clip_line c2x c2y $c1x $c1y $bbox2

	$w.c coords $id $c1x $c1y $c2x $c2y

    } else {
	# self loop
	set p1x [lindex $bbox1 0]
	set p1y [expr [lindex $bbox1 1] + 10]
	set p2x [expr [lindex $bbox1 0] + 20]
	set p2y [expr [lindex $bbox1 1]]
	$w.c coords $id $p1x $p1y [expr $p1x - 10] $p1y\
		[expr $p1x - 10] [expr $p1y - 25] $p2x [expr $p1y - 25]\
		$p2x $p2y
    }
}

#----------------------------------------------------------------------
# vgraph_update_edge
#

proc vgraph_update_edge {w node} {

    global $w.c

    # move edge
    catch {
	foreach i [set $w.c(edge1($node))] {
	    set node2 [set $w.c(node2($i))]
	    vgraph_auto_place_edge $w $i $node $node2
	}
    }
    catch {
	foreach i [set $w.c(edge2($node))] {
	    set node1 [set $w.c(node1($i))]
	    vgraph_auto_place_edge $w $i $node1 $node
	}
    }
}

#----------------------------------------------------------------------
# zooming
#
proc vgraph_zoomout {w} {

    set width [winfo width $w.c]
    set height [winfo height $w.c]
    vgraph_coords $w [expr $width/2] [expr $height/2] cx cy

    $w.c scale all $cx $cy .67 .67

    global $w.c
    set $w.c(xscale) [expr [set $w.c(xscale)] * .67]
    set $w.c(yscale) [expr [set $w.c(yscale)] * .67]

    vgraph_resize $w
}

proc vgraph_zoomin {w} {

    set b [$w.c coords band]
    if {$b != ""} {
	vgraph_zoombox $w $b
    } else {
	set width [winfo width $w.c]
	set height [winfo height $w.c]
	vgraph_coords $w [expr $width/2] [expr $height/2] cx cy
	$w.c scale all $cx $cy 1.5 1.5

	global $w.c
	set $w.c(xscale) [expr [set $w.c(xscale)] * 1.5]
	set $w.c(yscale) [expr [set $w.c(yscale)] * 1.5]

	vgraph_resize $w
    }
}

proc vgraph_zoomtofit {w} {
    set bbox [$w.c bbox all]
    set x1 [lindex $bbox 0]
    set y1 [lindex $bbox 1]
    set x2 [lindex $bbox 2]
    set y2 [lindex $bbox 3]

    set dx [expr $x2 - $x1]
    set dy [expr $y2 - $y1]
    if {$dy > $dx} {
	set x1 [expr $x1-($dy-$dx)/2]
	set x2 [expr $x2+($dy-$dx)/2]
    } else {
	set y1 [expr $y1-($dx-$dy)/2]
	set y2 [expr $y2+($dx-$dy)/2]
    }
    vgraph_zoombox $w "$x1 $y1 $x2 $y2"
}

proc vgraph_zoombox {w b} {
    set x1 [lindex $b 0]
    set y1 [lindex $b 1]
    set x2 [lindex $b 2]
    set y2 [lindex $b 3]
    set width [winfo width $w.c]
    set height [winfo height $w.c]
    
    vgraph_coords $w [expr $width/2] [expr $height/2] cx cy
    set d [expr [lindex $b 2] - [lindex $b 0]]
    if {$d == 0} return;
    set xscale [expr double($width - 40)/$d]
    set d [expr [lindex $b 3] - [lindex $b 1]]
    if {$d == 0} return;
    set yscale [expr double($height - 40)/$d]
    
    set dx [expr $cx - ($x1 + $x2)/2]
    set dy [expr $cy - ($y1 + $y2)/2]
    
    $w.c move all $dx $dy
    $w.c scale all $cx $cy $xscale $yscale

    global $w.c
    set $w.c(xscale) [expr [set $w.c(xscale)] * $xscale]
    set $w.c(yscale) [expr [set $w.c(yscale)] * $yscale]

    vgraph_resize $w
}

#----------------------------------------------------------------------
# vgraph_resize
#
# update when the size changes
#
proc vgraph_resize {w} {

    global $w.c visual

    if 0 { # don't redraw!
	# redraw all edges and nodes
	foreach i [$w.c find withtag boxes] {
	    set j [set $w.c(textid($i))]
	    # found the text j associated with box i
	    set bbox [$w.c bbox $j]
	    $w.c coords $i [expr [lindex $bbox 0] - 1]\
		    [expr [lindex $bbox 1] - 1]\
		    [expr [lindex $bbox 2] + 1]\
		    [expr [lindex $bbox 3] + 1]
	}
	foreach i [$w.c find withtag edges] {
	    set node1 [set $w.c(node1($i))];
	    set node2 [set $w.c(node2($i))];
	    vgraph_auto_place_edge $w $i $node1 $node2
	}
    }

    # update font
    set s [set $w.c(xscale)]
    if {$s < .5} {
	$w.c itemconfigure texts -font "-*-helvetica-medium-R-Normal--8-*"
    } elseif {$s < .7} {
	$w.c itemconfigure texts -font "-*-helvetica-medium-R-Normal--11-*"
    } else {
	$w.c itemconfigure texts -font $visual(GRAPH_FONT)
    }

    # update scrollregion
    set center_x [$w.c canvasx 0]
    set center_y [$w.c canvasy 0]
    vgraph_update_scrollregion $w

    set region [$w.c cget -scrollregion]
    set x1 [lindex $region 0]
    set x2 [lindex $region 2]
    set y1 [lindex $region 1]
    set y2 [lindex $region 3]

    $w.c xview moveto [expr ($center_x - $x1)/($x2 - $x1)]
    $w.c yview moveto [expr ($center_y - $y1)/($y2 - $y1)]
}

#----------------------------------------------------------------------
# vgraph_center
#
proc vgraph_center {w} {
    set bbox [$w.c coords band]
    if {$bbox == ""} {
	set bbox [$w.c bbox all]
    }

    set canvas_x [$w.c canvasx 0]
    set canvas_y [$w.c canvasy 0]

    set width [winfo width $w.c]
    set height [winfo height $w.c]
    set dx [expr -([lindex $bbox 0]+[lindex $bbox 2] - $width)/2]
    set dy [expr -([lindex $bbox 1]+[lindex $bbox 3] - $height)/2]
    $w.c move all $dx $dy

    vgraph_update_scrollregion $w
    vgraph_view $w [expr $canvas_x + $dx] [expr $canvas_y + $dy]
}

#----------------------------------------------------------------------
# vgraph_view
#
proc vgraph_view {w canvas_x canvas_y} {
    set region [$w.c cget -scrollregion]
    set x1 [lindex $region 0]
    set x2 [lindex $region 2]
    set y1 [lindex $region 1]
    set y2 [lindex $region 3]

    $w.c xview moveto [expr ($canvas_x - $x1)/($x2 - $x1)]
    $w.c yview moveto [expr ($canvas_y - $y1)/($y2 - $y1)]
}


#----------------------------------------------------------------------
# vgraph_view_node
#
proc vgraph_view_node {w node} {
    # not implemented yet
}

#----------------------------------------------------------------------
# layout
#
proc vgraph_layout {w obj method} {
    global $w.c visual
    set $w.c(xscale) 1
    set $w.c(yscale) 1

    # reset font
    $w.c itemconfigure texts -font $visual(GRAPH_FONT)

    vgraph layout $obj $method

    vgraph_update_scrollregion $w
    $w.c xview moveto 0
    $w.c yview moveto 0.4

    catch {$w.c lower edges texts}
    catch {$w.c lower edges boxes}

}

#----------------------------------------------------------------------
# update_scrollregion
#
proc vgraph_update_scrollregion {w} {

    set box [$w.c bbox all]
    set x1 [expr [lindex $box 0] - 100]
    set y1 [expr [lindex $box 1] - 100]
    set x2 [expr [lindex $box 2] + 100]
    set y2 [expr [lindex $box 3] + 100]

    set width [winfo width $w.c]
    set height [winfo height $w.c]
    if {[expr $x2 - $x1] < $width} {set x2 [expr $x1 + $width]}
    if {[expr $y2 - $y1] < $height} {set y2 [expr $y1 + $height]}

    $w.c configure -scrollregion "$x1 $y1 $x2 $y2"
}

#----------------------------------------------------------------------
# misc functions
#
proc vgraph_get_bbox {w id} {
    return [$w.c bbox $id]
}

proc vgraph_display {w mesg} {
    $w.m configure -text $mesg
}

#----------------------------------------------------------------------
# edge enter/leave bindings
#

proc vgraph_enter_edge {w x y} {
    global $w.c
    if {[set $w.c(state)] != ""} {
	return;
    }

    set edge [$w.c find withtag current]
    set $w.c(cur_edge) $edge

    set node1 [$w.c itemcget \
	    [set $w.c(textid([set $w.c(node1($edge))]))] -text]
    set node2 [$w.c itemcget \
	    [set $w.c(textid([set $w.c(node2($edge))]))] -text]
    
    set text ""
    switch [lindex [$w.c itemconfigure $edge -arrow] 4]\
	    "first" "set text {$node2 -> $node1}" \
	    "last" "set text {$node1 -> $node2}" \
	    "both" "set text {$node1 <-> $node2}" \
	    "none" "set text {$node1 - $node2}"

    $w.c itemconfigure $edge -fill red
    set node1 [set $w.c(node1($edge))]
    set node2 [set $w.c(node2($edge))]
    $w.c itemconfigure $node1 -outline red
    $w.c itemconfigure $node2 -outline red
    $w.c raise $edge all

    vgraph_display $w $text
}

proc vgraph_leave_edge {w x y} {
    global $w.c visual
    if {[set $w.c(state)] != ""} {
	return;
    }

    set edge [set $w.c(cur_edge)]
    $w.c itemconfigure $edge -fill $visual(GRAPH_EDGE_COLOR)
    set node1 [set $w.c(node1($edge))]
    set node2 [set $w.c(node2($edge))]
    $w.c itemconfigure $node1 -outline black
    $w.c itemconfigure $node2 -outline black
    $w.c lower $edge boxes

    vgraph_display $w ""
}

#----------------------------------------------------------------------
# node enter/leave bindings
#

proc vgraph_enter_node {w x y} {
    global $w.c
    if {[set $w.c(state)] != ""} {
	return;
    }

    set text [$w.c find withtag current]

    set node [set $w.c(boxid($text))]
    set $w.c(cur_node) $node
    $w.c itemconfigure $node -outline red
    catch {
	foreach i [set $w.c(edge1($node))] {
	    $w.c itemconfigure $i -fill red
	    $w.c raise $i all
	    set node2 [set $w.c(node2($i))]
	    $w.c itemconfigure $node2 -outline red
	}
    }
    catch {
	foreach i [set $w.c(edge2($node))] {
	    $w.c itemconfigure $i -fill red
	    $w.c raise $i all
	    set node1 [set $w.c(node1($i))]
	    $w.c itemconfigure $node1 -outline red
	}
    }
    
    set title [lindex [$w.c itemconfigure $text -text] 4]
    vgraph_display $w $title
}

proc vgraph_leave_node {w x y} {
    global visual $w.c
    if {[set $w.c(state)] != ""} {
	return;
    }

    set edgecolor $visual(GRAPH_EDGE_COLOR)
    set node [set $w.c(cur_node)]

    $w.c itemconfigure $node -outline black
    catch {
	foreach i [set $w.c(edge1($node))] {
	    $w.c itemconfigure $i -fill $edgecolor
	    $w.c lower $i boxes
	    set node2 [set $w.c(node2($i))]
	    $w.c itemconfigure $node2 -outline black
	}
    }
    catch {
	foreach i [set $w.c(edge2($node))] {
	    $w.c itemconfigure $i -fill $edgecolor
	    $w.c lower $i boxes
	    set node1 [set $w.c(node1($i))]
	    $w.c itemconfigure $node1 -outline black
	}
    }
    vgraph_display $w ""
}

#----------------------------------------------------------------------
# rubber band
#

proc vgraph_drag_start {w x y} {
    
    if {[lsearch [$w.c gettags current] selected] == -1} {
	# the node is not selected
	$w.c delete band
	vgraph_select $w current
    }

    global $w.c
    set $w.c(state) "drag"
    set $w.c(x) $x
    set $w.c(y) $y
}

proc vgraph_drag {w x y} {

    global $w.c
    set $w.c(state) "drag"

    # move node
    global $w.c
    if {[catch {set $w.c(x)}] == 1} {
	return;			# anchor is not set
    }

    set dx [expr $x - [set $w.c(x)]]
    set dy [expr $y - [set $w.c(y)]]
    $w.c move selected $dx $dy
    $w.c move band $dx $dy

    set $w.c(x) $x
    set $w.c(y) $y
}

proc vgraph_drag_end {w x y} {
    foreach id [$w.c find withtag selected] {
	# update edges
	set node [vgraph_get_boxid $w $id]
	vgraph_update_edge $w $node
    }
    global $w.c
    set $w.c(state) ""
}

proc vgraph_band_start {w x y unselect} {

    global $w.c
    if {[set $w.c(state)] == "disabled"} {
	return
    }

    if {$unselect == 1} {
	vgraph_select_clear $w
    }

    global $w.c
    vgraph_coords $w $x $y $w.c(x) $w.c(y)
    $w.c delete band
    $w.c create rectangle -10000 -10000 -10000 -10000 -outline red -tags band
}

proc vgraph_band_drag {w x y} {
    global $w.c
    if {[set $w.c(state)] == "disabled"} {
	return
    }

    if {[set $w.c(x)] != ""} {
	vgraph_coords $w $x $y cx cy
	$w.c coords band [set $w.c(x)] [set $w.c(y)] $cx $cy

	set $w.c(state) "drag"
    }
}

proc vgraph_band_drag_end {w x y} {
    global $w.c
    if {[set $w.c(state)] == "disabled"} {
	set $w.c(state) ""
	return
    }
    set $w.c(state) ""

    set selected 0
    if {[set $w.c(x)] != $x || [set $w.c(y)] != $y} {
	set b [$w.c find withtag band]
	if {$b != ""} {
	    foreach i [eval $w.c find overlapping [$w.c coords band]] {
		if {[$w.c type $i] == "rectangle" && $i != $b} {
		    vgraph_select_add $w $i
		    set selected 1
		}
	    }
	}
    }

    if {$selected == 0} {
	$w.c delete band
    }
}

#----------------------------------------------------------------------
# printing
#
proc vgraph_print {w} {
    $w.c delete band
    $w.c postscript -colormode mono
    vdialog .vdialog "" \
	    "Sent postscript to default printer." "" 0 Ok
}

proc vgraph_print_file {w} {
    $w.c delete band
    set file [select_file "" {Print to postcript file:} [pwd]]
    if {$file != ""} {
	$w.c postscript -file $file -colormode mono
	vdialog .vdialog "" \
		"Saved postscript as $file" "" 0 Ok
    }
}

#----------------------------------------------------------------------
# misc functions
#

proc clip_line {x_name y_name x0 y0 box} {
    upvar $x_name x
    upvar $y_name y
    set bx1 [lindex $box 0]
    set by1 [lindex $box 1]
    set bx2 [lindex $box 2]
    set by2 [lindex $box 3]

    set maxt 0
    if {$x != $x0} {
	if {$x0 < $bx1} {
	    set maxt [expr double($bx1 - $x0)/($x - $x0)]
	} elseif {$x0 > $bx2} {
	    set maxt [expr double($bx2 - $x0)/($x - $x0)]
	}
    }
    if {$y != $y0} {
	if {$y0 < $by1} {
	    set t [expr double($by1 - $y0)/($y - $y0)]
	    if {$t > $maxt} {set maxt $t}
	}
	if {$y0 > $by2} {
	    set t [expr double($by2 - $y0)/($y - $y0)]
	    if {$t > $maxt} {set maxt $t}
	}
    }

    set x [expr round($x0 + $maxt * ($x - $x0))]
    set y [expr round($y0 + $maxt * ($y - $y0))]
}

proc vgraph_help {} {
    set mesg \
"Mouse Bindings:
<Button 1>: Select node
<Button 2>: Move node(s)
<Button 3>: Pop-up menu
<Button 1>-drag: Rubber band selection
<Shift-Button 1>: Add/Remove node to/from selection"

    vdialog .vs_tmp Help $mesg "" 0 Ok 
}


#-------------------------------------------------------------------------
# Test code
if 0 {
    toplevel .g
    vgraph_create .g 0
    set i [vgraph_add_node .g Hi 150 100]
    set j [vgraph_add_node .g Hello 270 130]
    set k [vgraph_add_edge .g $i $j last]

    set x 100
    set y 100
    for {set i 0} {$i < 30} {incr i} {
	set node($i) [vgraph_add_node .g $i $x $y]
	incr x $i
	incr y $x
	set x [expr $x%500]
	set y [expr $y%500]
    }
    for {set i 0} {$i < 50} {incr i} {
	incr x 3
	incr y 4
	set x [expr $x%30]
	set y [expr $y%30]
	set n1 $node($x)
	set n2 $node($y)
	vgraph_add_edge .g $n1 $n2 last
    }
}

