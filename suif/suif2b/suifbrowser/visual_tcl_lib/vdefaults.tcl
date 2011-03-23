# vdefaults.tcl
#-------------------------------------------------------------------------

# To make Tk use a implicit focus model, uncomment the next line
# tk_focusFollowsMouse

#
# Visual Parameters
#

global visual
set visual(GRAPH_WIDTH) 500
set visual(GRAPH_HEIGHT) 500
set visual(GRAPH_FONT) "-*-helvetica-Bold-R-Normal--14-*"
#set visual(GRAPH_FONT) "9x15bold"
set visual(GRAPH_EDGE_COLOR) #4040f8

set visual(TEXT_COLUMNS) 64
set visual(TEXT_ROWS) 24
set visual(TEXT_SEL_BACKGROUND) #b0d0e0
set visual(TEXT_FONT) "-*-courier-medium-r-normal--*-160-*"
set visual(TEXT_BOLD_FONT) "-*-courier-bold-r-normal--*-160-*"
set visual(TEXT_ITALIC_FONT) "-*-*-medium-i-normal--*-160-*"

set visual(POPUP_BACKGROUND) #e0e0c0
set visual(DIALOG_BACKGROUND) #d0d0d0
set visual(DIALOG_FONT) "-*-helvetica-Bold-R-Normal--14*"

set visual(LISTBOX_TITLE_BACKGROUND) #e0e0e0
set visual(LISTBOX_TITLE_FOREGROUND) #404040

if {[catch {option readfile $env(HOME)/.visual.rc} error] == 1} {
    if {[catch {option readfile $env(VISUAL_CONFIG)/visual.rc} error] == 1} {
        if {[catch {option readfile /usr/local/lib/visual.rc} error] == 1} {
            if {[catch {option readfile
                        $env(SUIFHOME)/$env(MACHINE)/visual.rc} error] == 1} {
              if {[catch {option readfile $env(VISUAL_TCL)/visual.rc} error] == 1} {
                puts stdout "Warning: unable to find option file 'visual.rc'"
              }
            }
        }
    }
}

#
# Window Placement
#
set visual(geometry) {
    {"Source Viewer" "+20+80"}
    {"Suif Viewer" "+20+540"}
    {"Callgraph Viewer" "+720+100"}
    {"Info Viewer" "500x400+740+540"}
    {"Profile Viewer" "600x300+500+300"}
    {"Text Viewer" "+500+400"}
    {"Output Viewer" "+700+300"}
    {"Main Window" "+780+80"}
}

#
# Initialize some variables
#

# Global line number in the "go to line" dialog box for text widgets.
#
set visual(line_num) ""


