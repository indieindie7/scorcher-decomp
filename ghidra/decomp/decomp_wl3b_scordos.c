// ==== FUN_000408b5 @ 000408b5 body=45 ranges=1

void FUN_000408b5(undefined4 *param_1)

{
  FUN_000407d9(*param_1,param_1);
  return;
}


// ==== FUN_00040886 @ 00040886 body=47 ranges=1

void FUN_00040886(short *param_1)

{
  FUN_00040844((int)*param_1,param_1);
  return;
}


// ==== fn_40b12 @ 00040b12 body=71 ranges=1

/* Cross-build match fn_40b12 (xalign SCORWIN:00433996) */

int fn_40b12(byte *param_1,byte *param_2)

{
  byte bVar1;
  byte bVar2;
  
  while( true ) {
    bVar1 = *param_1;
    bVar2 = *param_2;
    if ((0x40 < bVar1) && (bVar1 < 0x5b)) {
      bVar1 = bVar1 + 0x20;
    }
    if ((0x40 < bVar2) && (bVar2 < 0x5b)) {
      bVar2 = bVar2 + 0x20;
    }
    if ((bVar1 != bVar2) || (bVar2 == 0)) break;
    param_1 = param_1 + 1;
    param_2 = param_2 + 1;
  }
  return (uint)bVar1 - (uint)bVar2;
}


// ==== FUN_0001ee15 @ 0001ee15 body=38 ranges=1

void FUN_0001ee15(void)

{
  undefined4 extraout_ECX;
  int iVar1;
  int extraout_EDX;
  int extraout_EBX;
  undefined8 uVar2;
  
  uVar2 = __CHK();
  iVar1 = 0;
  while (iVar1 != (int)((ulonglong)uVar2 >> 0x20)) {
    FUN_000408b5((int)uVar2);
    uVar2 = CONCAT44(extraout_ECX,extraout_EBX + 4);
    iVar1 = extraout_EDX + 1;
  }
  return;
}


// ==== FUN_0001ee3b @ 0001ee3b body=46 ranges=1

void FUN_0001ee3b(void)

{
  int iVar1;
  
  iVar1 = __CHK();
  FUN_00040886(iVar1 + 2);
  FUN_00040886(iVar1);
  FUN_0001eee0();
  return;
}


// ==== FUN_0001c728 @ 0001c728 body=84 ranges=2

void __stdcall FUN_0001c728(int param_1)

{
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  int extraout_ECX;
  int *extraout_EDX;
  int *extraout_EDX_00;
  int *piVar4;
  int *piVar5;
  int extraout_EBX;
  int unaff_ESI;
  int iVar6;
  undefined8 uVar7;
  
  uVar7 = __CHK();
  iVar6 = (int)((ulonglong)uVar7 >> 0x20);
  puVar1 = (undefined4 *)*(int *)uVar7;
  iVar2 = puVar1[1];
  piVar5 = puVar1 + 2;
  switch(*puVar1) {
  case 0:
    break;
  case 1:
    iVar6 = extraout_EBX;
    break;
  case 2:
    iVar6 = extraout_ECX;
    break;
  case 3:
    iVar6 = param_1;
    break;
  default:
    fn_3fb31();
    piVar5 = extraout_EDX;
    iVar6 = unaff_ESI;
  }
  if ((iVar6 == 0) || (iVar6 == DAT_000bf810)) {
    fn_3fb31();
    piVar5 = extraout_EDX_00;
  }
  if (iVar2 != 0) {
    piVar4 = piVar5;
    for (iVar3 = 0; piVar5 = piVar4 + 1, iVar3 != iVar2 + -1; iVar3 = iVar3 + 1) {
      iVar6 = *(int *)(*piVar4 + iVar6);
      piVar4 = piVar5;
    }
    iVar6 = iVar6 + *piVar4;
  }
  *(int *)uVar7 = (int)piVar5;
  FUN_00010757(iVar6);
  return;
}


// ==== FUN_0001c6bd @ 0001c6bd body=89 ranges=1

