// ==== wldpul_Init @ 00022995 body=179 ranges=1

void wldpul_Init(void)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  byte bVar6;
  uint uVar5;
  int *extraout_EBX;
  int *extraout_EBX_00;
  int *piVar7;
  int *piVar8;
  bool bVar9;
  undefined8 uVar10;
  uint uStack_14;
  
  uVar10 = __CHK();
  piVar7 = (int *)((ulonglong)uVar10 >> 0x20);
  iVar2 = (int)uVar10;
  iVar1 = *piVar7;
  if (iVar1 != 0) {
    bVar6 = *(byte *)(piVar7 + 0xe);
    if ((bVar6 & 0x10) == 0) {
      if ((bVar6 & 8) != 0) {
        return;
      }
    }
    else {
      *(byte *)(piVar7 + 0xe) = bVar6 & 0xf7;
    }
    if ((*(byte *)(piVar7 + 0xe) & 0x20) == 0) {
      iVar4 = fn_12030c(piVar7[0xb],PTR_FUN_000a648c);
      piVar7 = extraout_EBX;
    }
    else {
      iVar4 = piVar7[0xb];
    }
    piVar7[10] = piVar7[10] + iVar4;
    iVar4 = piVar7[0xd];
    if ((*(byte *)(piVar7 + 0xe) & 1) == 0) {
      iVar4 = iVar4 + -1;
    }
    bVar9 = false;
    piVar7[0xc] = (piVar7[10] >> 0x10) % iVar4;
    if (iVar4 <= piVar7[10] >> 0x10) {
      bVar9 = (*(byte *)(piVar7 + 0xe) & 0x10) != 0;
      if (bVar9) {
        bVar6 = *(byte *)(piVar7 + 0xe) & 0xe7;
        *(byte *)(piVar7 + 0xe) = bVar6;
        *(byte *)(piVar7 + 0xe) = bVar6 | 8;
      }
      if ((*(byte *)(piVar7 + 0xe) & 2) != 0) {
        bVar9 = true;
      }
    }
    iVar4 = *(int *)(iVar1 + piVar7[0xc] * 4);
    uVar5 = piVar7[0xc] + 1;
    if (uVar5 == piVar7[0xd]) {
      uVar5 = uVar5 ^ piVar7[0xd];
    }
    iVar1 = *(int *)(iVar1 + uVar5 * 4);
    uStack_14 = piVar7[10] & 0xffff;
    if ((*(byte *)(piVar7 + 0xe) & 4) != 0) {
      iVar3 = fn_10170(0xc000 - uStack_14 / 2);
      uStack_14 = (iVar3 >> 0xf) + 0x8000;
      piVar7 = extraout_EBX_00;
    }
    fn_22718(uStack_14,iVar4 + 0x10,iVar1 + 0x10,piVar7 + 1);
    piVar7 = piVar7 + 1;
    piVar8 = (int *)(iVar2 + 0x5c);
    for (iVar3 = 9; iVar3 != 0; iVar3 = iVar3 + -1) {
      *piVar8 = *piVar7;
      piVar7 = piVar7 + 1;
      piVar8 = piVar8 + 1;
    }
    for (iVar3 = 0; iVar3 != 0; iVar3 = iVar3 + -1) {
      *(char *)piVar8 = (char)*piVar7;
      piVar7 = (int *)((int)piVar7 + 1);
      piVar8 = (int *)((int)piVar8 + 1);
    }
    iVar3 = fn_12030c(*(int *)(iVar1 + 4) - *(int *)(iVar4 + 4),uStack_14);
    *(int *)(iVar2 + 0x20) = iVar3 + *(int *)(iVar4 + 4);
    iVar3 = fn_12030c(*(int *)(iVar1 + 8) - *(int *)(iVar4 + 8),uStack_14);
    *(int *)(iVar2 + 0x24) = iVar3 + *(int *)(iVar4 + 8);
    iVar3 = fn_12030c(*(int *)(iVar1 + 0xc) - *(int *)(iVar4 + 0xc),uStack_14);
    iVar1 = *(int *)(iVar2 + 0x98);
    *(int *)(iVar2 + 0x28) = iVar3 + *(int *)(iVar4 + 0xc);
    if (bVar9) {
      *(undefined4 *)(iVar1 + 0x18) = 0;
      *(undefined4 *)(iVar1 + 0x1c) = 0;
      *(undefined4 *)(iVar1 + 0x20) = 0;
    }
    else {
      *(int *)(iVar1 + 0x18) = *(int *)(iVar2 + 0x20) - *(int *)(iVar1 + 4);
      *(int *)(iVar1 + 0x1c) = *(int *)(iVar2 + 0x24) - *(int *)(iVar1 + 8);
      *(int *)(iVar1 + 0x20) = *(int *)(iVar2 + 0x28) - *(int *)(iVar1 + 0xc);
    }
    *(undefined4 *)(iVar1 + 4) = *(undefined4 *)(iVar2 + 0x20);
    *(undefined4 *)(iVar1 + 8) = *(undefined4 *)(iVar2 + 0x24);
    *(undefined4 *)(iVar1 + 0xc) = *(undefined4 *)(iVar2 + 0x28);
  }
  return;
}


// ==== trainwpul_RateLoop @ 00022ba0 body=18 ranges=1

void trainwpul_RateLoop(void)

{
  undefined8 uVar1;
  
  uVar1 = __CHK();
  wldpul_RateLoop((int)uVar1,(int)((ulonglong)uVar1 >> 0x20) + 0x10);
  return;
}


// ==== trainwpul_PreInit @ 00022bb2 body=13 ranges=1

void trainwpul_PreInit(void)

{
  ushort uVar1;
  int *piVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  int extraout_ECX;
  int *piVar6;
  int *piVar7;
  int *piVar8;
  int extraout_EDX;
  uint uVar9;
  int extraout_EBX;
  int extraout_EBX_00;
  int extraout_EBX_01;
  int extraout_EBX_02;
  int iVar10;
  undefined4 *puVar11;
  undefined4 *puVar12;
  int unaff_EDI;
  undefined8 uVar13;
  int iStack_18;
  uint uStack_14;
  uint uStack_10;
  
  uVar13 = __CHK();
  uVar13 = __CHK((int)uVar13,(int)((ulonglong)uVar13 >> 0x20) + 0x10);
  piVar8 = (int *)((ulonglong)uVar13 >> 0x20);
  iVar3 = (int)uVar13;
  iVar5 = *(int *)(iVar3 + 8);
  uVar4 = wl3_FindChunk(*(undefined4 *)(iVar3 + 0xb4),0x4d504f53);
  iVar5 = wl3_FindChunk(iVar5 + 0x5c,0x4d504f53,uVar4,uVar4);
  if ((extraout_ECX == 0) && (iVar5 == 0)) {
    dbg_Printf(0,0);
    piVar8[0xd] = extraout_EDX;
  }
  else {
    if (extraout_EBX_02 == 0) {
      uVar1 = *(ushort *)(iVar5 + 6);
      uStack_10 = 0;
      iVar10 = iVar5 + 8;
    }
    else {
      uVar1 = *(ushort *)(extraout_EBX_02 + 6);
      iVar10 = extraout_EBX_02 + 8;
      uStack_10 = 0;
      if (iVar5 != 0) {
        uStack_10 = (uint)*(ushort *)(iVar5 + 6);
        unaff_EDI = iVar5 + 8;
      }
    }
    uStack_14 = (uint)uVar1;
    iStack_18 = uStack_14 + uStack_10;
    if ((*(byte *)(piVar8 + 0xe) & 1) != 0) {
      iStack_18 = iStack_18 * 2 + -2;
    }
    s_total_alloc_d_d_3ff1c(piVar8,iStack_18 * 4,&DAT_000a1099,1);
    piVar7 = (int *)*piVar8;
    piVar2 = piVar7 + iStack_18;
    piVar6 = piVar7;
    for (uVar9 = 0; uVar9 != uStack_14; uVar9 = uVar9 + 1) {
      *piVar6 = iVar10;
      if ((uVar9 != 0) && ((*(byte *)(piVar8 + 0xe) & 1) != 0)) {
        *piVar2 = iVar10;
      }
      iVar10 = iVar10 + 0x34;
      piVar6 = piVar6 + 1;
      piVar2 = piVar2 + -1;
    }
    piVar6 = piVar7 + iStack_18;
    for (uVar9 = 0; uVar9 != uStack_10; uVar9 = uVar9 + 1) {
      *piVar7 = unaff_EDI;
      if ((uVar9 != 0) && ((*(byte *)(piVar8 + 0xe) & 1) != 0)) {
        *piVar6 = unaff_EDI;
      }
      unaff_EDI = unaff_EDI + 0x34;
      piVar7 = piVar7 + 1;
      piVar6 = piVar6 + -1;
    }
    iVar5 = *(int *)(*piVar8 + 4 + piVar8[0xc] * 4);
    iVar10 = fn_12030c(*(int *)(iVar5 + 4) - *(int *)(*(int *)(*piVar8 + piVar8[0xc] * 4) + 4),
                       piVar8[10]);
    *(int *)(iVar3 + 0x20) = iVar10 + *(int *)(extraout_EBX + 4);
    iVar10 = fn_12030c(*(int *)(iVar5 + 8) - *(int *)(extraout_EBX + 8),piVar8[10]);
    *(int *)(iVar3 + 0x24) = iVar10 + *(int *)(extraout_EBX_00 + 8);
    iVar5 = fn_12030c(*(int *)(iVar5 + 0xc) - *(int *)(extraout_EBX_00 + 0xc),piVar8[10]);
    *(int *)(iVar3 + 0x28) = iVar5 + *(int *)(extraout_EBX_01 + 0xc);
    puVar11 = (undefined4 *)(extraout_EBX_01 + 0x10);
    puVar12 = (undefined4 *)(iVar3 + 0x5c);
    for (iVar5 = 9; iVar5 != 0; iVar5 = iVar5 + -1) {
      *puVar12 = *puVar11;
      puVar11 = puVar11 + 1;
      puVar12 = puVar12 + 1;
    }
    for (iVar5 = 0; iVar5 != 0; iVar5 = iVar5 + -1) {
      *(undefined1 *)puVar12 = *(undefined1 *)puVar11;
      puVar11 = (undefined4 *)((int)puVar11 + 1);
      puVar12 = (undefined4 *)((int)puVar12 + 1);
    }
    piVar8[0xd] = iStack_18;
  }
  return;
}


// ==== wldpul_PreInit @ 00022bbf body=298 ranges=2

void wldpul_PreInit(void)

{
  ushort uVar1;
  int *piVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  int extraout_ECX;
  int *piVar6;
  int *piVar7;
  int *piVar8;
  int extraout_EDX;
  uint uVar9;
  int extraout_EBX;
  int extraout_EBX_00;
  int extraout_EBX_01;
  int extraout_EBX_02;
  int iVar10;
  undefined4 *puVar11;
  undefined4 *puVar12;
  int unaff_EDI;
  undefined8 uVar13;
  int iStack_18;
  uint uStack_14;
  uint uStack_10;
  
  uVar13 = __CHK();
  piVar8 = (int *)((ulonglong)uVar13 >> 0x20);
  iVar3 = (int)uVar13;
  iVar5 = *(int *)(iVar3 + 8);
  uVar4 = wl3_FindChunk(*(undefined4 *)(iVar3 + 0xb4),0x4d504f53);
  iVar5 = wl3_FindChunk(iVar5 + 0x5c,0x4d504f53,uVar4,uVar4);
  if ((extraout_ECX == 0) && (iVar5 == 0)) {
    dbg_Printf(0,0);
    piVar8[0xd] = extraout_EDX;
  }
  else {
    if (extraout_EBX_02 == 0) {
      uVar1 = *(ushort *)(iVar5 + 6);
      uStack_10 = 0;
      iVar10 = iVar5 + 8;
    }
    else {
      uVar1 = *(ushort *)(extraout_EBX_02 + 6);
      iVar10 = extraout_EBX_02 + 8;
      uStack_10 = 0;
      if (iVar5 != 0) {
        uStack_10 = (uint)*(ushort *)(iVar5 + 6);
        unaff_EDI = iVar5 + 8;
      }
    }
    uStack_14 = (uint)uVar1;
    iStack_18 = uStack_14 + uStack_10;
    if ((*(byte *)(piVar8 + 0xe) & 1) != 0) {
      iStack_18 = iStack_18 * 2 + -2;
    }
    s_total_alloc_d_d_3ff1c(piVar8,iStack_18 * 4,&DAT_000a1099,1);
    piVar7 = (int *)*piVar8;
    piVar2 = piVar7 + iStack_18;
    piVar6 = piVar7;
    for (uVar9 = 0; uVar9 != uStack_14; uVar9 = uVar9 + 1) {
      *piVar6 = iVar10;
      if ((uVar9 != 0) && ((*(byte *)(piVar8 + 0xe) & 1) != 0)) {
        *piVar2 = iVar10;
      }
      iVar10 = iVar10 + 0x34;
      piVar6 = piVar6 + 1;
      piVar2 = piVar2 + -1;
    }
    piVar6 = piVar7 + iStack_18;
    for (uVar9 = 0; uVar9 != uStack_10; uVar9 = uVar9 + 1) {
      *piVar7 = unaff_EDI;
      if ((uVar9 != 0) && ((*(byte *)(piVar8 + 0xe) & 1) != 0)) {
        *piVar6 = unaff_EDI;
      }
      unaff_EDI = unaff_EDI + 0x34;
      piVar7 = piVar7 + 1;
      piVar6 = piVar6 + -1;
    }
    iVar5 = *(int *)(*piVar8 + 4 + piVar8[0xc] * 4);
    iVar10 = fn_12030c(*(int *)(iVar5 + 4) - *(int *)(*(int *)(*piVar8 + piVar8[0xc] * 4) + 4),
                       piVar8[10]);
    *(int *)(iVar3 + 0x20) = iVar10 + *(int *)(extraout_EBX + 4);
    iVar10 = fn_12030c(*(int *)(iVar5 + 8) - *(int *)(extraout_EBX + 8),piVar8[10]);
    *(int *)(iVar3 + 0x24) = iVar10 + *(int *)(extraout_EBX_00 + 8);
    iVar5 = fn_12030c(*(int *)(iVar5 + 0xc) - *(int *)(extraout_EBX_00 + 0xc),piVar8[10]);
    *(int *)(iVar3 + 0x28) = iVar5 + *(int *)(extraout_EBX_01 + 0xc);
    puVar11 = (undefined4 *)(extraout_EBX_01 + 0x10);
    puVar12 = (undefined4 *)(iVar3 + 0x5c);
    for (iVar5 = 9; iVar5 != 0; iVar5 = iVar5 + -1) {
      *puVar12 = *puVar11;
      puVar11 = puVar11 + 1;
      puVar12 = puVar12 + 1;
    }
    for (iVar5 = 0; iVar5 != 0; iVar5 = iVar5 + -1) {
      *(undefined1 *)puVar12 = *(undefined1 *)puVar11;
      puVar11 = (undefined4 *)((int)puVar11 + 1);
      puVar12 = (undefined4 *)((int)puVar12 + 1);
    }
    piVar8[0xd] = iStack_18;
  }
  return;
}


// ==== triggerwpul_Init @ 00022c2a body=163 ranges=1

void triggerwpul_Init(void)

{
  byte bVar1;
  byte *pbVar2;
  undefined4 uVar3;
  int iVar4;
  uint uVar5;
  undefined4 extraout_ECX;
  int *extraout_ECX_00;
  undefined4 *extraout_ECX_01;
  undefined4 *puVar6;
  undefined4 extraout_EBX;
  undefined8 uVar7;
  undefined1 auStack_1c [4];
  undefined4 uStack_18;
  undefined4 uStack_14;
  undefined4 uStack_10;
  
  uVar7 = __CHK();
  puVar6 = (undefined4 *)((ulonglong)uVar7 >> 0x20);
  pbVar2 = (byte *)uVar7;
  uStack_14 = extraout_EBX;
  uStack_10 = extraout_ECX;
  uVar3 = fn_112f4(puVar6[3],auStack_1c);
  uStack_18 = 0;
  iVar4 = fn_1131c(uVar3,auStack_1c,&uStack_18);
  iVar4 = *(int *)(*(int *)(*(int *)(iVar4 + 0x98) + 0x24) + 0x7c) >> 0x10;
  if (iVar4 == 0) {
    *puVar6 = 0;
  }
  if (((*extraout_ECX_00 < iVar4) && (extraout_ECX_00[1] < iVar4)) && (iVar4 < extraout_ECX_00[2]))
  {
    *extraout_ECX_00 = iVar4;
    uVar5 = fn_10a36();
    if ((uVar5 & 0xf) == 0) {
      *(byte *)(puVar6 + 0x12) = *(byte *)(puVar6 + 0x12) | 0x10;
      *extraout_ECX_01 = 10000;
    }
  }
  wldpul_Init(pbVar2,puVar6 + 4,uStack_14,uStack_10);
  bVar1 = *pbVar2;
  *pbVar2 = bVar1 & 0xfe;
  if ((*(byte *)(puVar6 + 0x12) & 8) != 0) {
    *pbVar2 = bVar1 & 0xfe | 1;
  }
  return;
}


// ==== objpul_PreInit @ 00022ccd body=40 ranges=1

void objpul_PreInit(void)

{
  int iVar1;
  uint uVar2;
  int *extraout_ECX;
  int extraout_ECX_00;
  int extraout_ECX_01;
  int *piVar3;
  int extraout_EBX;
  int *piVar4;
  uint uVar5;
  int iVar6;
  uint uStack_10;
  
  __CHK();
  iVar1 = wl3_FindChunk(*(int *)(extraout_EBX + 0x14) + 100,0x4d504f53,extraout_ECX);
  if (iVar1 == 0) {
    dbg_Printf();
    *(undefined4 *)(extraout_ECX_01 + 0x34) = 0;
    FUN_00022d7a();
    return;
  }
  iVar6 = iVar1 + 8;
  uVar5 = (uint)*(ushort *)(iVar1 + 6);
  uStack_10 = uVar5;
  if ((*(byte *)(extraout_ECX_00 + 0x38) & 1) != 0) {
    uStack_10 = uVar5 * 2 - 2;
  }
  s_total_alloc_d_d_3ff1c(extraout_ECX,uStack_10 * 4,&DAT_000a10c2,1);
  piVar3 = (int *)*extraout_ECX;
  piVar4 = piVar3 + uStack_10;
  for (uVar2 = 0; uVar2 != uVar5; uVar2 = uVar2 + 1) {
    *piVar3 = iVar6;
    if ((uVar2 != 0) && ((*(byte *)(extraout_ECX + 0xe) & 1) != 0)) {
      *piVar4 = iVar6;
    }
    iVar6 = iVar6 + 0x34;
    piVar3 = piVar3 + 1;
    piVar4 = piVar4 + -1;
  }
  extraout_ECX[0xd] = uStack_10;
  return;
}


// ==== objpul_RateLoop @ 00022d81 body=17 ranges=1

void objpul_RateLoop(void)

{
  undefined4 extraout_ECX;
  
  __CHK();
  FUN_00037e24(extraout_ECX);
  return;
}


// ==== objpul_Frmupdate @ 00022d92 body=219 ranges=1

void objpul_Frmupdate(void)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int *extraout_ECX;
  int *extraout_ECX_00;
  int *piVar4;
  int extraout_ECX_01;
  int extraout_ECX_02;
  uint uVar5;
  int extraout_EBX;
  int iVar6;
  int extraout_EBX_00;
  undefined4 *puVar7;
  undefined4 *puVar8;
  
  __CHK();
  puVar7 = (undefined4 *)(*(int *)(extraout_EBX + 0x10) + 8);
  piVar4 = extraout_ECX;
  if (puVar7 == (undefined4 *)0x0) {
    fn_3fb31();
    piVar4 = extraout_ECX_00;
  }
  iVar3 = *piVar4;
  uVar2 = fn_1204a5(*(undefined4 *)(DAT_000bf80c + 0x130),piVar4[0xb],0x21);
  *(undefined4 *)(extraout_ECX_01 + 0x28) = uVar2;
  iVar6 = *(int *)(extraout_ECX_01 + 0x34);
  if ((*(byte *)(extraout_ECX_01 + 0x38) & 1) == 0) {
    iVar6 = iVar6 + -1;
  }
  iVar6 = (*(int *)(extraout_ECX_01 + 0x28) >> 0x10) % iVar6;
  *(int *)(extraout_ECX_01 + 0x30) = iVar6;
  iVar1 = *(int *)(iVar3 + iVar6 * 4);
  uVar5 = iVar6 + 1;
  if (uVar5 == *(uint *)(extraout_ECX_01 + 0x34)) {
    uVar5 = uVar5 ^ *(uint *)(extraout_ECX_01 + 0x34);
  }
  iVar3 = *(int *)(iVar3 + uVar5 * 4);
  uVar5 = *(uint *)(extraout_ECX_01 + 0x28) & 0xffff;
  iVar6 = extraout_ECX_01;
  if ((*(byte *)(extraout_ECX_01 + 0x38) & 4) != 0) {
    iVar3 = fn_10170(0xc000 - uVar5 / 2);
    uVar5 = (iVar3 >> 0xf) + 0x8000;
    iVar6 = extraout_ECX_02;
    iVar3 = extraout_EBX_00;
  }
  fn_22718(uVar5,iVar1 + 0x10,iVar3 + 0x10,(undefined4 *)(iVar6 + 4));
  puVar8 = (undefined4 *)(iVar6 + 4);
  for (iVar3 = 9; iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar7 = *puVar8;
    puVar8 = puVar8 + 1;
    puVar7 = puVar7 + 1;
  }
  for (iVar3 = 0; iVar3 != 0; iVar3 = iVar3 + -1) {
    *(undefined1 *)puVar7 = *(undefined1 *)puVar8;
    puVar8 = (undefined4 *)((int)puVar8 + 1);
    puVar7 = (undefined4 *)((int)puVar7 + 1);
  }
  FUN_00021875();
  return;
}


// ==== redlight_Coli @ 00022e6d body=207 ranges=1

void redlight_Coli(void)

{
  undefined4 uVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  undefined4 auStack_38 [8];
  undefined4 uStack_18;
  uint uStack_14;
  int iStack_10;
  
  iVar3 = __CHK();
  iVar3 = wl3_FindChunk(*(undefined4 *)(iVar3 + 0xb4),0x4d504f53);
  if (iVar3 == 0) {
    dbg_Printf();
    FUN_0002e5b5();
    return;
  }
  iStack_10 = iVar3 + 8;
  uStack_14 = (uint)*(ushort *)(iVar3 + 6);
  puVar4 = &DAT_000a70dc;
  puVar5 = auStack_38;
  for (iVar3 = 8; iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar5 = *puVar4;
    puVar4 = puVar4 + 1;
    puVar5 = puVar5 + 1;
  }
  while (uStack_14 = uStack_14 - 1, uStack_14 != 0xffffffff) {
    uVar1 = *(undefined4 *)(iStack_10 + 4);
    uStack_18 = *(undefined4 *)(iStack_10 + 0xc);
    uVar2 = *(undefined4 *)(iStack_10 + 8);
    iVar3 = fn_107ed(uVar1,uVar2,uStack_18,0x1f400);
    if (iVar3 != 0) {
      auStack_38[2] = uStack_18;
      auStack_38[0] = uVar1;
      auStack_38[1] = uVar2;
      s_extram_18f02(auStack_38,DAT_000c0c18 + 0x90,0,1,0x7fff,0x1c24,0,0,0);
    }
    iStack_10 = iStack_10 + 0x34;
  }
  return;
}


// ==== flashflare_Coli @ 00023043 body=46 ranges=1

void flashflare_Coli(void)

{
  undefined8 uVar1;
  
  uVar1 = __CHK();
  if ((*(byte *)uVar1 & 1) == 0) {
    s_WARNING_rout_lensflare_g_22f3c
              ((byte *)uVar1,0,0,0,
               *(int *)((ulonglong)uVar1 >> 0x20) * 7 + *(int *)(DAT_000bf80c + 0x30));
  }
  return;
}


// ==== lensflare_Coli @ 00023071 body=573 ranges=1

void lensflare_Coli(void)

{
  ushort uVar1;
  byte *pbVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  int *piVar6;
  undefined4 *puVar7;
  int iVar8;
  int *piVar9;
  undefined4 *puVar10;
  int iVar11;
  undefined4 auStack_a4 [13];
  undefined4 auStack_70 [8];
  int aiStack_50 [4];
  int iStack_40;
  int iStack_3c;
  int iStack_38;
  int iStack_34;
  int iStack_30;
  int iStack_2c;
  undefined1 auStack_28 [4];
  int iStack_24;
  int iStack_20;
  undefined4 uStack_1c;
  uint uStack_18;
  undefined4 uStack_14;
  int iStack_10;
  
  pbVar2 = (byte *)__CHK();
  if ((*pbVar2 & 1) == 0) {
    iVar3 = wl3_FindChunk(*(undefined4 *)(pbVar2 + 0xb4),0x4d504f53);
    if (iVar3 == 0) {
      dbg_Printf();
    }
    else {
      iStack_10 = iVar3 + 8;
      uVar1 = *(ushort *)(iVar3 + 6);
      piVar6 = &DAT_000a7188;
      piVar9 = aiStack_50;
      for (iVar3 = 6; iVar3 != 0; iVar3 = iVar3 + -1) {
        *piVar9 = *piVar6;
        piVar6 = piVar6 + 1;
        piVar9 = piVar9 + 1;
      }
      puVar7 = &DAT_000a71a0;
      puVar10 = auStack_70;
      for (iVar3 = 8; iVar3 != 0; iVar3 = iVar3 + -1) {
        *puVar10 = *puVar7;
        puVar7 = puVar7 + 1;
        puVar10 = puVar10 + 1;
      }
      puVar7 = &DAT_000a71c0;
      puVar10 = auStack_a4;
      uStack_18 = (uint)uVar1;
      for (iVar3 = 0xd; iVar3 != 0; iVar3 = iVar3 + -1) {
        *puVar10 = *puVar7;
        puVar7 = puVar7 + 1;
        puVar10 = puVar10 + 1;
      }
      while (uStack_18 = uStack_18 - 1, uStack_18 != 0xffffffff) {
        uVar4 = *(undefined4 *)(iStack_10 + 4);
        uStack_14 = *(undefined4 *)(iStack_10 + 8);
        uVar5 = *(undefined4 *)(iStack_10 + 0xc);
        iVar3 = fn_107ed(uVar4,uStack_14,uVar5,0x1f400);
        if (iVar3 != 0) {
          iStack_38 = DAT_000bf854 << 0xf;
          iStack_34 = DAT_000bf858 << 0xf;
          uStack_1c = fn_10107(&iStack_38,&iStack_34);
          auStack_70[1] = uStack_14;
          auStack_70[0] = uVar4;
          auStack_70[2] = uVar5;
          s_extram_18f02(auStack_70,DAT_000c0c18 + 0x84,1,0,0x7fff,0x1c24,0,0,0);
          uVar4 = fn_180c8(uVar4,uStack_14,uVar5,&iStack_30,&iStack_2c,auStack_28);
          iStack_24 = DAT_000bf85c * 0x10000 - iStack_30;
          iStack_20 = DAT_000bf860 * 0x10000 - iStack_2c;
          uVar5 = fn_10107(&iStack_24,&iStack_20);
          iStack_30 = iStack_30 >> 0x10;
          iStack_2c = iStack_2c >> 0x10;
          uVar5 = fn_1204a5(uVar5,0xa000,uStack_1c);
          iStack_24 = fn_12030c(iStack_24,uVar5);
          iStack_20 = fn_12030c(iStack_20,uVar5);
          iStack_40 = fn_12030c(0x66,uVar4);
          iStack_40 = iStack_40 >> 5;
          iVar8 = 0;
          iVar3 = fn_12030c(0xa3,uVar4);
          iVar11 = 0;
          iStack_3c = iVar3 >> 5;
          do {
            iVar3 = fn_12030c(iStack_24,*(undefined4 *)((int)auStack_a4 + iVar8));
            aiStack_50[0] = iStack_30 + iVar3;
            iVar3 = fn_12030c(iStack_20,*(undefined4 *)((int)auStack_a4 + iVar8));
            aiStack_50[1] = iStack_2c + iVar3;
            if (iVar11 != 1) {
              s_extram_18f02(aiStack_50,DAT_000c0c18 + 0x84,iVar11,0,0x7fff,0x52824,0,0,0);
            }
            iVar11 = iVar11 + 1;
            iVar8 = iVar8 + 4;
          } while (iVar11 != 5);
        }
        iStack_10 = iStack_10 + 0x34;
      }
    }
  }
  return;
}


