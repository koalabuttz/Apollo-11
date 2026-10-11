; ModuleID = 'tools/p66_pending_layout.c'
source_filename = "tools/p66_pending_layout.c"
target datalayout = "e-m:e-p:16:8-p1:8:8-i16:8-i32:8-i64:8-f32:8-f64:8-a:8-Fi8-n8"
target triple = "mos-c64"

@pending_layout_values = dso_local local_unnamed_addr constant [8 x i16] [i16 36, i16 130, i16 32, i16 198, i16 36, i16 166, i16 648, i16 27], align 1, !guid !0

!llvm.module.flags = !{!1, !2, !3}
!llvm.ident = !{!4}
!llvm.errno.tbaa = !{!5}

!0 = !{i64 7805552072811701551}
!1 = !{i32 7, !"frame-pointer", i32 2}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!4 = !{!"clang version 24.0.0git (https://github.com/llvm-mos/llvm-mos cd9145597347753c7cabd1c34d24722d4d8a8f96)"}
!5 = !{!6, !7, i64 0}
!6 = !{!"__libc_errno", !7, i64 0}
!7 = !{!"int", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}

^0 = module: (path: "[Regular LTO]", hash: (0, 0, 0, 0, 0))
^1 = gv: (name: "pending_layout_values", summaries: (variable: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 7805552072811701551
^2 = flags: 8
^3 = blockcount: 0
