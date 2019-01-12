; ModuleID = 'simple.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @_Z3addii(i32 %x, i32 %y) #0 {
  %1 = add nsw i32 %x, %y
  ret i32 %1
}

; Function Attrs: nounwind uwtable
define i32 @_Z3subii(i32 %x, i32 %y) #0 {
  %1 = sub nsw i32 0, %y
  %2 = call i32 @_Z3addii(i32 %x, i32 %1)
  ret i32 %2
}

; Function Attrs: nounwind uwtable
define i32 @_Z3absi(i32 %x) #0 {
  %1 = icmp sgt i32 %x, 0
  br i1 %1, label %2, label %3

; <label>:2                                       ; preds = %0
  br label %5

; <label>:3                                       ; preds = %0
  %4 = sub nsw i32 0, %x
  br label %5

; <label>:5                                       ; preds = %3, %2
  %.0 = phi i32 [ %x, %2 ], [ %4, %3 ]
  ret i32 %.0
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0-2ubuntu4 (tags/RELEASE_380/final)"}