// ==== smoke_Frmupdate @ 00023345 body=248 ranges=1

void smoke_Frmupdate(void)

{
  ushort uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  int iVar4;
  uint uVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  undefined4 auStack_38 [8];
  undefined4 uStack_18;
  uint uStack_14;
  int iStack_10;
  
  iVar4 = __CHK();
  iVar4 = wl3_FindChunk(*(undefined4 *)(iVar4 + 0xb4),0x4d504f53);
  if (iVar4 == 0) {
    dbg_Printf();
    FUN_0002e5b5();
    return;
  }
  iStack_10 = iVar4 + 8;
  uVar1 = *(ushort *)(iVar4 + 6);
  puVar6 = &DAT_000a71f4;
  puVar7 = auStack_38;
  for (iVar4 = 8; iVar4 != 0; iVar4 = iVar4 + -1) {
    *puVar7 = *puVar6;
    puVar6 = puVar6 + 1;
    puVar7 = puVar7 + 1;
  }
  auStack_38[5] = 0;
  auStack_38[4] = 0xfffff380;
  auStack_38[3] = 0;
  uStack_14 = (uint)uVar1;
  while (uStack_14 = uStack_14 - 1, uStack_14 != 0xffffffff) {
    uVar2 = *(undefined4 *)(iStack_10 + 4);
    uStack_18 = *(undefined4 *)(iStack_10 + 8);
    uVar3 = *(undefined4 *)(iStack_10 + 0xc);
    iVar4 = fn_107ed(uVar2,uStack_18,uVar3,0x1f400);
    if (iVar4 != 0) {
      uVar5 = fn_10a36();
      if ((*(uint *)(DAT_000bf80c + 0x30) & 3) == (uVar5 & 3)) {
        auStack_38[1] = uStack_18;
        auStack_38[0] = uVar2;
        auStack_38[2] = uVar3;
        s_extram_18f02(auStack_38,DAT_000c0c18 + 0x18,0,0xc,0x7fff,0x21d20,0,0,0);
      }
    }
    iStack_10 = iStack_10 + 0x34;
  }
  return;
}


// ==== speed0_Frmupdate @ 0002343d body=136 ranges=1

void speed0_Frmupdate(void)

{
  undefined1 *puVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  undefined8 uVar6;
  int iStack_18;
  undefined1 auStack_14 [4];
  undefined4 uStack_10;
  
  uVar6 = __CHK();
  iVar2 = (int)uVar6;
  iVar3 = fn_232ae(iVar2,&iStack_18,*(undefined4 *)((ulonglong)uVar6 >> 0x20));
  if (iVar3 < 0x1f400) {
    uVar4 = fn_112f4(7,auStack_14);
    uStack_10 = 0;
    iVar5 = fn_1131c(uVar4,auStack_14,&uStack_10);
    puVar1 = *(undefined1 **)(iStack_18 + 0x98);
    *(int *)(puVar1 + 0x18) = *(int *)(puVar1 + 0x18) + (*(int *)(iVar2 + 0x5c) >> 0x11);
    *(int *)(puVar1 + 0x1c) = *(int *)(puVar1 + 0x1c) + (*(int *)(iVar2 + 0x60) >> 0x11);
    iVar3 = *(int *)(iVar2 + 100);
    *puVar1 = 1;
    *(int *)(puVar1 + 0x20) = *(int *)(puVar1 + 0x20) + (iVar3 >> 0x11);
    *(int *)(iVar5 + 0x2c) = *(int *)(iVar5 + 0x2c) + 5;
    fn_1d7c8(iVar2);
  }
  return;
}


// ==== enecon01_PreInit @ 0002423f body=199 ranges=1

void enecon01_PreInit(void)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  char *extraout_EBX;
  char *pcVar5;
  undefined8 uVar6;
  undefined4 uStack_18;
  undefined4 uStack_14;
  undefined4 uStack_10;
  
  uVar6 = __CHK();
  puVar4 = (undefined4 *)((ulonglong)uVar6 >> 0x20);
  iVar2 = (int)uVar6;
  if (DAT_000a6650 == 2) {
    fn_1d7c8();
  }
  else {
    *(undefined4 **)(iVar2 + 0x9c) = puVar4;
    s_lev1_24188();
    iVar1 = *(int *)(iVar2 + 0x98);
    *(undefined4 *)(iVar1 + 0x10) = 64000;
    *(undefined4 *)(iVar1 + 0x14) = 0x96;
    fn_28366(iVar2,0,&DAT_00027e74,0x5dc00);
    pcVar5 = (char *)(puVar4 + 0x3d);
    if (DAT_000a6630 != 0) {
      uVar3 = fn_12030c(puVar4[0x3e],FUN_00012000);
      *(undefined4 *)(extraout_EBX + 4) = uVar3;
      pcVar5 = extraout_EBX;
    }
    if (*pcVar5 == '\x01') {
      s_got_joy_ptr_0_27bff(iVar2,&uStack_18,&uStack_14,&uStack_10);
      *(undefined4 *)(iVar2 + 0x20) = uStack_18;
      *(undefined4 *)(iVar2 + 0x24) = uStack_14;
      *(undefined4 *)(iVar2 + 0x28) = uStack_10;
      *puVar4 = *(undefined4 *)(iVar2 + 0x20);
      puVar4[1] = *(undefined4 *)(iVar2 + 0x24);
      puVar4[2] = *(undefined4 *)(iVar2 + 0x28);
    }
    fn_2390f(iVar2,puVar4);
  }
  return;
}


// ==== pickcon01_PreInit @ 00024306 body=46 ranges=1

void pickcon01_PreInit(void)

{
  int iVar1;
  undefined8 uVar2;
  
  uVar2 = __CHK();
  *(int *)((int)uVar2 + 0x9c) = (int)((ulonglong)uVar2 >> 0x20);
  s_lev1_24188();
  iVar1 = *(int *)((int)uVar2 + 0x98);
  *(undefined4 *)(iVar1 + 0x10) = 64000;
  *(undefined4 *)(iVar1 + 0x14) = 0x96;
  return;
}


// ==== bilstyring_PreInit @ 00024334 body=107 ranges=1

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void bilstyring_PreInit(void)

{
  int iVar1;
  int iVar2;
  int extraout_EDX;
  undefined8 uVar3;
  
  uVar3 = __CHK();
  iVar2 = (int)((ulonglong)uVar3 >> 0x20);
  _DAT_000c0cf8 = (int)uVar3;
  *(int *)(_DAT_000c0cf8 + 0x9c) = iVar2;
  iVar1 = *(int *)(_DAT_000c0cf8 + 0x98);
  *(undefined4 *)(iVar1 + 0x10) = 64000;
  *(undefined4 *)(iVar1 + 0x14) = 500;
  s_lev1_24188();
  if (*(int *)(*(int *)(extraout_EDX + 0x24) + 0x74) != 0) {
    DAT_000c0cec = 0;
    DAT_000c0ce8 = 0;
    *(undefined4 *)(iVar2 + 0xf8) = 0x10000;
    *(undefined4 *)(iVar2 + 0xd8) = 0xa000;
    fn_246f2(1);
  }
  return;
}


// ==== initglobal_PreInit @ 0002439f body=98 ranges=1

void initglobal_PreInit(void)

{
  undefined4 *extraout_EDX;
  undefined4 extraout_EDX_00;
  
  __CHK();
  s_error_loading_file_s_4027d(&DAT_000c0ce4,extraout_EDX[1],0xfffffffd);
  fn_24450(DAT_000c0ce4,extraout_EDX_00,1);
  s_error_loading_file_s_4027d(&DAT_000c0cdc,*extraout_EDX);
  bswap16_at(DAT_000c0cdc);
  bswap16_at((int)DAT_000c0cdc + 2);
  DAT_000c0ccc = *DAT_000c0cdc >> 0x10;
  return;
}


// ==== initglobal_RateLoop @ 00024401 body=79 ranges=1

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void initglobal_RateLoop(void)

{
  __CHK();
  _DAT_000c0cf8 = 0;
  if (DAT_000c0ce4 != 0) {
    fn_40108(&DAT_000c0ce4,0xfffffffd);
  }
  if (DAT_000c0cd8 != 0) {
    fn_40108(&DAT_000c0cd8,0);
  }
  fn_40108(&DAT_000c0cdc,1);
  return;
}


// ==== insrestarts_PreInit @ 000275be body=448 ranges=1

void insrestarts_PreInit(void)

{
  uint uVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  int iVar4;
  uint *puVar5;
  uint uVar6;
  undefined8 uVar7;
  undefined1 auStack_b4 [116];
  int iStack_40;
  uint *puStack_30;
  uint uStack_2c;
  uint uStack_28;
  undefined1 auStack_24 [4];
  int iStack_20;
  undefined1 auStack_1c [4];
  int iStack_18;
  uint uStack_14;
  uint uStack_10;
  
  uVar7 = __CHK();
  puVar3 = (undefined4 *)((ulonglong)uVar7 >> 0x20);
  uVar2 = wl3_FindChunk(*(int *)((int)uVar7 + 8) + 0x5c,0x4d504f53);
  uVar7 = fn_11361(puVar3[1],uVar2);
  iVar4 = (int)((ulonglong)uVar7 >> 0x20);
  uVar6 = iVar4 + 8;
  if ((int)uVar7 == 0) {
    s_ATAB3_111d8(*puVar3);
    FUN_0002777e();
    return;
  }
  iStack_40 = *(int *)((int)uVar7 + 8);
  if ((iStack_40 != 0) && (iVar4 != 0)) {
    uStack_14 = (uint)*(ushort *)(iVar4 + 6);
    s_total_alloc_d_d_3ff1c(&puStack_30,uStack_14 * 0x14 + 4,s_STMARKS_000a118c,1);
    s_ATAB3_111d8(*puVar3,puStack_30);
    puVar3[2] = puStack_30;
    *puStack_30 = uStack_14;
    uStack_10 = 0;
    iStack_18 = *DAT_000c0cdc >> 0x10;
    puVar5 = puStack_30 + 1;
    while (uStack_10 != uStack_14) {
      *puVar5 = uVar6;
      uStack_2c = *(uint *)(uVar6 + 4);
      uStack_28 = *(uint *)(uVar6 + 8);
      uVar1 = *(uint *)(uVar6 + 0xc);
      puVar5[1] = uStack_2c;
      puVar5[2] = uStack_28;
      puVar5[3] = uVar1;
      s_SVIN_1e64e(auStack_b4,uStack_2c,uStack_28,uVar1);
      fn_23a52(auStack_b4,uStack_2c,uStack_28,uVar1,auStack_24,&iStack_20,auStack_1c);
      *(short *)(puVar5 + 4) = auStack_24._2_2_;
      iStack_20 = (iStack_20 >> 0x10) - DAT_000c0cf4;
      if (iStack_20 < 0) {
        iStack_20 = iStack_20 + iStack_18;
      }
      uVar6 = uVar6 + 0x34;
      uStack_10 = uStack_10 + 1;
      *(short *)((int)puVar5 + 0x12) = (short)iStack_20;
      puVar5 = puVar5 + 5;
    }
  }
  return;
}


// ==== insrestarts_RateLoop @ 00027788 body=18 ranges=1

void insrestarts_RateLoop(void)

{
  int extraout_EDX;
  
  __CHK();
  FUN_00037e24(extraout_EDX + 8);
  return;
}


// ==== bikewire_PreInit @ 00027a6d body=32 ranges=1

void bikewire_PreInit(void)

{
  int extraout_ECX;
  
  __CHK();
  *(undefined4 *)(extraout_ECX + 8) = 0x10000000;
  *(undefined4 *)(extraout_ECX + 0x10) = 0x10000000;
  *(undefined4 *)(extraout_ECX + 0x18) = 0x10000000;
  return;
}


// ==== bikewire_Display @ 00027a8d body=191 ranges=1

void bikewire_Display(void)

{
  byte *pbVar1;
  int *extraout_ECX;
  int *extraout_ECX_00;
  int *piVar2;
  int iVar3;
  int extraout_EDX;
  int extraout_EDX_00;
  int extraout_EBX;
  int extraout_EBX_00;
  int iVar4;
  int *piVar5;
  int *piVar6;
  undefined8 uVar7;
  
  uVar7 = __CHK();
  iVar3 = (int)((ulonglong)uVar7 >> 0x20);
  pbVar1 = (byte *)uVar7;
  if ((*pbVar1 & 0x10) != 0) {
    piVar2 = extraout_ECX;
    iVar4 = extraout_EBX;
    if (iVar3 == 0) {
      fn_3fb31();
      piVar2 = extraout_ECX_00;
      iVar3 = extraout_EDX;
      iVar4 = extraout_EBX_00;
    }
    iVar4 = *(int *)(iVar4 + 0x10);
    piVar5 = (int *)(iVar4 + 8);
    if (piVar5 == (int *)0x0) {
      fn_3fb31();
      iVar3 = extraout_EDX_00;
    }
    fn_10989(piVar2,*(undefined4 *)(iVar3 + 100),*(undefined4 *)(iVar3 + 0x68),
             *(undefined4 *)(iVar3 + 0x6c));
    piVar6 = piVar5;
    for (iVar3 = 9; iVar3 != 0; iVar3 = iVar3 + -1) {
      *piVar6 = *piVar2;
      piVar2 = piVar2 + 1;
      piVar6 = piVar6 + 1;
    }
    for (iVar3 = 0; iVar3 != 0; iVar3 = iVar3 + -1) {
      *(char *)piVar6 = (char)*piVar2;
      piVar2 = (int *)((int)piVar2 + 1);
      piVar6 = (int *)((int)piVar6 + 1);
    }
    fn_10640(pbVar1,piVar5,iVar4 + 0xc,iVar4 + 0x10);
    fn_10640(pbVar1,iVar4 + 0x14,iVar4 + 0x18,iVar4 + 0x1c);
    fn_10640(pbVar1,iVar4 + 0x20,iVar4 + 0x24,iVar4 + 0x28);
    do {
      *piVar5 = *piVar5 << 2;
      piVar5 = piVar5 + 1;
    } while (piVar5 != (int *)(iVar4 + 0x2c));
    FUN_00021875();
    return;
  }
  return;
}


// ==== bilstyring_Display @ 00029d71 body=45 ranges=1

void bilstyring_Display(void)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  int extraout_EBX;
  int extraout_EBX_00;
  undefined8 uVar4;
  
  uVar4 = __CHK();
  puVar3 = (undefined4 *)((ulonglong)uVar4 >> 0x20);
  iVar1 = (int)uVar4;
  if (DAT_000c0ce8 == 1) {
    FUN_0002a370(iVar1,puVar3,0,0);
    return;
  }
  *(undefined4 **)(iVar1 + 0x9c) = puVar3;
  fn_238ca(iVar1,puVar3,*(undefined4 *)(iVar1 + 0x98));
  *(byte *)(extraout_EBX + 1) = *(byte *)(extraout_EBX + 1) & 0xfd;
  iVar2 = FUN_0002779a(iVar1,puVar3);
  if (iVar2 == 1) {
    *(byte *)(extraout_EBX_00 + 1) = *(byte *)(extraout_EBX_00 + 1) | 2;
  }
  *puVar3 = *(undefined4 *)(iVar1 + 0x20);
  puVar3[1] = *(undefined4 *)(iVar1 + 0x24);
  iVar2 = DAT_000bf7d4;
  puVar3[2] = *(undefined4 *)(iVar1 + 0x28);
  if (iVar2 == 0) {
    FUN_00024bc1(iVar1,puVar3,&DAT_000a743c,0x18,&DAT_000a7574,&DAT_000a7578);
  }
  fn_24563(iVar1,puVar3);
  FUN_00029c17(iVar1,puVar3);
  puVar3[4] = puVar3[4] + 0x1200;
  fn_24699(puVar3,puVar3);
  fn_246c5(iVar1);
  fn_26aef(iVar1,puVar3,*(undefined4 *)(*(int *)(iVar1 + 0xa0) + 0x10),
           *(undefined4 *)(*(int *)(iVar1 + 0xa0) + 8),6);
  if (DAT_000bf7d8 + -1 == DAT_000bf7d4) {
    fn_23f2b(iVar1);
    FUN_00027498(iVar1,puVar3);
  }
  fn_2597a(iVar1,puVar3);
  FUN_00027064(iVar1,puVar3);
  FUN_00026dbd(iVar1,puVar3);
  FUN_00026e65(iVar1,puVar3);
  if (DAT_000bf7d8 + -1 == DAT_000bf7d4) {
    fn_23f6b(iVar1);
    fn_23feb(iVar1);
  }
  fn_2390f(iVar1,puVar3);
  return;
}


// ==== enecon01_Display @ 0002a262 body=270 ranges=1

void enecon01_Display(void)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  char *extraout_ECX;
  int extraout_ECX_00;
  int extraout_ECX_01;
  int extraout_ECX_02;
  int iVar4;
  int extraout_EDX;
  int extraout_EDX_00;
  int extraout_EBX;
  int extraout_EBX_00;
  int extraout_EBX_01;
  int iVar5;
  int iVar6;
  
  iVar2 = __CHK();
  iVar4 = *(int *)(iVar2 + 0x9c);
  uVar1 = *(undefined4 *)(iVar4 + 0xf8);
  iVar6 = *(int *)(iVar4 + 0x174) + -10;
  if (DAT_000a6654 == 1) {
    DAT_000c5bfc = 0;
  }
  else {
    iVar3 = *(int *)(DAT_000bf80c + 0x130) / 0x21;
    iVar5 = iVar6 * 0xf;
    if (iVar5 + 10 < iVar3) {
      if (iVar3 < iVar5 + 0x32) {
        *(int *)(iVar4 + 0xf8) = (6 - iVar6) * 2000 + 100000;
        FUN_00027271(iVar2,iVar4);
        iVar4 = extraout_EBX;
      }
      if (*(int *)(iVar4 + 0x22) >> 0x10 != 0) {
        *(undefined2 *)(iVar4 + 0x24) = 0;
      }
      DAT_000c5bfc = 0;
    }
    else {
      DAT_000c5bfc = 1;
    }
  }
  FUN_000279df(iVar2);
  if (*extraout_ECX == '\x01') {
    FUN_0002a056(iVar2);
    iVar4 = extraout_ECX_00;
    iVar6 = extraout_EDX;
    iVar5 = extraout_EBX_00;
  }
  else {
    FUN_00029eba(iVar2,*(undefined4 *)(iVar2 + 0x98));
    iVar4 = extraout_ECX_01;
    iVar6 = extraout_EDX_00;
    iVar5 = extraout_EBX_01;
  }
  if ((((*(byte *)(iVar6 + 1) & 1) == 0) && (4 < *(int *)(iVar5 + 0xe8))) &&
     (0x32 < *(int *)(iVar5 + 0xf0))) {
    fn_1d7c8(iVar2);
    iVar4 = extraout_ECX_02;
  }
  *(undefined4 *)(iVar4 + 4) = uVar1;
  FUN_00021875();
  return;
}


// ==== pickopned_PreInit @ 0002a3f7 body=15 ranges=1

void pickopned_PreInit(void)

{
  int extraout_EBX;
  
  __CHK();
  *(byte *)(extraout_EBX + 0xd) = *(byte *)(extraout_EBX + 0xd) | 1;
  return;
}


// ==== pickopned_Frmupdate @ 0002a406 body=71 ranges=1

void pickopned_Frmupdate(void)

{
  uint uVar1;
  int iVar2;
  uint *extraout_ECX;
  int extraout_EBX;
  
  __CHK();
  uVar1 = *extraout_ECX;
  *extraout_ECX = uVar1 + 0x200;
  iVar2 = fn_10170(0xc000 - (uVar1 + 0x200 & 0xffff));
  iVar2 = fn_12030c(64000,iVar2 >> 0xf);
  *(int *)(*(int *)(extraout_EBX + 0x10) + 0x30) =
       *(int *)(*(int *)(extraout_EBX + 0x14) + 0x30) + iVar2;
  return;
}


// ==== bgexpl_Visible @ 0002a465 body=37 ranges=1

void bgexpl_Visible(void)

{
  byte *pbVar1;
  int iVar2;
  int extraout_ECX;
  
  pbVar1 = (byte *)__CHK();
  iVar2 = *(int *)(*(int *)(*(int *)(*(int *)(pbVar1 + 0x98) + 0x28) + 0x10) + 0x98);
  iVar2 = fn_1026b(*(undefined4 *)(iVar2 + 0x18),*(undefined4 *)(iVar2 + 0x1c),
                   *(undefined4 *)(iVar2 + 0x20));
  if (0x73a0 < iVar2) {
    iVar2 = *(int *)(pbVar1 + 0x98);
    *(undefined4 *)(iVar2 + 0x18) = *(undefined4 *)(extraout_ECX + 0x18);
    *(undefined4 *)(iVar2 + 0x1c) = *(undefined4 *)(extraout_ECX + 0x1c);
    *(undefined4 *)(iVar2 + 0x20) = *(undefined4 *)(extraout_ECX + 0x20);
    if (0 < *(int *)(iVar2 + 0x1c)) {
      *(undefined4 *)(iVar2 + 0x1c) = 0;
    }
    if ((*pbVar1 & 0x10) == 0) {
      fn_1d7c8(pbVar1);
      return;
    }
    fn_23631(0x1f,0x78,pbVar1 + 0x20);
    FUN_0001eb9b(pbVar1);
  }
  return;
}


// ==== pickjmp_PreInit @ 0002a6d3 body=60 ranges=1

void pickjmp_PreInit(void)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int extraout_ECX;
  int *piVar4;
  int extraout_EBX;
  int iVar5;
  undefined8 uVar6;
  
  uVar6 = __CHK();
  piVar4 = (int *)((ulonglong)uVar6 >> 0x20);
  iVar2 = (int)uVar6;
  *piVar4 = iVar2;
  iVar1 = DAT_000c0bc4;
  piVar4[1] = DAT_000c0bc4;
  DAT_000c0bc4 = DAT_000c0bc4 + 1000;
  iVar5 = 0;
  fn_109e5(iVar2 + 0x5c,0,iVar1,0);
  iVar1 = *(int *)(iVar2 + 8);
  iVar3 = 0;
  while (iVar3 != *(int *)(iVar1 + 0x10)) {
    iVar3 = stricmp(*(int *)(*(int *)(iVar5 + *(int *)(iVar2 + 0xc)) + 0x14) + 0x58,s_jexpl_000a12ea
                   );
    if (iVar3 == 0) {
      *(byte *)(extraout_EBX + 0xc) = *(byte *)(extraout_EBX + 0xc) | 4;
    }
    iVar5 = iVar5 + 4;
    iVar3 = extraout_ECX + 1;
  }
  return;
}


// ==== trainwpul_Init @ 0002a863 body=96 ranges=1

void trainwpul_Init(void)

{
  undefined4 extraout_ECX;
  undefined8 uVar1;
  
  uVar1 = __CHK();
  triggerwpul_Init();
  if ((*(byte *)((int)((ulonglong)uVar1 >> 0x20) + 0x48) & 8) == 0) {
    fn_23631(0x41,0x7d,(int)uVar1 + 0x20,0);
    DAT_000a6578 = extraout_ECX;
    return;
  }
  if (DAT_000a6578 == 0) {
    fn_3b42b(0x41,0,0);
    DAT_000a6578 = 1;
  }
  return;
}


// ==== kransfx_Frmupdate @ 0002a8c3 body=46 ranges=1

void kransfx_Frmupdate(void)

{
  __CHK();
  s_lort_d_2a79f(0x4a,0x6e,200,0xe1,0xe6,0xfa);
  return;
}


// ==== pumpsfx_Frmupdate @ 0002a8f1 body=36 ranges=1

void pumpsfx_Frmupdate(void)

{
  __CHK();
  s_lort_d_2a79f(0x30,0x6e,0x3c,0x5a,0x73,0x7f);
  return;
}


// ==== acidsfx_Frmupdate @ 0002a915 body=79 ranges=1

void acidsfx_Frmupdate(void)

{
  int iVar1;
  
  __CHK();
  iVar1 = s_lort_d_2a79f(0x123,0x73,0x37,0x3f,0x46,0x52);
  if (iVar1 == 0) {
    s_lort_d_2a79f(0x123,0x73,0x90,0x99,0xab,0xc3);
  }
  return;
}


// ==== pickjmp_Frmupdate @ 0002a964 body=331 ranges=1

void pickjmp_Frmupdate(void)

