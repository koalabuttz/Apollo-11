; ModuleID = '/tmp/p66-recurrence-sizes.c'
source_filename = "/tmp/p66-recurrence-sizes.c"
target datalayout = "e-m:e-p:16:8-p1:8:8-i16:8-i32:8-i64:8-f32:8-f64:8-a:8-Fi8-n8"
target triple = "mos-c64"

@metadata_size = dso_local constant [171 x i8] zeroinitializer, align 1, !guid !0
@binding_size = dso_local constant [103 x i8] zeroinitializer, align 1, !guid !1
@runtime_size = dso_local constant [36 x i8] zeroinitializer, align 1, !guid !2
@executive_size = dso_local constant [648 x i8] zeroinitializer, align 1, !guid !3

!llvm.module.flags = !{!4, !5, !6}
!llvm.ident = !{!7}

!0 = !{i64 2059199591849505202}
!1 = !{i64 1342017064935069695}
!2 = !{i64 3034469993408138278}
!3 = !{i64 -5273386092837914520}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{i32 1, !"ThinLTO", i32 0}
!6 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!7 = !{!"clang version 24.0.0git (https://github.com/llvm-mos/llvm-mos cd9145597347753c7cabd1c34d24722d4d8a8f96)"}

^0 = module: (path: "[Regular LTO]", hash: (0, 0, 0, 0, 0))
^1 = gv: (name: "binding_size", summaries: (variable: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 1342017064935069695
^2 = gv: (name: "metadata_size", summaries: (variable: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 2059199591849505202
^3 = gv: (name: "runtime_size", summaries: (variable: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 3034469993408138278
^4 = gv: (name: "executive_size", summaries: (variable: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 13173357980871637096
^5 = flags: 8
^6 = blockcount: 0