void FUN_0001c6bd(void)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  int iVar3;
  undefined4 extraout_ECX;
  undefined4 extraout_EBX;
  undefined8 uVar4;
  
  uVar4 = __CHK();
  iVar3 = (int)uVar4;
  uVar1 = *(undefined4 *)(iVar3 + 0xac);
  s_total_alloc_d_d_3ff1c((undefined4 *)(iVar3 + 0xac),0x10,s_1WebLink_000a0bc6 + 1,1);
  *(undefined4 *)(*(int *)(iVar3 + 0xac) + 0xc) = uVar1;
  puVar2 = *(undefined4 **)(iVar3 + 0xac);
  puVar2[2] = extraout_EBX;
  puVar2[1] = (int)((ulonglong)uVar4 >> 0x20);
  *puVar2 = extraout_ECX;
  return;
}


// ==== FUN_0001ccb0 @ 0001ccb0 body=65 ranges=1

int * FUN_0001ccb0(void)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  undefined8 uVar4;
  
  uVar4 = __CHK();
  piVar1 = (int *)uVar4;
  if (*piVar1 >> 0x10 != 0) {
    piVar2 = piVar1 + 1;
    for (iVar3 = 0; iVar3 != *piVar1 >> 0x10; iVar3 = iVar3 + 1) {
      if ((int)((ulonglong)uVar4 >> 0x20) == *piVar2) {
        return piVar2;
      }
      piVar2 = (int *)((int)piVar2 + (uint)*(ushort *)(piVar2 + 1));
    }
  }
  return (int *)0x0;
}


// ==== FUN_0001c7c7 @ 0001c7c7 body=116 ranges=2

void __stdcall FUN_0001c7c7(int *param_1)

{
  int *piVar1;
  uint *puVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  int *piVar6;
  undefined4 extraout_ECX;
  undefined4 extraout_EBX;
  undefined8 uVar7;
  
  uVar7 = __CHK();
  piVar1 = param_1 + 1;
  iVar3 = *param_1;
  param_1 = piVar1;
  do {
    while( true ) {
      if (piVar1 + iVar3 == param_1) {
        FUN_00010757();
        return;
      }
      piVar6 = (int *)FUN_0001c728(&param_1,(int)uVar7,(int)((ulonglong)uVar7 >> 0x20),extraout_EBX,
                                   extraout_ECX);
      puVar2 = (uint *)(param_1 + 1);
      iVar4 = *param_1;
      param_1 = param_1 + 2;
      uVar5 = *puVar2;
      if (1 < uVar5) break;
LAB_0001c843_1:
      fn_3fb31();
    }
    if (uVar5 < 3) {
      *(short *)piVar6 = (short)iVar4;
    }
    else {
      if (uVar5 != 4) goto LAB_0001c843_1;
      *piVar6 = iVar4;
    }
  } while( true );
}


// ==== FUN_0001c854 @ 0001c854 body=127 ranges=1

void __stdcall FUN_0001c854(int *param_1)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  undefined4 *puVar4;
  undefined4 extraout_ECX;
  uint uVar5;
  undefined4 extraout_EBX;
  int *piVar6;
  undefined8 uVar7;
  
  uVar7 = __CHK();
  piVar1 = param_1 + 1;
  iVar2 = *param_1;
  param_1 = piVar1;
  do {
    if (piVar1 + iVar2 == param_1) {
      FUN_00010757();
      return;
    }
    puVar4 = (undefined4 *)
             FUN_0001c728(&param_1,(int)uVar7,(int)((ulonglong)uVar7 >> 0x20),extraout_EBX,
                          extraout_ECX);
    *puVar4 = param_1;
    uVar5 = 0xffffffff;
    piVar6 = param_1;
    do {
      if (uVar5 == 0) break;
      uVar5 = uVar5 - 1;
      iVar3 = *piVar6;
      piVar6 = (int *)((int)piVar6 + 1);
    } while ((char)iVar3 != '\0');
    param_1 = (int *)((int)param_1 + (~uVar5 + 3 & 0xfffffffc));
  } while( true );
}


// ==== FUN_0001cbee @ 0001cbee body=94 ranges=1