{
  byte *pbVar1;
  int iVar2;
  int iVar3;
  int extraout_ECX;
  int extraout_ECX_00;
  int iVar4;
  int iVar5;
  int extraout_EBX;
  int extraout_EBX_00;
  undefined4 *puVar6;
  undefined4 *puVar7;
  undefined8 uVar8;
  undefined4 auStack_34 [8];
  int iStack_14;
  int iStack_10;
  
  uVar8 = __CHK();
  iVar4 = (int)((ulonglong)uVar8 >> 0x20);
  pbVar1 = (byte *)uVar8;
  iStack_10 = iVar4;
  fn_109e5(pbVar1 + 0x5c,0,(*(int *)(DAT_000bf80c + 0x130) - *(int *)(iVar4 + 8)) * 0x20,0);
  *(undefined4 *)(iVar4 + 8) = *(undefined4 *)(DAT_000bf80c + 0x130);
  iVar2 = fn_2a508(pbVar1,0,0,0,&iStack_14);
  iVar4 = iStack_14;
  if (iVar2 != 0) {
    iVar5 = *(int *)(iVar2 + 0xd8) + 0x2000;
    *(int *)(iVar2 + 0xd8) = iVar5;
    if (0x10000 < iVar5) {
      *(undefined4 *)(iVar2 + 0xd8) = 0x10000;
    }
    puVar6 = &DAT_000a76a0;
    puVar7 = auStack_34;
    for (iVar2 = 8; iVar2 != 0; iVar2 = iVar2 + -1) {
      *puVar7 = *puVar6;
      puVar6 = puVar6 + 1;
      puVar7 = puVar7 + 1;
    }
    auStack_34[0] = *(undefined4 *)(iVar4 + 0x20);
    auStack_34[1] = *(undefined4 *)(iStack_14 + 0x24);
    auStack_34[2] = *(undefined4 *)(iStack_14 + 0x28);
    s_extram_18f02(auStack_34,DAT_000c0c18 + 0x84,1,2,0x7fff,0x1c24,FUN_0002a73c,0,0);
    iVar4 = *(int *)(pbVar1 + 8);
    iVar2 = 0;
    for (iVar5 = 0; iVar5 != *(int *)(iVar4 + 0x10); iVar5 = iVar5 + 1) {
      iVar3 = stricmp(*(int *)(*(int *)(iVar2 + *(int *)(pbVar1 + 0xc)) + 0x14) + 0x58,
                      s_jexpl_000a12f9);
      iVar2 = extraout_ECX;
      if (iVar3 == 0) {
        *(byte *)(extraout_EBX + 0xc) = *(byte *)(extraout_EBX + 0xc) & 0xfb;
        s_CPYOBJINS_183c8(extraout_EBX);
        *(byte *)(extraout_EBX_00 + 0xc) = *(byte *)(extraout_EBX_00 + 0xc) | 4;
        iVar2 = extraout_ECX_00;
      }
      iVar2 = iVar2 + 4;
    }
    fn_3b42b(0x230,0x7c,0);
    FUN_0002a5c1(iStack_10);
    *pbVar1 = *pbVar1 | 1;
    FUN_0002e5b5();
    return;
  }
  return;
}


// ==== pickbost_PreInit @ 0002aaaf body=105 ranges=1

void pickbost_PreInit(void)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int extraout_ECX;
  int *piVar4;
  int extraout_EBX;
  int iVar5;
  undefined8 uVar6;
  
  uVar6 = __CHK();
  piVar4 = (int *)((ulonglong)uVar6 >> 0x20);
  iVar2 = (int)uVar6;
  *piVar4 = iVar2;
  iVar1 = DAT_000c0bc8;
  piVar4[1] = DAT_000c0bc8;
  DAT_000c0bc8 = DAT_000c0bc8 + 1000;
  iVar5 = 0;
  fn_109e5(iVar2 + 0x5c,0,0,iVar1);
  iVar1 = *(int *)(iVar2 + 8);
  iVar3 = 0;
  while (iVar3 != *(int *)(iVar1 + 0x10)) {
    iVar3 = stricmp(*(int *)(*(int *)(iVar5 + *(int *)(iVar2 + 0xc)) + 0x14) + 0x58,s_bexpl_000a12ff
                   );
    if (iVar3 == 0) {
      *(byte *)(extraout_EBX + 0xc) = *(byte *)(extraout_EBX + 0xc) | 4;
    }
    iVar5 = iVar5 + 4;
    iVar3 = extraout_ECX + 1;
  }
  return;
}


// ==== pickbost_Frmupdate @ 0002ab18 body=310 ranges=1

void pickbost_Frmupdate(void)

{
  byte *pbVar1;
  int iVar2;
  int iVar3;
  int extraout_ECX;
  int extraout_ECX_00;
  int iVar4;
  int iVar5;
  int extraout_EBX;
  int extraout_EBX_00;
  undefined4 *puVar6;
  undefined4 *puVar7;
  undefined8 uVar8;
  undefined4 auStack_34 [8];
  int iStack_14;
  int iStack_10;
  
  uVar8 = __CHK();
  iVar4 = (int)((ulonglong)uVar8 >> 0x20);
  pbVar1 = (byte *)uVar8;
  iStack_10 = iVar4;
  fn_109e5(pbVar1 + 0x5c,0,0,(*(int *)(DAT_000bf80c + 0x130) - *(int *)(iVar4 + 8)) * 0x20);
  *(undefined4 *)(iVar4 + 8) = *(undefined4 *)(DAT_000bf80c + 0x130);
  iVar2 = fn_2a508(pbVar1,0,0,0,&iStack_14);
  iVar4 = iStack_14;
  if (iVar2 != 0) {
    iVar5 = *(int *)(iVar2 + 0xdc) + 0x2000;
    *(int *)(iVar2 + 0xdc) = iVar5;
    if (0x10000 < iVar5) {
      *(undefined4 *)(iVar2 + 0xdc) = 0x10000;
    }
    puVar6 = &DAT_000a76c0;
    puVar7 = auStack_34;
    for (iVar2 = 8; iVar2 != 0; iVar2 = iVar2 + -1) {
      *puVar7 = *puVar6;
      puVar6 = puVar6 + 1;
      puVar7 = puVar7 + 1;
    }
    auStack_34[0] = *(undefined4 *)(iVar4 + 0x20);
    auStack_34[1] = *(undefined4 *)(iStack_14 + 0x24);
    auStack_34[2] = *(undefined4 *)(iStack_14 + 0x28);
    s_extram_18f02(auStack_34,DAT_000c0c18 + 0x84,1,2,0x7fff,0x1c24,FUN_0002a73c,0,0);
    iVar4 = *(int *)(pbVar1 + 8);
    iVar2 = 0;
    for (iVar5 = 0; iVar5 != *(int *)(iVar4 + 0x10); iVar5 = iVar5 + 1) {
      iVar3 = stricmp(*(int *)(*(int *)(iVar2 + *(int *)(pbVar1 + 0xc)) + 0x14) + 0x58,
                      s_bexpl_000a1305);
      iVar2 = extraout_ECX;
      if (iVar3 == 0) {
        *(byte *)(extraout_EBX + 0xc) = *(byte *)(extraout_EBX + 0xc) & 0xfb;
        s_CPYOBJINS_183c8(extraout_EBX);
        *(byte *)(extraout_EBX_00 + 0xc) = *(byte *)(extraout_EBX_00 + 0xc) | 4;
        iVar2 = extraout_ECX_00;
      }
      iVar2 = iVar2 + 4;
    }
    fn_3b42b(0x234,0x7c,0);
    FUN_0002a5c1(iStack_10);
    *pbVar1 = *pbVar1 | 1;
    FUN_0002e5b5();
    return;
  }
  return;
}


// ==== fire0_Frmupdate @ 0002ac4e body=114 ranges=1

void fire0_Frmupdate(void)

{
  int iVar1;
  undefined4 uVar2;
  int extraout_EBX;
  undefined4 uStack_14;
  int iStack_10;
  undefined4 uStack_c;
  
  uVar2 = __CHK();
  iVar1 = *(int *)(extraout_EBX + 0x14);
  uStack_14 = *(undefined4 *)(iVar1 + 0x2c);
  iStack_10 = *(int *)(iVar1 + 0x30) + -0x1f400;
  uStack_c = *(undefined4 *)(iVar1 + 0x34);
  fn_105cc(*(int *)(extraout_EBX + 0x10) + 8,&uStack_14,&iStack_10,&uStack_c);
  fn_1075d(uVar2,&uStack_14,&iStack_10,&uStack_c);
  fn_234c5(uStack_14,iStack_10,uStack_c,0,0,0);
  return;
}


// ==== spinn0_Frmupdate @ 0002acc0 body=51 ranges=1

void spinn0_Frmupdate(void)

{
  undefined4 *extraout_ECX;
  undefined4 *extraout_EDX;
  undefined4 *puVar1;
  int extraout_EBX;
  int iVar2;
  
  __CHK();
  iVar2 = *(int *)(extraout_EBX + 0x10) + 8;
  puVar1 = extraout_ECX;
  if (iVar2 == 0) {
    fn_3fb31();
    puVar1 = extraout_EDX;
  }
  fn_10989(iVar2,*puVar1,puVar1[1],puVar1[2]);
  return;
}


// ==== pickcon01_Display @ 0002acf3 body=134 ranges=1

void pickcon01_Display(void)

{
  undefined4 *puVar1;
  int iVar2;
  
  iVar2 = __CHK();
  puVar1 = *(undefined4 **)(iVar2 + 0x9c);
  fn_238ca(iVar2,puVar1);
  *puVar1 = *(undefined4 *)(iVar2 + 0x20);
  puVar1[1] = *(undefined4 *)(iVar2 + 0x24);
  puVar1[2] = *(undefined4 *)(iVar2 + 0x28);
  fn_24563(iVar2,puVar1);
  puVar1[4] = puVar1[4] + 0x1200;
  fn_24563(iVar2,puVar1);
  fn_24699(puVar1);
  fn_246c5(iVar2,puVar1);
  fn_26aef(iVar2,puVar1,*(undefined4 *)(*(int *)(iVar2 + 0xa0) + 0x10),
           *(undefined4 *)(*(int *)(iVar2 + 0xa0) + 8),2);
  fn_2597a(iVar2,puVar1);
  fn_2390f(iVar2,puVar1);
  return;
}


// ==== anim_PreInit @ 0002e5bb body=277 ranges=1

void anim_PreInit(void)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int extraout_ECX;
  uint uVar4;
  uint uVar5;
  int extraout_EDX;
  undefined4 *puVar6;
  int extraout_EBX;
  int extraout_EBX_00;
  int extraout_EBX_01;
  int extraout_EBX_02;
  undefined4 *puVar7;
  undefined4 *puStack_10;
  
  __CHK();
  iVar1 = *(int *)(extraout_EBX + 0x14);
  if ((*(byte *)(extraout_EBX + 0xc) & 0x10) == 0) {
    fn_3fb31();
    iVar1 = extraout_EBX_00;
  }
  puStack_10 = (undefined4 *)(*(int *)(iVar1 + 4) + iVar1);
  puVar7 = (undefined4 *)**(undefined4 **)(extraout_EBX + 0x10);
  iVar1 = wl3_FindChunk(iVar1 + 100,0x414e494d);
  iVar2 = extraout_EBX_01;
  if (iVar1 == 0) {
    fn_3fb31();
    iVar1 = extraout_EDX;
    iVar2 = extraout_EBX_02;
  }
  iVar1 = *(int *)(iVar1 + 8);
  iVar2 = *(int *)(iVar2 + 0x18) / iVar1;
  uVar5 = iVar2 * 6;
  *(int *)(extraout_ECX + 8) = iVar2;
  puVar6 = puStack_10;
  for (uVar4 = uVar5 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
    *puVar7 = *puVar6;
    puVar6 = puVar6 + 1;
    puVar7 = puVar7 + 1;
  }
  for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
    *(undefined1 *)puVar7 = *(undefined1 *)puVar6;
    puVar6 = (undefined4 *)((int)puVar6 + 1);
    puVar7 = (undefined4 *)((int)puVar7 + 1);
  }
  iVar2 = iVar1;
  if ((*(byte *)(extraout_ECX + 0x10) & 1) != 0) {
    iVar2 = iVar1 * 2 + -2;
  }
  s_total_alloc_d_d_3ff1c(extraout_ECX + 0x14,iVar2 * 4,&DAT_000a1588,1);
  puVar7 = *(undefined4 **)(extraout_ECX + 0x14);
  puVar6 = puVar7 + iVar2;
  for (iVar3 = 0; iVar3 != iVar1; iVar3 = iVar3 + 1) {
    *puVar7 = puStack_10;
    if ((iVar3 != 0) && ((*(byte *)(extraout_ECX + 0x10) & 1) != 0)) {
      *puVar6 = puStack_10;
    }
    puVar7 = puVar7 + 1;
    puVar6 = puVar6 + -1;
    puStack_10 = (undefined4 *)((int)puStack_10 + *(int *)(extraout_ECX + 8) * 6);
  }
  *(int *)(extraout_ECX + 4) = iVar2 << 0x10;
  *(byte *)(extraout_EBX + 0xc) = *(byte *)(extraout_EBX + 0xc) | 0xe0;
  return;
}


// ==== anim_RateLoop @ 0002e6d0 body=23 ranges=1

void anim_RateLoop(void)

{
  int extraout_ECX;
  
  __CHK();
  fn_40108(extraout_ECX + 0x14,1);
  return;
}


// ==== anim_Frmupdate @ 0002e6e7 body=274 ranges=1

void anim_Frmupdate(void)

{
  short sVar1;
  int iVar2;
  undefined4 *extraout_ECX;
  int extraout_ECX_00;
  int *extraout_ECX_01;
  int extraout_ECX_02;
  int extraout_ECX_03;
  short *psVar3;
  int extraout_EBX;
  int *extraout_EBX_00;
  int extraout_EBX_01;
  int extraout_EBX_02;
  short *psVar4;
  short *psVar5;
  uint uVar6;
  
  __CHK();
  psVar5 = (short *)**(undefined4 **)(extraout_EBX + 0x10);
  iVar2 = fn_12030c(*extraout_ECX,PTR_FUN_000a648c);
  iVar2 = *(int *)(extraout_ECX_00 + 0xc) + iVar2;
  *(int *)(extraout_ECX_00 + 0xc) = iVar2;
  if (*(int *)(extraout_ECX_00 + 4) <= iVar2) {
    *(int *)(extraout_ECX_00 + 0xc) = iVar2 - *(int *)(extraout_ECX_00 + 4);
  }
  iVar2 = *(int *)(extraout_ECX_00 + 0xc) + 0x10000;
  if (*(int *)(extraout_ECX_00 + 4) <= iVar2) {
    iVar2 = 0;
  }
  uVar6 = *(uint *)(extraout_ECX_00 + 0xc) & 0xffff;
  psVar4 = *(short **)
            (*(int *)(extraout_ECX_00 + 0x14) + (*(int *)(extraout_ECX_00 + 0xc) >> 0x10) * 4);
  psVar3 = *(short **)(*(int *)(extraout_ECX_00 + 0x14) + (iVar2 >> 0x10) * 4);
  for (iVar2 = 0; iVar2 != *(int *)(extraout_ECX_00 + 8) * 3; iVar2 = iVar2 + 3) {
    sVar1 = fn_12030c((int)*psVar3 - (int)*psVar4,uVar6);
    *psVar5 = sVar1 + (short)*extraout_EBX_00;
    sVar1 = fn_12030c((*extraout_ECX_01 >> 0x10) - (*extraout_EBX_00 >> 0x10),uVar6);
    psVar5[1] = sVar1 + *(short *)(extraout_EBX_01 + 2);
    sVar1 = fn_12030c((*(int *)(extraout_ECX_02 + 2) >> 0x10) -
                      (*(int *)(extraout_EBX_01 + 2) >> 0x10),uVar6);
    psVar3 = (short *)(extraout_ECX_03 + 6);
    psVar4 = (short *)(extraout_EBX_02 + 6);
    psVar5[2] = sVar1 + *(short *)(extraout_EBX_02 + 4);
    psVar5 = psVar5 + 3;
  }
  FUN_00021fd1(extraout_EBX);
  return;
}


// ==== ilogomain_PreInit @ 0002e806 body=125 ranges=1

void ilogomain_PreInit(void)

{
  __CHK();
  s_total_alloc_d_d_3ff1c(&DAT_000c5c44,&DAT_00096000,s_DRAWSCR_000a158d,1);
  s_error_loading_file_s_4027d(&DAT_000c5ec8,s_l__titlegfx_intro_SCAVLOGO_R0V_000a1595,1);
  s_error_loading_file_s_4027d(&DAT_000c5ecc,s_l__titlegfx_intro_SCORLOGO_R0V_000a15b4,1);
  s_error_loading_file_s_4027d(&DAT_000c5ed0,s_l__titlegfx_intro_ZYRINX96_R0V_000a15d3,1);
  s_R0V_pic_unknown_type_421830(DAT_000c5ec8);
  s_R0V_pic_unknown_type_421830(DAT_000c5ecc);
  s_R0V_pic_unknown_type_421830(DAT_000c5ed0);
  return;
}


// ==== ilogomain_Coli @ 0002e894 body=25 ranges=1

/* WARNING: Switch with 1 destination removed at 0x0002ea36 */

void ilogomain_Coli(void)

{
  uint uVar1;
  uint uVar2;
  undefined4 uVar3;
  
  __CHK();
  uVar1 = *(int *)(DAT_000bf80c + 0x130) / 2000;
  uVar2 = 0xffffffff;
  if (*(int *)(DAT_000bf80c + 0x130) == 0) {
    uVar2 = 0;
  }
  if (*(int *)(DAT_000bf80c + 0x134) / 2000 != uVar1) {
    uVar2 = uVar1;
  }
  if (*(int *)(DAT_000bf80c + 0x30) == 0) {
    DAT_000a657c = DAT_000c6294;
  }
  if (DAT_000a657c != DAT_000c6294) {
    uVar2 = 4;
  }
  DAT_000c5ed8 = 0;
  if (uVar2 != 0xffffffff) {
    uVar3 = DAT_000c5ec8;
    if (((uVar2 == 0) || (uVar3 = DAT_000c5ed0, uVar2 < 2)) || (uVar3 = DAT_000c5ecc, uVar2 == 2)) {
      fn_2f120(uVar3,DAT_000c5c44,0xffffffff,0xffffffff,0xffffffff,0xffffffff);
    }
    else {
      DAT_000c5ed8 = 1;
      fn_17b0b();
      fn_17875(s_title_000a15f2);
    }
  }
  if (DAT_000c5ed8 == 4) {
    DAT_000c5ed8 = 0;
  }
  fn_40bb0(&DAT_000c5c48,1,0x280);
  DAT_000a64bc = 0;
  fn_33dd4(DAT_000c5c44,&DAT_000c5c48);
  return;
}


// ==== ilogomain_RateLoop @ 0002ea3d body=70 ranges=1

void ilogomain_RateLoop(void)

{
  __CHK();
  fn_40108(&DAT_000c5c44,1);
  fn_40108(&DAT_000c5ec8,1);
  fn_40108(&DAT_000c5ecc,1);
  fn_40108(&DAT_000c5ed0,1);
  return;
}


// ==== titlemain_PreInit @ 0002fd1e body=65 ranges=1

void titlemain_PreInit(void)

