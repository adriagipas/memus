MEMUMIX_FILES= files('main.c',
                     'dirs.h',
                     'dirs.c',
                     'conf.h',
                     'conf.c',
                     'screen.h',
                     'screen.c',
                     'icon.h',
                     'frontend.h',
                     'frontend.c',
                     'ui.h',
                     'ui.c',
                     'ui_element.h',
                     'ui_element.c',
                     'ui_list.h',
                     'ui_list.c',
                     'ui_button.h',
                     'ui_button.c',
                     'ui_menu.h',
                     'ui_menu.c',
                     'ui_menu_bar.h',
                     'ui_menu_bar.c',
                     'ui_scrollbar.h',
                     'ui_scrollbar.c',
                     'ui_lp.h',
                     'ui_lp.c',
                     'ui_file_chooser.h',
                     'ui_file_chooser.c',
                     'ui_open_dialog.h',
                     'ui_open_dialog.c',
                     'ui_input.h',
                     'ui_input.c',
                     'ui_save_dialog.h',
                     'ui_save_dialog.c',
                     'ui_error_dialog.h',
                     'ui_error_dialog.c',
                     'ui_status.h',
                     'ui_status.c',
                     'cardreader.h',
                     'cardreader.c',
                     'cdir.h',
                     'cdir.c',
                     'lineprinter.h',
                     'lineprinter.c',
                     'cmix2c.h',
                     'cmix2c.c',
                     'magnetictapes.h',
                     'magnetictapes.c',
                     'cardpunch.h',
                     'cardpunch.c')

MEMUMIX= executable('memumix',
                    MEMUMIX_FILES,
                    dependencies : [GLIB2,SDL2],
                    include_directories : [COMMON_H,VGAFONT_H,MIX_H],
                    link_with : [COMMON,VGAFONT,MIX],
                    link_args : '-lm',
                    install : true)