void FUN_0001cbee(void)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 extraout_EBX;
  undefined8 uVar3;
  undefined4 uVar4;
  
  uVar3 = __CHK();
  uVar2 = (undefined4)((ulonglong)uVar3 >> 0x20);
  uVar1 = (undefined4)uVar3;
  uVar4 = 0;
  FUN_0001cb53();
  FUN_0001c8d3(uVar1,uVar2,extraout_EBX,0,0);
  FUN_0001ca58(uVar1,uVar2,extraout_EBX,0,0,uVar4);
  FUN_0001cac5(uVar1,uVar2,extraout_EBX,0,0);
  FUN_0001c93b(uVar1,uVar2,extraout_EBX,0,0);
  return;
}


// ==== FUN_0001cc4c @ 0001cc4c body=100 ranges=1

void __stdcall FUN_0001cc4c(undefined4 param_1)

{
  undefined4 uVar1;
  undefined4 extraout_ECX;
  undefined4 uVar2;
  undefined4 extraout_EBX;
  undefined4 unaff_ESI;
  undefined8 uVar3;
  undefined4 unaff_retaddr;
  undefined4 uVar4;
  
  uVar3 = __CHK();
  uVar2 = (undefined4)((ulonglong)uVar3 >> 0x20);
  uVar1 = (undefined4)uVar3;
  uVar4 = extraout_EBX;
  FUN_0001cb53(uVar1,uVar2);
  FUN_0001c8d3(uVar1,uVar2,param_1,extraout_ECX,unaff_retaddr);
  FUN_0001ca58(uVar1,uVar2,param_1,extraout_ECX,unaff_retaddr,param_1,uVar4);
  FUN_0001cac5(uVar1,uVar2,unaff_retaddr,extraout_ECX,unaff_ESI);
  FUN_00010754();
  return;
}


// ==== FUN_0001ed2f @ 0001ed2f body=66 ranges=1

void FUN_0001ed2f(void)

{
  int *piVar1;
  int *extraout_EDX;
  int *local_10;
  
  local_10 = (int *)__CHK();
  FUN_000408b5();
  piVar1 = local_10 + 1 + *local_10;
  local_10 = local_10 + 1;
  while (piVar1 != local_10) {
    FUN_0001ebdd(&local_10);
    local_10 = local_10 + 1;
    FUN_000408b5();
    piVar1 = extraout_EDX;
  }
  return;
}


// ==== FUN_0001edcd @ 0001edcd body=72 ranges=1

void FUN_0001edcd(void)

{
  int *piVar1;
  int *piVar2;
  int *extraout_EDX;
  int *local_14;
  
  local_14 = (int *)__CHK();
  FUN_000408b5();
  piVar2 = local_14 + 1 + *local_14;
  local_14 = local_14 + 1;
  while( true ) {
    piVar1 = local_14;
    if (piVar2 == local_14) break;
    local_14 = local_14 + 1;
    FUN_000408b5(piVar1);
    FUN_0001ebdd(&local_14);
    piVar2 = extraout_EDX;
  }
  return;
}


// ==== FUN_0001ed78 @ 0001ed78 body=85 ranges=1

void FUN_0001ed78(void)

{
  int *piVar1;
  int *extraout_EDX;
  int *local_14;
  
  local_14 = (int *)__CHK();
  FUN_000408b5();
  piVar1 = local_14 + 1 + *local_14;
  local_14 = local_14 + 1;
  while (piVar1 != local_14) {
    FUN_0001ebdd(&local_14);
    local_14 = local_14 + 1;
    FUN_000408b5();
    local_14 = local_14 + 1;
    FUN_000408b5();
    piVar1 = extraout_EDX;
  }
  return;
}


// ==== FUN_0001ec1c @ 0001ec1c body=102 ranges=1

void FUN_0001ec1c(void)