{
  int *piVar1;
  byte *pbVar2;
  int iVar3;
  int extraout_ECX;
  short *extraout_ECX_00;
  int iVar4;
  short *extraout_EDX;
  int extraout_EDX_00;
  int iVar5;
  short *extraout_EDX_01;
  short *psVar6;
  undefined4 extraout_EDX_02;
  byte *extraout_EDX_03;
  int extraout_EDX_04;
  undefined4 *extraout_EDX_05;
  int extraout_EBX;
  int *piVar7;
  byte *pbVar8;
  byte *pbVar9;
  bool bVar10;
  bool bVar11;
  
  __CHK();
  s_total_alloc_d_d_3ff1c(&DAT_000c5fcc,0x54,s_TITLESEL_000a1629,1);
  DAT_000c5fd0 = 0;
  DAT_000c5fd4 = 0;
  *DAT_000c5fcc = 1;
  FUN_0002c6b0();
  DAT_000c5fcc[0x10] = 0xffffffff;
  fn_40bb0(&DAT_000c5c48,1,0x280,1);
  fn_402ef(&DAT_000c5fc4,0x10000,0xa1632);
  fn_2f558(DAT_000c5fc4,&DAT_00096000,0xa1637,1);
  s_total_alloc_d_d_3ff1c(&DAT_000c5c44);
  s_error_loading_file_s_4027d(&DAT_000c5fb4,s_l__titlegfx_title_detail_R0V_000a163f,0);
  s_error_loading_file_s_4027d(&DAT_000c5fb8,s_l__titlegfx_title_detail1f_R0V_000a165c,0);
  s_error_loading_file_s_4027d(&DAT_000c5fbc,s_l__titlegfx_title_detail1t_R0V_000a167b,0);
  s_error_loading_file_s_4027d(&DAT_000c5fc0,s_l__titlegfx_title_detail1w_R0V_000a169a,0);
  s_error_loading_file_s_4027d(&DAT_000c5fa4,s_l__titlegfx_title_2dbackon_R0V_000a16b9,0);
  s_error_loading_file_s_4027d(&DAT_000c5fa8,s_l__titlegfx_title_2dbackof_R0V_000a16d8,0);
  s_error_loading_file_s_4027d(&DAT_000c5fac,s_l__titlegfx_title_3dbackon_R0V_000a16f7,0);
  s_error_loading_file_s_4027d(&DAT_000c5fb0,s_l__titlegfx_title_3dbackof_R0V_000a1716,0);
  s_R0V_pic_unknown_type_421830(DAT_000c5fa4);
  s_R0V_pic_unknown_type_421830(DAT_000c5fa8);
  s_R0V_pic_unknown_type_421830(DAT_000c5fac);
  s_R0V_pic_unknown_type_421830(DAT_000c5fb0);
  s_R0V_pic_unknown_type_421830(DAT_000c5fb4);
  s_R0V_pic_unknown_type_421830(DAT_000c5fb8);
  s_R0V_pic_unknown_type_421830(DAT_000c5fbc,extraout_EDX_02,1);
  s_R0V_pic_unknown_type_421830(DAT_000c5fc0,s_l__titlegfx_title_BestTime_R0V_000a1735);
  s_error_loading_file_s_4027d(&DAT_000c5ee0);
  s_error_loading_file_s_4027d(&DAT_000c5ee4,s_l__titlegfx_title_CONFIG_R0V_000a1754,1);
  s_error_loading_file_s_4027d(&DAT_000c5ee8,s_l__titlegfx_title_COPYRI_1_R0V_000a1771,1);
  s_error_loading_file_s_4027d(&DAT_000c5fc8,s_l__titlegfx_title_FONTY3_1_R0V_000a1790,1);
  s_error_loading_file_s_4027d(&DAT_000c5eec,s_l__titlegfx_title_INPUT_R0V_000a17af,1);
  s_error_loading_file_s_4027d(&DAT_000c5f88,s_l__titlegfx_title_JOYPAD_R0V_000a17cb,1);
  s_error_loading_file_s_4027d(&DAT_000c5f8c,s_l__titlegfx_title_KEYBOARD_R0V_000a17e8,1);
  s_error_loading_file_s_4027d(&DAT_000c5ef0,s_l__titlegfx_title_MUSICO_1_R0V_000a1807,1);
  s_error_loading_file_s_4027d(&DAT_000c5f90,s_l__titlegfx_title_Default_R0V_000a1826,1);
  s_error_loading_file_s_4027d(&DAT_000c5f94,s_l__titlegfx_title_User_R0V_000a1844,1);
  s_error_loading_file_s_4027d(&DAT_000c5f98,s_l__titlegfx_title_Control_R0V_000a185f,1);
  s_error_loading_file_s_4027d(&DAT_000c5ef4,s_l__titlegfx_title_MUSIC_R0V_000a187d,1);
  s_error_loading_file_s_4027d(&DAT_000c5ef8,s_l__titlegfx_title_OPTIONS_R0V_000a1899,1);
  s_error_loading_file_s_4027d(&DAT_000c5f0c,s_l__titlegfx_title_SOUNDO_1_R0V_000a18b7,1);
  s_error_loading_file_s_4027d(&DAT_000c5f10,s_l__titlegfx_title_SOUNDO_2_R0V_000a18d6,1);
  s_error_loading_file_s_4027d(&DAT_000c5efc,s_l__titlegfx_title_STARTO_1_R0V_000a18f5,1);
  s_error_loading_file_s_4027d(&DAT_000c5f00,s_l__titlegfx_title_STARTR_1_R0V_000a1914,1);
  s_error_loading_file_s_4027d(&DAT_000c5f04,s_l__titlegfx_title_TITLEBG_R0V_000a1933,1);
  s_error_loading_file_s_4027d(&DAT_000c5f08,s_l__titlegfx_title_TITLELO_R0V_000a1951,1);
  s_error_loading_file_s_4027d(&DAT_000c5f9c,s_l__titlegfx_title_NORMAL_R0V_000a196f,1);
  s_error_loading_file_s_4027d(&DAT_000c5fa0,s_l__titlegfx_title_HARD_R0V_000a198c,1);
  s_error_loading_file_s_4027d(&DAT_000c5f14,s_L__titlegfx_selectL_BESTLTI_R0V_000a19a7,1);
  s_error_loading_file_s_4027d(&DAT_000c5f18,s_L__titlegfx_selectL_BESTTTI_R0V_000a19c7,1);
  s_error_loading_file_s_4027d(&DAT_000c5f1c,s_L__titlegfx_selectL_BGGRID_R0V_000a19e7,1);
  s_error_loading_file_s_4027d(&DAT_000c5f20,s_L__titlegfx_selectL_selstage_R0V_000a1a06,1);
  s_error_loading_file_s_4027d(&DAT_000c5f24,s_L__titlegfx_selectL_seltal_R0V_000a1a27,1);
  s_error_loading_file_s_4027d(&DAT_000c5f2c,s_L__titlegfx_selectB_MapB1out_R0V_000a1a46,0);
  s_error_loading_file_s_4027d(&DAT_000c5f34,s_L__titlegfx_selectB_MapB2out_R0V_000a1a67,3);
  s_error_loading_file_s_4027d(&DAT_000c5f3c,s_L__titlegfx_selectB_MapB3out_R0V_000a1a88,0);
  s_error_loading_file_s_4027d(&DAT_000c5f44,s_L__titlegfx_selectB_MapB4out_R0V_000a1aa9,0);
  s_error_loading_file_s_4027d(&DAT_000c5f4c,s_L__titlegfx_selectB_MapB5out_R0V_000a1aca,0);
  s_error_loading_file_s_4027d(&DAT_000c5f54,s_L__titlegfx_selectB_MapB6out_R0V_000a1aeb,0);
  s_error_loading_file_s_4027d(&DAT_000c5f28,s_L__titlegfx_selectB_MapB1in_R0V_000a1b0c,2);
  s_error_loading_file_s_4027d(&DAT_000c5f30,s_L__titlegfx_selectB_MapB2in_R0V_000a1b2c,2);
  s_error_loading_file_s_4027d(&DAT_000c5f38,s_L__titlegfx_selectB_MapB3in_R0V_000a1b4c,2);
  s_error_loading_file_s_4027d(&DAT_000c5f40,s_L__titlegfx_selectB_MapB4in_R0V_000a1b6c,2);
  s_error_loading_file_s_4027d(&DAT_000c5f48,s_L__titlegfx_selectB_MapB5in_R0V_000a1b8c,2);
  s_error_loading_file_s_4027d(&DAT_000c5f50,s_L__titlegfx_selectB_MapB6in_R0V_000a1bac,2);
  s_error_loading_file_s_4027d(&DAT_000c5f5c,s_l__titlegfx_gfxmode_256COL_R0V_000a1bcc,0);
  s_error_loading_file_s_4027d(&DAT_000c5f60,s_l__titlegfx_gfxmode_320X200_R0V_000a1beb,0);
  s_error_loading_file_s_4027d(&DAT_000c5f64,s_l__titlegfx_gfxmode_320X240_R0V_000a1c0b,0);
  s_error_loading_file_s_4027d(&DAT_000c5f68,s_l__titlegfx_gfxmode_640X400_R0V_000a1c2b,0);
  s_error_loading_file_s_4027d(&DAT_000c5f6c,s_l__titlegfx_gfxmode_640X480_R0V_000a1c4b,0);
  s_error_loading_file_s_4027d(&DAT_000c5f70,s_l__titlegfx_gfxmode_400X300_R0V_000a1c6b,0);
  s_error_loading_file_s_4027d(&DAT_000c5f74,s_l__titlegfx_gfxmode_512X384_R0V_000a1c8b,0);
  s_error_loading_file_s_4027d(&DAT_000c5f78,s_l__titlegfx_gfxmode_FULLSCR_R0V_000a1cab,0);
  s_error_loading_file_s_4027d(&DAT_000c5f7c,s_l__titlegfx_gfxmode_HICOLOR_R0V_000a1ccb,0);
  s_error_loading_file_s_4027d(&DAT_000c5f58,s_l__titlegfx_gfxmode_MODEOSV_R0V_000a1ceb,0);
  s_error_loading_file_s_4027d(&DAT_000c5f80,s_l__titlegfx_gfxmode_WINDOW_R0V_000a1d0b,0);
  s_error_loading_file_s_4027d(&DAT_000c5f84,s_l__titlegfx_gfxmode_WIDESCR_R0V_000a1d2a,0);
  s_error_loading_file_s_4027d(&DAT_000c5edc,s_l__titlegfx_title_BCTP_R0V_000a1d4a,4);
  s_R0V_pic_unknown_type_421830(DAT_000c5f90);
  s_R0V_pic_unknown_type_421830(DAT_000c5f94);
  s_R0V_pic_unknown_type_421830(DAT_000c5f98);
  s_R0V_pic_unknown_type_421830(DAT_000c5f88);
  s_R0V_pic_unknown_type_421830(DAT_000c5f8c);
  s_R0V_pic_unknown_type_421830(DAT_000c5f5c);
  s_R0V_pic_unknown_type_421830(DAT_000c5f60);
  s_R0V_pic_unknown_type_421830(DAT_000c5f64);
  s_R0V_pic_unknown_type_421830(DAT_000c5f68);
  s_R0V_pic_unknown_type_421830(DAT_000c5f6c);
  s_R0V_pic_unknown_type_421830(DAT_000c5f74);
  s_R0V_pic_unknown_type_421830(DAT_000c5f70);
  s_R0V_pic_unknown_type_421830(DAT_000c5f78);
  s_R0V_pic_unknown_type_421830(DAT_000c5f7c);
  s_R0V_pic_unknown_type_421830(DAT_000c5f58);
  s_R0V_pic_unknown_type_421830(DAT_000c5f80);
  s_R0V_pic_unknown_type_421830(DAT_000c5f84);
  s_R0V_pic_unknown_type_421830(DAT_000c5fc8);
  s_R0V_pic_unknown_type_421830(DAT_000c5edc);
  s_R0V_pic_unknown_type_421830(DAT_000c5ee0);
  s_R0V_pic_unknown_type_421830(DAT_000c5ee4);
  s_R0V_pic_unknown_type_421830(DAT_000c5ee8);
  s_R0V_pic_unknown_type_421830(DAT_000c5eec);
  s_R0V_pic_unknown_type_421830(DAT_000c5ef0);
  s_R0V_pic_unknown_type_421830(DAT_000c5ef4);
  s_R0V_pic_unknown_type_421830(DAT_000c5ef8);
  s_R0V_pic_unknown_type_421830(DAT_000c5efc);
  s_R0V_pic_unknown_type_421830(DAT_000c5f00);
  s_R0V_pic_unknown_type_421830(DAT_000c5f04);
  s_R0V_pic_unknown_type_421830(DAT_000c5f08);
  s_R0V_pic_unknown_type_421830(DAT_000c5f0c);
  s_R0V_pic_unknown_type_421830(DAT_000c5f10);
  s_R0V_pic_unknown_type_421830(DAT_000c5f9c);
  s_R0V_pic_unknown_type_421830(DAT_000c5fa0);
  s_R0V_pic_unknown_type_421830(DAT_000c5f14);
  s_R0V_pic_unknown_type_421830(DAT_000c5f18);
  s_R0V_pic_unknown_type_421830(DAT_000c5f1c);
  s_R0V_pic_unknown_type_421830(DAT_000c5f20);
  s_R0V_pic_unknown_type_421830(DAT_000c5f24);
  s_R0V_pic_unknown_type_421830(DAT_000c5f2c);
  s_R0V_pic_unknown_type_421830(DAT_000c5f34);
  s_R0V_pic_unknown_type_421830(DAT_000c5f3c);
  s_R0V_pic_unknown_type_421830(DAT_000c5f44);
  s_R0V_pic_unknown_type_421830(DAT_000c5f4c);
  s_R0V_pic_unknown_type_421830(DAT_000c5f54);
  s_R0V_pic_unknown_type_421830(DAT_000c5f28);
  s_R0V_pic_unknown_type_421830(DAT_000c5f30);
  s_R0V_pic_unknown_type_421830(DAT_000c5f38);
  s_R0V_pic_unknown_type_421830(DAT_000c5f40);
  s_R0V_pic_unknown_type_421830(DAT_000c5f48);
  pbVar2 = (byte *)__CHK(DAT_000c5f50);
  iVar4 = 7;
  bVar10 = false;
  iVar3 = 0;
  bVar11 = true;
  pbVar8 = pbVar2;
  pbVar9 = &DAT_000a0ff9;
  do {
    if (iVar4 == 0) break;
    iVar4 = iVar4 + -1;
    bVar10 = *pbVar8 < *pbVar9;
    bVar11 = *pbVar8 == *pbVar9;
    pbVar8 = pbVar8 + 1;
    pbVar9 = pbVar9 + 1;
  } while (bVar11);
  if (!bVar11) {
    iVar3 = (1 - (uint)bVar10) - (uint)(bVar10 != 0);
  }
  if (iVar3 != 0) {
    fn_3fb31();
    pbVar2 = extraout_EDX_03;
  }
  bswap32_at(pbVar2 + 8);
  bswap32_at(extraout_EDX_04 + 0xc);
  *extraout_EDX_05 = extraout_EDX_05[2];
  extraout_EDX_05[1] = extraout_EDX_05[3];
  piVar1 = extraout_EDX_05 + 4;
  extraout_EDX_05[2] = piVar1;
  iVar3 = extraout_EDX_05[1];
  piVar7 = piVar1;
  for (iVar4 = 0; iVar4 != *(int *)(pbVar2 + 4); iVar4 = iVar4 + 1) {
    bswap32_at(piVar7);
    if (*piVar7 != -1) {
      bswap16_at((int)piVar1 + *piVar7 * 2 + iVar3 * 4);
      iVar5 = extraout_EDX_00 + 2;
      do {
        bswap16_at(iVar5);
        iVar5 = (int)*extraout_EDX_01;
        psVar6 = extraout_EDX_01 + 1;
        while( true ) {
          if (iVar5 + -1 == -1) break;
          bswap16_at(psVar6,psVar6,iVar5 + -1,psVar6 + 1);
          psVar6 = extraout_ECX_00;
          iVar5 = extraout_EBX;
        }
        bswap16_at(psVar6,psVar6,0xffffffff,psVar6 + 1);
        iVar5 = extraout_ECX;
      } while (*extraout_EDX != 0);
    }
    piVar7 = piVar7 + 1;
  }
  FUN_0002a6cc();
  return;
}


// ==== titlemain_RateLoop @ 000304cb body=868 ranges=1

void titlemain_RateLoop(void)

{
  __CHK();
  fn_40108(&DAT_000c5fcc,1);
  fn_40323(&DAT_000c5fc4,1);
  FUN_0002c65e();
  fn_40108(&DAT_000c5c44,1);
  fn_40108(&DAT_000c5edc,4);
  fn_40108(&DAT_000c5ee0,1);
  fn_40108(&DAT_000c5ee4,1);
  fn_40108(&DAT_000c5ee8,1);
  fn_40108(&DAT_000c5eec,1);
  fn_40108(&DAT_000c5ef0,1);
  fn_40108(&DAT_000c5ef4,1);
  fn_40108(&DAT_000c5ef8,1);
  fn_40108(&DAT_000c5efc,1);
  fn_40108(&DAT_000c5f00,1);
  fn_40108(&DAT_000c5f04,1);
  fn_40108(&DAT_000c5f08,1);
  fn_40108(&DAT_000c5f0c,1);
  fn_40108(&DAT_000c5f10,1);
  fn_40108(&DAT_000c5fc8,1);
  fn_40108(&DAT_000c5f9c,1);
  fn_40108(&DAT_000c5fa0,1);
  fn_40108(&DAT_000c5f14,1);
  fn_40108(&DAT_000c5f18,1);
  fn_40108(&DAT_000c5f1c,1);
  fn_40108(&DAT_000c5f20,1);
  fn_40108(&DAT_000c5f24,1);
  fn_40108(&DAT_000c5f2c,0);
  fn_40108(&DAT_000c5f34,3);
  fn_40108(&DAT_000c5f3c,0);
  fn_40108(&DAT_000c5f44,0);
  fn_40108(&DAT_000c5f4c,0);
  fn_40108(&DAT_000c5f54,0);
  fn_40108(&DAT_000c5f28,2);
  fn_40108(&DAT_000c5f30,2);
  fn_40108(&DAT_000c5f38,2);
  fn_40108(&DAT_000c5f40,2);
  fn_40108(&DAT_000c5f48,2);
  fn_40108(&DAT_000c5f50,2);
  fn_40108(&DAT_000c5fa4,0);
  fn_40108(&DAT_000c5fa8,0);
  fn_40108(&DAT_000c5fac,0);
  fn_40108(&DAT_000c5fb0,0);
  fn_40108(&DAT_000c5fb4,0);
  fn_40108(&DAT_000c5fb8,0);
  fn_40108(&DAT_000c5fbc,0);
  fn_40108(&DAT_000c5fc0,0);
  fn_40108(&DAT_000c5f90,1);
  fn_40108(&DAT_000c5f94,1);
  fn_40108(&DAT_000c5f98,1);
  fn_40108(&DAT_000c5f88,1);
  fn_40108(&DAT_000c5f8c,1);
  fn_40108(&DAT_000c5f5c,0);
  fn_40108(&DAT_000c5f60,0);
  fn_40108(&DAT_000c5f64,0);
  fn_40108(&DAT_000c5f68,0);
  fn_40108(&DAT_000c5f6c,0);
  fn_40108(&DAT_000c5f70,0);
  fn_40108(&DAT_000c5f74,0);
  fn_40108(&DAT_000c5f78,0);
  fn_40108(&DAT_000c5f7c,0);
  fn_40108(&DAT_000c5f58,0);
  fn_40108(&DAT_000c5f80,0);
  FUN_0002c657(&DAT_000c5f84);
  return;
}


// ==== titlemain_Coli @ 000334a4 body=1874 ranges=1

void titlemain_Coli(void)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  undefined *puStack_14;
  int iStack_10;
  
  __CHK();
  FUN_0002159e();
  iVar5 = *(int *)(DAT_000bf80c + 0x130) - *(int *)(DAT_000bf80c + 0x134);
  iVar3 = 2;
  for (piVar2 = (int *)(DAT_000bf988 + 8); (iVar3 != 0x20 && (*piVar2 == 0)); piVar2 = piVar2 + 1) {
    iVar3 = iVar3 + 1;
  }
  if (iVar3 != 0x20) {
    DAT_000a65b4 = 30000;
  }
  DAT_000a65b4 = DAT_000a65b4 - iVar5;
  if (DAT_000a65b4 < 0) {
    DAT_000a65b4 = 10000;
    fn_17b0b();
    fn_17875((&PTR_s_lev5d1_000a83b0)[DAT_000c6070]);
    DAT_000c6070 = DAT_000c6070 + 1;
    if (DAT_000c6070 == 4) {
      DAT_000c6070 = 0;
    }
  }
  if (DAT_000c5fcc[9] == 0) {
    DAT_000c5fd0 = 1;
    FUN_0002f081(DAT_000c5f04,DAT_000c5c44,0xffffffff);
    fn_2f3bc(DAT_000c5f08,DAT_000c5c44,0xffffffff);
    fn_2f3bc(DAT_000c5ee8,DAT_000c5c44,0xffffffff);
    fn_2f3bc(DAT_000c5efc,DAT_000c5c44,0xffffffff);
    piVar2 = DAT_000c5fcc;
    DAT_000c5fcc[0x12] = 1;
    piVar2[0x11] = 1;
  }
  if (DAT_000c5fcc[9] == 8) {
    DAT_000c5fd0 = 0;
  }
  if (DAT_000c5fd0 != 1) {
    iVar3 = FUN_00021530(s_XXXYZZZYYYZXX_000a1da5);
    if (iVar3 == 1) {
      fn_17b0b();
      fn_17875(s_endseq_000a1db3);
    }
    iVar3 = FUN_00021530(s_XYXZY_000a1dba);
    if (iVar3 == 1) {
      DAT_000a668c = DAT_000a668c | 1;
    }
  }
  piVar2 = DAT_000c5fcc;
  iVar3 = DAT_000bf988;
  DAT_000c5fcc[9] = DAT_000c5fcc[9] + 1;
  if ((*(char *)(iVar3 + 0xd0) == '\x01') && (DAT_000c5fd0 == 0)) {
    if (piVar2[0xc] < 1) {
      if (*(int *)((&PTR_DAT_000a82fc)[piVar2[10] * 3] + (piVar2[0xb] + 1) * 0x18) != -1) {
        piVar2[0xb] = piVar2[0xb] + 1;
      }
      piVar2 = DAT_000c5fcc;
      DAT_000c5fcc[0xc] = 0x96;
      piVar2[0xd] = 0;
      if (1 < piVar2[0xf]) {
        piVar2[0xc] = 100;
      }
      DAT_000c5fcc[0xf] = DAT_000c5fcc[0xf] + 1;
    }
  }
  else {
    DAT_000c5fcc[0xc] = 0;
  }
  if ((*(char *)(DAT_000bf988 + 200) == '\x01') && (DAT_000c5fd0 == 0)) {
    if (DAT_000c5fcc[0xd] < 1) {
      if (DAT_000c5fcc[0xb] != 0) {
        DAT_000c5fcc[0xb] = DAT_000c5fcc[0xb] + -1;
      }
      piVar2 = DAT_000c5fcc;
      DAT_000c5fcc[0xd] = 0x96;
      piVar2[0xc] = 0;
      if (1 < piVar2[0xf]) {
        piVar2[0xd] = 100;
      }
      DAT_000c5fcc[0xf] = DAT_000c5fcc[0xf] + 1;
    }
  }
  else {
    DAT_000c5fcc[0xd] = 0;
  }
  if ((DAT_000c5fcc[0xd] == 0) && (DAT_000c5fcc[0xc] == 0)) {
    DAT_000c5fcc[0xf] = 0;
  }
  piVar1 = DAT_000c5fcc;
  iVar3 = DAT_000c5fcc[10];
  piVar2 = DAT_000c5fcc + 0xd;
  DAT_000c5fcc[0xc] = DAT_000c5fcc[0xc] - iVar5;
  piVar1[0xd] = *piVar2 - iVar5;
  puStack_14 = (&PTR_DAT_000a82fc)[iVar3 * 3];
  if (DAT_000c5fd4 == 0) {
    if ((*(char *)(DAT_000bf988 + 0x1c) == '\x01') && (DAT_000c5fd0 == 0)) {
      if (piVar1[0x11] == 0) {
        FUN_00033421(*(undefined4 *)(puStack_14 + piVar1[0xb] * 0x18 + 8),
                     *(undefined4 *)(puStack_14 + piVar1[0xb] * 0x18 + 0xc),piVar1 + 10,&puStack_14,
                     piVar1 + 0x10,piVar1 + 0xb);
        DAT_000c5fcc[0x11] = 1;
      }
    }
    else {
      DAT_000c5fcc[0x11] = 0;
    }
  }
  if (((*(char *)(DAT_000bf988 + 0xe) == '\x01') || (*(char *)(DAT_000bf988 + 1) == '\x01')) &&
     (DAT_000c5fd0 == 0)) {
    if (DAT_000c5fcc[0x12] == 0) {
      FUN_00033421(*(undefined4 *)(puStack_14 + DAT_000c5fcc[0xb] * 0x18 + 0x10),
                   *(undefined4 *)(puStack_14 + DAT_000c5fcc[0xb] * 0x18 + 0x14),DAT_000c5fcc + 10,
                   &puStack_14,DAT_000c5fcc + 0x10,DAT_000c5fcc + 0xb);
      DAT_000c5fcc[0x12] = 1;
    }
  }
  else {
    DAT_000c5fcc[0x12] = 0;
  }
  iVar3 = *(int *)(&PTR_DAT_000a8300)[DAT_000c5fcc[10] * 3];
  if (DAT_000c5fcc[0xb] == DAT_000c5fcc[0x10]) {
    (*(code *)(&PTR_FUN_000a8304)[DAT_000c5fcc[10] * 3])(1,DAT_000c5fcc[0xb]);
  }
  else {
    fn_2f3bc(iVar3,DAT_000c5c44,(*(int *)(puStack_14 + DAT_000c5fcc[0xb] * 0x18) * 0x1e0) / 0x1e0,
             (*(int *)((int)(puStack_14 + DAT_000c5fcc[0xb] * 0x18) + 4) * 0x1e0) / 0x1e0);
    s_REMRGB_2f639(iVar3,DAT_000c5c44,
                   (*(int *)(puStack_14 + DAT_000c5fcc[0xb] * 0x18) * 0x1e0) / 0x1e0,
                   (*(int *)((int)(puStack_14 + DAT_000c5fcc[0xb] * 0x18) + 4) * 0x1e0) / 0x1e0);
    iVar5 = DAT_000c5fcc[0x10];
    if (iVar5 != -1) {
      if (iVar3 == DAT_000c5eec) {
        fn_2f1d9(DAT_000c5f04,DAT_000c5c44,(*(int *)(puStack_14 + iVar5 * 0x18) * 0x1e0) / 0x1e0,
                 (*(int *)((int)(puStack_14 + iVar5 * 0x18) + 4) * 0x1e0) / 0x1e0,0,0xeb);
      }
      else {
        fn_2f120(DAT_000c5f04,DAT_000c5c44,(*(int *)(puStack_14 + iVar5 * 0x18) * 0x1e0) / 0x1e0,
                 (*(int *)((int)(puStack_14 + iVar5 * 0x18) + 4) * 0x1e0) / 0x1e0,0xffffffff,
                 0xffffffff);
      }
      fn_2f3bc(iVar3,DAT_000c5c44,(*(int *)(puStack_14 + DAT_000c5fcc[0x10] * 0x18) * 0x1e0) / 0x1e0
               ,(*(int *)((int)(puStack_14 + DAT_000c5fcc[0x10] * 0x18) + 4) * 0x1e0) / 0x1e0);
    }
    (*(code *)(&PTR_FUN_000a8304)[DAT_000c5fcc[10] * 3])(3,DAT_000c5fcc[0xb],DAT_000c5fcc[0x10]);
    DAT_000c5fcc[0x10] = DAT_000c5fcc[0xb];
  }
  if (DAT_000a64c0 == 1) {
    fn_40bb0(&DAT_000c5c48,1,0x280);
  }
  if (*DAT_000c5fcc == 1) {
    for (iVar3 = 0; (iVar3 != 0x280 && ((&DAT_000c5c48)[iVar3] == '\0')); iVar3 = iVar3 + 1) {
    }
    for (iStack_10 = 0x27f; (iStack_10 != -1 && ((&DAT_000c5c48)[iStack_10] == '\0'));
        iStack_10 = iStack_10 + -1) {
    }
    iStack_10 = iStack_10 + 2;
    if (iVar3 == 0x280) {
      iVar3 = (DAT_000c5fcc[0x13] << 0x10) / 0x1e0;
      s_lort_20451(0,iVar3,0x10000,(DAT_000c5fcc[0x14] << 0x10) / 0x1e0,0,iVar3);
      piVar2 = DAT_000c5fcc;
      DAT_000c5fcc[0x13] = 0;
      piVar2[0x14] = 0;
    }
    else {
      iVar5 = iVar3;
      iVar4 = iStack_10;
      if ((iVar3 != 0) || (iStack_10 != 0)) {
        if (DAT_000c5fcc[0x13] < iVar3) {
          iVar5 = DAT_000c5fcc[0x13];
        }
        if (iStack_10 < DAT_000c5fcc[0x14]) {
          iVar4 = DAT_000c5fcc[0x14];
        }
      }
      iVar5 = (iVar5 << 0x10) / 0x1e0;
      s_lort_20451(0,iVar5,0x10000,(iVar4 << 0x10) / 0x1e0,0,iVar5);
      piVar2 = DAT_000c5fcc;
      DAT_000c5fcc[0x13] = iVar3;
      piVar2[0x14] = iStack_10;
    }
  }
  DAT_000a64bc = 0;
  fn_33dd4(DAT_000c5c44,&DAT_000c5c48);
  (*(code *)(&PTR_FUN_000a8304)[DAT_000c5fcc[10] * 3])(2,DAT_000c5fcc[0xb],DAT_000c5fcc[0x10]);
  FUN_0002e5b5();
  return;
}


// ==== optionsmain_Coli @ 0003454b body=1789 ranges=1

void optionsmain_Coli(void)

