; ModuleID = 'build/native-exchange-size.c'
source_filename = "build/native-exchange-size.c"
target datalayout = "e-m:e-p:16:8-p1:8:8-i16:8-i32:8-i64:8-f32:8-f64:8-a:8-Fi8-n8"
target triple = "mos-c64"

%struct.apollo_overlay_runtime = type { ptr, %struct.apollo_overlay_backend, ptr, ptr, i8, i8, i8 }
%struct.apollo_overlay_backend = type { ptr, ptr, ptr, ptr, ptr }
%struct.apollo_overlay_module = type { i8, ptr, i16, i16 }
%struct.c64_overlay_io = type { i8, i8, i8, i8, i8, i8 }
%struct.exec_exclusion = type { ptr, ptr, ptr }
%struct.exec_overlay_dispatch = type { ptr, ptr }
%struct.exec_request = type { i16, %struct.exec_address, i8 }
%struct.exec_address = type { i16, i16 }
%struct.lm_interpreter_exchange_arguments = type { ptr, ptr }

@loader = internal global %struct.apollo_overlay_runtime zeroinitializer, align 1, !guid !0
@lm_exchange_modules = internal constant [35 x %struct.apollo_overlay_module] [%struct.apollo_overlay_module { i8 1, ptr @.str, i16 8713, i16 -3963 }, %struct.apollo_overlay_module { i8 2, ptr @.str.1, i16 8713, i16 14652 }, %struct.apollo_overlay_module { i8 3, ptr @.str.2, i16 8713, i16 -12850 }, %struct.apollo_overlay_module { i8 4, ptr @.str.3, i16 8713, i16 -29195 }, %struct.apollo_overlay_module { i8 5, ptr @.str.4, i16 8713, i16 20073 }, %struct.apollo_overlay_module { i8 6, ptr @.str.5, i16 8713, i16 -11965 }, %struct.apollo_overlay_module { i8 7, ptr @.str.6, i16 8713, i16 18038 }, %struct.apollo_overlay_module { i8 8, ptr @.str.7, i16 8713, i16 8307 }, %struct.apollo_overlay_module { i8 9, ptr @.str.8, i16 8713, i16 27784 }, %struct.apollo_overlay_module { i8 10, ptr @.str.9, i16 8713, i16 18960 }, %struct.apollo_overlay_module { i8 11, ptr @.str.10, i16 8713, i16 -16841 }, %struct.apollo_overlay_module { i8 12, ptr @.str.11, i16 8713, i16 -6508 }, %struct.apollo_overlay_module { i8 13, ptr @.str.12, i16 8713, i16 9383 }, %struct.apollo_overlay_module { i8 14, ptr @.str.13, i16 8713, i16 11609 }, %struct.apollo_overlay_module { i8 15, ptr @.str.14, i16 8713, i16 19110 }, %struct.apollo_overlay_module { i8 16, ptr @.str.15, i16 8713, i16 -8871 }, %struct.apollo_overlay_module { i8 17, ptr @.str.16, i16 8713, i16 15319 }, %struct.apollo_overlay_module { i8 18, ptr @.str.17, i16 8713, i16 -6035 }, %struct.apollo_overlay_module { i8 19, ptr @.str.18, i16 8713, i16 8259 }, %struct.apollo_overlay_module { i8 20, ptr @.str.19, i16 8713, i16 32537 }, %struct.apollo_overlay_module { i8 21, ptr @.str.20, i16 8713, i16 8283 }, %struct.apollo_overlay_module { i8 22, ptr @.str.21, i16 8713, i16 -8400 }, %struct.apollo_overlay_module { i8 23, ptr @.str.22, i16 8713, i16 31572 }, %struct.apollo_overlay_module { i8 24, ptr @.str.23, i16 8713, i16 3052 }, %struct.apollo_overlay_module { i8 25, ptr @.str.24, i16 8713, i16 -15351 }, %struct.apollo_overlay_module { i8 26, ptr @.str.25, i16 8713, i16 20424 }, %struct.apollo_overlay_module { i8 27, ptr @.str.26, i16 8713, i16 -15133 }, %struct.apollo_overlay_module { i8 28, ptr @.str.27, i16 8713, i16 -30335 }, %struct.apollo_overlay_module { i8 29, ptr @.str.28, i16 8713, i16 -1093 }, %struct.apollo_overlay_module { i8 30, ptr @.str.29, i16 8713, i16 -25062 }, %struct.apollo_overlay_module { i8 31, ptr @.str.30, i16 8713, i16 -17098 }, %struct.apollo_overlay_module { i8 32, ptr @.str.31, i16 8713, i16 12878 }, %struct.apollo_overlay_module { i8 -106, ptr @.str.32, i16 7464, i16 20188 }, %struct.apollo_overlay_module { i8 -103, ptr @.str.33, i16 4291, i16 -5030 }, %struct.apollo_overlay_module { i8 -102, ptr @.str.34, i16 2400, i16 -11091 }], align 1, !guid !1
@backend_io = internal global %struct.c64_overlay_io { i8 8, i8 0, i8 0, i8 0, i8 0, i8 0 }, align 1, !guid !2
@guard = internal constant %struct.exec_exclusion { ptr @enter, ptr @leave, ptr null }, align 1, !guid !3
@services = internal constant %struct.exec_overlay_dispatch { ptr @scheduler, ptr null }, align 1, !guid !4
@lm_change_hash = internal unnamed_addr constant [64 x i32] [i32 1461280118, i32 -807408134, i32 -127088042, i32 2135715985, i32 -1607829723, i32 -1308606312, i32 953714895, i32 631440871, i32 -1392797963, i32 1328154144, i32 -123808210, i32 616317432, i32 247089870, i32 323827136, i32 -462602389, i32 -1042745753, i32 -745262646, i32 -929532661, i32 1541312894, i32 1212583714, i32 -985478884, i32 -142873707, i32 -1311983881, i32 1621529154, i32 1475564596, i32 1676200997, i32 399723190, i32 346074999, i32 -1415401043, i32 -427905525, i32 -389525087, i32 -834842289, i32 -492936034, i32 424850988, i32 -1555788188, i32 1586536202, i32 304318025, i32 -418067977, i32 -1080984497, i32 -1830092942, i32 -245835216, i32 -1756720163, i32 977753106, i32 1753769825, i32 1006283894, i32 -210662936, i32 -1544939018, i32 525459296, i32 -29842321, i32 -140862236, i32 -2022486695, i32 -816755186, i32 -1862788106, i32 1659086096, i32 -1363954399, i32 -46219329, i32 2077272429, i32 -1539635273, i32 852962060, i32 -686011300, i32 1040139330, i32 923905661, i32 921889151, i32 805339411], align 1, !guid !5
@lm_change_next = internal unnamed_addr constant [64 x [3 x i16]] [[3 x i16] [i16 1779, i16 26624, i16 1], [3 x i16] [i16 1030, i16 25600, i16 0], [3 x i16] [i16 1780, i16 26625, i16 1], [3 x i16] [i16 1031, i16 25601, i16 0], [3 x i16] [i16 1781, i16 26642, i16 1], [3 x i16] [i16 1025, i16 25619, i16 0], [3 x i16] [i16 1782, i16 26643, i16 1], [3 x i16] [i16 1026, i16 25636, i16 0], [3 x i16] [i16 1783, i16 26660, i16 1], [3 x i16] [i16 1027, i16 25637, i16 0], [3 x i16] [i16 1784, i16 26678, i16 1], [3 x i16] [i16 1028, i16 25654, i16 0], [3 x i16] [i16 1778, i16 26679, i16 1], [3 x i16] [i16 1029, i16 25655, i16 0], [3 x i16] [i16 1779, i16 26688, i16 1], [3 x i16] [i16 1030, i16 25665, i16 0], [3 x i16] [i16 1780, i16 26689, i16 1], [3 x i16] [i16 1031, i16 25682, i16 0], [3 x i16] [i16 1781, i16 26706, i16 1], [3 x i16] [i16 1025, i16 25683, i16 0], [3 x i16] [i16 1782, i16 26724, i16 1], [3 x i16] [i16 1026, i16 25700, i16 0], [3 x i16] [i16 1783, i16 26725, i16 1], [3 x i16] [i16 1027, i16 25701, i16 0], [3 x i16] [i16 1784, i16 26742, i16 1], [3 x i16] [i16 1028, i16 25718, i16 0], [3 x i16] [i16 1778, i16 26743, i16 1], [3 x i16] [i16 1025, i16 25600, i16 0], [3 x i16] [i16 1782, i16 26624, i16 1], [3 x i16] [i16 1026, i16 25601, i16 0], [3 x i16] [i16 1783, i16 26625, i16 1], [3 x i16] [i16 1027, i16 25618, i16 0], [3 x i16] [i16 1784, i16 26643, i16 1], [3 x i16] [i16 1028, i16 25619, i16 0], [3 x i16] [i16 1778, i16 26660, i16 1], [3 x i16] [i16 1029, i16 25636, i16 0], [3 x i16] [i16 1779, i16 26661, i16 1], [3 x i16] [i16 1030, i16 25654, i16 0], [3 x i16] [i16 1780, i16 26678, i16 1], [3 x i16] [i16 1031, i16 25655, i16 0], [3 x i16] [i16 1781, i16 26679, i16 1], [3 x i16] [i16 1025, i16 25664, i16 0], [3 x i16] [i16 1782, i16 26689, i16 1], [3 x i16] [i16 1026, i16 25665, i16 0], [3 x i16] [i16 1783, i16 26706, i16 1], [3 x i16] [i16 1027, i16 25682, i16 0], [3 x i16] [i16 1784, i16 26707, i16 1], [3 x i16] [i16 1028, i16 25700, i16 0], [3 x i16] [i16 1778, i16 26724, i16 1], [3 x i16] [i16 1029, i16 25701, i16 0], [3 x i16] [i16 1779, i16 26725, i16 1], [3 x i16] [i16 1030, i16 25718, i16 0], [3 x i16] [i16 1780, i16 26743, i16 1], [3 x i16] [i16 1031, i16 25719, i16 0], [3 x i16] [i16 1784, i16 26624, i16 1], [3 x i16] [i16 1028, i16 25600, i16 0], [3 x i16] [i16 1778, i16 26625, i16 1], [3 x i16] [i16 1029, i16 25618, i16 0], [3 x i16] [i16 1779, i16 26642, i16 1], [3 x i16] [i16 1030, i16 25619, i16 0], [3 x i16] [i16 1780, i16 26643, i16 1], [3 x i16] [i16 1031, i16 25636, i16 0], [3 x i16] [i16 1781, i16 26661, i16 1], [3 x i16] [i16 1025, i16 25637, i16 0]], align 1, !guid !6
@exchange_metadata_size = dso_local local_unnamed_addr constant [735 x i8] zeroinitializer, align 1, !guid !7
@.str = private unnamed_addr constant [3 x i8] c"X1\00", align 1, !guid !8
@.str.1 = private unnamed_addr constant [3 x i8] c"X2\00", align 1, !guid !9
@.str.2 = private unnamed_addr constant [3 x i8] c"X3\00", align 1, !guid !10
@.str.3 = private unnamed_addr constant [3 x i8] c"X4\00", align 1, !guid !11
@.str.4 = private unnamed_addr constant [3 x i8] c"X5\00", align 1, !guid !12
@.str.5 = private unnamed_addr constant [3 x i8] c"X6\00", align 1, !guid !13
@.str.6 = private unnamed_addr constant [3 x i8] c"X7\00", align 1, !guid !14
@.str.7 = private unnamed_addr constant [3 x i8] c"X8\00", align 1, !guid !15
@.str.8 = private unnamed_addr constant [3 x i8] c"X9\00", align 1, !guid !16
@.str.9 = private unnamed_addr constant [4 x i8] c"X10\00", align 1, !guid !17
@.str.10 = private unnamed_addr constant [4 x i8] c"X11\00", align 1, !guid !18
@.str.11 = private unnamed_addr constant [4 x i8] c"X12\00", align 1, !guid !19
@.str.12 = private unnamed_addr constant [4 x i8] c"X13\00", align 1, !guid !20
@.str.13 = private unnamed_addr constant [4 x i8] c"X14\00", align 1, !guid !21
@.str.14 = private unnamed_addr constant [4 x i8] c"X15\00", align 1, !guid !22
@.str.15 = private unnamed_addr constant [4 x i8] c"X16\00", align 1, !guid !23
@.str.16 = private unnamed_addr constant [4 x i8] c"X17\00", align 1, !guid !24
@.str.17 = private unnamed_addr constant [4 x i8] c"X18\00", align 1, !guid !25
@.str.18 = private unnamed_addr constant [4 x i8] c"X19\00", align 1, !guid !26
@.str.19 = private unnamed_addr constant [4 x i8] c"X20\00", align 1, !guid !27
@.str.20 = private unnamed_addr constant [4 x i8] c"X21\00", align 1, !guid !28
@.str.21 = private unnamed_addr constant [4 x i8] c"X22\00", align 1, !guid !29
@.str.22 = private unnamed_addr constant [4 x i8] c"X23\00", align 1, !guid !30
@.str.23 = private unnamed_addr constant [4 x i8] c"X24\00", align 1, !guid !31
@.str.24 = private unnamed_addr constant [4 x i8] c"X25\00", align 1, !guid !32
@.str.25 = private unnamed_addr constant [4 x i8] c"X26\00", align 1, !guid !33
@.str.26 = private unnamed_addr constant [4 x i8] c"X27\00", align 1, !guid !34
@.str.27 = private unnamed_addr constant [4 x i8] c"X28\00", align 1, !guid !35
@.str.28 = private unnamed_addr constant [4 x i8] c"X29\00", align 1, !guid !36
@.str.29 = private unnamed_addr constant [4 x i8] c"X30\00", align 1, !guid !37
@.str.30 = private unnamed_addr constant [4 x i8] c"X31\00", align 1, !guid !38
@.str.31 = private unnamed_addr constant [4 x i8] c"X32\00", align 1, !guid !39
@.str.32 = private unnamed_addr constant [5 x i8] c"S150\00", align 1, !guid !40
@.str.33 = private unnamed_addr constant [5 x i8] c"S153\00", align 1, !guid !41
@.str.34 = private unnamed_addr constant [5 x i8] c"X154\00", align 1, !guid !42
@__const.callback.ticket = private unnamed_addr constant %struct.exec_request { i16 15872, %struct.exec_address { i16 1025, i16 25607 }, i8 0 }, align 1, !guid !43

