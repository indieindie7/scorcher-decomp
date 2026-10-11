// ==== FUN_0001eee0 @ 0001eee0 body=423 ranges=1

void __stdcall FUN_0001eee0(void)

{
  ushort uVar1;
  uint uVar2;
  uint *extraout_ECX;
  uint *extraout_ECX_00;
  uint *extraout_ECX_01;
  uint *extraout_ECX_02;
  uint *extraout_ECX_03;
  uint *extraout_ECX_04;
  uint *extraout_ECX_05;
  uint *extraout_ECX_06;
  uint *extraout_ECX_07;
  uint *puVar3;
  int in_ECX;
  int extraout_ECX_08;
  int extraout_ECX_09;
  uint *extraout_ECX_10;
  uint *extraout_ECX_11;
  ushort *extraout_EDX;
  uint *puVar4;
  int iVar5;
  uint uVar6;
  int extraout_EBX;
  int extraout_EBX_00;
  int *unaff_EBP;
  int unaff_EDI;
  bool bVar7;
  
  do {
    if (unaff_EDI == *unaff_EBP >> 0x10) {
      fn_15511();
      return;
    }
    FUN_000408b5(in_ECX);
    FUN_00040886(extraout_ECX_08 + 6);
    FUN_00040886(extraout_ECX_09 + 4);
    puVar4 = extraout_ECX_10 + 2;
    uVar2 = extraout_ECX_10[1];
    uVar1 = *(ushort *)((int)extraout_ECX_10 + 6);
    uVar6 = *extraout_ECX_10;
    if (uVar6 < 0x4e414d45) {
      if (uVar6 < 0x47505452) {
        if (0x414c4f42 < uVar6) {
          if (uVar6 < 0x414c4f44) {
            FUN_0001ed78(puVar4);
            puVar3 = extraout_ECX_06;
            goto LAB_0001eed9;
          }
          if (uVar6 == 0x414e494d) goto LAB_0001eed4;
        }
LAB_0001ef6a:
        FUN_0003f539();
        FUN_0003faa9();
        puVar3 = extraout_ECX_11;
      }
      else if (uVar6 < 0x47505453) {
        FUN_0001ed2f(puVar4);
        puVar3 = extraout_ECX_04;
      }
      else if (uVar6 < 0x4d415458) {
        if (uVar6 != 0x4c4f4144) goto LAB_0001ef6a;
        FUN_0001ec1c(puVar4);
        puVar3 = extraout_ECX;
      }
      else {
        if (0x4d415458 < uVar6) {
          bVar7 = uVar6 == 0x4d504f53;
          goto LAB_0001ef68;
        }
        iVar5 = 9;
LAB_0001ee8c:
        FUN_0001ee15(puVar4,iVar5);
        puVar3 = extraout_ECX_02;
      }
    }
    else if (uVar6 < 0x4e414d46) {
      FUN_000408b5(extraout_ECX_10 + 6);
      puVar4 = (uint *)(extraout_EBX_00 + 0x14);
LAB_0001eed4:
      FUN_000408b5(puVar4);
      puVar3 = extraout_ECX_07;
    }
    else {
      if (uVar6 < 0x52524f55) {
        if (0x50505451 < uVar6) {
          if (uVar6 < 0x50505453) {
            FUN_0001edcd(puVar4);
            puVar3 = extraout_ECX_05;
            goto LAB_0001eed9;
          }
          if (uVar6 == 0x524f5554) goto LAB_0001ee95;
        }
        goto LAB_0001ef6a;
      }
      if (uVar6 < 0x52524f56) {
LAB_0001ee95:
        uVar6 = 0;
        puVar3 = extraout_ECX_10;
        while (uVar6 != uVar1) {
          FUN_0001ee3b(puVar4);
          puVar4 = (uint *)((int)extraout_EDX + (uint)*extraout_EDX);
          puVar3 = extraout_ECX_03;
          uVar6 = extraout_EBX + 1;
        }
      }
      else {
        if (uVar6 < 0x5354524e) {
          bVar7 = uVar6 == 0x534d4154;
LAB_0001ef68:
          if (bVar7) {
            iVar5 = (uint)uVar1 * 0xd;
            goto LAB_0001ee8c;
          }
          goto LAB_0001ef6a;
        }
        if (uVar6 < 0x5354524f) {
          FUN_0001ec82(puVar4);
          puVar3 = extraout_ECX_01;
        }
        else {
          if (uVar6 != 0x56414c53) goto LAB_0001ef6a;
          FUN_0001ecda(puVar4);
          puVar3 = extraout_ECX_00;
        }
      }
    }
LAB_0001eed9:
    unaff_EDI = unaff_EDI + 1;
    in_ECX = (int)puVar3 + (uint)(ushort)uVar2;
  } while( true );
}