{
  int *piVar1;
  undefined4 uVar2;
  int *piVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  undefined *puStack_18;
  int iStack_14;
  int iStack_10;
  
  __CHK();
  piVar1 = DAT_000c5fcc;
  uVar2 = DAT_000c5c44;
  if (DAT_000c5fcc[9] == 0) {
    DAT_000c5fcc[0x12] = 1;
    DAT_000c5fd0 = 1;
    piVar1[0x11] = 1;
    fn_2f120(DAT_000c5f04,uVar2,0xffffffff,0xffffffff,0xffffffff,0xffffffff);
    fn_2f3bc(DAT_000c5f08,DAT_000c5c44,0xffffffff);
    fn_2f3bc(DAT_000c5ee4,DAT_000c5c44,0xffffffff);
  }
  piVar1 = DAT_000c5fcc;
  iVar6 = DAT_000bf80c;
  if (DAT_000c5fcc[9] == 8) {
    DAT_000c5fd0 = 0;
  }
  DAT_000c5fcc[9] = DAT_000c5fcc[9] + 1;
  iVar6 = *(int *)(iVar6 + 0x130) - *(int *)(iVar6 + 0x134);
  if ((*(char *)(DAT_000bf988 + 0xd0) == '\x01') && (DAT_000c5fd0 == 0)) {
    if (piVar1[0xc] < 1) {
      if (*(int *)((&PTR_DAT_000a8648)[piVar1[10] * 3] + (piVar1[0xb] + 1) * 0x18) == -1) {
        piVar1[0xb] = 0;
      }
      else {
        piVar1[0xb] = piVar1[0xb] + 1;
      }
      piVar1 = DAT_000c5fcc;
      DAT_000c5fcc[0xc] = 0x96;
      piVar1[0xd] = 0;
      if (1 < piVar1[0xf]) {
        piVar1[0xc] = 100;
      }
      DAT_000c5fcc[0xf] = DAT_000c5fcc[0xf] + 1;
    }
  }
  else {
    DAT_000c5fcc[0xc] = 0;
  }
  if ((*(char *)(DAT_000bf988 + 200) == '\x01') && (DAT_000c5fd0 == 0)) {
    if (DAT_000c5fcc[0xd] < 1) {
      if (DAT_000c5fcc[0xb] == 0) {
        while( true ) {
          if (*(int *)((&PTR_DAT_000a8648)[DAT_000c5fcc[10] * 3] + (DAT_000c5fcc[0xb] + 1) * 0x18)
              == -1) break;
          DAT_000c5fcc[0xb] = DAT_000c5fcc[0xb] + 1;
        }
      }
      else {
        DAT_000c5fcc[0xb] = DAT_000c5fcc[0xb] + -1;
      }
      piVar1 = DAT_000c5fcc;
      DAT_000c5fcc[0xd] = 0x96;
      piVar1[0xc] = 0;
      if (1 < piVar1[0xf]) {
        piVar1[0xd] = 100;
      }
      DAT_000c5fcc[0xf] = DAT_000c5fcc[0xf] + 1;
    }
  }
  else {
    DAT_000c5fcc[0xd] = 0;
  }
  if ((DAT_000c5fcc[0xd] == 0) && (DAT_000c5fcc[0xc] == 0)) {
    DAT_000c5fcc[0xf] = 0;
  }
  piVar3 = DAT_000c5fcc;
  piVar1 = DAT_000c5fcc + 0xc;
  DAT_000c5fcc[0xd] = DAT_000c5fcc[0xd] - iVar6;
  piVar3[0xc] = *piVar1 - iVar6;
  puStack_18 = (&PTR_DAT_000a8648)[piVar3[10] * 3];
  iStack_14 = 0;
  if ((*(char *)(DAT_000bf988 + 0x1c) == '\x01') && (DAT_000c5fd0 == 0)) {
    if (piVar3[0x11] == 0) {
      fn_344d7(*(undefined4 *)(puStack_18 + piVar3[0xb] * 0x18 + 8),
               *(undefined4 *)(puStack_18 + piVar3[0xb] * 0x18 + 0xc),piVar3 + 10,&puStack_18,
               piVar3 + 0x10,piVar3 + 0xb);
      iStack_14 = 1;
      DAT_000c5fcc[0x11] = 1;
    }
  }
  else {
    DAT_000c5fcc[0x11] = 0;
  }
  if (((*(char *)(DAT_000bf988 + 0xe) == '\x01') || (*(char *)(DAT_000bf988 + 1) == '\x01')) &&
     (DAT_000c5fd0 == 0)) {
    if (DAT_000c5fcc[0x12] == 0) {
      if (*(int *)(puStack_18 + DAT_000c5fcc[0xb] * 0x18 + 0x10) == -1) {
        if (*(char *)(DAT_000bf988 + 1) == '\x01') {
          *(undefined1 *)(DAT_000bf988 + 1) = 0;
          fn_17b0b();
        }
      }
      fn_344d7(*(undefined4 *)(puStack_18 + DAT_000c5fcc[0xb] * 0x18 + 0x10),
               *(undefined4 *)(puStack_18 + DAT_000c5fcc[0xb] * 0x18 + 0x14),DAT_000c5fcc + 10,
               &puStack_18,DAT_000c5fcc + 0x10,DAT_000c5fcc + 0xb);
      iStack_14 = 1;
      DAT_000c5fcc[0x12] = 1;
    }
  }
  else {
    DAT_000c5fcc[0x12] = 0;
  }
  iVar6 = *(int *)(&PTR_DAT_000a864c)[DAT_000c5fcc[10] * 3];
  if (DAT_000c5fcc[0xb] == DAT_000c5fcc[0x10]) {
    (*(code *)(&PTR_DAT_000a8650)[DAT_000c5fcc[10] * 3])(1,DAT_000c5fcc[0xb]);
  }
  else {
    fn_2f3bc(iVar6,DAT_000c5c44,(*(int *)(puStack_18 + DAT_000c5fcc[0xb] * 0x18) * 0x1e0) / 0x1e0,
             (*(int *)((int)(puStack_18 + DAT_000c5fcc[0xb] * 0x18) + 4) * 0x1e0) / 0x1e0);
    s_REMRGB_2f639(iVar6,DAT_000c5c44,
                   (*(int *)(puStack_18 + DAT_000c5fcc[0xb] * 0x18) * 0x1e0) / 0x1e0,
                   (*(int *)((int)(puStack_18 + DAT_000c5fcc[0xb] * 0x18) + 4) * 0x1e0) / 0x1e0);
    iVar4 = DAT_000c5fcc[0x10];
    if (iVar4 != -1) {
      if (iVar6 == DAT_000c5eec) {
        fn_2f1d9(DAT_000c5f04,DAT_000c5c44,(*(int *)(puStack_18 + iVar4 * 0x18) * 0x1e0) / 0x1e0,
                 (*(int *)((int)(puStack_18 + iVar4 * 0x18) + 4) * 0x1e0) / 0x1e0,0,0xeb);
      }
      else {
        fn_2f120(DAT_000c5f04,DAT_000c5c44,(*(int *)(puStack_18 + iVar4 * 0x18) * 0x1e0) / 0x1e0,
                 (*(int *)((int)(puStack_18 + iVar4 * 0x18) + 4) * 0x1e0) / 0x1e0,0xffffffff,
                 0xffffffff);
      }
      fn_2f3bc(iVar6,DAT_000c5c44,(*(int *)(puStack_18 + DAT_000c5fcc[0x10] * 0x18) * 0x1e0) / 0x1e0
               ,(*(int *)((int)(puStack_18 + DAT_000c5fcc[0x10] * 0x18) + 4) * 0x1e0) / 0x1e0);
    }
    (*(code *)(&PTR_DAT_000a8650)[DAT_000c5fcc[10] * 3])(3,DAT_000c5fcc[0xb],DAT_000c5fcc[0x10]);
    DAT_000c5fcc[0x10] = DAT_000c5fcc[0xb];
  }
  if (DAT_000a64c0 == 1) {
    fn_40bb0(&DAT_000c5c48,1,0x280);
  }
  if (*DAT_000c5fcc == 1) {
    for (iVar6 = 0; (iVar6 != 0x280 && ((&DAT_000c5c48)[iVar6] == '\0')); iVar6 = iVar6 + 1) {
    }
    for (iStack_10 = 0x27f; (iStack_10 != -1 && ((&DAT_000c5c48)[iStack_10] == '\0'));
        iStack_10 = iStack_10 + -1) {
    }
    iStack_10 = iStack_10 + 2;
    if (iVar6 == 0x280) {
      iVar6 = (DAT_000c5fcc[0x13] << 0x10) / 0x1e0;
      s_lort_20451(0,iVar6,0x10000,(DAT_000c5fcc[0x14] << 0x10) / 0x1e0,0,iVar6);
      piVar1 = DAT_000c5fcc;
      DAT_000c5fcc[0x13] = 0;
      piVar1[0x14] = 0;
    }
    else {
      iVar4 = iVar6;
      iVar5 = iStack_10;
      if ((iVar6 != 0) || (iStack_10 != 0)) {
        if (DAT_000c5fcc[0x13] < iVar6) {
          iVar4 = DAT_000c5fcc[0x13];
        }
        if (iStack_10 < DAT_000c5fcc[0x14]) {
          iVar5 = DAT_000c5fcc[0x14];
        }
      }
      iVar4 = (iVar4 << 0x10) / 0x1e0;
      s_lort_20451(0,iVar4,0x10000,(iVar5 << 0x10) / 0x1e0,0,iVar4);
      piVar1 = DAT_000c5fcc;
      DAT_000c5fcc[0x13] = iVar6;
      piVar1[0x14] = iStack_10;
    }
  }
  DAT_000a64bc = 0;
  fn_33dd4(DAT_000c5c44,&DAT_000c5c48);
  if (iStack_14 == 0) {
    (*(code *)(&PTR_DAT_000a8650)[DAT_000c5fcc[10] * 3])(2,DAT_000c5fcc[0xb],DAT_000c5fcc[0x10]);
  }
  FUN_00021c79();
  return;
}


// ==== optionsmain_PreInit @ 00034c48 body=957 ranges=1

void optionsmain_PreInit(void)

{
  undefined4 *puVar1;
  undefined4 extraout_EDX;
  undefined4 extraout_EBX;
  
  __CHK();
  s_total_alloc_d_d_3ff1c(&DAT_000c5fcc,0x54,s_SELTITLE_000a1dc6,1);
  puVar1 = DAT_000c5fcc;
  *DAT_000c5fcc = 1;
  DAT_000c5fd0 = 0;
  puVar1[0x10] = 0xffffffff;
  fn_40bb0(&DAT_000c5c48,1,0x280,0);
  fn_402ef(&DAT_000c5fc4,0x10000,s_GREMAP_000a1dcf);
  fn_2f558(DAT_000c5fc4,&DAT_00096000,s_DRAWSCR_000a1dd6,1);
  s_total_alloc_d_d_3ff1c(&DAT_000c5c44);
  s_error_loading_file_s_4027d(&DAT_000c5ee4,s_l__titlegfx_ingame_ICONFIG_R0V_000a1dde,0);
  s_error_loading_file_s_4027d(&DAT_000c5fc8,s_l__titlegfx_title_FONTY3_1_R0V_000a1dfd,0);
  s_error_loading_file_s_4027d(&DAT_000c5eec,s_l__titlegfx_title_INPUT_R0V_000a1e1c,0);
  s_error_loading_file_s_4027d(&DAT_000c5f88,s_l__titlegfx_ingame_IJOYPAD_R0V_000a1e38,0);
  s_error_loading_file_s_4027d(&DAT_000c5f8c,s_l__titlegfx_ingame_IKEYBO_1_R0V_000a1e57,0);
  s_error_loading_file_s_4027d(&DAT_000c5ef0,s_l__titlegfx_title_MUSICO_1_R0V_000a1e77,0);
  s_error_loading_file_s_4027d(&DAT_000c5f0c,s_l__titlegfx_title_SOUNDO_1_R0V_000a1e96,0);
  s_error_loading_file_s_4027d(&DAT_000c5f10,s_l__titlegfx_title_SOUNDO_2_R0V_000a1eb5,0);
  s_error_loading_file_s_4027d(&DAT_000c5f04,s_l__titlegfx_title_COPYRI_1_R0V_000a1ed4,0);
  s_error_loading_file_s_4027d(&DAT_000c5f08,s_l__titlegfx_title_TITLELO_R0V_000a1ef3,0);
  s_error_loading_file_s_4027d(&DAT_000c5f5c,s_l__titlegfx_gfxmode_256COL_R0V_000a1f11,0);
  s_error_loading_file_s_4027d(&DAT_000c5f60,s_l__titlegfx_gfxmode_320X200_R0V_000a1f30,0);
  s_error_loading_file_s_4027d(&DAT_000c5f64,s_l__titlegfx_gfxmode_320X240_R0V_000a1f50,0);
  s_error_loading_file_s_4027d(&DAT_000c5f68,s_l__titlegfx_gfxmode_640X400_R0V_000a1f70,0);
  s_error_loading_file_s_4027d(&DAT_000c5f6c,s_l__titlegfx_gfxmode_640X480_R0V_000a1f90,0);
  s_error_loading_file_s_4027d(&DAT_000c5f70,s_l__titlegfx_gfxmode_400X300_R0V_000a1fb0,0);
  s_error_loading_file_s_4027d(&DAT_000c5f74,s_l__titlegfx_gfxmode_512X384_R0V_000a1fd0,0);
  s_error_loading_file_s_4027d(&DAT_000c5f78,s_l__titlegfx_gfxmode_FULLSCR_R0V_000a1ff0,0);
  s_error_loading_file_s_4027d(&DAT_000c5f7c,s_l__titlegfx_gfxmode_HICOLOR_R0V_000a2010,0);
  s_error_loading_file_s_4027d(&DAT_000c5f58,s_l__titlegfx_gfxmode_MODEOSV_R0V_000a2030,0);
  s_error_loading_file_s_4027d(&DAT_000c5f80,s_l__titlegfx_gfxmode_WINDOW_R0V_000a2050,0);
  s_error_loading_file_s_4027d(&DAT_000c5f84,s_l__titlegfx_gfxmode_WIDESCR_R0V_000a206f,0);
  s_error_loading_file_s_4027d(&DAT_000c5fb4,s_l__titlegfx_title_detail_R0V_000a208f,0);
  s_error_loading_file_s_4027d(&DAT_000c5fb8,s_l__titlegfx_title_detail1f_R0V_000a20ac,0);
  s_error_loading_file_s_4027d(&DAT_000c5fbc,s_l__titlegfx_title_detail1t_R0V_000a20cb,0);
  s_error_loading_file_s_4027d(&DAT_000c5fc0,s_l__titlegfx_title_detail1w_R0V_000a20ea,0);
  s_error_loading_file_s_4027d(&DAT_000c5fa4,s_l__titlegfx_title_2dbackon_R0V_000a2109,0);
  s_error_loading_file_s_4027d(&DAT_000c5fa8,s_l__titlegfx_title_2dbackof_R0V_000a2128,0);
  s_error_loading_file_s_4027d(&DAT_000c5fac,s_l__titlegfx_title_3dbackon_R0V_000a2147,0);
  s_error_loading_file_s_4027d(&DAT_000c5fb0,s_l__titlegfx_title_3dbackof_R0V_000a2166,0);
  s_R0V_pic_unknown_type_421830(DAT_000c5f84);
  s_R0V_pic_unknown_type_421830(DAT_000c5fa4);
  s_R0V_pic_unknown_type_421830(DAT_000c5fa8);
  s_R0V_pic_unknown_type_421830(DAT_000c5fac);
  s_R0V_pic_unknown_type_421830(DAT_000c5fb0);
  s_R0V_pic_unknown_type_421830(DAT_000c5f88);
  s_R0V_pic_unknown_type_421830(DAT_000c5f8c);
  s_R0V_pic_unknown_type_421830(DAT_000c5f5c);
  s_R0V_pic_unknown_type_421830(DAT_000c5f60);
  s_R0V_pic_unknown_type_421830(DAT_000c5f64);
  s_R0V_pic_unknown_type_421830(DAT_000c5f68);
  s_R0V_pic_unknown_type_421830(DAT_000c5f6c);
  s_R0V_pic_unknown_type_421830(DAT_000c5f70);
  s_R0V_pic_unknown_type_421830(DAT_000c5f74);
  s_R0V_pic_unknown_type_421830(DAT_000c5f78);
  s_R0V_pic_unknown_type_421830(DAT_000c5f7c);
  s_R0V_pic_unknown_type_421830(DAT_000c5f58);
  s_R0V_pic_unknown_type_421830(DAT_000c5f80);
  s_R0V_pic_unknown_type_421830(DAT_000c5fb4);
  s_R0V_pic_unknown_type_421830(DAT_000c5fb8);
  s_R0V_pic_unknown_type_421830(DAT_000c5fbc);
  s_R0V_pic_unknown_type_421830(DAT_000c5fc0);
  s_R0V_pic_unknown_type_421830(DAT_000c5fc8);
  s_R0V_pic_unknown_type_421830(DAT_000c5ee4);
  s_R0V_pic_unknown_type_421830(DAT_000c5eec);
  s_R0V_pic_unknown_type_421830(DAT_000c5ef0);
  s_R0V_pic_unknown_type_421830(DAT_000c5f04);
  s_R0V_pic_unknown_type_421830(DAT_000c5f08);
  s_R0V_pic_unknown_type_421830(DAT_000c5f0c);
  s_R0V_pic_unknown_type_421830(DAT_000c5f10,extraout_EDX,0);
  DAT_000c5ee8 = extraout_EBX;
  return;
}


// ==== optionsmain_RateLoop @ 00035005 body=410 ranges=1

void optionsmain_RateLoop(void)

{
  __CHK();
  fn_40108(&DAT_000c5fcc,1);
  fn_40323(&DAT_000c5fc4,0);
  fn_40108(&DAT_000c5c44,1);
  fn_40108(&DAT_000c5fa4,0);
  fn_40108(&DAT_000c5fa8,0);
  fn_40108(&DAT_000c5fac,0);
  fn_40108(&DAT_000c5fb0,0);
  fn_40108(&DAT_000c5fb4,0);
  fn_40108(&DAT_000c5fb8,0);
  fn_40108(&DAT_000c5fbc,0);
  fn_40108(&DAT_000c5fc0,0);
  fn_40108(&DAT_000c5f88,0);
  fn_40108(&DAT_000c5f8c,0);
  fn_40108(&DAT_000c5f5c,0);
  fn_40108(&DAT_000c5f60,0);
  fn_40108(&DAT_000c5f64,0);
  fn_40108(&DAT_000c5f68,0);
  fn_40108(&DAT_000c5f6c,0);
  fn_40108(&DAT_000c5f70,0);
  fn_40108(&DAT_000c5f74,0);
  fn_40108(&DAT_000c5f78,0);
  fn_40108(&DAT_000c5f7c,0);
  fn_40108(&DAT_000c5f58,0);
  fn_40108(&DAT_000c5f80,0);
  fn_40108(&DAT_000c5f84,0);
  fn_40108(&DAT_000c5ee4,0);
  fn_40108(&DAT_000c5fc8,0);
  fn_40108(&DAT_000c5eec,0);
  fn_40108(&DAT_000c5ef0,0);
  fn_40108(&DAT_000c5f0c,0);
  fn_40108(&DAT_000c5f10,0);
  fn_40108(&DAT_000c5f04,0);
  FUN_0002c657(&DAT_000c5f08);
  return;
}


// ==== himain_Coli @ 00035891 body=924 ranges=1

void himain_Coli(void)

{
  bool bVar1;
  int *piVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  char *pcVar6;
  int iVar7;
  uint uVar8;
  int iVar9;
  int unaff_EBP;
  undefined4 *puVar10;
  undefined4 *puVar11;
  
  __CHK();
  if ((*(int *)(DAT_000bf80c + 0x30) == 0) || (DAT_000a65e0 != DAT_000c608c)) {
    FUN_00035546(0);
  }
  if (DAT_000a64c0 == 1) {
    fn_40bb0(&DAT_000c5c48,1,0x280);
  }
  if (*DAT_000c5fcc == 1) {
    for (iVar4 = 0; (iVar4 != 0x280 && ((&DAT_000c5c48)[iVar4] == '\0')); iVar4 = iVar4 + 1) {
    }
    for (iVar5 = 0x27f; (iVar5 != -1 && ((&DAT_000c5c48)[iVar5] == '\0')); iVar5 = iVar5 + -1) {
    }
    iVar5 = iVar5 + 2;
    if (iVar4 == 0x280) {
      iVar4 = (DAT_000c5fcc[0x13] << 0x10) / 0x1e0;
      s_lort_20451(0,iVar4,0x10000,(DAT_000c5fcc[0x14] << 0x10) / 0x1e0,0,iVar4);
      piVar2 = DAT_000c5fcc;
      DAT_000c5fcc[0x13] = 0;
      piVar2[0x14] = 0;
    }
    else {
      iVar7 = iVar4;
      iVar9 = iVar5;
      if ((iVar4 != 0) || (iVar5 != 0)) {
        if (DAT_000c5fcc[0x13] < iVar4) {
          iVar7 = DAT_000c5fcc[0x13];
        }
        if (iVar5 < DAT_000c5fcc[0x14]) {
          iVar9 = DAT_000c5fcc[0x14];
        }
      }
      iVar7 = (iVar7 << 0x10) / 0x1e0;
      s_lort_20451(0,iVar7,0x10000,(iVar9 << 0x10) / 0x1e0,0,iVar7);
      piVar2 = DAT_000c5fcc;
      DAT_000c5fcc[0x13] = iVar4;
      piVar2[0x14] = iVar5;
    }
  }
  DAT_000a65e0 = DAT_000c608c;
  DAT_000a64bc = 0;
  fn_33dd4(DAT_000c5c44,&DAT_000c5c48);
  iVar4 = FUN_00035546(2);
  bVar1 = false;
  if (((DAT_000a65d8 == 0) && (*(char *)(DAT_000bf988 + 0x1c) == '\x01')) && (iVar4 == 2)) {
    DAT_000a65d8 = -1;
    bVar1 = true;
  }
  if ((DAT_000a65dc == 0) && (*(char *)(DAT_000bf988 + 1) == '\x01')) {
    DAT_000a65dc = -1;
    bVar1 = true;
  }
  if (bVar1) {
    iVar5 = DAT_000c5c30;
    if ((DAT_000c608c == 0) || (iVar5 = DAT_000c5c34, DAT_000c608c < 2)) {
      iVar5 = iVar5 + DAT_000a6658 * 0x3c;
    }
    else {
      iVar5 = DAT_000c5c2c;
      if (DAT_000c608c != 2) {
        iVar5 = unaff_EBP;
      }
    }
    puVar10 = &DAT_000a6668;
    puVar11 = (undefined4 *)((&DAT_000c6080)[DAT_000c608c] * 0xc + iVar5);
    for (iVar7 = 1; iVar7 != 0; iVar7 = iVar7 + -1) {
      *puVar11 = *puVar10;
      puVar10 = puVar10 + 1;
      puVar11 = puVar11 + 1;
    }
    for (iVar5 = 0; iVar5 != 0; iVar5 = iVar5 + -1) {
      *(undefined1 *)puVar11 = *(undefined1 *)puVar10;
      puVar10 = (undefined4 *)((int)puVar10 + 1);
      puVar11 = (undefined4 *)((int)puVar11 + 1);
    }
    FUN_0003aded(4,&DAT_000a218d,3);
    iVar5 = FUN_00040b59(&DAT_000a6668);
    if (iVar5 == 0) {
      DAT_000a668c = DAT_000a668c | 2;
      fn_3b42b(0x230,0x80,0);
    }
    iVar5 = FUN_00040b59(&DAT_000a6668,&DAT_000a2191,3);
    if (iVar5 == 0) {
      DAT_000a668c = DAT_000a668c | 1;
      fn_3b42b(0x230,0x80,0);
    }
    FUN_0003ae0c();
    uVar8 = DAT_000c608c + 1;
    for (iVar5 = uVar8 * 4;
        (uVar3 = DAT_000c608c, iVar5 != 0xc &&
        (uVar3 = uVar8, *(int *)((int)&DAT_000c6080 + iVar5) == -1)); iVar5 = iVar5 + 4) {
      uVar8 = uVar8 + 1;
    }
    DAT_000c608c = uVar3;
    if (uVar8 == 3) {
      DAT_000a65e0 = 0xffffffff;
      fn_17b0b();
      DAT_000a6658 = DAT_000a6658 + 1;
      if ((DAT_000a6650 == 1) && (DAT_000a665c == 1)) {
        if (DAT_000a6678 < DAT_000a6658) {
          DAT_000a6678 = DAT_000a6658;
        }
        if (DAT_000a6658 == 6) {
          pcVar6 = s_endseq_000a2195;
        }
        else {
          pcVar6 = (&PTR_DAT_000a6694)[DAT_000a6658];
        }
      }
      else {
        pcVar6 = s_title_000a219c;
      }
      fn_17875(pcVar6);
    }
  }
  if ((iVar4 == 2) && (*(char *)(DAT_000bf988 + 0x1c) == '\0')) {
    DAT_000a65d8 = 0;
  }
  if (*(byte *)(DAT_000bf988 + 1) == 0) {
    DAT_000a65dc = (uint)*(byte *)(DAT_000bf988 + 1);
    FUN_00022d7a();
    return;
  }
  return;
}


// ==== himain_PreInit @ 00035c48 body=706 ranges=2

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void himain_PreInit(void)

{
  int iVar1;
  int iVar2;
  int iVar3;
  char *pcVar4;
  int iVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  
  __CHK();
  FUN_0002c6b0();
  if (DAT_000a6658 == 0) {
    DAT_000c0c00 = DAT_000a6658;
  }
  DAT_000c5fcc = (undefined4 *)0x0;
  iVar3 = DAT_000c5c00;
  iVar5 = DAT_000c5c00;
  for (iVar2 = 4; iVar2 < DAT_000a6660 * 4; iVar2 = iVar2 + 4) {
    iVar1 = *(int *)((int)&DAT_000c5c00 + iVar2);
    iVar5 = iVar5 + iVar1;
    if (iVar1 < iVar3) {
      iVar3 = iVar1;
    }
  }
  DAT_000c6080 = -1;
  DAT_000c6084 = 0xffffffff;
  _DAT_000c6088 = 0xffffffff;
  if (iVar3 != 0) {
    DAT_000c6084 = FUN_0003519f(DAT_000a6658 * 0x3c + DAT_000c5c34);
  }
  if (DAT_000a6650 == 1) {
    if (DAT_000a665c == 0) {
      if (DAT_000a6658 != 0) {
        _DAT_000c6088 = FUN_0003519f(DAT_000c5c2c,DAT_000a6658,DAT_000c0c00);
      }
    }
    else {
      if (DAT_000a6658 == 5) {
        _DAT_000c6088 = FUN_0003519f(DAT_000c5c2c,6,iVar5);
      }
      DAT_000c6080 = FUN_0003519f(DAT_000a6658 * 0x3c + DAT_000c5c30,DAT_000a6658,iVar5);
      if (DAT_000c6080 == 0) {
        puVar6 = &DAT_000c5c00;
        puVar7 = (undefined4 *)(DAT_000a6658 * 0x10 + DAT_000c5c28);
        for (iVar3 = 4; iVar3 != 0; iVar3 = iVar3 + -1) {
          *puVar7 = *puVar6;
          puVar6 = puVar6 + 1;
          puVar7 = puVar7 + 1;
        }
        for (iVar3 = 0; iVar3 != 0; iVar3 = iVar3 + -1) {
          *(undefined1 *)puVar7 = *(undefined1 *)puVar6;
          puVar6 = (undefined4 *)((int)puVar6 + 1);
          puVar7 = (undefined4 *)((int)puVar7 + 1);
        }
      }
    }
  }
  iVar2 = 0;
  for (iVar3 = 0;
      (iVar1 = DAT_000c608c, iVar3 != 0xc &&
      (iVar1 = iVar2, *(int *)((int)&DAT_000c6080 + iVar3) == -1)); iVar3 = iVar3 + 4) {
    iVar2 = iVar2 + 1;
  }
  DAT_000c608c = iVar1;
  DAT_000c0c00 = iVar5;
  if (iVar2 == 3) {
    fn_17b0b();
    if ((DAT_000a6650 == 1) && (DAT_000a665c == 1)) {
      DAT_000a6658 = DAT_000a6658 + 1;
      pcVar4 = (&PTR_DAT_000a6694)[DAT_000a6658];
    }
    else {
      pcVar4 = s_title_000a21a2;
    }
    fn_17875(pcVar4);
    FUN_00021875();
    return;
  }
  s_total_alloc_d_d_3ff1c(&DAT_000c5fcc,0x54,s_SELTITLE_000a21a8,1);
  puVar6 = DAT_000c5fcc;
  *DAT_000c5fcc = 1;
  DAT_000c5fd0 = 0;
  puVar6[0x10] = 0xffffffff;
  fn_40bb0(&DAT_000c5c48,1,0x280,0);
  fn_402ef(&DAT_000c5fc4,0x10000,s_GREMAP_000a21b1);
  fn_2f558(DAT_000c5fc4,&DAT_00096000,s_DRAWSCR_000a21b8,1);
  s_total_alloc_d_d_3ff1c(&DAT_000c5c44);
  s_error_loading_file_s_4027d(&DAT_000c5fc8,s_l__titlegfx_title_FONTY3_1_R0V_000a21c0,1);
  s_error_loading_file_s_4027d(&DAT_000c5f04,s_l__titlegfx_title_TITLEBG_R0V_000a21df,1);
  switch(DAT_000a6658) {
  case 0:
    pcVar4 = s_L__titlegfx_selectB_MapB1out_R0V_000a21fd;
    break;
  case 1:
    pcVar4 = s_L__titlegfx_selectB_MapB2out_R0V_000a221e;
    break;
  case 2:
    pcVar4 = s_L__titlegfx_selectB_MapB3out_R0V_000a223f;
    break;
  case 3:
    pcVar4 = s_L__titlegfx_selectB_MapB4out_R0V_000a2260;
    break;
  case 4:
    pcVar4 = s_L__titlegfx_selectB_MapB5out_R0V_000a2281;
    break;
  case 5:
    pcVar4 = s_L__titlegfx_selectB_MapB6out_R0V_000a22a2;
    break;
  default:
    goto switchD_00035fde_default;
  }
  s_error_loading_file_s_4027d(&DAT_000c5f2c,pcVar4,1);
switchD_00035fde_default:
  s_error_loading_file_s_4027d(&DAT_000c5f1c,s_L__titlegfx_selectL_BGGRID_R0V_000a22c3,1);
  s_error_loading_file_s_4027d(&DAT_000c5f14,s_L__titlegfx_selectL_BESTLTI_R0V_000a22e2,1);
  s_error_loading_file_s_4027d(&DAT_000c5f18,s_L__titlegfx_selectL_BESTTTI_R0V_000a2302,1);
  s_error_loading_file_s_4027d(&DAT_000c5edc,s_l__titlegfx_title_BCTP_R0V_000a2322,1);
  s_error_loading_file_s_4027d(&DAT_000c5f24,s_L__titlegfx_selectL_seltal_R0V_000a233d,1);
  s_R0V_pic_unknown_type_421830(DAT_000c5fc8);
  s_R0V_pic_unknown_type_421830(DAT_000c5f04);
  s_R0V_pic_unknown_type_421830(DAT_000c5f2c);
  s_R0V_pic_unknown_type_421830(DAT_000c5f1c);
  s_R0V_pic_unknown_type_421830(DAT_000c5f14);
  s_R0V_pic_unknown_type_421830(DAT_000c5f18);
  s_R0V_pic_unknown_type_421830(DAT_000c5edc);
  s_R0V_pic_unknown_type_421830(DAT_000c5f24);
  FUN_00021875();
  return;
}