; Function Attrs: minsize nounwind optsize
define dso_local range(i16 0, 2) i16 @unused_diagnostic_main() local_unnamed_addr #0 !guid !53 {
  %1 = alloca i16, align 1
  %2 = alloca %struct.apollo_overlay_backend, align 1
  call void @llvm.lifetime.start.p0(ptr nonnull %1) #4
  store volatile i8 0, ptr inttoptr (i16 -16256 to ptr), align 128, !tbaa !54
  %3 = load volatile i8, ptr inttoptr (i16 1 to ptr), align 1, !tbaa !54
  %4 = tail call i8 asm sideeffect "php\0Asei\0Apla", "=a,~{memory}"() #4, !srcloc !55
  %5 = and i8 %4, 4
  %6 = icmp eq i8 %5, 0
  br i1 %6, label %8, label %7

7:                                                ; preds = %0
  tail call void asm sideeffect "sei", "~{memory}"() #4, !srcloc !56
  br label %9

8:                                                ; preds = %0
  tail call void asm sideeffect "cli", "~{memory}"() #4, !srcloc !57
  br label %9

9:                                                ; preds = %7, %8
  call void @c64_overlay_backend(ptr dead_on_unwind nonnull writable sret(%struct.apollo_overlay_backend) align 1 %2, ptr noundef nonnull @backend_io) #5
  call void @apollo_overlay_init(ptr noundef nonnull @loader, ptr noundef nonnull @lm_exchange_modules, i8 noundef zeroext 35, ptr nofreeobj noundef nonnull align 1 dead_on_return dereferenceable(10) %2, ptr noundef nonnull inttoptr (i16 -32768 to ptr), ptr noundef nonnull inttoptr (i16 28672 to ptr)) #5
  br label %10

