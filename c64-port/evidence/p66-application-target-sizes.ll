; ModuleID = 'build/p66-application-sizes.c'
source_filename = "build/p66-application-sizes.c"
target datalayout = "e-m:e-p:16:8-p1:8:8-i16:8-i32:8-i64:8-f32:8-f64:8-a:8-Fi8-n8"
target triple = "mos-c64"

@application_metadata_size = dso_local local_unnamed_addr global [163 x i8] zeroinitializer, align 1, !guid !0
@application_binding_size = dso_local local_unnamed_addr global [95 x i8] zeroinitializer, align 1, !guid !1
@executive_view_size = dso_local local_unnamed_addr global [648 x i8] zeroinitializer, align 1, !guid !2
@task_views_size = dso_local local_unnamed_addr global [76 x i8] zeroinitializer, align 1, !guid !3

!llvm.module.flags = !{!4, !5, !6}
!llvm.ident = !{!7}
!llvm.errno.tbaa = !{!8}

!0 = !{i64 768241200423734420}
!1 = !{i64 -8227481286398182915}
!2 = !{i64 5973446270594844778}
!3 = !{i64 -2101469085790448907}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{i32 1, !"ThinLTO", i32 0}
!6 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!7 = !{!"clang version 24.0.0git (https://github.com/llvm-mos/llvm-mos cd9145597347753c7cabd1c34d24722d4d8a8f96)"}
!8 = !{!9, !10, i64 0}
!9 = !{!"__libc_errno", !10, i64 0}
!10 = !{!"int", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}

^0 = module: (path: "[Regular LTO]", hash: (0, 0, 0, 0, 0))
^1 = gv: (name: "application_metadata_size", summaries: (variable: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 1, constant: 0)))) ; guid = 768241200423734420
^2 = gv: (name: "executive_view_size", summaries: (variable: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 1, constant: 0)))) ; guid = 5973446270594844778
^3 = gv: (name: "application_binding_size", summaries: (variable: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 1, constant: 0)))) ; guid = 10219262787311368701
^4 = gv: (name: "task_views_size", summaries: (variable: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 1, constant: 0)))) ; guid = 16345274987919102709
^5 = flags: 8
^6 = blockcount: 0