// ==== FUN_0001c8d3 @ 0001c8d3 body=104 ranges=1

void __stdcall FUN_0001c8d3(undefined4 param_1)

{
  int iVar1;
  undefined4 extraout_ECX;
  undefined4 uVar2;
  undefined4 extraout_EBX;
  undefined4 unaff_EBP;
  undefined4 unaff_ESI;
  undefined4 unaff_EDI;
  undefined8 uVar3;
  undefined4 uVar4;
  
  uVar3 = __CHK();
  uVar2 = (undefined4)((ulonglong)uVar3 >> 0x20);
  uVar4 = extraout_EBX;
  iVar1 = FUN_0001ccb0((int)uVar3,0x56414c53);
  if (iVar1 != 0) {
    FUN_0001c7c7(uVar2,uVar4,extraout_ECX,param_1,iVar1 + 8,uVar4,unaff_EBP,unaff_EDI,unaff_ESI);
  }
  iVar1 = FUN_0001ccb0((int)uVar3,0x5354524e);
  if (iVar1 != 0) {
    FUN_0001c854(uVar2,uVar4,extraout_ECX,param_1,iVar1 + 8);
    FUN_00010754();
    return;
  }
  return;
}


// ==== FUN_0001c93b @ 0001c93b body=285 ranges=1

void __stdcall FUN_0001c93b(undefined4 param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 extraout_ECX;
  int extraout_ECX_00;
  uint uVar3;
  undefined4 uVar4;
  undefined4 extraout_EBX;
  uint *puVar5;
  undefined8 uVar6;
  uint *puStack_24;
  int iStack_20;
  uint *puStack_1c;
  undefined4 uStack_18;
  undefined4 uStack_14;
  int *piStack_10;
  
  uVar6 = __CHK();
  uVar4 = (undefined4)((ulonglong)uVar6 >> 0x20);
  uStack_18 = extraout_ECX;
  uStack_14 = extraout_EBX;
  iVar2 = FUN_0001ccb0((int)uVar6,0x4c4f4144);
  if (iVar2 != 0) {
    puStack_24 = (uint *)(iVar2 + 0xc);
    puStack_1c = puStack_24 + *(int *)(iVar2 + 8);
    while (puStack_1c != puStack_24) {
      piStack_10 = (int *)FUN_0001c728(&puStack_24,uVar4,uStack_14,uStack_18,param_1);
      puVar5 = puStack_24 + 1;
      uVar3 = *puStack_24;
      puStack_24 = puVar5;
      if (*(uint *)(DAT_000bf80c + 0x28) <= uVar3) {
        fn_3fb31();
      }
      iVar2 = *(int *)(*(int *)(DAT_000bf80c + 0x24) + uVar3 * 8 + 4);
      if (iVar2 == 0) {
        s_error_loading_file_s_4027d(&iStack_20,puStack_24,1);
        iVar2 = DAT_000bf80c;
        *(int *)(extraout_ECX_00 + 4 + *(int *)(DAT_000bf80c + 0x24)) = iStack_20;
        *(int *)(extraout_ECX_00 + *(int *)(iVar2 + 0x24)) =
             *(int *)(extraout_ECX_00 + *(int *)(iVar2 + 0x24)) + 1;
        FUN_0001c6bd(uVar4,uVar3,0,3);
        iVar2 = iStack_20;
      }
      iStack_20 = iVar2;
      uVar3 = 0xffffffff;
      puVar5 = puStack_24;
      do {
        if (uVar3 == 0) break;
        uVar3 = uVar3 - 1;
        uVar1 = *puVar5;
        puVar5 = (uint *)((int)puVar5 + 1);
      } while ((char)uVar1 != '\0');
      puStack_24 = (uint *)((int)puStack_24 + (~uVar3 + 3 & 0xfffffffc));
      *piStack_10 = iStack_20;
    }
  }
  FUN_00010757();
  return;
}