10:                                               ; preds = %126, %9
  %11 = phi i16 [ 0, %9 ], [ %128, %126 ]
  %12 = phi i16 [ 0, %9 ], [ %127, %126 ]
  %13 = icmp eq i16 %11, 64
  br i1 %13, label %129, label %14

14:                                               ; preds = %10
  %15 = and i16 %11, 1
  %16 = icmp eq i16 %15, 0
  br i1 %16, label %18, label %17

17:                                               ; preds = %14
  call void asm sideeffect "sei", "~{memory}"() #4, !srcloc !58
  br label %19

18:                                               ; preds = %14
  call void asm sideeffect "cli", "~{memory}"() #4, !srcloc !59
  br label %19

19:                                               ; preds = %18, %17
  %20 = call i8 asm sideeffect "php\0Asei\0Apla", "=a,~{memory}"() #4, !srcloc !55
  store i16 %15, ptr inttoptr (i16 -16376 to ptr), align 8, !tbaa !60
  store i16 -1, ptr %1, align 1, !tbaa !63
  %21 = lshr i16 %11, 1
  %22 = trunc nuw nsw i16 %21 to i8
  %23 = add nuw nsw i8 %22, 1
  %24 = call zeroext i8 @apollo_overlay_call(ptr noundef nonnull @loader, i8 noundef zeroext %23, i16 noundef -16384, ptr noundef nonnull %1) #5
  %25 = icmp eq i8 %24, 0
  br i1 %25, label %26, label %29

26:                                               ; preds = %19
  %27 = load i16, ptr %1, align 1, !tbaa !63
  %28 = icmp eq i16 %27, 0
  br i1 %28, label %35, label %29

29:                                               ; preds = %26, %19
  %30 = add i16 %12, 1
  %31 = and i8 %20, 4
  %32 = icmp eq i8 %31, 0
  br i1 %32, label %34, label %33

33:                                               ; preds = %29
  call void asm sideeffect "sei", "~{memory}"() #4, !srcloc !56
  br label %126

34:                                               ; preds = %29
  call void asm sideeffect "cli", "~{memory}"() #4, !srcloc !57
  br label %126