{
  int iVar1;
  uint uVar2;
  int *piVar3;
  int *extraout_EDX;
  int *local_18;
  
  local_18 = (int *)__CHK();
  FUN_000408b5();
  piVar3 = local_18 + 1 + *local_18;
  local_18 = local_18 + 1;
  do {
    if (piVar3 == local_18) {
      return;
    }
    FUN_0001ebdd(&local_18);
    local_18 = local_18 + 1;
    FUN_000408b5();
    uVar2 = 0xffffffff;
    piVar3 = local_18;
    do {
      if (uVar2 == 0) break;
      uVar2 = uVar2 - 1;
      iVar1 = *piVar3;
      piVar3 = (int *)((int)piVar3 + 1);
    } while ((char)iVar1 != '\0');
    local_18 = (int *)((int)local_18 + (~uVar2 + 3 & 0xfffffffc));
    piVar3 = extraout_EDX;
  } while( true );
}


// ==== FUN_0001ec82 @ 0001ec82 body=88 ranges=1

void FUN_0001ec82(void)

{
  int iVar1;
  uint uVar2;
  int *piVar3;
  int *extraout_EDX;
  int *local_18;
  
  local_18 = (int *)__CHK();
  FUN_000408b5();
  piVar3 = local_18 + 1 + *local_18;
  local_18 = local_18 + 1;
  do {
    if (piVar3 == local_18) {
      return;
    }
    FUN_0001ebdd(&local_18);
    uVar2 = 0xffffffff;
    piVar3 = local_18;
    do {
      if (uVar2 == 0) break;
      uVar2 = uVar2 - 1;
      iVar1 = *piVar3;
      piVar3 = (int *)((int)piVar3 + 1);
    } while ((char)iVar1 != '\0');
    local_18 = (int *)((int)local_18 + (~uVar2 + 3 & 0xfffffffc));
    piVar3 = extraout_EDX;
  } while( true );
}


// ==== FUN_0001ecda @ 0001ecda body=23 ranges=1

void FUN_0001ecda(void)

{
  int *piVar1;
  int *extraout_EDX;
  int *local_14;
  
  local_14 = (int *)__CHK();
  FUN_000408b5();
  piVar1 = local_14 + 1 + *local_14;
  local_14 = local_14 + 1;
  while (piVar1 != local_14) {
    FUN_0001ebdd(&local_14);
    local_14 = local_14 + 1;
    FUN_000408b5();
    local_14 = local_14 + 1;
    FUN_000408b5();
    piVar1 = extraout_EDX;
  }
  return;
}


// ==== FUN_0003f539 @ 0003f539 body=414 ranges=1

void __stdcall FUN_0003f539(undefined4 param_1)

{
  uint uVar1;
  undefined4 uVar2;
  int extraout_EDX;
  int iVar3;
  int extraout_EDX_00;
  undefined8 uVar4;
  byte local_230 [512];
  undefined *local_30;
  int local_2c;
  undefined1 *local_20;
  int local_1c;
  
  if (DAT_000a64e0 == 0) {
    local_20 = &stack0x00000008;
    vsprintf(local_230,param_1,&local_20);
    local_20 = (undefined1 *)0x0;
    local_1c = 0;
    local_2c = 0;
    iVar3 = extraout_EDX;
    while (uVar1 = DAT_000a92d4 >> 2, DAT_000d2014 < (int)uVar1) {
      if (local_230[local_2c] < 0x20) {
        iVar3 = DAT_000a92d8 % 6;
        (&DAT_000c7fd4)[(DAT_000a92d8 / 6 + -1) * 200 + DAT_000d2014] = 0;
        uVar1 = (uint)local_230[local_2c];
        if (uVar1 != 10) break;
        local_1c = 1;
        FUN_0003f497();
        iVar3 = extraout_EDX_00;
      }
      else {
        iVar3 = (DAT_000a92d8 / 6 + -1) * 200 + DAT_000d2014;
        (&DAT_000c7fd4)[iVar3] = local_230[local_2c];
        DAT_000d2014 = DAT_000d2014 + 1;
      }
      local_2c = local_2c + 1;
    }
    uVar4 = CONCAT44(iVar3,uVar1);
    if (DAT_000d2018 == 1) {
      local_30 = PTR_s_c__3dzwork_outlort_txt_000a9060;
      iVar3 = FUN_00040b59(s_error_000a427f,local_230,5);
      if (((iVar3 == 0) || (iVar3 = FUN_00040b59(s_fatal_000a4285,local_230,5), iVar3 == 0)) ||
         (iVar3 = FUN_00040b59(&DAT_000a428b,local_230,4), iVar3 == 0)) {
        local_30 = PTR_s_c__3dzwork_outlort_err_000a9064;
      }
      uVar2 = fn_4a38c(local_230);
      uVar4 = s_Error_creating2_s_3f1f2(local_30,local_230,uVar2);
    }
    iVar3 = FUN_00020e58((int)uVar4,(int)((ulonglong)uVar4 >> 0x20));
    if ((iVar3 == 0) && (local_1c == 1)) {
      FUN_0003fa7d();
    }
  }
  return;
}