// ==== himain_RateLoop @ 00035fe6 body=190 ranges=1

void himain_RateLoop(void)

{
  __CHK();
  if (DAT_000c5fcc != 0) {
    FUN_0002c683();
    fn_40108(&DAT_000c5fcc,1);
    fn_40323(&DAT_000c5fc4,0);
    fn_40108(&DAT_000c5c44,1);
    fn_40108(&DAT_000c5fc8,1);
    fn_40108(&DAT_000c5f04,1);
    fn_40108(&DAT_000c5f2c,1);
    fn_40108(&DAT_000c5f1c,1);
    fn_40108(&DAT_000c5f14,1);
    fn_40108(&DAT_000c5f18,1);
    fn_40108(&DAT_000c5edc,1);
    fn_40108(&DAT_000c5f24,1);
    return;
  }
  return;
}


// ==== install_Coli @ 000365b1 body=1885 ranges=1

void install_Coli(void)

{
  char cVar1;
  int *piVar2;
  undefined4 uVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  undefined4 extraout_EDX;
  int extraout_EBX;
  int extraout_EBX_00;
  int iVar7;
  char *pcVar8;
  char *pcVar9;
  char *pcVar10;
  undefined8 uVar11;
  char *pcStack_24;
  char *pcStack_20;
  int iStack_1c;
  int iStack_18;
  int iStack_14;
  int *piStack_10;
  
  __CHK();
  piStack_10 = &DAT_000c6090;
  if (DAT_000c6098 == 0) {
    fn_2f120(DAT_000c5f08,DAT_000c5c44,0xffffffff,0xffffffff,0xffffffff,0xffffffff);
    fn_2f3bc(DAT_000c5f04,DAT_000c5c44,0xffffffff);
    fn_2f3bc(DAT_000c5ee8,DAT_000c5c44,0xffffffff);
    FUN_000360a4(0,0,0,0,0xffffffff);
    FUN_0002fae8(0x6e,0x13b,DAT_000c60d0,0xffffffff);
  }
  if (*(char *)(DAT_000bf988 + 1) == '\x01') {
    DAT_000c6298 = 1;
  }
  iStack_14 = piStack_10[1];
  if (iStack_14 == 0) {
    if (*piStack_10 != 0) {
      fn_2f120(DAT_000c5f08,DAT_000c5c44,0xdc,0x118,0xffffffff,0xffffffff);
      fn_2f3bc(DAT_000c60e0,DAT_000c5c44,0xffffffff);
      piStack_10[3] = 0;
    }
    FUN_000360a4(piStack_10[0x10],0,0x6e,0x13b,0);
    piVar2 = piStack_10;
    if (*(char *)(DAT_000bf988 + 0x1c) == '\x01') {
      iVar5 = -1;
      pcVar9 = (char *)piStack_10[0x10];
      do {
        if (iVar5 == 0) break;
        iVar5 = iVar5 + -1;
        cVar1 = *pcVar9;
        pcVar9 = pcVar9 + 1;
      } while (cVar1 != '\0');
      if (iVar5 == -2) {
        piStack_10[1] = 2;
      }
      else {
        iStack_1c = 0;
        uVar6 = 0xffffffff;
        pcVar9 = (char *)piStack_10[0x10];
        do {
          if (uVar6 == 0) break;
          uVar6 = uVar6 - 1;
          cVar1 = *pcVar9;
          pcVar9 = pcVar9 + 1;
        } while (cVar1 != '\0');
        if (((char *)piStack_10[0x10])[~uVar6 - 2] != '\\') {
          pcVar8 = &DAT_000a2360;
          iVar5 = -1;
          pcVar9 = (char *)piStack_10[0x10];
          do {
            pcVar10 = pcVar9;
            if (iVar5 == 0) break;
            iVar5 = iVar5 + -1;
            pcVar10 = pcVar9 + 1;
            cVar1 = *pcVar9;
            pcVar9 = pcVar10;
          } while (cVar1 != '\0');
          pcVar10 = pcVar10 + -1;
          do {
            cVar1 = *pcVar8;
            *pcVar10 = cVar1;
            if (cVar1 == '\0') break;
            cVar1 = pcVar8[1];
            pcVar8 = pcVar8 + 2;
            pcVar10[1] = cVar1;
            pcVar10 = pcVar10 + 2;
          } while (cVar1 != '\0');
          iStack_1c = 1;
        }
        if (*(char *)(piStack_10[0x10] + 1) == ':') {
          s_total_alloc_d_d_3ff1c(&pcStack_24,0x100,&DAT_000a2362,1);
          pcVar9 = (char *)piVar2[0x10];
          pcVar8 = pcStack_24;
          do {
            cVar1 = *pcVar9;
            *pcVar8 = cVar1;
            if (cVar1 == '\0') break;
            cVar1 = pcVar9[1];
            pcVar9 = pcVar9 + 2;
            pcVar8[1] = cVar1;
            pcVar8 = pcVar8 + 2;
          } while (cVar1 != '\0');
          pcVar8 = s_test_bin_000a2367;
          iVar5 = -1;
          pcVar9 = pcStack_24;
          do {
            pcVar10 = pcVar9;
            if (iVar5 == 0) break;
            iVar5 = iVar5 + -1;
            pcVar10 = pcVar9 + 1;
            cVar1 = *pcVar9;
            pcVar9 = pcVar10;
          } while (cVar1 != '\0');
          pcVar10 = pcVar10 + -1;
          do {
            cVar1 = *pcVar8;
            *pcVar10 = cVar1;
            if (cVar1 == '\0') break;
            cVar1 = pcVar8[1];
            pcVar8 = pcVar8 + 2;
            pcVar10[1] = cVar1;
            pcVar10 = pcVar10 + 2;
          } while (cVar1 != '\0');
          uVar3 = FUN_0003f35f(*(undefined1 *)piStack_10[0x10],extraout_EDX,piStack_10);
          uVar11 = s_FBUF_3719e(uVar3,uVar3);
          *(int *)(extraout_EBX + 0x28) = (int)uVar11;
          if ((int)((ulonglong)uVar11 >> 0x20) < (int)uVar11 + 1000) {
            *(undefined4 *)(extraout_EBX + 4) = 4;
          }
          else if ((*(char *)(*(int *)(extraout_EBX + 0x40) + 1) == ':') &&
                  (iVar5 = FUN_0003f0f0(pcStack_24), iVar5 == 1)) {
            FUN_0003f0ce(pcStack_24);
            *(undefined4 *)(extraout_EBX_00 + 4) = 1;
          }
          else {
            piStack_10[1] = 2;
          }
          fn_40108(&pcStack_24,1);
        }
        else {
          piStack_10[1] = 2;
        }
        if ((piStack_10[1] != 1) && (iStack_1c == 1)) {
          uVar6 = 0xffffffff;
          pcVar9 = (char *)piStack_10[0x10];
          do {
            if (uVar6 == 0) break;
            uVar6 = uVar6 - 1;
            cVar1 = *pcVar9;
            pcVar9 = pcVar9 + 1;
          } while (cVar1 != '\0');
          *(undefined1 *)((~uVar6 - 2) + piStack_10[0x10]) = 0;
        }
      }
    }
  }
  else if (iStack_14 == 1) {
    if (*piStack_10 != 1) {
      fn_2f120(DAT_000c5f04,DAT_000c5c44,0xdc,0x168,0xffffffff,0xffffffff);
      fn_2f3bc(DAT_000c60e4,DAT_000c5c44,0xffffffff);
      piStack_10[3] = 0;
      piStack_10[0xd] = piStack_10[10] / 0x1a8;
      piStack_10[0xd] = piStack_10[10] / 0x1a8 << 2;
    }
    iVar5 = FUN_00036f71(piStack_10);
    if (iVar5 == 0) {
      piStack_10[1] = 3;
    }
    FUN_00036519(piStack_10);
  }
  else if (iStack_14 == 3) {
    if (*piStack_10 != 3) {
      piStack_10[0xe] = *(int *)(DAT_000bf80c + 0x130);
      fn_2f120(DAT_000c5f08,DAT_000c5c44,0xdc,0x118,0xffffffff,0xffffffff);
      fn_2f3bc(DAT_000c60dc,DAT_000c5c44,0xffffffff);
      piStack_10[3] = 0;
    }
    if (piStack_10[0xe] + 1000 < *(int *)(DAT_000bf80c + 0x130)) {
      piStack_10[0xb] = 0;
      FUN_00036519();
    }
    if ((*(char *)(DAT_000bf988 + 0x1c) == '\x01') ||
       (piStack_10[0xe] + 0x9c4 < *(int *)(DAT_000bf80c + 0x130))) {
      pcVar8 = &DAT_000a2375;
      s_total_alloc_d_d_3ff1c(&pcStack_20,0x94,&DAT_000a2370,1);
      piVar2 = piStack_10;
      pcVar9 = pcStack_20;
      do {
        cVar1 = *pcVar8;
        *pcVar9 = cVar1;
        if (cVar1 == '\0') break;
        cVar1 = pcVar8[1];
        pcVar8 = pcVar8 + 2;
        pcVar9[1] = cVar1;
        pcVar9 = pcVar9 + 2;
      } while (cVar1 != '\0');
      pcVar9 = (char *)piStack_10[0x10];
      iVar5 = -1;
      pcVar8 = pcStack_20;
      do {
        pcVar10 = pcVar8;
        if (iVar5 == 0) break;
        iVar5 = iVar5 + -1;
        pcVar10 = pcVar8 + 1;
        cVar1 = *pcVar8;
        pcVar8 = pcVar10;
      } while (cVar1 != '\0');
      pcVar10 = pcVar10 + -1;
      do {
        cVar1 = *pcVar9;
        *pcVar10 = cVar1;
        if (cVar1 == '\0') break;
        cVar1 = pcVar9[1];
        pcVar9 = pcVar9 + 2;
        pcVar10[1] = cVar1;
        pcVar10 = pcVar10 + 2;
      } while (cVar1 != '\0');
      uVar6 = 0xffffffff;
      pcVar9 = pcStack_20;
      do {
        if (uVar6 == 0) break;
        uVar6 = uVar6 - 1;
        cVar1 = *pcVar9;
        pcVar9 = pcVar9 + 1;
      } while (cVar1 != '\0');
      if (pcStack_20[~uVar6 - 2] == '\\') {
        pcStack_20[~uVar6 - 2] = '\0';
      }
      FUN_0003f3df(pcStack_20);
      pcVar9 = (char *)piVar2[0x10];
      pcVar8 = pcStack_20;
      do {
        cVar1 = *pcVar9;
        *pcVar8 = cVar1;
        if (cVar1 == '\0') break;
        cVar1 = pcVar9[1];
        pcVar9 = pcVar9 + 2;
        pcVar8[1] = cVar1;
        pcVar8 = pcVar8 + 2;
      } while (cVar1 != '\0');
      if (pcStack_20[1] == ':') {
        pcStack_20[2] = '\0';
      }
      FUN_0003f3df(pcStack_20,1);
      fn_40108(&pcStack_20);
      fn_3fb31();
    }
  }
  else {
    if (iStack_14 == 2) {
      if (*piStack_10 != 2) {
        piStack_10[0xe] = *(int *)(DAT_000bf80c + 0x130);
        fn_2f120(DAT_000c5f08,DAT_000c5c44,0xdc,0x118,0xffffffff,0xffffffff);
        fn_2f3bc(DAT_000c60e8,DAT_000c5c44,0xffffffff);
        piStack_10[3] = 0;
      }
      if (*(int *)(DAT_000bf80c + 0x130) <= piStack_10[0xe] + 1000) goto LAB_00036bbe;
    }
    else {
      if (iStack_14 != 4) goto LAB_00036bbe;
      if (*piStack_10 != 4) {
        piStack_10[0xe] = *(int *)(DAT_000bf80c + 0x130);
        fn_2f120(DAT_000c5f08,DAT_000c5c44,0xdc,0x118,0xffffffff,0xffffffff);
        fn_2f3bc(DAT_000c60ec,DAT_000c5c44,0xffffffff);
        piStack_10[3] = 0;
      }
      if (*(int *)(DAT_000bf80c + 0x130) <= piStack_10[0xe] + 1000) goto LAB_00036bbe;
    }
    piStack_10[1] = 0;
  }
LAB_00036bbe:
  iVar5 = DAT_000a64c0;
  *piStack_10 = iStack_14;
  if (iVar5 == 1) {
    fn_40bb0(&DAT_000c5c48,1,0x280);
  }
  if (piStack_10[3] < -2) {
    for (iVar5 = 0; (iVar5 != 0x280 && ((&DAT_000c5c48)[iVar5] == '\0')); iVar5 = iVar5 + 1) {
    }
    for (iStack_18 = 0x27f; (iStack_18 != -1 && ((&DAT_000c5c48)[iStack_18] == '\0'));
        iStack_18 = iStack_18 + -1) {
    }
    iStack_18 = iStack_18 + 2;
    if (iVar5 == 0x280) {
      iVar5 = (piStack_10[5] << 0x10) / 0x1e0;
      s_lort_20451(0,iVar5,0x10000,(piStack_10[7] << 0x10) / 0x1e0,0,iVar5);
      piStack_10[5] = 0;
      piStack_10[7] = 0;
    }
    else {
      iVar4 = iVar5;
      iVar7 = iStack_18;
      if ((iVar5 != 0) || (iStack_18 != 0)) {
        if (piStack_10[5] < iVar5) {
          iVar4 = piStack_10[5];
        }
        if (iStack_18 < piStack_10[7]) {
          iVar7 = piStack_10[7];
        }
      }
      iVar4 = (iVar4 << 0x10) / 0x1e0;
      s_lort_20451(0,iVar4,0x10000,(iVar7 << 0x10) / 0x1e0,0,iVar4);
      piStack_10[5] = iVar5;
      piStack_10[7] = iStack_18;
    }
  }
  else {
    piStack_10[3] = piStack_10[3] + -1;
  }
  DAT_000a64bc = 0;
  piStack_10[2] = piStack_10[2] + 1;
  fn_33dd4(DAT_000c5c44,&DAT_000c5c48);
  FUN_0002e5b5();
  return;
}


// ==== install_PreInit @ 00036d0e body=433 ranges=1

void install_PreInit(void)

{
  char cVar1;
  char *pcVar2;
  char *pcVar3;
  
  __CHK();
  fn_40bb0(&DAT_000c6090,0,0x44,1);
  DAT_000c6090 = 0xffffffff;
  pcVar2 = s_c__Games_Scorcher__000a23b9;
  s_total_alloc_d_d_3ff1c(&DAT_000c60d0,0x100,&DAT_000a23b6);
  pcVar3 = DAT_000c60d0;
  do {
    cVar1 = *pcVar2;
    *pcVar3 = cVar1;
    if (cVar1 == '\0') break;
    cVar1 = pcVar2[1];
    pcVar2 = pcVar2 + 2;
    pcVar3[1] = cVar1;
    pcVar3 = pcVar3 + 2;
  } while (cVar1 != '\0');
  fn_40bb0(&DAT_000c5c48,1,0x280,1);
  s_total_alloc_d_d_3ff1c(&DAT_000c5c44,&DAT_00096000,s_DRAWSCR_000a23cc);
  s_error_loading_file_s_4027d(&DAT_000c5fc8,s_l__titlegfx_title_FONTY3_1_R0V_000a23d4,1);
  s_error_loading_file_s_4027d(&DAT_000c60e0,s_l__titlegfx_install_dir_R0V_000a23f3,1);
  s_error_loading_file_s_4027d(&DAT_000c5f04,s_l__titlegfx_install_baggrund_R0V_000a240f,1);
  s_error_loading_file_s_4027d(&DAT_000c5f08,s_l__titlegfx_title_TITLELO_R0V_000a2430,1);
  s_error_loading_file_s_4027d(&DAT_000c60e8,s_l__titlegfx_install_invalid_R0V_000a244e,1);
  s_error_loading_file_s_4027d(&DAT_000c60e4,s_l__titlegfx_install_copying_R0V_000a246e,1);
  s_error_loading_file_s_4027d(&DAT_000c5ee8,s_l__titlegfx_title_COPYRI_1_R0V_000a248e,1);
  s_error_loading_file_s_4027d(&DAT_000c60dc,s_l__titlegfx_install_complete_R0V_000a24ad,1);
  s_error_loading_file_s_4027d(&DAT_000c60ec,s_l__titlegfx_install_outasp_R0V_000a24ce,1);
  s_R0V_pic_unknown_type_421830(DAT_000c5ee8);
  s_R0V_pic_unknown_type_421830(DAT_000c5f04);
  s_R0V_pic_unknown_type_421830(DAT_000c5f08);
  s_R0V_pic_unknown_type_421830(DAT_000c5fc8);
  s_R0V_pic_unknown_type_421830(DAT_000c60e0);
  s_R0V_pic_unknown_type_421830(DAT_000c60e8);
  s_R0V_pic_unknown_type_421830(DAT_000c60e4);
  s_R0V_pic_unknown_type_421830(DAT_000c60dc);
  s_R0V_pic_unknown_type_421830(DAT_000c60ec);
  DAT_000c5f1c = DAT_000c5f04;
  return;
}


// ==== install_RateLoop @ 00036ebf body=178 ranges=1

void install_RateLoop(void)

{
  __CHK();
  fn_40108(&DAT_000c60d0,1);
  fn_40108(&DAT_000c5c44,1);
  fn_40108(&DAT_000c5fc8,1);
  fn_40108(&DAT_000c60e0,1);
  fn_40108(&DAT_000c5f04,1);
  fn_40108(&DAT_000c5f08,1);
  fn_40108(&DAT_000c60e8,1);
  fn_40108(&DAT_000c60e4,1);
  fn_40108(&DAT_000c5ee8,1);
  fn_40108(&DAT_000c60dc,1);
  fn_40108(&DAT_000c60ec,1);
  return;
}


// ==== demosel_PreInit @ 0003723c body=212 ranges=1

void demosel_PreInit(void)

{
  char cVar1;
  char *pcVar2;
  char *pcVar3;
  
  __CHK();
  pcVar2 = s_c__Games_Scorcher__000a2526;
  fn_40bb0(&DAT_000c6090,0,0x44);
  DAT_000c6090 = 0xffffffff;
  pcVar3 = DAT_000c60d0;
  do {
    cVar1 = *pcVar2;
    *pcVar3 = cVar1;
    if (cVar1 == '\0') break;
    cVar1 = pcVar2[1];
    pcVar2 = pcVar2 + 2;
    pcVar3[1] = cVar1;
    pcVar3 = pcVar3 + 2;
  } while (cVar1 != '\0');
  fn_40bb0(&DAT_000c5c48,1,0x280,1);
  s_total_alloc_d_d_3ff1c(&DAT_000c5c44,&DAT_00096000,s_DRAWSCR_000a2539);
  s_error_loading_file_s_4027d(&DAT_000c5f04,s_l__titlegfx_SelectB_DemoBG_R0V_000a2541,1);
  s_error_loading_file_s_4027d(&DAT_000c60f0,s_l__titlegfx_SelectB_DemoLev1_R0V_000a2560,1);
  s_error_loading_file_s_4027d(&DAT_000c60f4,s_l__titlegfx_SelectB_DemoLev5_R0V_000a2581,1);
  s_R0V_pic_unknown_type_421830(DAT_000c5f04);
  s_R0V_pic_unknown_type_421830(DAT_000c60f0);
  s_R0V_pic_unknown_type_421830(DAT_000c60f4);
  DAT_000c5f1c = DAT_000c5f04;
  return;
}


// ==== demosel_RateLoop @ 00037310 body=70 ranges=1

void demosel_RateLoop(void)

{
  __CHK();
  fn_40108(&DAT_000c5c44,1);
  fn_40108(&DAT_000c5f04,1);
  fn_40108(&DAT_000c60f0,1);
  fn_40108(&DAT_000c60f4,1);
  return;
}


// ==== demosel_Coli @ 00037356 body=766 ranges=1

void demosel_Coli(void)

{
  undefined *puVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  undefined4 extraout_EDX;
  undefined4 extraout_EBX;
  int iVar6;
  
  __CHK();
  s_playcdok_1001f(3);
  if (DAT_000c6098 == 0) {
    fn_2f120(DAT_000c5f04,DAT_000c5c44,0xffffffff,0xffffffff,0xffffffff,0xffffffff);
    fn_2f44b(DAT_000c60f0,DAT_000c5c44,0xffffffff,0xffffffff,0xffffffff,0xffffffff);
  }
  if (*(char *)(DAT_000bf988 + 0xcb) == '\x01') {
    DAT_000c6094 = 0;
  }
  if (*(char *)(DAT_000bf988 + 0xcd) == '\x01') {
    DAT_000c6094 = 1;
  }
  if (DAT_000c6090 != DAT_000c6094) {
    DAT_000c6090 = DAT_000c6094;
    fn_2f120(DAT_000c5f04,DAT_000c5c44,0xdc,0x1cc,0xffffffff,0xffffffff);
    uVar2 = DAT_000c60f4;
    if (DAT_000c6094 == 0) {
      uVar2 = DAT_000c60f0;
    }
    fn_2f44b(uVar2,DAT_000c5c44,0xffffffff,0xffffffff,0xffffffff,0xffffffff);
  }
  if ((DAT_000a65ec == 0) && (*(char *)(DAT_000bf988 + 1) == '\x01')) {
    DAT_000a65ec = -1;
    DAT_000c6298 = 1;
  }
  if ((10 < DAT_000c6098) && (*(char *)(DAT_000bf988 + 1) == '\0')) {
    DAT_000a65ec = 0;
  }
  if ((DAT_000c60b4 == 1) && (*(char *)(DAT_000bf988 + 0x1c) == '\x01')) {
    DAT_000c60b4 = 0;
    fn_17b0b();
    DAT_000a65ec = -1;
    if (DAT_000c6094 == 0) {
      fn_17875(&DAT_000a25a2);
      DAT_000a6658 = extraout_EDX;
    }
    else {
      fn_17875(&DAT_000a25a7,DAT_000c6094,4);
      DAT_000a6658 = extraout_EBX;
    }
    puVar1 = PTR_DAT_000a64b8;
    *(undefined4 *)(PTR_DAT_000a64b8 + 0xf0) = 0xffffffff;
    DAT_000a6650 = 0;
    *(undefined4 *)(puVar1 + 0xec) = 0;
  }
  if (*(char *)(DAT_000bf988 + 0x1c) == '\0') {
    DAT_000c60b4 = 1;
  }
  if (DAT_000a64c0 == 1) {
    fn_40bb0(&DAT_000c5c48,1,0x280);
  }
  if (DAT_000c609c < -2) {
    for (iVar3 = 0; (iVar3 != 0x280 && ((&DAT_000c5c48)[iVar3] == '\0')); iVar3 = iVar3 + 1) {
    }
    for (iVar4 = 0x27f; (iVar4 != -1 && ((&DAT_000c5c48)[iVar4] == '\0')); iVar4 = iVar4 + -1) {
    }
    iVar4 = iVar4 + 2;
    if (iVar3 == 0x280) {
      iVar3 = (DAT_000c60a4 << 0x10) / 0x1e0;
      s_lort_20451(0,iVar3,0x10000,(DAT_000c60ac << 0x10) / 0x1e0,0,iVar3);
      DAT_000c60a4 = 0;
      DAT_000c60ac = 0;
    }
    else {
      iVar5 = iVar3;
      iVar6 = iVar4;
      if ((iVar3 != 0) || (iVar4 != 0)) {
        if (DAT_000c60a4 < iVar3) {
          iVar5 = DAT_000c60a4;
        }
        if (iVar4 < DAT_000c60ac) {
          iVar6 = DAT_000c60ac;
        }
      }
      iVar5 = (iVar5 << 0x10) / 0x1e0;
      s_lort_20451(0,iVar5,0x10000,(iVar6 << 0x10) / 0x1e0,0,iVar5);
      DAT_000c60a4 = iVar3;
      DAT_000c60ac = iVar4;
    }
  }
  else {
    DAT_000c609c = DAT_000c609c + -1;
  }
  DAT_000a64bc = 0;
  DAT_000c6098 = DAT_000c6098 + 1;
  fn_33dd4(DAT_000c5c44,&DAT_000c5c48);
  FUN_0002e5b5();
  return;
}


// ==== demoexit_Init @ 0003799c body=89 ranges=1

void demoexit_Init(void)

{
  int *extraout_EDX;
  
  __CHK();
  if (*(int *)(DAT_000bf80c + 0x30) == 0) {
    DAT_000a65f0 = DAT_000c6294;
  }
  if ((DAT_000a65f0 != DAT_000c6294) || (*extraout_EDX < *(int *)(DAT_000bf80c + 300) >> 8)) {
    fn_17b0b();
    fn_17875(s_title_000a25ac);
  }
  return;
}


