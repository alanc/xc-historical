; $XConsortium$
        .SPACE  $TEXT$
        .SUBSPA $CODE$
        .export cr16
        .PROC
        .CALLINFO
        .ENTRY
cr16
        bv      (%rp)
	mfctl	16,%ret0
        .PROCEND
