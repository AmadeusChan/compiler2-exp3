; ModuleID = 'foo-raw.ll'
source_filename = "foo.c"

@arr = common global [1024 x i32] zeroinitializer, align 16

define i32 @abs(i32 %i) {
entry:
  %cmp = icmp slt i32 %i, 0
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %sub = sub nsw i32 0, %i
  br label %if.end

if.else:                                          ; preds = %entry
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %a.0 = phi i32 [ %sub, %if.then ], [ %i, %if.else ]
  ret i32 %a.0
}

define i32 @foo1(i32 %i) {
entry:
  %and = and i32 %i, 1023
  %idxprom = sext i32 %and to i64
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr, i64 0, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  ret i32 %0
}

define i32 @foo2(i32 %i) {
entry:
  %cmp = icmp sge i32 %i, 0
  br i1 %cmp, label %land.lhs.true, label %if.else

land.lhs.true:                                    ; preds = %entry
  %cmp1 = icmp slt i32 %i, 1024
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %land.lhs.true
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr, i64 0, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  br label %return

if.else:                                          ; preds = %land.lhs.true, %entry
  br label %return

return:                                           ; preds = %if.else, %if.then
  %retval.0 = phi i32 [ %0, %if.then ], [ -1, %if.else ]
  ret i32 %retval.0
}

define i32 @foo3(i32 %i) {
entry:
  %cmp = icmp sge i32 %i, 0
  br i1 %cmp, label %land.lhs.true, label %if.else

land.lhs.true:                                    ; preds = %entry
  %cmp1 = icmp slt i32 %i, 1024
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %land.lhs.true
  br label %if.end

if.else:                                          ; preds = %land.lhs.true, %entry
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %x.0 = phi i32 [ %i, %if.then ], [ 0, %if.else ]
  %idxprom = sext i32 %x.0 to i64
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr, i64 0, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  ret i32 %0
}

define i32 @bar1(i32 %i) {
entry:
  %and = and i32 %i, 2047
  %idxprom = sext i32 %and to i64
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr, i64 0, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  ret i32 %0
}

define i32 @bar2(i32 %i) {
entry:
  %cmp = icmp sge i32 %i, 0
  br i1 %cmp, label %land.lhs.true, label %if.else

land.lhs.true:                                    ; preds = %entry
  %cmp1 = icmp sle i32 %i, 1024
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %land.lhs.true
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr, i64 0, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  br label %return

if.else:                                          ; preds = %land.lhs.true, %entry
  br label %return

return:                                           ; preds = %if.else, %if.then
  %retval.0 = phi i32 [ %0, %if.then ], [ -1, %if.else ]
  ret i32 %retval.0
}

define i32 @bar3(i32 %i) {
entry:
  %cmp = icmp sge i32 %i, 0
  br i1 %cmp, label %land.lhs.true, label %if.else

land.lhs.true:                                    ; preds = %entry
  %cmp1 = icmp sle i32 %i, 1024
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %land.lhs.true
  br label %if.end

if.else:                                          ; preds = %land.lhs.true, %entry
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %x.0 = phi i32 [ %i, %if.then ], [ 0, %if.else ]
  %idxprom = sext i32 %x.0 to i64
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr, i64 0, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  ret i32 %0
}

define i32 @interproc(i32 %i) {
entry:
  %addx = add i32 1, 1
  %subx = sub i32 10, 2
  %mulx = mul i32 2, %subx
  %shlx = shl i32 %mulx, 2
  %lshrx = lshr i32 %shlx, 2
  %ashrx = ashr i32 %lshrx, 2
  %andx = and i32 %ashrx, 255
  %orx = or i32 %ashrx, 255
  %xorx = xor i32 %ashrx, 255
  %eqx = icmp eq i32 2, %addx
  %nex = icmp ne i32 10, %addx
  %ugtx = icmp ugt i32 0, %addx
  %ugex = icmp uge i32 1, %addx
  %ultx = icmp ult i32 10, %addx
  %ulex = icmp ule i32 2, %addx
  %slex = icmp sle i32 2147483648, %addx
  %zextx = zext i1 %slex to i32
  %sextx = sext i1 %slex to i32

  %call = call i32 @abs(i32 %i)
  %cmp = icmp slt i32 %call, 1024
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %idxprom = sext i32 %call to i64
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr, i64 0, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  br label %return

if.else:                                          ; preds = %entry
  br label %return

return:                                           ; preds = %if.else, %if.then
  %retval.0 = phi i32 [ %0, %if.then ], [ -1, %if.else ]
  ret i32 %retval.0
}
