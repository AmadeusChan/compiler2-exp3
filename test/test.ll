; ModuleID = 'test.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @neg(i32 %x) #0 {
  %1 = sub nsw i32 0, %x
  ret i32 %1
}

; Function Attrs: nounwind readnone uwtable
define i32 @abs(i32 %x) #1 {
  %1 = icmp sge i32 %x, 0
  br i1 %1, label %2, label %3

; <label>:2                                       ; preds = %0
  br label %5

; <label>:3                                       ; preds = %0
  %4 = call i32 @neg(i32 %x)
  br label %5

; <label>:5                                       ; preds = %3, %2
  %y.0 = phi i32 [ %x, %2 ], [ %4, %3 ]
  ret i32 %y.0
}

; Function Attrs: nounwind uwtable
define void @test(i32 %x) #0 {
  %addx = add i32 %x, 2
  %subx = sub i32 %x, 2
  %mulx = mul i32 %x, 2
  %shlx = shl i32 %x, 2
  %lshrx = lshr i32 %x, 2
  %ashrx = ashr i32 %x, 2
  %andx = and i32 %x, 2
  %orx = or i32 %x, 2
  %xorx = xor i32 %x, 2
  %zextx = zext i32 %x to i64
  %sextx = sext i32 %x to i64
  ret void
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0-2ubuntu4 (tags/RELEASE_380/final)"}