35:                                               ; preds = %26
  call void @llvm.memset.p0.i16(ptr noundef nonnull align 256 dereferenceable(735) inttoptr (i16 -16128 to ptr), i8 0, i16 735, i1 false)
  call void @p66_native_import_executive(ptr noundef nonnull inttoptr (i16 -16128 to ptr), ptr noundef nonnull inttoptr (i16 28672 to ptr)) #5
  store ptr inttoptr (i16 -15480 to ptr), ptr inttoptr (i16 -15412 to ptr), align 4, !tbaa !64
  store ptr inttoptr (i16 28672 to ptr), ptr inttoptr (i16 -15410 to ptr), align 2, !tbaa !76
  store ptr inttoptr (i16 -16384 to ptr), ptr inttoptr (i16 -15408 to ptr), align 16, !tbaa !77
  store ptr @change, ptr inttoptr (i16 -15397 to ptr), align 1, !tbaa !78
  %36 = call zeroext i16 @exec_native_resolver_init(ptr noundef nonnull inttoptr (i16 -15480 to ptr), ptr noundef nonnull inttoptr (i16 -16128 to ptr), ptr noundef nonnull @resolve, ptr noundef nonnull inttoptr (i16 -15412 to ptr), ptr noundef nonnull inttoptr (i16 -15412 to ptr)) #5
  %37 = icmp eq i16 %36, 0
  br i1 %37, label %38, label %44

38:                                               ; preds = %35
  %39 = call zeroext i16 @exec_native_bind_irq(ptr noundef nonnull inttoptr (i16 -15480 to ptr), ptr noundef nonnull inttoptr (i16 -15444 to ptr), ptr noundef nonnull @guard) #5
  %40 = icmp eq i16 %39, 0
  br i1 %40, label %41, label %44

41:                                               ; preds = %38
  %42 = call zeroext i16 @exec_overlay_bind(ptr noundef nonnull inttoptr (i16 -15480 to ptr), ptr noundef nonnull @services) #5
  %43 = icmp eq i16 %42, 0
  br i1 %43, label %46, label %44

44:                                               ; preds = %41, %38, %35
  %45 = add i16 %12, 1
  br label %46

46:                                               ; preds = %44, %41
  %47 = phi i16 [ %45, %44 ], [ %12, %41 ]
  store i8 1, ptr inttoptr (i16 -15469 to ptr), align 1, !tbaa !79
  store i8 1, ptr inttoptr (i16 -15457 to ptr), align 1, !tbaa !80
  store i16 513, ptr inttoptr (i16 -15474 to ptr), align 2, !tbaa !81
  store i16 26631, ptr inttoptr (i16 -15472 to ptr), align 16, !tbaa !82
  store i16 0, ptr inttoptr (i16 -15454 to ptr), align 2, !tbaa !83
  store i16 16384, ptr inttoptr (i16 -15456 to ptr), align 32, !tbaa !84
  %48 = and i8 %20, 4
  %49 = icmp eq i8 %48, 0
  br i1 %49, label %51, label %50

50:                                               ; preds = %46
  call void asm sideeffect "sei", "~{memory}"() #4, !srcloc !56
  br label %52

51:                                               ; preds = %46
  call void asm sideeffect "cli", "~{memory}"() #4, !srcloc !57
  br label %52

52:                                               ; preds = %50, %51
  %53 = call zeroext i16 @exec_native_step(ptr noundef nonnull inttoptr (i16 -15480 to ptr)) #5
  %54 = icmp eq i16 %53, 0
  br i1 %54, label %55, label %112

55:                                               ; preds = %52
  %56 = load i16, ptr inttoptr (i16 -15401 to ptr), align 1, !tbaa !85
  %57 = icmp eq i16 %56, 1
  br i1 %57, label %58, label %112

58:                                               ; preds = %55
  %59 = load i16, ptr inttoptr (i16 -15399 to ptr), align 1, !tbaa !86
  %60 = icmp eq i16 %59, 0
  br i1 %60, label %61, label %112

61:                                               ; preds = %58
  %62 = load volatile i8, ptr inttoptr (i16 -15415 to ptr), align 1, !tbaa !87
  %63 = icmp eq i8 %62, 0
  br i1 %63, label %64, label %112

64:                                               ; preds = %61
  %65 = load volatile i8, ptr inttoptr (i16 -15416 to ptr), align 8, !tbaa !88
  %66 = icmp eq i8 %65, 1
  br i1 %66, label %67, label %112

67:                                               ; preds = %64, %79
  %68 = phi i32 [ %89, %79 ], [ -2128831035, %64 ]
  %69 = phi i16 [ %90, %79 ], [ 0, %64 ]
  %70 = icmp eq i16 %69, 2052
  br i1 %70, label %91, label %71

71:                                               ; preds = %67
  %72 = icmp samesign ult i16 %69, 2048
  br i1 %72, label %73, label %75

73:                                               ; preds = %71
  %74 = getelementptr inbounds nuw [2 x i8], ptr inttoptr (i16 28672 to ptr), i16 %69
  br label %79

75:                                               ; preds = %71
  switch i16 %69, label %78 [
    i16 2048, label %79
    i16 2049, label %76
    i16 2050, label %77
  ]

76:                                               ; preds = %75
  br label %79

77:                                               ; preds = %75
  br label %79

78:                                               ; preds = %75
  br label %79

79:                                               ; preds = %78, %77, %76, %75, %73
  %80 = phi ptr [ %74, %73 ], [ inttoptr (i16 -16378 to ptr), %78 ], [ inttoptr (i16 -16382 to ptr), %76 ], [ inttoptr (i16 -16380 to ptr), %77 ], [ inttoptr (i16 -16384 to ptr), %75 ]
  %81 = load i16, ptr %80, align 1, !tbaa !63
  %82 = and i16 %81, 255
  %83 = zext nneg i16 %82 to i32
  %84 = xor i32 %68, %83
  %85 = mul i32 %84, 16777619
  %86 = lshr i16 %81, 8
  %87 = zext nneg i16 %86 to i32
  %88 = xor i32 %85, %87
  %89 = mul i32 %88, 16777619
  %90 = add nuw nsw i16 %69, 1
  br label %67

91:                                               ; preds = %67
  %92 = getelementptr inbounds nuw [4 x i8], ptr @lm_change_hash, i16 %11
  %93 = load i32, ptr %92, align 1, !tbaa !89
  %94 = icmp eq i32 %68, %93
  br i1 %94, label %95, label %112

