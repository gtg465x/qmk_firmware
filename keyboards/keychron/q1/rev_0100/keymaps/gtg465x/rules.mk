VIA_ENABLE = yes
MOUSEKEY_ENABLE = no
RGB_MATRIX_USER_ENABLE = yes

ifeq ($(strip $(RGB_MATRIX_ENABLE)), yes)
    ifeq ($(strip $(RGB_MATRIX_USER_ENABLE)), yes)
        OPT_DEFS += -DRGB_MATRIX_USER_ENABLE
        SRC += rgb_matrix_user.c
    endif
endif