// ==== FUN_0001ca58 @ 0001ca58 body=109 ranges=1

void __stdcall FUN_0001ca58(undefined4 param_1)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 extraout_ECX;
  undefined4 extraout_EBX;
  undefined8 uVar3;
  undefined4 *puStack_14;
  undefined4 *puStack_10;
  
  uVar3 = __CHK();
  iVar1 = FUN_0001ccb0((int)uVar3,0x47505452);
  if (iVar1 != 0) {
    puStack_10 = (undefined4 *)(iVar1 + 0xc) + *(int *)(iVar1 + 8);
    puStack_14 = (undefined4 *)(iVar1 + 0xc);
    while( true ) {
      if (puStack_10 == puStack_14) break;
      puVar2 = (undefined4 *)
               FUN_0001c728(&puStack_14,(int)((ulonglong)uVar3 >> 0x20),extraout_EBX,extraout_ECX,
                            param_1);
      *puVar2 = *puStack_14;
      puStack_14 = puStack_14 + 1;
    }
  }
  FUN_00010757();
  return;
}


// ==== FUN_0001cac5 @ 0001cac5 body=142 ranges=1

void __stdcall FUN_0001cac5(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 extraout_ECX;
  undefined4 uVar4;
  undefined4 extraout_EBX;
  undefined8 uVar5;
  undefined4 *puStack_1c;
  undefined4 *puStack_18;
  undefined4 uStack_14;
  undefined4 uStack_10;
  
  uVar5 = __CHK();
  uVar4 = (undefined4)((ulonglong)uVar5 >> 0x20);
  uStack_14 = extraout_ECX;
  uStack_10 = extraout_EBX;
  iVar2 = FUN_0001ccb0((int)uVar5,0x50505452);
  if (iVar2 != 0) {
    puStack_1c = (undefined4 *)(iVar2 + 0xc);
    puStack_18 = puStack_1c + *(int *)(iVar2 + 8);
    while( true ) {
      if (puStack_18 == puStack_1c) break;
      uVar1 = *puStack_1c;
      puStack_1c = puStack_1c + 1;
      uVar3 = FUN_0001c728(&puStack_1c,uVar4,uStack_10,uStack_14,param_1);
      FUN_000111d8(uVar1,uVar3,uVar1,1);
      FUN_0001c6bd(uVar4,uVar3);
    }
  }
  FUN_00010757();
  return;
}


// ==== FUN_0001ccf1 @ 0001ccf1 body=166 ranges=1

void __stdcall FUN_0001ccf1(undefined4 param_1,int param_2)

{
  int iVar1;
  int iVar2;
  undefined4 extraout_ECX;
  undefined4 extraout_EBX;
  ushort *puVar3;
  undefined8 uVar4;
  undefined4 uStack_10;
  
  uVar4 = __CHK();
  iVar1 = FUN_0001ccb0((int)((ulonglong)uVar4 >> 0x20),0x524f5554);
  if (iVar1 != 0) {
    puVar3 = (ushort *)(iVar1 + 8);
    for (uStack_10 = 0; *(ushort *)(iVar1 + 6) != uStack_10; uStack_10 = uStack_10 + 1) {
      iVar2 = FUN_0001ccb0(puVar3,0x4e414d45);
      if (iVar2 == 0) {
        return;
      }
      iVar2 = fn_40b12((int)uVar4,iVar2 + 8);
      if (iVar2 == 0) {
        if (param_2 == 0) {
          FUN_0001cbee(puVar3,extraout_EBX,extraout_ECX);
        }
        else {
          FUN_0001cc4c(puVar3,extraout_EBX,extraout_ECX,param_1,param_2);
        }
      }
      puVar3 = (ushort *)((int)puVar3 + (uint)*puVar3);
    }
  }
  return;
}