95:                                               ; preds = %91
  %96 = load i16, ptr inttoptr (i16 -15474 to ptr), align 2, !tbaa !81
  %97 = getelementptr inbounds nuw [6 x i8], ptr @lm_change_next, i16 %11
  %98 = load i16, ptr %97, align 1, !tbaa !63
  %99 = icmp eq i16 %96, %98
  br i1 %99, label %100, label %112

100:                                              ; preds = %95
  %101 = load i16, ptr inttoptr (i16 -15472 to ptr), align 16, !tbaa !82
  %102 = getelementptr inbounds nuw i8, ptr %97, i16 2
  %103 = load i16, ptr %102, align 1, !tbaa !63
  %104 = icmp eq i16 %101, %103
  br i1 %104, label %105, label %112

105:                                              ; preds = %100
  %106 = load i8, ptr inttoptr (i16 -15457 to ptr), align 1, !tbaa !80
  %107 = getelementptr inbounds nuw i8, ptr %97, i16 4
  %108 = load i16, ptr %107, align 1, !tbaa !63
  %109 = icmp ne i16 %108, 0
  %110 = zext i1 %109 to i8
  %111 = icmp eq i8 %106, %110
  br i1 %111, label %114, label %112

112:                                              ; preds = %105, %100, %95, %91, %64, %61, %58, %55, %52
  %113 = add i16 %47, 1
  br label %114

114:                                              ; preds = %112, %105
  %115 = phi i16 [ %113, %112 ], [ %47, %105 ]
  %116 = call i8 asm sideeffect "php\0Apla", "=a,~{memory}"() #4, !srcloc !91
  %117 = and i8 %116, 4
  %118 = zext nneg i8 %117 to i16
  %119 = shl nuw nsw i16 %15, 2
  %120 = icmp eq i16 %119, %118
  br i1 %120, label %121, label %124

121:                                              ; preds = %114
  %122 = load volatile i8, ptr inttoptr (i16 1 to ptr), align 1, !tbaa !54
  %123 = icmp eq i8 %122, %3
  br i1 %123, label %126, label %124

124:                                              ; preds = %121, %114
  %125 = add i16 %115, 1
  br label %126

126:                                              ; preds = %34, %33, %121, %124
  %127 = phi i16 [ %115, %121 ], [ %125, %124 ], [ %30, %33 ], [ %30, %34 ]
  %128 = add nuw nsw i16 %11, 1
  br label %10

129:                                              ; preds = %10
  br i1 %6, label %131, label %130

130:                                              ; preds = %129
  call void asm sideeffect "sei", "~{memory}"() #4, !srcloc !56
  br label %132

131:                                              ; preds = %129
  call void asm sideeffect "cli", "~{memory}"() #4, !srcloc !57
  br label %132

132:                                              ; preds = %130, %131
  %133 = trunc i16 %12 to i8
  store volatile i8 %133, ptr inttoptr (i16 -16255 to ptr), align 1, !tbaa !54
  %134 = lshr i16 %12, 8
  %135 = trunc nuw i16 %134 to i8
  store volatile i8 %135, ptr inttoptr (i16 -16254 to ptr), align 2, !tbaa !54
  store volatile i8 64, ptr inttoptr (i16 -16253 to ptr), align 1, !tbaa !54
  store volatile i8 0, ptr inttoptr (i16 -16252 to ptr), align 4, !tbaa !54
  store volatile i8 3, ptr inttoptr (i16 -16251 to ptr), align 1, !tbaa !54
  store volatile i8 -91, ptr inttoptr (i16 -16256 to ptr), align 128, !tbaa !54
  %136 = icmp ne i16 %12, 0
  %137 = zext i1 %136 to i16
  call void @llvm.lifetime.end.p0(ptr nonnull %1) #4
  ret i16 %137
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(ptr captures(none)) #1

; Function Attrs: minsize nounwind optsize
define internal zeroext i8 @enter(ptr nofree readnone captures(none) %0) #0 !guid !92 {
  %2 = tail call i8 asm sideeffect "php\0Asei\0Apla", "=a,~{memory}"() #4, !srcloc !55
  ret i8 %2
}

; Function Attrs: minsize nounwind optsize
define internal void @leave(ptr nofree readnone captures(none) %0, i8 noundef zeroext %1) #0 !guid !93 {
  %3 = and i8 %1, 4
  %4 = icmp eq i8 %3, 0
  br i1 %4, label %6, label %5

5:                                                ; preds = %2
  tail call void asm sideeffect "sei", "~{memory}"() #4, !srcloc !56
  br label %7

6:                                                ; preds = %2
  tail call void asm sideeffect "cli", "~{memory}"() #4, !srcloc !57
  br label %7

7:                                                ; preds = %6, %5
  ret void
}

