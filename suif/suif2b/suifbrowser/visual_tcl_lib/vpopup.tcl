#-----------------------------------------------------------------------
# popup menu
#
# To create a popup menu
# 1) create the menu using the standard "menu" commands
# 2) call popup_init to init the popup menus
#----------------------------------------------------------------------

#
# Initailize a popup menu
#
proc popup_init {w menu button} {
    bind $w <ButtonPress-$button> "popup_post $menu %X %Y"
}

#
# post popup menu at coord (x,y)
#

proc popup_post {w x y} {
    catch {tk_popup $w $x $y}
}
