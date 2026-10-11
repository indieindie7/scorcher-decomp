// ==== FUN_000408b5 @ 000408b5 body=45 ranges=1

void FUN_000408b5(undefined4 *param_1)

{
  FUN_000407d9(*param_1,param_1);
  return;
}


// ==== __CHK @ 00040a8a body=16 ranges=1

/* Watcom runtime: __CHK (stk, full) */

undefined4 __stdcall __CHK(undefined4 param_1)

{
  undefined4 in_EAX;
  
  LOCK();
  UNLOCK();
  s_Stack_Overflow_40a9d(param_1);
  return in_EAX;
}


// ==== FUN_00018f02 @ 00018f02 body=10 ranges=1

void FUN_00018f02(void)

{
                    /* WARNING: Subroutine does not return */
  __CHK();
}


// ==== FUN_00030455 @ 00030455 body=10 ranges=1

void FUN_00030455(void)

{
                    /* WARNING: Subroutine does not return */
  __CHK();
}