// ==== FUN_0001cd97 @ 0001cd97 body=188 ranges=1

int __stdcall FUN_0001cd97(int param_1,undefined4 param_2)

{
  int iVar1;
  ushort *puVar2;
  undefined4 uVar3;
  char *pcVar4;
  int iVar5;
  int *piVar6;
  int extraout_ECX;
  int extraout_ECX_00;
  int extraout_ECX_01;
  undefined4 extraout_ECX_02;
  byte *extraout_ECX_03;
  uint uVar7;
  int extraout_EBX;
  int extraout_EBX_00;
  int extraout_EBX_01;
  int extraout_EBX_02;
  char *pcVar8;
  char *pcVar9;
  undefined8 uVar10;
  int iStack_4c;
  int iStack_48;
  int iStack_44;
  uint uStack_40;
  int iStack_3c;
  undefined1 auStack_38 [4];
  undefined1 auStack_34 [4];
  char *pcStack_30;
  int iStack_2c;
  uint uStack_28;
  char *pcStack_24;
  int iStack_20;
  char *pcStack_1c;
  undefined4 uStack_18;
  ushort *puStack_14;
  int iStack_10;
  
  uVar10 = __CHK();
  iStack_44 = (int)((ulonglong)uVar10 >> 0x20);
  iStack_4c = extraout_ECX;
  iStack_48 = extraout_EBX;
  iStack_2c = FUN_0001ccb0((int)uVar10,0x524f5554,param_2);
  uStack_40 = 0;
  iStack_3c = 0;
  puStack_14 = (ushort *)(iStack_2c + 8);
  iVar1 = 0;
  if (iStack_2c != 0) {
    if ((extraout_EBX_00 != 0) && (iVar1 = FUN_0001ccb0(extraout_EBX_00,0x52524f55), iVar1 != 0)) {
      puVar2 = (ushort *)(iVar1 + 8);
      uStack_40 = (uint)*(ushort *)(iVar1 + 6);
      for (uVar7 = 0; uVar7 != uStack_40; uVar7 = uVar7 + 1) {
        DAT_000bf7f8 = DAT_000bf7f8 + -1;
        *DAT_000bf7f8 = puVar2;
        puVar2 = (ushort *)((int)puVar2 + (uint)*puVar2);
      }
    }
    iStack_20 = iStack_48 + 0xb8;
    pcStack_24 = s_WROUT_000a0c6d + 6;
    pcStack_1c = s_OROUT_000a0c7d + 6;
    iVar1 = 0;
    for (uStack_28 = 0; *(ushort *)(iStack_2c + 6) != uStack_28; uStack_28 = uStack_28 + 1) {
      uVar3 = FUN_0001ccb0(puStack_14,0x4e414d45);
      uStack_18 = FUN_000112f4(DAT_000a64a0,auStack_38,uVar3,0);
      if (extraout_EBX_01 == 0) {
        return iVar1;
      }
      pcStack_30 = (char *)(extraout_EBX_01 + 8);
      while( true ) {
        pcVar4 = (char *)FUN_0001131c(uStack_18,auStack_38,auStack_34);
        iVar5 = extraout_ECX_00;
        if (pcVar4 == (char *)0x0) break;
LAB_0001cecd:
        if (*pcVar4 == '\0') goto LAB_0001d07f;
        iVar5 = fn_40b12(pcVar4,pcStack_30);
        if (iVar5 != 0) goto LAB_0001d08d;
        iStack_10 = iVar5;
        if (param_1 == 0) {
          if (*(int *)(pcStack_30 + 0x10) == 0) {
            iStack_4c = 0;
          }
          else {
            pcVar8 = pcStack_30;
            pcVar9 = pcStack_24;
            for (iVar1 = 2; iVar1 != 0; iVar1 = iVar1 + -1) {
              *(undefined4 *)pcVar9 = *(undefined4 *)pcVar8;
              pcVar8 = pcVar8 + 4;
              pcVar9 = pcVar9 + 4;
            }
            for (iVar1 = 1; iVar1 != 0; iVar1 = iVar1 + -1) {
              *pcVar9 = *pcVar8;
              pcVar8 = pcVar8 + 1;
              pcVar9 = pcVar9 + 1;
            }
            FUN_0001265b(iStack_48,&iStack_4c,*(undefined4 *)(pcStack_30 + 0x10),s_WROUT_000a0c6d,1,
                         *(undefined4 *)(pcStack_30 + 0x14));
          }
          uVar7 = uStack_40;
          iVar1 = iStack_4c;
          FUN_0001cbee(puStack_14,iStack_48,iStack_4c);
          iVar5 = 0;
          while (iVar5 != uVar7 << 2) {
            iVar5 = FUN_0001ccb0(*(undefined4 *)((int)DAT_000bf7f8 + iVar5),0x4e414d45);
            iVar5 = fn_40b12(iVar5 + 8,pcStack_30);
            if (iVar5 == 0) {
              FUN_0001cbee(extraout_ECX_02,iStack_48,iStack_4c);
              break;
            }
            iVar5 = extraout_EBX_02 + 4;
          }
        }
        else {
          if (*(int *)(pcStack_30 + 0x10) == 0) {
            iStack_3c = 0;
          }
          else {
            pcVar8 = pcStack_30;
            pcVar9 = pcStack_1c;
            for (iVar1 = 2; iVar1 != 0; iVar1 = iVar1 + -1) {
              *(undefined4 *)pcVar9 = *(undefined4 *)pcVar8;
              pcVar8 = pcVar8 + 4;
              pcVar9 = pcVar9 + 4;
            }
            for (iVar1 = 1; iVar1 != 0; iVar1 = iVar1 + -1) {
              *pcVar9 = *pcVar8;
              pcVar8 = pcVar8 + 1;
              pcVar9 = pcVar9 + 1;
            }
            FUN_0001265b(iStack_48,&iStack_3c,*(undefined4 *)(pcStack_30 + 0x10),s_OROUT_000a0c7d,1,
                         *(undefined4 *)(pcStack_30 + 0x14));
          }
          iVar1 = iStack_3c;
          FUN_0001cc4c(puStack_14,iStack_48,iStack_4c,param_1,iStack_3c);
          iStack_10 = 1;
        }
        if (iStack_44 != 0) {
          FUN_0001ccf1(pcStack_30,iStack_44,iStack_48,iStack_4c,param_1,iStack_3c);
        }
        piVar6 = (int *)FUN_00011474(iStack_20);
        if (*(int *)(pcVar4 + 0x20) != 0) {
          *extraout_ECX_03 = *extraout_ECX_03 | 4;
        }
        if (*(int *)(pcVar4 + 0x28) != 0) {
          *extraout_ECX_03 = *extraout_ECX_03 | 0x80;
        }
        piVar6[1] = (int)pcVar4;
        piVar6[2] = iStack_48;
        piVar6[3] = iStack_4c;
        piVar6[4] = param_1;
        piVar6[5] = iStack_3c;
        iVar5 = 1;
        *piVar6 = iStack_10;
LAB_0001d07f:
        if (iVar5 == 1) break;
      }
      puStack_14 = (ushort *)((int)puStack_14 + (uint)*puStack_14);
    }
    DAT_000bf7f8 = DAT_000bf7f8 + uStack_40;
  }
  return iVar1;
LAB_0001d08d:
  pcVar4 = pcVar4 + 0x34;
  iVar5 = extraout_ECX_01;
  goto LAB_0001cecd;
}