// ==== camwpul_Display @ 00037a65 body=900 ranges=1

void camwpul_Display(void)

{
  byte bVar1;
  undefined *puVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  undefined4 extraout_ECX;
  int *extraout_ECX_00;
  uint extraout_ECX_01;
  uint uVar6;
  int *piVar7;
  undefined4 extraout_EBX;
  int iVar8;
  int *piVar9;
  int *piVar10;
  undefined8 uVar11;
  undefined1 auStack_24 [4];
  undefined4 uStack_20;
  undefined4 uStack_1c;
  byte *pbStack_18;
  int *piStack_14;
  undefined1 uStack_10;
  
  uVar11 = __CHK();
  piVar7 = (int *)((ulonglong)uVar11 >> 0x20);
  iVar3 = (int)uVar11;
  if ((*(byte *)(piVar7 + 0x2e) & 8) == 0) {
    DAT_000c0cd4 = 0;
    DAT_000c0cf0 = 0;
  }
  else {
    if (DAT_000c0ce8 != 1) {
      return;
    }
    piVar7[0x1c] = 0;
    piVar7[0x1d] = 10000;
  }
  uStack_1c = extraout_ECX;
  uVar4 = fn_112f4(piVar7[0x1e],auStack_24);
  uStack_20 = 0;
  pbStack_18 = (byte *)fn_1131c(uVar4,auStack_24,&uStack_20);
  iVar8 = *(int *)(*(int *)(*(int *)(pbStack_18 + 0x98) + 0x24) + 0x7c) >> 0x10;
  if (extraout_ECX_00[1] == 0) {
    piStack_14 = (int *)0x21;
    iVar5 = *(int *)(DAT_000bf80c + 0x130) / 0x21;
    uStack_10 = iVar5 < 0x3c;
    DAT_000c0cf0 = (uint)(byte)uStack_10;
    if (iVar5 < 0x5a) {
      iVar8 = 1;
    }
  }
  if (iVar8 == 0) {
    *extraout_ECX_00 = 0;
  }
  if (extraout_ECX_00[2] < iVar8) {
    bVar1 = *(byte *)(extraout_ECX_00 + 0x12);
    *(byte *)(extraout_ECX_00 + 0x12) = bVar1 | 8;
    extraout_ECX_00[0xe] = 0;
    *(byte *)(extraout_ECX_00 + 0x12) = bVar1 & 0xef | 8;
  }
  if ((((*(byte *)(extraout_ECX_00 + 0x12) & 8) != 0) && (extraout_ECX_00[1] < iVar8)) &&
     (iVar8 < extraout_ECX_00[2])) {
    *extraout_ECX_00 = iVar8;
    *(byte *)(extraout_ECX_00 + 0x12) = *(byte *)(extraout_ECX_00 + 0x12) | 0x10;
    *extraout_ECX_00 = 10000;
  }
  wldpul_Init(iVar3,extraout_ECX_00 + 4,extraout_EBX,uStack_1c);
  if ((*(byte *)(extraout_ECX_00 + 0x12) & 8) == 0) {
    fn_3f6d7();
    if ((*(byte *)(piVar7 + 0x2e) & 2) == 0) {
      *pbStack_18 = *pbStack_18 & 0xfe;
    }
    else {
      *pbStack_18 = *pbStack_18 | 1;
    }
    iVar8 = fn_1026b(*(int *)(iVar3 + 0x20) - *piVar7,*(int *)(iVar3 + 0x24) - piVar7[1],
                     *(int *)(iVar3 + 0x28) - piVar7[2]);
    if (0x138800 < iVar8) {
      *piVar7 = *(int *)(iVar3 + 0x20);
      piVar7[1] = *(int *)(iVar3 + 0x24);
      iVar8 = *(int *)(iVar3 + 0x28);
      piVar7[0xc] = 0;
      piVar7[0xd] = 0;
      piVar7[2] = iVar8;
      piVar7[0xe] = 0;
      fn_37715(piVar7 + 0xf,(int *)(iVar3 + 0x5c));
      piVar9 = (int *)(iVar3 + 0x5c);
      piVar10 = piVar7 + 3;
      for (uVar6 = extraout_ECX_01 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
        *piVar10 = *piVar9;
        piVar9 = piVar9 + 1;
        piVar10 = piVar10 + 1;
      }
      for (uVar6 = extraout_ECX_01 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
        *(char *)piVar10 = (char)*piVar9;
        piVar9 = (int *)((int)piVar9 + 1);
        piVar10 = (int *)((int)piVar10 + 1);
      }
    }
    iVar8 = fn_12030c(*(int *)(iVar3 + 0x20) - *piVar7,0x2000);
    piVar7[0xc] = piVar7[0xc] + iVar8;
    iVar8 = fn_12030c(*(int *)(iVar3 + 0x24) - piVar7[1],0x2000);
    piVar7[0xd] = piVar7[0xd] + iVar8;
    iVar8 = fn_12030c(*(int *)(iVar3 + 0x28) - piVar7[2],0x2000);
    piVar7[0xe] = piVar7[0xe] + iVar8;
    *piVar7 = *piVar7 + piVar7[0xc];
    piVar7[1] = piVar7[1] + piVar7[0xd];
    piVar7[2] = piVar7[2] + piVar7[0xe];
    iVar8 = fn_12030c(piVar7[0xc],0x6000);
    piVar7[0xc] = iVar8;
    iVar8 = fn_12030c(piVar7[0xd],0x6000);
    piVar7[0xd] = iVar8;
    iVar8 = fn_12030c(piVar7[0xe],0x6000);
    piVar7[0xe] = iVar8;
    piStack_14 = piVar7 + 3;
    fn_37770(piVar7 + 0xf,(int *)(iVar3 + 0x5c),piStack_14);
    *(int *)(iVar3 + 0x20) = *piVar7;
    *(int *)(iVar3 + 0x24) = piVar7[1];
    *(int *)(iVar3 + 0x28) = piVar7[2];
    piVar9 = piStack_14;
    piVar10 = (int *)(iVar3 + 0x5c);
    for (iVar8 = 9; iVar8 != 0; iVar8 = iVar8 + -1) {
      *piVar10 = *piVar9;
      piVar9 = piVar9 + 1;
      piVar10 = piVar10 + 1;
    }
    for (iVar8 = 0; iVar8 != 0; iVar8 = iVar8 + -1) {
      *(char *)piVar10 = (char)*piVar9;
      piVar9 = (int *)((int)piVar9 + 1);
      piVar10 = (int *)((int)piVar10 + 1);
    }
    piVar9 = piStack_14;
    piVar10 = (int *)(PTR_DAT_000a86d4 + 0x1c);
    for (iVar8 = 9; iVar8 != 0; iVar8 = iVar8 + -1) {
      *piVar10 = *piVar9;
      piVar9 = piVar9 + 1;
      piVar10 = piVar10 + 1;
    }
    for (iVar8 = 0; puVar2 = PTR_DAT_000a86d4, iVar8 != 0; iVar8 = iVar8 + -1) {
      *(char *)piVar10 = (char)*piVar9;
      piVar9 = (int *)((int)piVar9 + 1);
      piVar10 = (int *)((int)piVar10 + 1);
    }
    *(int *)(PTR_DAT_000a86d4 + 4) = *piVar7;
    *(int *)(puVar2 + 8) = piVar7[1];
    *(int *)(puVar2 + 0xc) = piVar7[2];
    *(undefined4 *)(puVar2 + 0x10) = *(undefined4 *)(pbStack_18 + 0x20);
    *(undefined4 *)(puVar2 + 0x14) = *(undefined4 *)(pbStack_18 + 0x24);
    uVar4 = *(undefined4 *)(pbStack_18 + 0x28);
    *(undefined4 *)(puVar2 + 0x18) = uVar4;
    if ((*(byte *)(piVar7 + 0x2e) & 1) != 0) {
      fn_379f5(puVar2 + 0x1c,*(undefined4 *)(puVar2 + 4),*(undefined4 *)(puVar2 + 8),
               *(undefined4 *)(puVar2 + 0xc),*(undefined4 *)(puVar2 + 0x10),
               *(undefined4 *)(puVar2 + 0x14),uVar4);
    }
    if ((*(byte *)(piVar7 + 0x2e) & 4) == 0) {
      *(undefined4 *)(PTR_DAT_000a64b8 + 0xec) = 0;
    }
    else {
      *(undefined4 *)(PTR_DAT_000a64b8 + 0xec) = 1;
    }
  }
  else {
    fn_3f6d7();
  }
  DAT_000c0b80 = iVar3;
  fn_37654();
  return;
}


// ==== camwpul_PreInit @ 00037de9 body=34 ranges=1

void camwpul_PreInit(void)

{
  byte *pbVar1;
  int iVar2;
  undefined8 uVar3;
  
  uVar3 = __CHK();
  iVar2 = (int)((ulonglong)uVar3 >> 0x20);
  pbVar1 = (byte *)uVar3;
  wldpul_PreInit(pbVar1,iVar2 + 0x7c);
  *pbVar1 = *pbVar1 | 1;
  pbVar1 = (byte *)(iVar2 + 0xb4);
  *pbVar1 = *pbVar1 | 0x20;
  return;
}


// ==== camwpul_RateLoop @ 00037e0b body=13 ranges=1

void camwpul_RateLoop(void)

{
  undefined4 extraout_ECX;
  undefined4 extraout_EDX;
  undefined4 extraout_EBX;
  undefined8 uVar1;
  
  uVar1 = __CHK();
  __CHK((int)uVar1,(int)((ulonglong)uVar1 >> 0x20) + 0x7c,extraout_EBX,extraout_ECX,4);
  fn_40108(extraout_EDX,1);
  return;
}


// ==== wldpul_RateLoop @ 00037e18 body=12 ranges=1

void wldpul_RateLoop(void)

{
  undefined4 extraout_EDX;
  
  __CHK();
  fn_40108(extraout_EDX,1);
  return;
}


// ==== camlir_Display @ 00037e2e body=646 ranges=1

void camlir_Display(void)

{
  undefined *puVar1;
  byte *pbVar2;
  undefined4 uVar3;
  int iVar4;
  uint extraout_ECX;
  uint uVar5;
  int *piVar6;
  byte *pbVar7;
  byte *pbVar8;
  undefined8 uVar9;
  undefined1 auStack_1c [4];
  undefined4 uStack_18;
  byte *pbStack_14;
  byte *pbStack_10;
  
  uVar9 = __CHK();
  piVar6 = (int *)((ulonglong)uVar9 >> 0x20);
  pbVar2 = (byte *)uVar9;
  *(undefined4 *)PTR_DAT_000a86d4 = 0;
  wldpul_Init(pbVar2,piVar6 + 0x1f);
  if ((*(byte *)(piVar6 + 0x2d) & 8) == 0) {
    pbStack_10 = (byte *)0x0;
    if (piVar6[0x1e] == 0) {
      fn_3fb31();
    }
    else {
      uVar3 = fn_112f4(piVar6[0x1e],auStack_1c);
      uStack_18 = 0;
      pbStack_10 = (byte *)fn_1131c(uVar3,auStack_1c,&uStack_18);
      *pbStack_10 = *pbStack_10 & 0xfe;
    }
    iVar4 = fn_1026b(*(int *)(pbVar2 + 0x20) - *piVar6,*(int *)(pbVar2 + 0x24) - piVar6[1],
                     *(int *)(pbVar2 + 0x28) - piVar6[2]);
    if (0x138800 < iVar4) {
      *piVar6 = *(int *)(pbVar2 + 0x20);
      piVar6[1] = *(int *)(pbVar2 + 0x24);
      iVar4 = *(int *)(pbVar2 + 0x28);
      piVar6[0xc] = 0;
      piVar6[0xd] = 0;
      piVar6[2] = iVar4;
      piVar6[0xe] = 0;
      fn_37715(piVar6 + 0xf,pbVar2 + 0x5c);
      pbVar8 = pbVar2 + 0x5c;
      pbVar7 = (byte *)(piVar6 + 3);
      for (uVar5 = extraout_ECX >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
        *(undefined4 *)pbVar7 = *(undefined4 *)pbVar8;
        pbVar8 = pbVar8 + 4;
        pbVar7 = pbVar7 + 4;
      }
      for (uVar5 = extraout_ECX & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
        *pbVar7 = *pbVar8;
        pbVar8 = pbVar8 + 1;
        pbVar7 = pbVar7 + 1;
      }
    }
    iVar4 = fn_12030c(*(int *)(pbVar2 + 0x20) - *piVar6,0x2000);
    piVar6[0xc] = piVar6[0xc] + iVar4;
    iVar4 = fn_12030c(*(int *)(pbVar2 + 0x24) - piVar6[1],0x2000);
    piVar6[0xd] = piVar6[0xd] + iVar4;
    iVar4 = fn_12030c(*(int *)(pbVar2 + 0x28) - piVar6[2],0x2000);
    piVar6[0xe] = piVar6[0xe] + iVar4;
    *piVar6 = *piVar6 + piVar6[0xc];
    piVar6[1] = piVar6[1] + piVar6[0xd];
    piVar6[2] = piVar6[2] + piVar6[0xe];
    iVar4 = fn_12030c(piVar6[0xc],0x6000);
    piVar6[0xc] = iVar4;
    iVar4 = fn_12030c(piVar6[0xd],0x6000);
    piVar6[0xd] = iVar4;
    iVar4 = fn_12030c(piVar6[0xe],0x6000);
    piVar6[0xe] = iVar4;
    pbStack_14 = pbVar2 + 0x5c;
    fn_37770(piVar6 + 0xf,pbStack_14,piVar6 + 3);
    *(int *)(pbVar2 + 0x20) = *piVar6;
    *(int *)(pbVar2 + 0x24) = piVar6[1];
    *(int *)(pbVar2 + 0x28) = piVar6[2];
    pbVar8 = (byte *)(piVar6 + 3);
    pbVar7 = pbStack_14;
    for (iVar4 = 9; iVar4 != 0; iVar4 = iVar4 + -1) {
      *(undefined4 *)pbVar7 = *(undefined4 *)pbVar8;
      pbVar8 = pbVar8 + 4;
      pbVar7 = pbVar7 + 4;
    }
    for (iVar4 = 0; iVar4 != 0; iVar4 = iVar4 + -1) {
      *pbVar7 = *pbVar8;
      pbVar8 = pbVar8 + 1;
      pbVar7 = pbVar7 + 1;
    }
    pbVar8 = pbStack_14;
    pbVar7 = PTR_DAT_000a86d4 + 0x1c;
    for (iVar4 = 9; iVar4 != 0; iVar4 = iVar4 + -1) {
      *(undefined4 *)pbVar7 = *(undefined4 *)pbVar8;
      pbVar8 = pbVar8 + 4;
      pbVar7 = pbVar7 + 4;
    }
    for (iVar4 = 0; iVar4 != 0; iVar4 = iVar4 + -1) {
      *pbVar7 = *pbVar8;
      pbVar8 = pbVar8 + 1;
      pbVar7 = pbVar7 + 1;
    }
    pbVar7 = pbVar2 + 0x20;
    pbVar8 = PTR_DAT_000a86d4;
    for (iVar4 = 3; pbVar8 = pbVar8 + 4, iVar4 != 0; iVar4 = iVar4 + -1) {
      *(undefined4 *)pbVar8 = *(undefined4 *)pbVar7;
      pbVar7 = pbVar7 + 4;
    }
    for (iVar4 = 0; puVar1 = PTR_DAT_000a86d4, iVar4 != 0; iVar4 = iVar4 + -1) {
      *pbVar8 = *pbVar7;
      pbVar7 = pbVar7 + 1;
      pbVar8 = pbVar8 + 1;
    }
    if (pbStack_10 != (byte *)0x0) {
      *(undefined4 *)(PTR_DAT_000a86d4 + 0x10) = *(undefined4 *)(pbStack_10 + 0x20);
      *(undefined4 *)(puVar1 + 0x14) = *(undefined4 *)(pbStack_10 + 0x24);
      uVar3 = *(undefined4 *)(pbStack_10 + 0x28);
      *(undefined4 *)(puVar1 + 0x18) = uVar3;
      fn_379f5(puVar1 + 0x1c,*(undefined4 *)(puVar1 + 4),*(undefined4 *)(puVar1 + 8),
               *(undefined4 *)(puVar1 + 0xc),*(undefined4 *)(puVar1 + 0x10),
               *(undefined4 *)(puVar1 + 0x14),uVar3);
    }
    DAT_000c0b80 = pbVar2;
    fn_37654();
  }
  else {
    *pbVar2 = *pbVar2 | 1;
    fn_1d7c8();
  }
  return;
}


// ==== camlir_PreInit @ 000380b4 body=78 ranges=1

void camlir_PreInit(void)

{
  byte bVar1;
  byte *pbVar2;
  undefined4 extraout_ECX;
  int iVar3;
  undefined4 extraout_EBX;
  undefined8 uVar4;
  
  uVar4 = __CHK();
  iVar3 = (int)((ulonglong)uVar4 >> 0x20);
  pbVar2 = (byte *)uVar4;
  wldpul_PreInit(pbVar2,iVar3 + 0x7c);
  *pbVar2 = *pbVar2 | 1;
  bVar1 = *(byte *)(iVar3 + 0xb4);
  *(byte *)(iVar3 + 0xb4) = bVar1 | 0x30;
  *(byte *)(iVar3 + 0xb4) = bVar1 & 0xf7 | 0x30;
  camlir_Display(pbVar2,iVar3,extraout_EBX,extraout_ECX);
  FUN_00022d7a();
  return;
}


// ==== camlir_RateLoop @ 00038102 body=10 ranges=1

void camlir_RateLoop(void)

{
  undefined4 extraout_ECX;
  undefined4 extraout_EDX;
  undefined4 extraout_EBX;
  undefined8 uVar1;
  
  uVar1 = __CHK();
  __CHK((int)uVar1,(int)((ulonglong)uVar1 >> 0x20) + 0x7c,extraout_EBX,extraout_ECX,4);
  fn_40108(extraout_EDX,1);
  return;
}


// ==== freecam_RateLoop @ 000381b0 body=25 ranges=1

void freecam_RateLoop(void)

{
  __CHK();
  fn_127a0(&PTR_DAT_000a64b8,1);
  return;
}


// ==== freecam_PreInit @ 000381c9 body=211 ranges=2

void freecam_PreInit(void)

{
  int *piVar1;
  undefined *puVar2;
  undefined *puVar3;
  undefined4 uVar4;
  int iVar5;
  int extraout_EDX;
  int iVar6;
  int iVar7;
  
  uVar4 = __CHK();
  fn_12701(uVar4,&PTR_DAT_000a64b8,0x11c,s_CAMRAM_000a25ed,1,s_CameraRAM_000a25e3);
  __CHK();
  iVar6 = *(int *)(DAT_000bf80c + 0x20);
  iVar7 = *(int *)(iVar6 + 8);
  while ((iVar6 != 0 && (iVar7 = *(int *)(iVar6 + 8), *(int *)(iVar7 + 0x10) == 0))) {
    iVar6 = *(int *)(iVar6 + 0x10);
  }
  if (iVar6 == 0) {
    iVar6 = *(int *)(DAT_000bf80c + 0x20);
  }
  dbg_Printf(iVar7 + 0x4c,iVar6);
  puVar2 = PTR_DAT_000a86d4;
  piVar1 = *(int **)(extraout_EDX + 4);
  iVar7 = *piVar1;
  iVar6 = *(int *)((int)piVar1 + iVar7 + 0x34) + *(int *)(extraout_EDX + 0x28);
  iVar5 = *(int *)((int)piVar1 + iVar7 + 0x2c) + *(int *)(extraout_EDX + 0x20);
  puVar3 = PTR_DAT_000a86d4 + 0x1c;
  iVar7 = *(int *)((int)piVar1 + iVar7 + 0x30) + *(int *)(extraout_EDX + 0x24);
  *(int *)(PTR_DAT_000a86d4 + 4) = iVar5 + -0x3e800;
  *(int *)(puVar2 + 8) = iVar7;
  *(int *)(puVar2 + 0xc) = iVar6;
  *(int *)(puVar2 + 0x10) = iVar5;
  *(int *)(puVar2 + 0x14) = iVar7;
  *(int *)(puVar2 + 0x18) = iVar6;
  fn_379f5(puVar3,*(undefined4 *)(puVar2 + 4),*(undefined4 *)(puVar2 + 8),
           *(undefined4 *)(puVar2 + 0xc),*(undefined4 *)(puVar2 + 0x10),
           *(undefined4 *)(puVar2 + 0x14),iVar6);
  FUN_0002a6cc();
  return;
}


// ==== freecam_Init @ 00038224 body=724 ranges=1

void freecam_Init(void)

{
  undefined *puVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 uVar4;
  int extraout_ECX;
  int extraout_ECX_00;
  int extraout_ECX_01;
  int extraout_ECX_02;
  int extraout_ECX_03;
  int extraout_ECX_04;
  int extraout_ECX_05;
  int extraout_ECX_06;
  int extraout_ECX_07;
  int iVar5;
  undefined4 extraout_EBX;
  undefined4 extraout_EBX_00;
  undefined4 extraout_EBX_01;
  undefined4 extraout_EBX_02;
  undefined4 extraout_EBX_03;
  undefined4 extraout_EBX_04;
  int iVar6;
  int iVar7;
  int iVar8;
  int iStack_14;
  int iStack_10;
  
  uVar2 = __CHK();
  if (9 < *(int *)(DAT_000bf80c + 0x30)) {
    iVar7 = 0;
    iVar6 = 0;
    iStack_14 = 0;
    iStack_10 = 0;
    iVar3 = 0;
    iVar8 = 0;
    if ((*(char *)(DAT_000bf988 + 0x1d) == '\x01') && (*(char *)(DAT_000bf988 + 0x7f) != '\x01')) {
      if (*(char *)(DAT_000bf988 + 0xcb) == '\x01') {
        iVar3 = -32000;
      }
      if (*(char *)(DAT_000bf988 + 0xcd) == '\x01') {
        iVar3 = iVar3 + 32000;
      }
      if (*(char *)(DAT_000bf988 + 200) == '\x01') {
        iStack_10 = -32000;
      }
      if (*(char *)(DAT_000bf988 + 0xd0) == '\x01') {
        iStack_10 = iStack_10 + 32000;
      }
    }
    if (*(char *)(DAT_000bf988 + 0x1d) != '\x01') {
      if (*(char *)(DAT_000bf988 + 200) == '\x01') {
        iVar8 = 32000;
      }
      if (*(char *)(DAT_000bf988 + 0xd0) == '\x01') {
        iVar8 = iVar8 + -32000;
      }
      if (*(char *)(DAT_000bf988 + 0xcb) == '\x01') {
        iVar7 = 0x147;
      }
      if (*(char *)(DAT_000bf988 + 0xcd) == '\x01') {
        iVar7 = iVar7 + -0x147;
      }
    }
    if (*(char *)(DAT_000bf988 + 0x7f) == '\x01') {
      if (*(char *)(DAT_000bf988 + 200) == '\x01') {
        iStack_14 = 0x147;
      }
      if (*(char *)(DAT_000bf988 + 0xd0) == '\x01') {
        iStack_14 = iStack_14 + -0x147;
      }
      if (*(char *)(DAT_000bf988 + 0xcb) == '\x01') {
        iVar6 = 0x147;
      }
      if (*(char *)(DAT_000bf988 + 0xcd) == '\x01') {
        iVar6 = iVar6 + -0x147;
      }
    }
    uVar4 = fn_12030c(iVar3,(int)PTR_FUN_000a648c << 2);
    iVar3 = fn_120305(uVar4,*(undefined4 *)(PTR_DAT_000a86d4 + 0x34),uVar4);
    *(int *)(extraout_ECX + 4) = *(int *)(extraout_ECX + 4) + iVar3;
    iVar3 = fn_120305(extraout_EBX,*(undefined4 *)(PTR_DAT_000a86d4 + 0x38));
    *(int *)(extraout_ECX_00 + 8) = *(int *)(extraout_ECX_00 + 8) + iVar3;
    iVar3 = fn_120305(extraout_EBX_00,*(undefined4 *)(PTR_DAT_000a86d4 + 0x3c));
    iVar5 = (int)PTR_FUN_000a648c << 2;
    *(int *)(extraout_ECX_01 + 0xc) = *(int *)(extraout_ECX_01 + 0xc) + iVar3;
    uVar4 = fn_12030c(iStack_10,iVar5);
    iVar3 = fn_120305(uVar4,*(undefined4 *)(PTR_DAT_000a86d4 + 0x28),uVar4);
    *(int *)(extraout_ECX_02 + 4) = *(int *)(extraout_ECX_02 + 4) + iVar3;
    iVar3 = fn_120305(extraout_EBX_01,*(undefined4 *)(PTR_DAT_000a86d4 + 0x2c));
    *(int *)(extraout_ECX_03 + 8) = *(int *)(extraout_ECX_03 + 8) + iVar3;
    iVar3 = fn_120305(extraout_EBX_02,*(undefined4 *)(PTR_DAT_000a86d4 + 0x30));
    iVar5 = (int)PTR_FUN_000a648c << 2;
    *(int *)(extraout_ECX_04 + 0xc) = *(int *)(extraout_ECX_04 + 0xc) + iVar3;
    uVar4 = fn_12030c(iVar8,iVar5);
    iVar3 = fn_120305(uVar4,*(undefined4 *)(PTR_DAT_000a86d4 + 0x1c),uVar4);
    *(int *)(extraout_ECX_05 + 4) = *(int *)(extraout_ECX_05 + 4) + iVar3;
    iVar3 = fn_120305(extraout_EBX_03,*(undefined4 *)(PTR_DAT_000a86d4 + 0x20));
    *(int *)(extraout_ECX_06 + 8) = *(int *)(extraout_ECX_06 + 8) + iVar3;
    iVar3 = fn_120305(extraout_EBX_04,*(undefined4 *)(PTR_DAT_000a86d4 + 0x24));
    puVar1 = PTR_DAT_000a86d4;
    *(int *)(extraout_ECX_07 + 0xc) = *(int *)(extraout_ECX_07 + 0xc) + iVar3;
    fn_109e5(puVar1 + 0x1c,iStack_14,iVar7,iVar6);
    *(undefined4 *)PTR_DAT_000a86d4 = 0;
    DAT_000c0b80 = uVar2;
    fn_37654();
  }
  return;
}


