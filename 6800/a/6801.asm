        MACEXP  on

        CPU     6801

        org     $1000

        lds     $FF
        ldaa    #$01
        ldab    #$80
        ldx     #$20
        abx
        stab    0,X
        nop
        nop

        end