// ==== FUN_0003faa9 @ 0003faa9 body=136 ranges=1

undefined8 FUN_0003faa9(undefined4 param_1,undefined4 param_2)

{
  undefined4 local_1c;
  
  if (DAT_000bf988 == 0) {
    local_1c = 1;
  }
  else {
    do {
      if (*(char *)(DAT_000c629c + 0x1c) == '\x01') goto LAB_0003fadb;
    } while (*(char *)(DAT_000c629c + 1) != '\x01');
    do {
    } while (*(char *)(DAT_000c629c + 1) == '\x01');
    local_1c = 0;
  }
LAB_0003fb25:
  return CONCAT44(param_2,local_1c);
LAB_0003fadb:
  do {
  } while (*(char *)(DAT_000c629c + 0x1c) == '\x01');
  local_1c = 1;
  goto LAB_0003fb25;
}


// ==== FUN_000112f4 @ 000112f4 body=40 ranges=1

void FUN_000112f4(void)

{
  undefined8 uVar1;
  
  uVar1 = __CHK();
  *(undefined4 *)((ulonglong)uVar1 >> 0x20) =
       *(undefined4 *)(*(int *)(DAT_000bf80c + 0x2c) + (int)uVar1 * 0x14);
  return;
}


// ==== FUN_0001131c @ 0001131c body=69 ranges=1

int FUN_0001131c(void)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  int *piVar5;
  int *piVar6;
  uint *extraout_EBX;
  undefined8 uVar7;
  
  uVar7 = __CHK();
  piVar5 = (int *)((ulonglong)uVar7 >> 0x20);
  piVar6 = (int *)*piVar5;
  uVar1 = *extraout_EBX;
  if ((*piVar6 == 0) && (*(int *)((int)uVar7 + 0xc) <= (int)uVar1)) {
    return 0;
  }
  uVar2 = *(uint *)((int)uVar7 + 8);
  uVar4 = uVar1 + 1;
  iVar3 = piVar6[uVar1 + 2];
  if (uVar4 == uVar2) {
    piVar6 = (int *)*piVar6;
    uVar4 = uVar4 ^ uVar2;
  }
  *extraout_EBX = uVar4;
  *piVar5 = (int)piVar6;
  return iVar3;
}


// ==== FUN_00011474 @ 00011474 body=29 ranges=1

void FUN_00011474(void)

{
  int *piVar1;
  int iStack_14;
  
  piVar1 = (int *)__CHK();
  s_total_alloc_d_d_3ff1c(&iStack_14,0x1c,&DAT_000a01e9,1);
  *(int *)(iStack_14 + 0x18) = *piVar1;
  *piVar1 = iStack_14;
  return;
}


// ==== FUN_0001265b @ 0001265b body=89 ranges=1

void __stdcall FUN_0001265b(undefined4 param_1,undefined4 param_2)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 extraout_ECX;
  int *piVar3;
  int extraout_EBX;
  undefined8 uVar4;
  
  uVar4 = __CHK();
  piVar3 = (int *)((ulonglong)uVar4 >> 0x20);
  s_total_alloc_d_d_3ff1c(piVar3,extraout_EBX + 4,extraout_ECX,param_1);
  puVar1 = (undefined4 *)*piVar3;
  puVar2 = puVar1 + 1;
  *piVar3 = (int)puVar2;
  FUN_000123e6((int)uVar4 + 0xa4,puVar2,extraout_EBX,param_2);
  *puVar1 = *(undefined4 *)(*(int *)((int)uVar4 + 0xa4) + 8);
  return;
}