; Function Attrs: minsize optsize
declare dso_local void @apollo_overlay_init(ptr noundef, ptr noundef, i8 noundef zeroext, ptr nofreeobj noundef align 1 dead_on_return dereferenceable(10), ptr noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: minsize optsize
declare dso_local void @c64_overlay_backend(ptr dead_on_unwind writable sret(%struct.apollo_overlay_backend) align 1, ptr noundef) local_unnamed_addr #2

; Function Attrs: minsize optsize
declare dso_local zeroext i8 @apollo_overlay_call(ptr noundef, i8 noundef zeroext, i16 noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i16(ptr writeonly captures(none), i8, i16, i1 immarg) #3

; Function Attrs: minsize optsize
declare dso_local void @p66_native_import_executive(ptr noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: minsize nounwind optsize
define internal i16 @change(ptr nofree readnone captures(none) %0, ptr nofree noundef readnone captures(address) %1, ptr noundef %2, ptr noundef %3) #0 !guid !94 {
  %5 = alloca %struct.lm_interpreter_exchange_arguments, align 1
  %6 = alloca i16, align 1
  call void @llvm.lifetime.start.p0(ptr nonnull %5) #4
  store ptr %2, ptr %5, align 1, !tbaa !95
  %7 = getelementptr inbounds nuw i8, ptr %5, i16 2
  store ptr %3, ptr %7, align 1, !tbaa !97
  call void @llvm.lifetime.start.p0(ptr nonnull %6) #4
  store i16 -1, ptr %6, align 1, !tbaa !63
  %8 = icmp eq ptr %1, inttoptr (i16 28672 to ptr)
  br i1 %8, label %9, label %15

9:                                                ; preds = %4
  %10 = ptrtoint ptr %5 to i16
  %11 = call zeroext i8 @apollo_overlay_call(ptr noundef nonnull @loader, i8 noundef zeroext -102, i16 noundef %10, ptr noundef nonnull %6) #5
  %12 = icmp eq i8 %11, 0
  br i1 %12, label %13, label %15

13:                                               ; preds = %9
  %14 = load i16, ptr %6, align 1, !tbaa !63
  br label %15

15:                                               ; preds = %4, %9, %13
  %16 = phi i16 [ %14, %13 ], [ -1, %9 ], [ -1, %4 ]
  call void @llvm.lifetime.end.p0(ptr nonnull %6) #4
  call void @llvm.lifetime.end.p0(ptr nonnull %5) #4
  ret i16 %16
}

; Function Attrs: minsize optsize
declare dso_local zeroext i16 @exec_native_resolver_init(ptr noundef, ptr noundef, ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: minsize nounwind optsize
define internal ptr @resolve(ptr noundef %0, i16 %1, i16 %2, i8 noundef zeroext %3) #0 !guid !98 {
  %5 = icmp eq i16 %1, 513
  br i1 %5, label %6, label %10

6:                                                ; preds = %4
  %7 = icmp eq i16 %2, 26631
  br i1 %7, label %8, label %10

8:                                                ; preds = %6
  %9 = icmp eq i8 %3, 1
  br i1 %9, label %12, label %10

10:                                               ; preds = %8, %6, %4
  %11 = tail call ptr @lm_interpreter_exchange_resolve(ptr noundef %0, i16 %1, i16 %2, i8 noundef zeroext %3) #5
  br label %12

12:                                               ; preds = %8, %10
  %13 = phi ptr [ %11, %10 ], [ @callback, %8 ]
  ret ptr %13
}

; Function Attrs: minsize optsize
declare dso_local zeroext i16 @exec_native_bind_irq(ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: minsize optsize
declare dso_local zeroext i16 @exec_overlay_bind(ptr noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: minsize optsize
declare dso_local zeroext i16 @exec_native_step(ptr noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(ptr captures(none)) #1

; Function Attrs: minsize nounwind optsize
define internal zeroext i8 @callback(ptr noundef %0, ptr noundef %1, ptr noundef %2) #0 !guid !99 {
  %4 = alloca %struct.exec_request, align 1
  %5 = tail call zeroext i8 @lm_interpreter_exchange_callback(ptr noundef %0, ptr noundef %1, ptr noundef %2) #5
  call void @llvm.lifetime.start.p0(ptr nonnull %4) #4
  call void @llvm.memcpy.p0.p0.i16(ptr noundef nonnull align 1 dereferenceable(7) %4, ptr noundef nonnull align 1 dereferenceable(7) @__const.callback.ticket, i16 7, i1 false)
  %6 = call zeroext i16 @exec_irq_post(ptr noundef nonnull inttoptr (i16 -15444 to ptr), ptr noundef nonnull %4) #5
  %7 = icmp eq i16 %6, 0
  br i1 %7, label %9, label %8

8:                                                ; preds = %3
  store i16 6, ptr inttoptr (i16 -15468 to ptr), align 4, !tbaa !100
  br label %9

9:                                                ; preds = %8, %3
  call void @llvm.lifetime.end.p0(ptr nonnull %4) #4
  ret i8 %5
}

; Function Attrs: minsize optsize
declare dso_local ptr @lm_interpreter_exchange_resolve(ptr noundef, i16, i16, i8 noundef zeroext) local_unnamed_addr #2

; Function Attrs: minsize optsize
declare dso_local zeroext i8 @lm_interpreter_exchange_callback(ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i16(ptr noalias writeonly captures(none), ptr noalias readonly captures(none), i16, i1 immarg) #1

; Function Attrs: minsize optsize
declare dso_local zeroext i16 @exec_irq_post(ptr noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: minsize nounwind optsize
define internal i16 @scheduler(ptr nofree readnone captures(none) %0, ptr noundef %1, i16 noundef %2) #0 !guid !101 {
  %4 = alloca i16, align 1
  call void @llvm.lifetime.start.p0(ptr nonnull %4) #4
  store i16 -1, ptr %4, align 1, !tbaa !63
  %5 = getelementptr inbounds nuw i8, ptr %1, i16 8
  %6 = load i8, ptr %5, align 1, !tbaa !102
  %7 = tail call zeroext i8 @exec_overlay_module(i16 noundef %2, i8 noundef zeroext %6) #5
  %8 = ptrtoint ptr %1 to i16
  %9 = call zeroext i8 @apollo_overlay_call(ptr noundef nonnull @loader, i8 noundef zeroext %7, i16 noundef %8, ptr noundef nonnull %4) #5
  %10 = icmp eq i8 %9, 0
  br i1 %10, label %11, label %13

11:                                               ; preds = %3
  %12 = load i16, ptr %4, align 1, !tbaa !63
  br label %13

13:                                               ; preds = %3, %11
  %14 = phi i16 [ %12, %11 ], [ -1, %3 ]
  call void @llvm.lifetime.end.p0(ptr nonnull %4) #4
  ret i16 %14
}

; Function Attrs: minsize optsize
declare dso_local zeroext i8 @exec_overlay_module(i16 noundef, i8 noundef zeroext) local_unnamed_addr #2

attributes #0 = { minsize nounwind optsize "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="-static-stack" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { minsize optsize "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="-static-stack" }
attributes #3 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: write) }
attributes #4 = { nounwind }
attributes #5 = { minsize nounwind optsize }

!llvm.module.flags = !{!44, !45, !46}
!llvm.ident = !{!47}
!llvm.errno.tbaa = !{!48}

!0 = !{i64 6277172205549186795}
!1 = !{i64 -4228178824228603091}
!2 = !{i64 6101619063449698216}
!3 = !{i64 9201588302767607164}
!4 = !{i64 2123546650842869208}
!5 = !{i64 3865886022675681546}
!6 = !{i64 -1775495445694664417}
!7 = !{i64 -3639099368693231099}
!8 = !{i64 5442620975753461232}
!9 = !{i64 1568388962224897120}
!10 = !{i64 4240847075189224339}
!11 = !{i64 -7020298946865255665}
!12 = !{i64 -8125679564335409967}
!13 = !{i64 -4997913454656144205}
!14 = !{i64 -1803747788397206575}
!15 = !{i64 -3483108387491045013}
!16 = !{i64 -5107290028957025355}
!17 = !{i64 7151299723056765910}
!18 = !{i64 -254844016997925457}
!19 = !{i64 -3712571009949255453}
!20 = !{i64 1144938092229627263}
!21 = !{i64 -3567674349419284605}
!22 = !{i64 1894432614531599954}
!23 = !{i64 591419160425205254}
!24 = !{i64 -1304788044742187447}
!25 = !{i64 -7556911649507746482}
!26 = !{i64 -5820921497858271981}
!27 = !{i64 6496016286162686627}
!28 = !{i64 3478222430770890786}
!29 = !{i64 -6612771038734350003}
!30 = !{i64 -5157708592827996344}
!31 = !{i64 562334834804551439}
!32 = !{i64 3672490430045385160}
!33 = !{i64 2104688589876170492}
!34 = !{i64 -4398680353289305995}
!35 = !{i64 -2225584472509856556}
!36 = !{i64 -2584870631192905122}
!37 = !{i64 -2953366875945132716}
!38 = !{i64 8837250750312150018}
!39 = !{i64 -7680118871870554308}
!40 = !{i64 -4409521231247647833}
!41 = !{i64 -3161566282948232593}
!42 = !{i64 7790086951386647242}
!43 = !{i64 -5091148689080061546}
!44 = !{i32 7, !"frame-pointer", i32 2}
!45 = !{i32 1, !"ThinLTO", i32 0}
!46 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!47 = !{!"clang version 24.0.0git (https://github.com/llvm-mos/llvm-mos cd9145597347753c7cabd1c34d24722d4d8a8f96)"}
!48 = !{!49, !50, i64 0}
!49 = !{!"__libc_errno", !50, i64 0}
!50 = !{!"int", !51, i64 0}
!51 = !{!"omnipotent char", !52, i64 0}
!52 = !{!"Simple C/C++ TBAA"}
!53 = !{i64 -8529188956117541824}
!54 = !{!51, !51, i64 0}
!55 = !{i64 17943, i64 17949, i64 17954}
!56 = !{i64 18058}
!57 = !{i64 18098}
!58 = !{i64 20014}
!59 = !{i64 20054}
!60 = !{!61, !50, i64 8}
!61 = !{!"", !62, i64 0, !50, i64 4, !50, i64 6, !50, i64 8}
!62 = !{!"", !50, i64 0, !50, i64 2}
!63 = !{!50, !50, i64 0}
!64 = !{!65, !68, i64 716}
!65 = !{!"", !66, i64 0, !67, i64 648, !73, i64 684, !74, i64 716}
!66 = !{!"", !51, i64 0, !51, i64 192, !50, i64 632, !50, i64 634, !51, i64 636, !50, i64 640, !50, i64 642, !50, i64 644, !50, i64 646}
!67 = !{!"", !68, i64 0, !68, i64 2, !68, i64 4, !62, i64 6, !51, i64 10, !51, i64 11, !69, i64 12, !70, i64 14, !71, i64 16, !50, i64 18, !51, i64 20, !68, i64 21, !51, i64 23, !62, i64 24, !72, i64 28, !68, i64 30, !68, i64 32, !68, i64 34}
!68 = !{!"any pointer", !51, i64 0}
!69 = !{!"short", !51, i64 0}
!70 = !{!"p1 _ZTS14exec_irq_queue", !68, i64 0}
!71 = !{!"p1 _ZTS14exec_exclusion", !68, i64 0}
!72 = !{!"p1 _ZTS20exec_native_services", !68, i64 0}
!73 = !{!"exec_irq_queue", !51, i64 0, !51, i64 28, !51, i64 29, !51, i64 30, !51, i64 31}
!74 = !{!"", !68, i64 0, !68, i64 2, !68, i64 4, !75, i64 6, !50, i64 11, !50, i64 13, !68, i64 15, !68, i64 17}
!75 = !{!"", !62, i64 0, !51, i64 4}
!76 = !{!65, !68, i64 718}
!77 = !{!65, !68, i64 720}
!78 = !{!65, !68, i64 731}
!79 = !{!65, !51, i64 659}
!80 = !{!65, !51, i64 671}
!81 = !{!65, !50, i64 654}
!82 = !{!65, !50, i64 656}
!83 = !{!65, !50, i64 674}
!84 = !{!65, !50, i64 672}
!85 = !{!65, !50, i64 727}
!86 = !{!65, !50, i64 729}
!87 = !{!65, !51, i64 713}
!88 = !{!65, !51, i64 712}
!89 = !{!90, !90, i64 0}
!90 = !{!"long", !51, i64 0}
!91 = !{i64 21394, i64 21400}
!92 = !{i64 -1449436431262109961}
!93 = !{i64 -7909853088438791161}
!94 = !{i64 485364026260540533}
!95 = !{!96, !68, i64 0}
!96 = !{!"", !68, i64 0, !68, i64 2}
!97 = !{!96, !68, i64 2}
!98 = !{i64 -2140248726263943234}
!99 = !{i64 1523781111817368945}
!100 = !{!65, !69, i64 660}
!101 = !{i64 -5353372106772179117}
!102 = !{!103, !51, i64 8}
!103 = !{!"", !68, i64 0, !68, i64 2, !62, i64 4, !51, i64 8}

^0 = module: (path: "[Regular LTO]", hash: (0, 0, 0, 0, 0))
^1 = gv: (name: "lm_interpreter_exchange_resolve") ; guid = 331297580796577925
^2 = gv: (name: "change", summaries: (function: (module: ^0, flags: (linkage: internal, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 20, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 0, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0), calls: ((callee: ^15)), refs: (^22)))) ; guid = 485364026260540533
^3 = gv: (name: ".str.23", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 562334834804551439
^4 = gv: (name: "exec_native_step") ; guid = 575549766134339217
^5 = gv: (name: ".str.15", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 591419160425205254
^6 = gv: (name: ".str.12", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 1144938092229627263
^7 = gv: (name: "apollo_overlay_init") ; guid = 1459626548539130241
^8 = gv: (name: "callback", summaries: (function: (module: ^0, flags: (linkage: internal, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 11, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 0, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0), calls: ((callee: ^18, tail: 1), (callee: ^24)), refs: (^44)))) ; guid = 1523781111817368945
^9 = gv: (name: ".str.1", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 1568388962224897120
^10 = gv: (name: ".str.14", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 1894432614531599954
^11 = gv: (name: ".str.25", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 2104688589876170492
^12 = gv: (name: "services", summaries: (variable: (module: ^0, flags: (linkage: internal, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1), refs: (^41)))) ; guid = 2123546650842869208
^13 = gv: (name: "exec_overlay_module") ; guid = 2863414175460923129
^14 = gv: (name: ".str.20", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 3478222430770890786
^15 = gv: (name: "apollo_overlay_call") ; guid = 3563527489867794098
^16 = gv: (name: ".str.24", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 3672490430045385160
^17 = gv: (name: "lm_change_hash", summaries: (variable: (module: ^0, flags: (linkage: internal, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 3865886022675681546
^18 = gv: (name: "lm_interpreter_exchange_callback") ; guid = 4193158116456816793
^19 = gv: (name: ".str.2", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 4240847075189224339
^20 = gv: (name: ".str", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 5442620975753461232
^21 = gv: (name: "backend_io", summaries: (variable: (module: ^0, flags: (linkage: internal, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 1, constant: 0)))) ; guid = 6101619063449698216
^22 = gv: (name: "loader", summaries: (variable: (module: ^0, flags: (linkage: internal, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 1, constant: 0)))) ; guid = 6277172205549186795
^23 = gv: (name: ".str.19", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 6496016286162686627
^24 = gv: (name: "exec_irq_post") ; guid = 6662311378049079791
^25 = gv: (name: ".str.9", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 7151299723056765910
^26 = gv: (name: ".str.34", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 7790086951386647242
^27 = gv: (name: "exec_overlay_bind") ; guid = 8241529551924779833
^28 = gv: (name: ".str.30", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 8837250750312150018
^29 = gv: (name: "guard", summaries: (variable: (module: ^0, flags: (linkage: internal, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1), refs: (^61, ^33)))) ; guid = 9201588302767607164
^30 = gv: (name: "c64_overlay_backend") ; guid = 9761448738326156058
^31 = gv: (name: "unused_diagnostic_main", summaries: (function: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 173, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 0, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 1, mustBeUnreachable: 0), calls: ((callee: ^30), (callee: ^7), (callee: ^15), (callee: ^40), (callee: ^36), (callee: ^52), (callee: ^27), (callee: ^4)), refs: (^21, ^22, ^48, ^2, ^58, ^29, ^12, ^17, ^60)))) ; guid = 9917555117592009792
^32 = gv: (name: ".str.4", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 10321064509374141649
^33 = gv: (name: "leave", summaries: (function: (module: ^0, flags: (linkage: internal, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 8, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 0, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 1, mustBeUnreachable: 0)))) ; guid = 10536890985270760455
^34 = gv: (name: ".str.31", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 10766625201838997308
^35 = gv: (name: ".str.17", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 10889832424201805134
^36 = gv: (name: "exec_native_resolver_init") ; guid = 11048362312572572719
^37 = gv: (name: ".str.3", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 11426445126844295951
^38 = gv: (name: ".str.21", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 11833973034975201613
^39 = gv: (name: ".str.18", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 12625822575851279635
^40 = gv: (name: "p66_native_import_executive") ; guid = 12729308957722506809
^41 = gv: (name: "scheduler", summaries: (function: (module: ^0, flags: (linkage: internal, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 15, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 0, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0), calls: ((callee: ^13, tail: 1), (callee: ^15)), refs: (^22)))) ; guid = 13093371966937372499
^42 = gv: (name: ".str.22", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 13289035480881555272
^43 = gv: (name: ".str.8", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 13339454044752526261
^44 = gv: (name: "__const.callback.ticket", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 13355595384629490070
^45 = gv: (name: ".str.5", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 13448830619053407411
^46 = gv: (name: ".str.32", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 14037222842461903783
^47 = gv: (name: ".str.26", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 14048063720420245621
^48 = gv: (name: "lm_exchange_modules", summaries: (variable: (module: ^0, flags: (linkage: internal, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1), refs: (^26, ^54, ^46, ^34, ^28, ^55, ^56, ^57, ^47, ^11, ^16, ^3, ^42, ^38, ^14, ^23, ^39, ^35, ^62, ^5, ^10, ^51, ^6, ^49, ^63, ^25, ^43, ^53, ^59, ^45, ^32, ^37, ^19, ^9, ^20)))) ; guid = 14218565249480948525
^49 = gv: (name: ".str.11", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 14734173063760296163
^50 = gv: (name: "exchange_metadata_size", summaries: (variable: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 14807644705016320517
^51 = gv: (name: ".str.13", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 14879069724290267011
^52 = gv: (name: "exec_native_bind_irq") ; guid = 14903794033606134914
^53 = gv: (name: ".str.7", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 14963635686218506603
^54 = gv: (name: ".str.33", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 15285177790761319023
^55 = gv: (name: ".str.29", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 15493377197764418900
^56 = gv: (name: ".str.28", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 15861873442516646494
^57 = gv: (name: ".str.27", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 16221159601199695060
^58 = gv: (name: "resolve", summaries: (function: (module: ^0, flags: (linkage: internal, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 10, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 0, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0), calls: ((callee: ^1, tail: 1)), refs: (^8)))) ; guid = 16306495347445608382
^59 = gv: (name: ".str.6", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 16642996285312345041
^60 = gv: (name: "lm_change_next", summaries: (variable: (module: ^0, flags: (linkage: internal, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 16671248628014887199
^61 = gv: (name: "enter", summaries: (function: (module: ^0, flags: (linkage: internal, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 2, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 0, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 1, mustBeUnreachable: 0)))) ; guid = 16997307642447441655
^62 = gv: (name: ".str.16", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 17141956028967364169
^63 = gv: (name: ".str.10", summaries: (variable: (module: ^0, flags: (linkage: private, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 0, constant: 1)))) ; guid = 18191900056711626159
^64 = flags: 8
^65 = blockcount: 0