// ==== moincam_PreInit @ 00039615 body=175 ranges=1

void moincam_PreInit(void)

{
  int extraout_EDX;
  
  __CHK();
  fn_1275e(&PTR_DAT_000a64b8,0x11c,s_CAMRAM_000a25fe,1,s_CameraRAM_000a25f4);
  fn_1275e(&DAT_000c6138,0x78,s_MOVERRAM_000a260e,1,s_CamMover_000a2605);
  *(undefined4 *)(extraout_EDX + 0x44) = 0x2ee00;
  *(undefined **)(extraout_EDX + 0x48) = &DAT_0009c400;
  *(undefined4 *)(extraout_EDX + 0x4c) = 0xffffce00;
  *(undefined4 *)(extraout_EDX + 0x50) = 0x3e800;
  *(undefined4 *)(extraout_EDX + 0x54) = 0x8000;
  *(undefined4 *)(extraout_EDX + 100) = 0x2ee00;
  *(undefined4 *)(extraout_EDX + 0x68) = 0;
  *(undefined4 *)(extraout_EDX + 0x6c) = 0xffff8300;
  *(undefined **)(extraout_EDX + 0x70) = &DAT_0007d000;
  *(undefined4 *)(extraout_EDX + 0x74) = 0x8000;
  *(undefined1 *)(extraout_EDX + 0xc) = 4;
  *(undefined4 *)(PTR_DAT_000a64b8 + 0xec) = 0;
  fn_1c283();
  fn_2163d();
  return;
}


// ==== moincam_RateLoop @ 000396c4 body=45 ranges=1

void moincam_RateLoop(void)

{
  __CHK();
  fn_216be();
  fn_127a0(&PTR_DAT_000a64b8,1);
  fn_127a0(&DAT_000c6138,1);
  return;
}


// ==== moincam_Display @ 000396f1 body=846 ranges=1

void moincam_Display(void)

{
  byte bVar1;
  undefined *puVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  byte *pbVar6;
  int iVar7;
  int extraout_ECX;
  undefined4 extraout_ECX_00;
  undefined4 extraout_ECX_01;
  undefined4 extraout_ECX_02;
  undefined4 extraout_ECX_03;
  int extraout_ECX_04;
  undefined4 extraout_ECX_05;
  int extraout_EDX;
  int extraout_EBX;
  int extraout_EBX_00;
  int extraout_EBX_01;
  undefined1 auStack_34 [4];
  undefined4 uStack_30;
  int iStack_2c;
  int iStack_28;
  int iStack_24;
  int iStack_20;
  int iStack_1c;
  int iStack_18;
  undefined4 uStack_14;
  int iStack_10;
  
  iVar3 = __CHK();
  DAT_000c0b80 = iVar3;
  *(undefined4 *)PTR_DAT_000a86d4 = 0;
  fn_3810c();
  iVar7 = *(int *)(*(int *)(iVar3 + 0x98) + 0x24);
  iVar4 = fn_11361(*(undefined4 *)(extraout_EDX + 0x88));
  uVar5 = 0;
  if (iVar4 != 0) {
    uVar5 = *(undefined4 *)(iVar4 + 8);
  }
  *(undefined4 *)(iVar7 + 0x74) = uVar5;
  uVar5 = fn_112f4(*(undefined4 *)(extraout_ECX + 0x84),auStack_34);
  uStack_30 = 0;
  pbVar6 = (byte *)fn_1131c(uVar5,auStack_34,&uStack_30);
  if (*(int *)(iVar7 + 0x74) == 0) {
    fn_39499(pbVar6,extraout_ECX_00);
  }
  else {
    if (DAT_000c0cd4 == 1) {
      DAT_000c0c10 = DAT_000c0c10 + 1;
      iStack_10 = DAT_000c0c10 * 0x800;
      *pbVar6 = *pbVar6 & 0xfe;
      if (iStack_10 < 0x10000) {
        fn_3f6d7();
        fn_39499(pbVar6,extraout_ECX_01);
        fn_3860d(iVar3,extraout_ECX_02,extraout_ECX_02);
        fn_389d6(pbVar6,iVar3);
        iStack_2c = *(int *)(PTR_DAT_000a86d4 + 0x10) - *(int *)(PTR_DAT_000a86d4 + 4);
        iStack_28 = *(int *)(PTR_DAT_000a86d4 + 0x14) - *(int *)(PTR_DAT_000a86d4 + 8);
        iStack_24 = *(int *)(PTR_DAT_000a86d4 + 0x18) - *(int *)(PTR_DAT_000a86d4 + 0xc);
        fn_386b5();
        iStack_20 = *(int *)(PTR_DAT_000a86d4 + 0x10) - *(int *)(PTR_DAT_000a86d4 + 4);
        iStack_1c = *(int *)(PTR_DAT_000a86d4 + 0x14) - *(int *)(PTR_DAT_000a86d4 + 8);
        iStack_18 = *(int *)(PTR_DAT_000a86d4 + 0x18) - *(int *)(PTR_DAT_000a86d4 + 0xc);
        fn_102da(&iStack_2c,&iStack_28,&iStack_24,0x10000);
        fn_102da(&iStack_20,&iStack_1c,&iStack_18,0x10000);
        iVar4 = fn_12030c(iStack_20 - iStack_2c,iStack_10);
        *(int *)(PTR_DAT_000a86d4 + 0x10) = iStack_2c + iVar4 + *(int *)(PTR_DAT_000a86d4 + 4);
        iVar4 = fn_12030c(iStack_1c - iStack_28,iStack_10);
        *(int *)(PTR_DAT_000a86d4 + 0x14) = iVar4 + iStack_28 + *(int *)(PTR_DAT_000a86d4 + 8);
        iVar4 = fn_12030c(iStack_18 - iStack_24,iStack_10);
        puVar2 = PTR_DAT_000a86d4;
        bVar1 = *PTR_DAT_000a86d4;
        *(int *)(PTR_DAT_000a86d4 + 0x18) = iVar4 + iStack_24 + *(int *)(PTR_DAT_000a86d4 + 0xc);
        *puVar2 = bVar1 & 0xfc;
        iVar4 = *(int *)(pbVar6 + 0x98);
        DAT_000c0c14 = fn_100d3(*(undefined4 *)(iVar4 + 0x18),*(undefined4 *)(iVar4 + 0x1c),
                                *(undefined4 *)(iVar4 + 0x20),*(int *)(iVar7 + 0x20) >> 0xc,
                                *(int *)(iVar7 + 0x24) >> 0xc,*(int *)(iVar7 + 0x28) >> 0xc);
      }
      else {
        fn_12030c(*(undefined4 *)(PTR_DAT_000a64b8 + 0xdc),64000);
        fn_3860d(iVar3,extraout_ECX_03);
        DAT_000c0c14 = fn_12030c(DAT_000c0c14,0xfe00);
        uStack_14 = *(undefined4 *)(iVar7 + 0x20);
        iVar4 = fn_120305(uStack_14,DAT_000c0c14,PTR_DAT_000a86d4);
        *(int *)(extraout_EBX + 4) = *(int *)(extraout_EBX + 4) + iVar4;
        iVar4 = fn_120305(*(undefined4 *)(iVar7 + 0x24),DAT_000c0c14,PTR_DAT_000a86d4);
        *(int *)(extraout_EBX_00 + 8) = *(int *)(extraout_EBX_00 + 8) + iVar4;
        iVar7 = fn_120305(*(undefined4 *)(iVar7 + 0x28),DAT_000c0c14,PTR_DAT_000a86d4);
        *(int *)(extraout_EBX_01 + 0xc) = *(int *)(extraout_EBX_01 + 0xc) + iVar7;
        puVar2 = PTR_DAT_000a86d4;
        *(undefined4 *)(extraout_ECX_04 + 0x14) = *(undefined4 *)(PTR_DAT_000a86d4 + 4);
        *(undefined4 *)(extraout_ECX_04 + 0x18) = *(undefined4 *)(puVar2 + 8);
        *(undefined4 *)(extraout_ECX_04 + 0x1c) = *(undefined4 *)(puVar2 + 0xc);
        fn_389d6(pbVar6,iVar3,extraout_ECX_04);
        fn_386b5();
        *PTR_DAT_000a86d4 = *PTR_DAT_000a86d4 & 0xfc;
      }
      if ((*PTR_DAT_000a86d4 & 1) == 0) {
        fn_379f5(PTR_DAT_000a86d4 + 0x1c,*(undefined4 *)(PTR_DAT_000a86d4 + 4),
                 *(undefined4 *)(PTR_DAT_000a86d4 + 8),*(undefined4 *)(PTR_DAT_000a86d4 + 0xc),
                 *(undefined4 *)(PTR_DAT_000a86d4 + 0x10),*(undefined4 *)(PTR_DAT_000a86d4 + 0x14),
                 *(undefined4 *)(PTR_DAT_000a86d4 + 0x18));
      }
    }
    else {
      DAT_000c0c10 = 0;
      fn_39499(pbVar6,extraout_ECX_00);
      fn_3860d(iVar3,extraout_ECX_05,extraout_ECX_05);
      fn_389d6(pbVar6,iVar3);
      fn_39261(pbVar6);
      if ((*PTR_DAT_000a86d4 & 1) == 0) {
        fn_379f5(PTR_DAT_000a86d4 + 0x1c,*(undefined4 *)(PTR_DAT_000a86d4 + 4),
                 *(undefined4 *)(PTR_DAT_000a86d4 + 8),*(undefined4 *)(PTR_DAT_000a86d4 + 0xc),
                 *(undefined4 *)(PTR_DAT_000a86d4 + 0x10),*(undefined4 *)(PTR_DAT_000a86d4 + 0x14),
                 *(undefined4 *)(PTR_DAT_000a86d4 + 0x18));
      }
    }
    fn_37654(iVar3);
  }
  return;
}


// ==== spdcon_PreInit @ 00039a3f body=61 ranges=1

void spdcon_PreInit(void)

{
  byte *pbVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  
  iVar4 = __CHK();
  iVar2 = *(int *)(iVar4 + 8);
  iVar5 = 0;
  for (iVar6 = 0; iVar6 != *(int *)(iVar2 + 0x10); iVar6 = iVar6 + 1) {
    iVar3 = *(int *)(iVar5 + *(int *)(iVar4 + 0xc));
    if (*(char *)(*(int *)(iVar3 + 0x14) + 0x58) != '[') {
      pbVar1 = (byte *)(iVar3 + 0xc);
      *pbVar1 = *pbVar1 | 4;
    }
    iVar5 = iVar5 + 4;
  }
  return;
}


// ==== spdcon_Visible @ 00039a7c body=368 ranges=1

void spdcon_Visible(void)

{
  undefined1 *puVar1;
  undefined4 uVar2;
  int extraout_ECX;
  undefined4 extraout_ECX_00;
  int iVar3;
  int iVar4;
  int iVar5;
  undefined8 uVar6;
  int iStack_2c;
  int iStack_28;
  int iStack_24;
  undefined1 auStack_20 [4];
  undefined1 auStack_1c [4];
  undefined1 auStack_18 [4];
  undefined4 uStack_14;
  int iStack_10;
  
  uVar6 = __CHK();
  iVar3 = (int)((ulonglong)uVar6 >> 0x20);
  iVar5 = (int)uVar6;
  puVar1 = *(undefined1 **)(*(int *)(*(int *)(*(int *)(iVar5 + 0x98) + 0x28) + 0x10) + 0x98);
  iStack_10 = *(int *)(*(int *)(puVar1 + 0x28) + 0xc);
  if (*(char *)(*(int *)(iStack_10 + 0x14) + 0x58) == '[') {
    fn_3b42b(0x23d,0x73,0);
    *(undefined2 *)(iVar3 + 0xc) = 1;
    uVar2 = fn_112f4(*(undefined4 *)(iVar3 + 0x10),auStack_18);
    uStack_14 = 0;
    while( true ) {
      iVar5 = fn_1131c(uVar2,auStack_18,&uStack_14);
      if (iVar5 == 0) break;
      uVar2 = extraout_ECX_00;
      if (iVar5 != iVar3) {
        *(undefined2 *)(iVar5 + 0xe) = 1;
      }
    }
  }
  else {
    fn_3b42b(0x21c,0x78,0);
    iVar4 = *(int *)(extraout_ECX + 0x48) >> 0x10;
    if (iVar4 != *(int *)(*(int *)(iVar5 + 8) + 0x10) + -1) {
      iVar4 = *(int *)(*(int *)(*(int *)(iVar5 + 0xc) + 4 + iVar4 * 4) + 0x14);
      *puVar1 = 1;
      iStack_2c = *(int *)(iVar4 + 0x2c);
      iStack_28 = *(int *)(iVar4 + 0x30);
      iStack_24 = *(int *)(iVar4 + 0x34);
      fn_1075d(iVar5,&iStack_2c,&iStack_28,&iStack_24);
      iVar5 = iStack_2c - *(int *)(puVar1 + 4) >> 0x1f;
      *(int *)(puVar1 + 0x18) =
           *(int *)(puVar1 + 0x18) +
           ((int)(((iStack_2c - *(int *)(puVar1 + 4)) + iVar5 * -8) - (uint)(iVar5 << 2 < 0)) >> 3);
      iVar5 = iStack_28 - *(int *)(puVar1 + 8) >> 0x1f;
      *(int *)(puVar1 + 0x1c) =
           *(int *)(puVar1 + 0x1c) +
           ((int)(((iStack_28 - *(int *)(puVar1 + 8)) + iVar5 * -8) - (uint)(iVar5 << 2 < 0)) >> 3);
      iVar5 = iStack_24 - *(int *)(puVar1 + 0xc) >> 0x1f;
      iVar5 = *(int *)(puVar1 + 0x20) +
              ((int)(((iStack_24 - *(int *)(puVar1 + 0xc)) + iVar5 * -8) - (uint)(iVar5 << 2 < 0))
              >> 3);
      *(int *)(puVar1 + 0x20) = iVar5;
      uVar2 = fn_112f4(7,auStack_20,iVar5,0);
      iVar5 = fn_1131c(uVar2,auStack_20,auStack_1c);
      *(int *)(iVar5 + 0x2c) = *(int *)(iVar5 + 0x2c) + 5;
      *(undefined4 *)(iVar5 + 0x30) = *(undefined4 *)(iVar3 + 0x14);
    }
  }
  *(byte *)(iStack_10 + 0xc) = *(byte *)(iStack_10 + 0xc) | 4;
  return;
}


// ==== stpcon_Visible @ 00039bec body=55 ranges=1

void stpcon_Visible(void)

{
  byte *pbVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  undefined1 *extraout_ECX;
  undefined4 extraout_ECX_00;
  int iVar5;
  undefined8 uVar6;
  undefined1 auStack_20 [4];
  undefined1 auStack_1c [4];
  undefined1 auStack_18 [4];
  undefined4 uStack_14;
  int iStack_10;
  
  uVar6 = __CHK();
  iVar5 = (int)((ulonglong)uVar6 >> 0x20);
  iVar2 = *(int *)(*(int *)(*(int *)(*(int *)(*(int *)(*(int *)((int)uVar6 + 0x98) + 0x28) + 0x10) +
                                    0x98) + 0x28) + 0xc);
  iStack_10 = *(int *)(iVar2 + 0x14);
  if (*(char *)(iStack_10 + 0x58) == '[') {
    fn_3b42b(0x24a,0x73,0);
    *(undefined2 *)(iVar5 + 0xc) = 1;
    uVar3 = fn_112f4(*(undefined4 *)(iVar5 + 0x10),auStack_18);
    uStack_14 = 0;
    while( true ) {
      iVar4 = fn_1131c(uVar3,auStack_18,&uStack_14);
      if (iVar4 == 0) break;
      uVar3 = extraout_ECX_00;
      if (iVar4 != iVar5) {
        *(undefined2 *)(iVar4 + 0xe) = 1;
      }
    }
  }
  else {
    fn_3b42b(0x2f,0x78,0);
    if (*(int *)(iStack_10 + 0x48) >> 0x10 != *(int *)(*(int *)((int)uVar6 + 8) + 0x10) + -1) {
      *extraout_ECX = 1;
      *(undefined4 *)(extraout_ECX + 0x18) = 0;
      *(undefined4 *)(extraout_ECX + 0x1c) = 0;
      *(undefined4 *)(extraout_ECX + 0x20) = 0;
      uVar3 = fn_112f4(7,auStack_20,0);
      fn_1131c(uVar3,auStack_20,auStack_1c);
    }
  }
  pbVar1 = (byte *)(iVar2 + 0xc);
  *pbVar1 = *pbVar1 | 4;
  return;
}


// ==== spdcon_Init @ 00039cd9 body=302 ranges=1

void spdcon_Init(void)

{
  byte *pbVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 extraout_ECX;
  int *piVar4;
  int iVar5;
  int iVar6;
  undefined8 uVar7;
  undefined1 auStack_14 [4];
  undefined4 uStack_10;
  
  uVar7 = __CHK();
  piVar4 = (int *)((ulonglong)uVar7 >> 0x20);
  pbVar1 = (byte *)uVar7;
  if (piVar4[3] >> 0x10 != 0) {
    *pbVar1 = *pbVar1 | 1;
    FUN_00022d7a();
    return;
  }
  *pbVar1 = *pbVar1 & 0xfe;
  if (*(int *)((int)piVar4 + 10) >> 0x10 != 0) {
    iVar2 = *(int *)(pbVar1 + 8);
    if (piVar4[2] == *(int *)(iVar2 + 0x10)) {
      piVar4[2] = 0;
      if (*(int *)((int)piVar4 + 10) >> 0x10 != 1) {
        *(undefined2 *)(piVar4 + 3) = 0;
        iVar6 = 0;
        iVar5 = 0;
        do {
          if (iVar6 == *(int *)(iVar2 + 0x10)) {
LAB_00039d74:
            uVar3 = fn_112f4(piVar4[4],auStack_14);
            uStack_10 = 0;
            while (iVar2 = fn_1131c(uVar3,auStack_14,&uStack_10), iVar2 != 0) {
              *(undefined2 *)(iVar2 + 0xe) = 0;
              uVar3 = extraout_ECX;
            }
            return;
          }
          if (*(char *)(*(int *)(*(int *)(iVar5 + *(int *)(pbVar1 + 0xc)) + 0x14) + 0x58) == '[') {
            pbVar1 = (byte *)(*(int *)(iVar5 + *(int *)(pbVar1 + 0xc)) + 0xc);
            *pbVar1 = *pbVar1 & 0xfb;
            goto LAB_00039d74;
          }
          iVar5 = iVar5 + 4;
          iVar6 = iVar6 + 1;
        } while( true );
      }
      iVar2 = *piVar4;
      *(undefined2 *)(piVar4 + 3) = 0xffff;
      if (iVar2 == 0) {
        iVar2 = 200;
      }
      piVar4[1] = iVar2 << 0x10;
    }
    iVar2 = piVar4[1] - (int)PTR_FUN_000a648c;
    piVar4[1] = iVar2;
    if (iVar2 < 1) {
      iVar2 = *(int *)(*(int *)(pbVar1 + 0xc) + piVar4[2] * 4);
      piVar4[1] = 0;
      piVar4[2] = piVar4[2] + 1;
      if (*(char *)(*(int *)(iVar2 + 0x14) + 0x58) != '[') {
        if (*(int *)((int)piVar4 + 10) >> 0x10 != 1) {
          *(byte *)(iVar2 + 0xc) = *(byte *)(iVar2 + 0xc) | 4;
          FUN_00022d7a();
          return;
        }
        *(byte *)(iVar2 + 0xc) = *(byte *)(iVar2 + 0xc) & 0xfb;
        FUN_00022d7a();
        return;
      }
    }
  }
  return;
}


// ==== setvisi_Init @ 00039e07 body=72 ranges=1

void setvisi_Init(void)

{
  byte *pbVar1;
  int iVar2;
  
  pbVar1 = (byte *)__CHK();
  if (DAT_000a86d8 == -1) {
    iVar2 = stricmp(*(int *)(pbVar1 + 8) + 0x4c,s_pokal_000a2623);
    if (iVar2 == 0) {
LAB_00039e4a:
      *pbVar1 = *pbVar1 & 0xfe;
      return;
    }
  }
  else {
    iVar2 = stricmp(*(int *)(pbVar1 + 8) + 0x4c,s_pokal_000a2629);
    if (iVar2 != 0) goto LAB_00039e4a;
  }
  *pbVar1 = *pbVar1 | 1;
  return;
}


// ==== setvisi_PreInit @ 00039e4f body=46 ranges=1

void setvisi_PreInit(void)

{
  __CHK();
  DAT_000a86d8 = (DAT_000a6630 == 0) - 1;
  setvisi_Init();
  return;
}


// ==== setvisi_RateLoop @ 00039e7d body=11 ranges=1

void setvisi_RateLoop(void)

{
  __CHK();
  return;
}


// ==== endseqcam_Init @ 00039f8f body=728 ranges=1

void endseqcam_Init(void)

{
  undefined *puVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int extraout_EDX;
  int *piVar5;
  int *piVar6;
  int aiStack_30 [6];
  int iStack_18;
  int iStack_14;
  int iStack_10;
  
  iVar2 = __CHK();
  piVar5 = &DAT_000a86ec;
  piVar6 = aiStack_30;
  for (iVar4 = 6; iVar4 != 0; iVar4 = iVar4 + -1) {
    *piVar6 = *piVar5;
    piVar5 = piVar5 + 1;
    piVar6 = piVar6 + 1;
  }
  if (DAT_000a86d8 == -1) {
    if ((DAT_000a65f4 == 0) && (*(char *)(DAT_000bf988 + 0x1c) == '\x01')) {
      DAT_000a86d8 = *(int *)(DAT_000bf80c + 0x130);
    }
    if (DAT_000a86d8 == -1) {
      s_extram_18f02(aiStack_30,DAT_000c0c18 + 0xe4,0,1,0x7fff,0x10880,0,0,0);
    }
  }
  iVar4 = wl3_FindChunk(*(undefined4 *)(iVar2 + 0xb4),0x4d504f53);
  iStack_18 = 0;
  iStack_10 = 0x2ee00;
  iStack_14 = 0;
  iVar2 = iVar4 + 8;
  iVar3 = fn_10170(0xc000 - (*(int *)(DAT_000bf80c + 0x130) * 5) / 2);
  iVar3 = fn_12030c((iVar3 >> 0xf) + 0x8000,0x1f400);
  iStack_14 = iStack_14 - iVar3;
  iVar3 = fn_10170(0xc000 - (*(int *)(DAT_000bf80c + 0x130) << 2) / 2);
  iVar3 = fn_12030c((iVar3 >> 0xf) + 0x8000,0x2ee00);
  iVar3 = iVar3 + 0x1f400;
  if (DAT_000a86d8 == -1) {
    iVar2 = iVar4 + 0x3c;
    iStack_10 = 0x2ee00;
    iStack_14 = -0x1f400;
    iVar3 = 0x3e800;
  }
  fn_102da(&iStack_18,&iStack_14,&iStack_10,iVar3);
  fn_10483(0,*(int *)(DAT_000bf80c + 0x130) * 2,0,&iStack_18,&iStack_14,&iStack_10);
  puVar1 = PTR_DAT_000a86d4;
  *(undefined4 *)(PTR_DAT_000a86d4 + 0x10) = *(undefined4 *)(iVar2 + 4);
  *(undefined4 *)(puVar1 + 0x14) = *(undefined4 *)(iVar2 + 8);
  *(undefined4 *)(puVar1 + 0x18) = *(undefined4 *)(iVar2 + 0xc);
  *(int *)(puVar1 + 4) = iStack_18 + *(int *)(iVar2 + 4);
  *(int *)(puVar1 + 8) = iStack_14 + *(int *)(iVar2 + 8);
  *(int *)(puVar1 + 0xc) = iStack_10 + *(int *)(iVar2 + 0xc);
  if (DAT_000a86d8 != -1) {
    iVar2 = (*(int *)(DAT_000bf80c + 0x130) + -3000) - DAT_000a86d8;
    if (iVar2 < 0) {
      iVar2 = 0;
    }
    else if ((DAT_000a65f4 == 0) && (*(char *)(DAT_000bf988 + 0x1c) == '\x01')) {
      DAT_000a65f4 = -1;
      fn_17b0b();
      fn_17875(s_title_000a2632);
      iVar2 = extraout_EDX;
    }
    aiStack_30[1] = iVar2 * -4 + 0x8000;
    s_extram_18f02(aiStack_30,DAT_000c0c18 + 0x150,0,1,0x7fff,0x10880,0,0,0);
    s_extram_18f02(aiStack_30,DAT_000c0c18 + 0x150,1,1,0x7fff,0x10880,0,0,0);
  }
  *PTR_DAT_000a86d4 = *PTR_DAT_000a86d4 | 0x20;
  if (*(char *)(DAT_000bf988 + 0x1c) != '\x01') {
    DAT_000a65f4 = 0;
  }
  fn_379f5(PTR_DAT_000a86d4 + 0x1c,*(undefined4 *)(PTR_DAT_000a86d4 + 4),
           *(undefined4 *)(PTR_DAT_000a86d4 + 8),*(undefined4 *)(PTR_DAT_000a86d4 + 0xc),
           *(undefined4 *)(PTR_DAT_000a86d4 + 0x10),*(undefined4 *)(PTR_DAT_000a86d4 + 0x14),
           *(undefined4 *)(PTR_DAT_000a86d4 + 0x18));
  fn_37654(DAT_000c0b80);
  FUN_00021c79();
  return;
}


// ==== endseqcam_RateLoop @ 0003a267 body=35 ranges=1

void endseqcam_RateLoop(void)

{
  __CHK();
  fn_40108(&DAT_000c0b80,1);
  fn_216be();
  fn_127a0(&PTR_DAT_000a64b8,1);
  return;
}


// ==== endseqcam_PreInit @ 0003a28a body=103 ranges=1

void endseqcam_PreInit(void)

{
  undefined4 uVar1;
  
  uVar1 = __CHK();
  DAT_000a86d8 = 0;
  s_total_alloc_d_d_3ff1c(&DAT_000c0b80,0xbc,s_CAMWORLD_000a2638,1);
  fn_12701(uVar1,&PTR_DAT_000a64b8,0x11c,s_CAMRAM_000a264b,1,s_CameraRAM_000a2641);
  *PTR_DAT_000a86d4 = *PTR_DAT_000a86d4 & 0xfc;
  *(undefined4 *)(PTR_DAT_000a64b8 + 0xec) = 0;
  fn_1c283();
  fn_2163d();
  return;
}


